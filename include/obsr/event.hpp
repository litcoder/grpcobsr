#pragma once
#include <memory>
#include <obsr/stockrepo.hpp>
#include <vector>

class StockUpdateEvent
{
public:
  StockUpdateEvent(const std::shared_ptr<Stock> stock) : _mStock(stock) {}
  inline std::string getStockSymbol() { return _mStock->getSymbol(); }
  inline double getStockPrice() { return _mStock->getPrice(); }

private:
  const std::shared_ptr<Stock> _mStock;
};

class Subscriber
{
public:
  virtual void onPriceUpdate(StockUpdateEvent &ev);
};

class Publisher
{
public:
  void notify(StockUpdateEvent &ev);
  void addSubscriber(std::shared_ptr<Subscriber> sub);

private:
  std::vector<std::shared_ptr<Subscriber>> _mSubs;
};
