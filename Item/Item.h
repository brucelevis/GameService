#pragma once

#include <memory>
#include <string>
#include "Asset.h"
#include "P_Header.h"

namespace Adoter
{

namespace pb = google::protobuf;

/*
 * 类说明：
 *
 * 所有物品基类
 *
 * */
class Item : public std::enable_shared_from_this<Item>
{
public:
	std::shared_ptr<real::Item_Item> _stuff;
public:
	Item();
	virtual ~Item();
	
	explicit Item(pb::Message* message);
	explicit Item(std::shared_ptr<real::Item_Item>);
	explicit Item(real::Item_Item& item);

	//获取物品内容
	virtual std::string* GetStuff() { return this->_stuff->mutable_stuff(); }
	//获取物品类型
	virtual real::ConfigType GetType() { return this->_stuff->type_t(); }
	//是否可使用	
	virtual bool CanUse();
	//返回值：消耗数量
	virtual int32_t Use(); 
};

}
