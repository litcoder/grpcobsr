#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include <obsr/event.hpp>
#include <obsr/readreactor.hpp>
#include <spdlog/spdlog.h>
#include <stockprice.grpc.pb.h>

using google::protobuf::Empty;
using grpc::Channel;
using grpc::ClientReadReactor;
using grpc::Status;

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