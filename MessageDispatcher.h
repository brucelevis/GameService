#pragma once

#include "PriorityQueue.h"

namespace Adoter
{

using namespace std::chrono;

class PriorityQueueItem : public std::enable_shared_from_this<PriorityQueueItem>
{
private:
	real::MsgItem _item;
public:

	PriorityQueueItem(real::MsgItem item)
	{
		this->_item = item;
	}

	bool operator < (const PriorityQueueItem& other) const
	{
		return _item.priority() > other._item.priority();	//最小值优先，优先级级别：1~10，默认为10
	}

	real::MsgItem& Get()
	{
		return _item;
	}
};

class MessageDispatcher : public std::enable_shared_from_this<MessageDispatcher>
{
private:
	std::mutex _mutex;
	std::shared_ptr<std::thread> _thread;
	PriorityQueue<std::shared_ptr<PriorityQueueItem>> _messages;
public:
	~MessageDispatcher()
	{
		_thread->join();
	}
	
	//消息队列存盘(一般游戏逻辑不需做此操作)
	void Save() 
	{
		//real::MsgItems items;	//用于存盘
	}	

	//消息队列加载(一般游戏逻辑不需做此操作)
	void Load() 
	{
		//real::MsgItems items;	//用于加载
	}		

	MessageDispatcher()
	{
		_thread = std::make_shared<std::thread>(std::bind(&MessageDispatcher::Dispatcher, this));
	}

	static MessageDispatcher& Instance()
	{
		static MessageDispatcher _instance;
		return _instance;
	}

	void SendMessage(real::MsgItem& msg)
	{
		auto item = std::make_shared<PriorityQueueItem>(msg);
		system_clock::time_point curr_time = system_clock::now();
		msg.set_time(system_clock::to_time_t(curr_time));	//发送时间
		_messages.Emplace(item);
	}

	bool Empty()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		return _messages.Empty();
	}

	void Dispatcher()
	{
		while(true)
		{
			if (Empty()) 
			{
				sleep(1);
				continue;
			}
			real::MsgItem& message = _messages.GetNext()->Get();
			int32_t delay = 0;
			if (message.has_delay()) delay = message.delay();
			system_clock::time_point curr_time = system_clock::now();
			if (delay > 0 && system_clock::to_time_t(curr_time) < message.time() + delay) continue;	//还没到发送时间
			int64_t receiver = message.receiver();
			Discharge(receiver, message);
			usleep(200);
		}
	}

	void Discharge(int64_t receiver, real::MsgItem& message)
	{
		auto entity = EntityInstance.GetEntity(receiver);
		if (!entity) return;
		entity->HandleMessage(message);		//交给各个接受者处理
	}
};

#define DispatcherInstance MessageDispatcher::Instance()

}
