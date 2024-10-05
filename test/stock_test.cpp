#include <gtest/gtest.h>
#include <memory>
#include <obsr/stockrepo.hpp>
#include <spdlog/spdlog.h>

using namespace ::testing;
using namespace std;

struct StockTest : Test
{
};

TEST_F(StockTest, testStockPriceUpdate)
{
  auto intc = Stock("INTC", 22.0);
  EXPECT_STREQ(intc.getSymbol().c_str(), "INTC");
  EXPECT_EQ(intc.getPrice(), 22.0);

  intc.updatePriceBy(0.5);
  EXPECT_EQ(intc.getPrice(), 22.5);

  intc.updatePriceBy(-0.3);
  EXPECT_EQ(intc.getPrice(), 22.2);
}

struct StockRepoTest : Test
{
};

TEST_F(StockRepoTest, addStocksToRepo)
{
  auto repo = StockRepository();
  repo.addStock(std::make_shared<Stock>("GOOGL", 10.10));
  repo.addStock(std::make_shared<Stock>("AMZN", 20.20));
  repo.addStock(std::make_shared<Stock>("MSFT", 30.30));
  repo.addStock(std::make_shared<Stock>("AAPL", 40.40));
  repo.addStock(std::make_shared<Stock>("NFLX", 50.50));

  EXPECT_EQ(repo.numStocks(), 5);

  auto googl = repo.getStock("GOOGL");
  EXPECT_NE(googl, nullptr);
  EXPECT_STREQ(googl->getSymbol().c_str(), "GOOGL");

  auto inval = repo.getStock("INVALID_STOCK");
  EXPECT_EQ(nullptr, inval);
}

TEST_F(StockRepoTest, pickRandomStock)
{
  auto repo = StockRepository();
  repo.addStock(std::make_shared<Stock>("GOOGL", 10.10));
  repo.addStock(std::make_shared<Stock>("AMZN", 20.20));
  repo.addStock(std::make_shared<Stock>("MSFT", 30.30));
  repo.addStock(std::make_shared<Stock>("AAPL", 40.40));
  repo.addStock(std::make_shared<Stock>("NFLX", 50.50));

  for (int i = 0; i < 5; ++i)
  {
    auto s1 = repo.pickRandomStock();
    auto s2 = repo.pickRandomStock();
    EXPECT_NE(s1, s2);
  }
}

TEST_F(StockRepoTest, updateRandomPrice)
{
  auto v1 = StockRepository::randomValueChange();
  auto v2 = StockRepository::randomValueChange();

  EXPECT_NE(v1, v2);

  EXPECT_GE(v1, -5.0);
  EXPECT_LE(v1, 5.0);

  EXPECT_GE(v2, -5.0);
  EXPECT_LE(v2, 5.0);
}