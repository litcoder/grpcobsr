#include <grpc/grpc.h>
#include <obsr/event.hpp>
#include <obsr/readreactor.hpp>
#include <spdlog/spdlog.h>
#include <stockprice.grpc.pb.h>

StockPriceReadReactor::StockPriceReadReactor(
    std::shared_ptr<StockService::Stub> stub, std::shared_ptr<Publisher> pub)
    : _mPub(pub)
{

  ::google::protobuf::Empty empty;
  stub->async()->UpdateStockPrice(&_mContext, &empty, this);
  StartRead(&_mResp);
  StartCall();
}

void StockPriceReadReactor::OnReadDone(bool ok)
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

void StockPriceReadReactor::OnDone(const ::grpc::Status &s)
{
  spdlog::info("OnDone()");
  std::unique_lock<std::mutex> l(_mMtx);
  _mStatus = s;
  _mAllDone = true;
  _mCondVar.notify_one();
}

::grpc::Status StockPriceReadReactor::Await()
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
