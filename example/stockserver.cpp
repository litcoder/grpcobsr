#include <chrono>
#include <obsr/stockrepo.hpp>
#include <spdlog/spdlog.h>
#include <string>
#include <thread>

#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <stockprice.grpc.pb.h>

using google::protobuf::Empty;
using grpc::CallbackServerContext;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerWriteReactor;
using grpc::Status;
using grpc::StatusCode;

class StockPriceWriteReactor : public ServerWriteReactor<::StockPriceResponse>
{
public:
  StockPriceWriteReactor(int evCnt) : _mReqEventCount(evCnt), _mCurEventCount(0)
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

  void OnWriteDone(bool ok) override
  {
    // spdlog::info("OnWriteDone()");
    if (!ok)
    {
      spdlog::error("Failed to write.");
      Finish(Status(StatusCode::UNKNOWN, "Unexpected failure"));
    }

    // Update every 1000ms.
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    NextWrite();
  }

  void OnDone() override
  {
    spdlog::info("OnDone() PRC completed.");
    delete this;
  }

  void OnCancel() override { spdlog::info("OnCancel()"); }

private:
  void NextWrite()
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
    Finish(Status::OK);
  }

  int _mReqEventCount;
  int _mCurEventCount;
  StockPriceResponse _mResp;
  StockRepository _mStockRepo;
};

class StockServiceImpl final : public StockService::CallbackService
{
public:
  StockServiceImpl(int evCnt = 0) : _mEventCount(evCnt) {}

  ServerWriteReactor<::StockPriceResponse> *UpdateStockPrice(
      CallbackServerContext *context,
      const google::protobuf::Empty *empty) override
  {
    return new StockPriceWriteReactor(_mEventCount);
  }

private:
  int _mEventCount;
};

int main(int argc, char *argv[])
{
  int evCnt = 0;

  if (argc > 1)
  {
    try
    {
      evCnt = std::stoi(argv[1]);
    }
    catch (std::invalid_argument)
    {
      spdlog::warn("Invalid argument, infinite number of events will be sent.");
      evCnt = 0;
    }
  }

  std::string serverAddr("0.0.0.0:50051");
  StockServiceImpl service(evCnt);

  ServerBuilder builder;
  builder.AddListeningPort(serverAddr, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<Server> server(builder.BuildAndStart());
  spdlog::info("Server listening on {} for {} event(s)", serverAddr, evCnt);
  server->Wait();

  return 0;
}