#include <obsr/event.hpp>
#include <spdlog/spdlog.h>
#include <string>

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include <stockprice.grpc.pb.h>

using google::protobuf::Empty;
using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReadReactor;
using grpc::Status;

class StockPriceReadReactor : public ClientReadReactor<StockPriceResponse>
{
public:
  StockPriceReadReactor(
      std::shared_ptr<StockService::Stub> stub, std::shared_ptr<Publisher> pub)
      : _mPub(pub)
  {
    Empty empty;
    stub->async()->UpdateStockPrice(&_mContext, &empty, this);
    StartRead(&_mResp);
    StartCall();
  }

  void OnReadDone(bool ok) override
  {
    if (ok)
    {
      auto symbol = _mResp.symbol();
      auto price = _mResp.price();

      // spdlog::info("{} / {}", _mResp.symbol(), _mResp.price());
      // Publish event to subscribers.
      if (_mPub != nullptr)
      {
        auto stock = std::make_shared<Stock>(symbol, price);
        auto ev = StockUpdateEvent(stock);
        _mPub->notify(ev);
      }

      StartRead(&_mResp);
    }
  }

  void OnDone(const Status &s) override
  {
    spdlog::info("OnDone()");
    std::unique_lock<std::mutex> l(_mMtx);
    _mStatus = s;
    _mAllDone = true;
    _mCondVar.notify_one();
  }

  Status Await()
  {
    std::unique_lock<std::mutex> l(_mMtx);
    _mCondVar.wait(
        l,
        [this]
        {
          return _mAllDone;
        });
    return std::move(_mStatus);
  }

private:
  std::shared_ptr<Publisher> _mPub;
  ClientContext _mContext;
  StockPriceResponse _mResp;

  std::mutex _mMtx;
  std::condition_variable _mCondVar;
  Status _mStatus;
  bool _mAllDone = false;
};

class StockClient
{
public:
  StockClient(std::shared_ptr<Channel> channel, std::shared_ptr<Publisher> pub)
      : _mStub(StockService::NewStub(channel)), _mPub(pub)
  {
  }

  void updateStockPrice()
  {
    StockPriceReadReactor reader(_mStub, _mPub);
    Status status = reader.Await();
    if (status.ok())
    {
      spdlog::info("PriceListing succeed.");
    }
    else
    {
      spdlog::error("Failed to get prices.");
      spdlog::error("{}({})", status.error_message(), status.error_code());
    }
  }

private:
  std::shared_ptr<StockService::Stub> _mStub;
  std::shared_ptr<Publisher> _mPub;
};

int main(int argc, char *argv[])
{
  auto channel = grpc::CreateChannel(
      "127.0.0.1:50051", grpc::InsecureChannelCredentials());

  // Create a subscriber.
  auto sub = std::make_shared<Subscriber>();

  // Add the subscriber to publisher.
  auto pub = std::make_shared<Publisher>();
  pub->addSubscriber(sub);

  // Initiate server-side streaming.
  StockClient c(channel, pub);
  c.updateStockPrice();

  return 0;
}