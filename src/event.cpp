#include <obsr/event.hpp>
#include <spdlog/spdlog.h>

void Publisher::notify(StockUpdateEvent &ev)
{
  for (const auto &s : _mSubs)
  {
    s->onPriceUpdate(ev);
  }
}

void Publisher::addSubscriber(std::shared_ptr<Subscriber> sub)
{
  _mSubs.push_back(sub);
}

void Subscriber::onPriceUpdate(StockUpdateEvent &ev)
{
  spdlog::info(
      "Received stock update event: {} {}",
      ev.getStockSymbol(),
      ev.getStockPrice());
}