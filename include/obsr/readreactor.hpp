#pragma once
#include <condition_variable>
#include <grpc/grpc.h>
#include <grpcpp/client_context.h>
#include <obsr/event.hpp>
#include <stockprice.grpc.pb.h>

class StockPriceReadReactor
    : public ::grpc::ClientReadReactor<::StockPriceResponse>
{
public:
  StockPriceReadReactor(
      std::shared_ptr<StockService::Stub> stub, std::shared_ptr<Publisher> pub);
  void OnReadDone(bool ok) override;
  void OnDone(const ::grpc::Status &s) override;
  ::grpc::Status Await();

private:
  std::shared_ptr<Publisher> _mPub;
  ::grpc::ClientContext _mContext;
  ::StockPriceResponse _mResp;

  std::mutex _mMtx;
  std::condition_variable _mCondVar;
  ::grpc::Status _mStatus;
  bool _mAllDone = false;
};
