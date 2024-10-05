#include <chrono>
#include <grpc/grpc.h>
#include <obsr/stockrepo.hpp>
#include <obsr/writereactor.hpp>
#include <spdlog/spdlog.h>
#include <stockprice.grpc.pb.h>

StockPriceWriteReactor::StockPriceWriteReactor(int evCnt)
    : _mReqEventCount(evCnt), _mCurEventCount(0)
{
  _mStockRepo = StockRepository();
  _mStockRepo.addStock(std::make_shared<Stock>("GOOGL", 10.10));
  _mStockRepo.addStock(std::make_shared<Stock>("AMZN", 20.20));
  _mStockRepo.addStock(std::make_shared<Stock>("MSFT", 30.30));
  _mStockRepo.addStock(std::make_shared<Stock>("AAPL", 40.40));
  _mStockRepo.addStock(std::make_shared<Stock>("NFLX", 50.50));

  spdlog::info("Repo initialized with {}stocks", _mStockRepo.numStocks());

  // Start writing stream data.
  NextWrite();
}

void StockPriceWriteReactor::OnWriteDone(bool ok)
{
  // spdlog::info("OnWriteDone()");
  if (!ok)
  {
    spdlog::error("Failed to write.");
    Finish(::grpc::Status(::grpc::StatusCode::UNKNOWN, "Unexpected failure"));
  }

  // Update every 1000ms.
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  NextWrite();
}

void StockPriceWriteReactor::OnDone()
{
  spdlog::info("OnDone() PRC completed.");
  delete this;
}

void StockPriceWriteReactor::OnCancel() { spdlog::info("OnCancel()"); }

void StockPriceWriteReactor::NextWrite()
{
  auto stock = _mStockRepo.pickRandomStock();

  if (stock == nullptr)
  {
    spdlog::error("Invalid symbol. Skipping");
  }
  else if (_mReqEventCount == 0 || (_mReqEventCount > _mCurEventCount))
  {
    // Update price by random value.
    auto v = StockRepository::randomValueChange();
    stock->updatePriceBy(v);

    auto symbol = stock->getSymbol();
    auto price = stock->getPrice();

    spdlog::info(
        "Notify {} at ${} ({}/{})",
        symbol,
        price,
        _mCurEventCount + 1,
        _mReqEventCount);

    _mResp.set_symbol(symbol);
    _mResp.set_price(price);

    StartWrite(&_mResp);
    ++_mCurEventCount;
    return;
  }

  // Indicate end of the stream.
  // Won't be called for the endless streaming.
  Finish(::grpc::Status::OK);
}
