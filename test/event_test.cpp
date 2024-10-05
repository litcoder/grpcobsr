#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <obsr/event.hpp>
#include <spdlog/spdlog.h>

using namespace ::testing;
using namespace std;

struct EventTest : Test
{
  struct MockSubscriber : public Subscriber
  {
    MOCK_METHOD(void, onPriceUpdate, (StockUpdateEvent &), (override));
  };
};

TEST_F(EventTest, testEventNotified)
{
  auto pub = Publisher();

  auto sub = std::make_shared<MockSubscriber>();
  pub.addSubscriber(sub);

  auto stock = std::make_shared<Stock>("INTC", 20.33);
  auto ev = StockUpdateEvent(stock);
  EXPECT_CALL(*sub, onPriceUpdate(_));
  pub.notify(ev);
}
