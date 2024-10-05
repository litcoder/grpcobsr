#include <chrono>
#include <cmath>
#include <memory>
#include <obsr/event.hpp>
#include <obsr/stockrepo.hpp>
#include <string>

StockRepository::StockRepository() {}

bool StockRepository::addStock(std::shared_ptr<Stock> stock)
{
  auto it = _mStockMap.find(stock->getSymbol());
  if (it != _mStockMap.end())
  {
    // Already added.
    return false;
  }
  _mStockMap.insert(std::pair<std::string, std::shared_ptr<Stock>>(
      stock->getSymbol(), stock));
  return true;
}

std::shared_ptr<Stock> StockRepository::getStock(const std::string &symbol)
{
  auto it = _mStockMap.find(symbol);
  if (it == _mStockMap.end())
  {
    // Unable to find the symbol.
    return nullptr;
  }
  return it->second;
}

std::shared_ptr<Stock> StockRepository::pickRandomStock() const
{
  int randIdx = rand() % _mStockMap.size();
  auto randomIt = std::next(std::begin(_mStockMap), randIdx);
  return randomIt->second;
}

float StockRepository::randomValueChange()
{
  double lowerBound = -5.0;
  double upperBound = 5.0;

  auto r = static_cast<float>(rand());
  auto f = (r / RAND_MAX);
  auto v = lowerBound + (f * (upperBound - lowerBound));
  return std::round(v * 100) / 100.0;
}