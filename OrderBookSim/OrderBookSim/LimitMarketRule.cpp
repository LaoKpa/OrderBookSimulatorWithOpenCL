#include "StdAfx.h"
#include "LimitMarketRule.h"


LimitMarketRule::LimitMarketRule(void)
{}

LimitMarketRule::~LimitMarketRule(void)
{}

bool LimitMarketRule::fitsCriteria(OrderBook* orderBook, Order* order)
{
	bool result = false;

	if (order->isLimit())
	{
		std::list<Order> orders;

		if (order->isBuy())
			orders = orderBook->getSellOrders();
		else if (order->isSell())
			orders = orderBook->getBuyOrders();
		else
			throw new std::exception("Trade of unknown direction. Unable to check critera");

		std::list<Order>::iterator it;
		for (it = orders.begin(); it != orders.end(); it++)
		{
			if (it->isMarket() && it->getParticipant() != order->getParticipant())
			{
				result = true;
				break;
			}
		}
	}

	return result;
}

Trade* LimitMarketRule::processRule(OrderBook*& orderBook, Order*& order)
{
	Order* matchedOrder = NULL;
	Trade* trade = NULL;

	std::list<Order> orders;

	if (order->isBuy())
		orders = orderBook->getSellOrders();
	else
		orders = orderBook->getBuyOrders();

	matchedOrder = &orders.front();

	auto it = orders.begin();
	while (matchedOrder->getParticipant() == order->getParticipant())
	{
		if (it->getParticipant() != order->getParticipant() && it->isMarket() && order->isLimit())
		{
			matchedOrder = &*it;
			break;
		}

		it++;
		if (it == orders.end())
			return NULL;
	}

	double price = order->getPrice();
	int size = matchedOrder->getSize();

	if (matchedOrder->getSize() > order->getSize())
		size = order->getSize();

	trade = new Trade(*order, *matchedOrder, price, size, orderBook->getTime());

	orderBook->updateOrderSize(order, (order->getSize() - size));
	orderBook->updateOrderSize(matchedOrder, (matchedOrder->getSize() - size));

	orderBook->setLastPrice(price);

	matchedOrder = NULL;
	delete matchedOrder;

	return trade;
}