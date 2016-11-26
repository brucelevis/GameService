#include "Player.h"
#include "Protocol.h"
#include "CommonUtil.h"
#include <iostream>

namespace Adoter
{

Player::Player()
{
	//协议处理回调初始化
	//ProtocolInstance.AddHandler(Asset::META_TYPE_LOGIN, std::bind(&Player::Login, this, std::placeholders::_1));
}

Player::Player(int64_t player_id, std::shared_ptr<WorldSession> session) : Player()/*委派构造函数*/
{
	this->_ID = player_id;
	this->_session = session; //地址拷贝
	this->_common_prop.set_id(player_id);
}

int32_t Player::Load()
{
	//////初始化包裹：新建角色或者新增包裹会调用一次
	do {
		const pb::EnumDescriptor* enum_desc = Asset::INVENTORY_TYPE_descriptor();
		if (!enum_desc) return 0;
		int32_t curr_inventories_size = _stuff.inventory().inventories_size(); 
		if (curr_inventories_size == Asset::INVENTORY_TYPE_COUNT - 1) break; 
		for (int inventory_index = curr_inventories_size; inventory_index < Asset::INVENTORY_TYPE_COUNT - 1; ++inventory_index)
		{
			auto inventory = _stuff.mutable_inventory()->mutable_inventories()->Add(); //增加新包裹，且初始化数据
			inventory->set_inventory_type((Asset::INVENTORY_TYPE)(inventory_index + 1));
			int inventory_size = enum_desc->value(inventory_index)->options().GetExtension(Asset::inventory_size); //默认格子数：如果修改需要清档
			inventory->mutable_items()->Reserve(inventory_size); //设置默认格子数，即包裹大小
			std::cout << "Add inventory type:" << (inventory_index + 1) << ", size:" << inventory_size << std::endl;
		}
	} while(false);
	this->_common_prop.ParseFromString(this->_stuff.common_prop()); //公共属性
	return 0;
}

int32_t Player::Save()
{
	this->_stuff.set_common_prop(this->_common_prop.SerializeAsString()); //公共属性
	return 0;
}

int32_t Player::Login()
{
	std::cout << __func__ << std::endl;
	if (Load()) return 1;
	EnterScene();
	return 0;
}

int32_t Player::Logout()
{
	return 0;
}

void Player::EnterScene() 
{
	Asset::Vector3 pos;
	int32_t x = CommonUtil::Random(0, 20);	
	int32_t z = CommonUtil::Random(0, 20);	
	pos.set_x(x);
	pos.set_z(z);
	this->UpdatePosition(pos);
	GetScene()->EnterScene(this);
	
	Asset::MsgItem item;
	item.set_type(Asset::MSG_TYPE_AOI_ENTER);
	item.set_content(this->_stuff.common_prop()); //属性数据
	this->SendNearbyMessage(item);
}

void Player::LeaveScene()
{
	GetScene()->LeaveScene(this);
	Asset::MsgItem item;
	item.set_type(Asset::MSG_TYPE_AOI_LEAVE);
	this->SendNearbyMessage(item);
}

void Player::StepMove(Asset::Vector3& position)
{
	GetScene()->UpdatePosition(this, position); //更新场景中的位置
	this->UpdatePosition(position); //更新自己位置
	Asset::MsgItem item; //广播给周围玩家
	item.set_type(Asset::MSG_TYPE_AOI_MOVE);
	item.set_content(this->_stuff.common_prop()); //属性数据
	this->SendNearbyMessage(item); //通知给周围玩家
}

void Player::HandleMessage(Asset::MsgItem& item)
{
	//std::cout << __func__ << ":sender:" << item.sender() << ", receiver:" << item.receiver() << ", type:" << item.type() << std::endl;
	
	switch (item.type())
	{
	case Asset::MSG_TYPE_AOI_ENTER:
	{
	}
	break;
	
	case Asset::MSG_TYPE_AOI_LEAVE:
	{
	}
	break;
	
	case Asset::MSG_TYPE_AOI_MOVE:
	{
	}
	break;
	
	default:
	{
		std::cout << __func__ << ":No handler, type:" << item.type() << std::endl;
	}
	break;

	}
}

void Player::SendMessage(Asset::MsgItem& item)
{
	DispatcherInstance.SendMessage(item);
}	

void Player::SendNearbyMessage(Asset::MsgItem& item)
{
	std::vector<int64_t> entities;
	if (GetScene()->GetNearByEntities(this, entities)) 
	{
		item.set_sender(this->_ID);
		for (auto it = entities.begin(); it != entities.end(); ++it)
		{
			auto near_entity = EntityInstance.GetEntity(*it);	
			if (!near_entity) continue;
			item.set_receiver(near_entity->_ID);
			SendMessage(item);
		}
	}
}

void Player::SendProtocol(pb::Message& message)
{
	const pb::FieldDescriptor* field = message.GetDescriptor()->FindFieldByName("type_t");
	if (!field) return;
	
	int type_t = field->default_value_enum()->number();
	if (!Asset::MsgType_IsValid(type_t)) return;	//如果不合法，不检查会宕线
	
	Asset::Meta meta;
	meta.set_type_t((Asset::MetaType)type_t);
	meta.set_stuff(message.SerializeAsString());
	std::string content = meta.SerializeAsString();
	this->GetSession()->AsyncSend(content);
}

void Player::SendNearbyProtocol(pb::Message& message) 
{
	std::vector<int64_t> entities;
	if (GetScene()->GetNearByEntities(this, entities)) 
	{
		for (auto it = entities.begin(); it != entities.end(); ++it)
		{
			auto near_entity = EntityInstance.GetEntity(*it);	
			if (!near_entity) continue;
			near_entity->SendProtocol(message);
		}
	}
}

}
