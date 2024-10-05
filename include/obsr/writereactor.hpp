#include <grpc/grpc.h>
#include <obsr/stockrepo.hpp>
#include <stockprice.grpc.pb.h>

class StockPriceWriteReactor
    : public ::grpc::ServerWriteReactor<::StockPriceResponse>
{
public:
  StockPriceWriteReactor(int evCnt);

  void OnWriteDone(bool ok) override;
  void OnDone() override;
  void OnCancel() override;

private:
  void NextWrite();

  int _mReqEventCount;
  int _mCurEventCount;
  StockPriceResponse _mResp;
  StockRepository _mStockRepo;
};
