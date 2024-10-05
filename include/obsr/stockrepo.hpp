#pragma once
#include <map>
#include <memory>
#include <obsr/stockrepo.hpp>
#include <string>

class Stock
{
public:
  Stock(const std::string symbol, double price)
      : _mSymbol(symbol), _mPrice(price)
  {
  }

  inline double getPrice() const { return _mPrice; }
  inline std::string getSymbol() const { return _mSymbol; }
  inline void updatePriceBy(double change) { _mPrice += change; }

private:
  std::string _mSymbol;
  double _mPrice;
};

class StockRepository
{
public:
  StockRepository();
  bool addStock(std::shared_ptr<Stock> stock);
  inline size_t numStocks() const { return _mStockMap.size(); }
  std::shared_ptr<Stock> getStock(const std::string &symbol);
  std::shared_ptr<Stock> pickRandomStock() const;
  static float randomValueChange();

private:
  std::map<std::string, std::shared_ptr<Stock>> _mStockMap;
};