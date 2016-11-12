#include "Item.h"

namespace Adoter
{

Item::~Item()
{
}

Item::Item()
{
	_stuff = std::make_shared<real::Item_Item>();
}

Item::Item(pb::Message* message) : Item()/*委托构造函数*/
{
	const pb::FieldDescriptor* field = message->GetDescriptor()->FindFieldByName("type_t");
	if (field) _stuff->set_type_t((real::ConfigType)field->default_value_enum()->number());
	_stuff->set_stuff(message->SerializeAsString());
}

Item::Item(real::Item_Item& item)
{
	_stuff->CopyFrom(item);
}

Item::Item(std::shared_ptr<real::Item_Item> item)
{
	_stuff = item;
}

bool Item::CanUse() 
{
	pb::Message* message = AssetInstance.GetMessage(_stuff->type_t());
	if (!message) return false;
	bool result = message->ParseFromString(_stuff->stuff());
	if (!result) return false;
	const pb::FieldDescriptor* field = message->GetDescriptor()->FindFieldByNumber(2); //物品公共属性
	if (!field) return false;
	const pb::Message& const_item_common_prop = message->GetReflection()->GetMessage(*message, field);
	//////防止异常
	try{
		pb::Message& item_common_prop = const_cast<pb::Message&>(const_item_common_prop);
		real::Item_CommonProp& common_prop = dynamic_cast<real::Item_CommonProp&>(item_common_prop);
		int min_level_limit = common_prop.min_level_limit();
		int max_level_limit = common_prop.max_level_limit();
		std::cout << "物品级别限制：最小(" << min_level_limit << ") 最大(" << max_level_limit << ")" << std::endl;
	}
	catch(std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
		return false;
	}
	return true; 
}

//返回值代表消耗物品数量
int32_t Item::Use() 
{ 
	if (!CanUse()) return 0;
	return 1; 
}

}
