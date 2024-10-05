#include <chrono>
#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <obsr/stockrepo.hpp>
#include <obsr/writereactor.hpp>
#include <spdlog/spdlog.h>
#include <stockprice.grpc.pb.h>
#include <string>

using google::protobuf::Empty;
using grpc::CallbackServerContext;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerWriteReactor;

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