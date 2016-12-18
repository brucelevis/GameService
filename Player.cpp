#include "Player.h"
#include "Protocol.h"
#include "CommonUtil.h"
#include "RedisManager.h"

#include <iostream>
#include <hiredis.h>

namespace Adoter
{

Player::Player()
{
	//协议默认处理函数
	_method = std::bind(&Player::DefaultMethod, this, std::placeholders::_1);
	//协议处理回调初始化
	AddHandler(Asset::META_TYPE_C2S_LOGIN, std::bind(&Player::CmdLogin, this, std::placeholders::_1));
	AddHandler(Asset::META_TYPE_C2S_ENTER_SCENE, std::bind(&Player::CmdEnterScene, this, std::placeholders::_1));
	AddHandler(Asset::META_TYPE_C2S_MOVE, std::bind(&Player::CmdMove, this, std::placeholders::_1));
}

Player::Player(int64_t player_id, std::shared_ptr<WorldSession> session) : Player()/*委派构造函数*/
{
	this->SetID(player_id);	//设置玩家ID
	this->_session = session; //地址拷贝
}

int32_t Player::Load()
{
	//加载数据库
	std::shared_ptr<Redis> redis = std::make_shared<Redis>();
	std::string stuff = redis->GetPlayer(this->_ID); //不能用引用
	if (stuff == "")
	{
		std::cout << __func__ << " error, not found player_id:" << this->_ID << std::endl;
		return 0;
	}
	//初始化结构数据
	this->_stuff.ParseFromString(stuff);
	//初始化基类公共属性
	this->_super_common_prop = this->_stuff.common_prop();
	//初始化包裹，创建角色或者增加包裹会调用一次
	do {
		const pb::EnumDescriptor* enum_desc = Asset::INVENTORY_TYPE_descriptor();
		if (!enum_desc) return 0;
		int32_t curr_inventories_size = _stuff.inventory().inventories_size(); 
		if (curr_inventories_size == enum_desc->value_count() - 1) break; 
		for (int inventory_index = curr_inventories_size; inventory_index < enum_desc->value_count() - 1; ++inventory_index)
		{
			auto inventory = _stuff.mutable_inventory()->mutable_inventories()->Add(); //增加新包裹，且初始化数据
			inventory->set_inventory_type((Asset::INVENTORY_TYPE)(inventory_index + 1));
			int inventory_size = enum_desc->value(inventory_index)->options().GetExtension(Asset::inventory_size); //默认格子数：如果修改需要清档
			if (inventory->items_size() >= inventory_size) continue;
			inventory->mutable_items()->Reserve(inventory_size); //设置默认格子数，即包裹大小
			//提示信息
			const pb::EnumValueDescriptor *enum_value = enum_desc->value(inventory_index);
			if (!enum_value) break;
			std::cout << "Add inventory type:" << enum_value->name() << ", size:" << inventory_size << std::endl;
		}
	} while(false);

	return 0;
}

int32_t Player::Save()
{
	//初始化实体类型
	if (this->_super_common_prop.entity_type() != Asset::ENTITY_TYPE_PLAYER/*玩家*/)
		this->_super_common_prop.set_entity_type(Asset::ENTITY_TYPE_PLAYER); //创建角色会调用
	//存储基类公共属性
	this->_stuff.mutable_common_prop()->CopyFrom(this->_super_common_prop); //公共属性
	//存入数据库
	std::string stuff = this->_stuff.SerializeAsString();
	std::shared_ptr<Redis> redis = std::make_shared<Redis>();
	redis->SavePlayer(this->_ID, stuff);

	return 0;
}

int32_t Player::CmdLogin(pb::Message* message)
{
	if (Load()) return 1;
	//发送数据给客户端
	SendPlayer();
	return 0;
}

int32_t Player::Logout(pb::Message* message)
{
	//非存盘数据
	this->_stuff.mutable_provisional_prop()->Clear(); 
	Save();	//存盘
	OnLeaveScene(); //场景处理
	return 0;
}

void Player::SendPlayer()
{
	Asset::PlayerInfo player_info;
	player_info.mutable_player()->CopyFrom(this->_stuff);
	this->SendProtocol(player_info);
}

int32_t Player::OnEnterGame() 
{
	if (Load()) return 1;
	SendPlayer(); //发送数据给玩家
	return 0;
}

int32_t Player::CmdEnterScene(pb::Message* message) 
{
	Asset::EnterScene* enter_scene = dynamic_cast<Asset::EnterScene*>(message);
	if (!enter_scene) return 1;

	int64_t scene_id = enter_scene->scene_id();
	if (scene_id == 0) scene_id = 589825;
	OnEnterScene(scene_id); //调用基类接口，通知周围玩家，同时在场景中初始化该玩家
	return 0;
}

int32_t Player::LeaveScene(pb::Message* message)
{
	OnLeaveScene(); //场景处理
	return 0;
}

int32_t Player::CmdMove(pb::Message* message)
{
	Asset::Move* move = dynamic_cast<Asset::Move*>(message);
	if (!move) return 1;

	Asset::Vector3 position = move->move_to();
	StepMove(position); //移动位置
	
	return 0;
}

bool Player::HandleMessage(const Asset::MsgItem& item)
{
	std::cout << __func__ << ":sender:" << item.sender() << ", receiver:" << item.receiver() << ", type:" << item.type() << std::endl;
	
	switch (item.type())
	{
	//进入视野
	case Asset::MSG_TYPE_AOI_ENTER:
	{
		Asset::CommonProp common_prop;
		if (!common_prop.ParseFromString(item.content())) return false;
		if (!IsInSight(common_prop.position())) return false;	//如果看不到该实体，则退出
		
		Asset::SurroundingsChanged proto;
		proto.set_action_type(Asset::SurroundingsChanged_ACTION_TYPE_ACTION_TYPE_ENTER);
		proto.set_entity_id(common_prop.entity_id());
		proto.set_content(item.content());
		this->SendProtocol(proto); 
	}
	break;
	//离开视野
	case Asset::MSG_TYPE_AOI_LEAVE:
	{
		Asset::CommonProp common_prop;
		if (!common_prop.ParseFromString(item.content())) return false;

		Asset::SurroundingsChanged proto;
		proto.set_action_type(Asset::SurroundingsChanged_ACTION_TYPE_ACTION_TYPE_LEAVE);
		proto.set_entity_id(common_prop.entity_id());
		this->SendProtocol(proto); 
	}
	break;
	//移动	
	case Asset::MSG_TYPE_AOI_MOVE:
	{
		Asset::CommonProp common_prop;
		if (!common_prop.ParseFromString(item.content())) return false;

		Asset::SurroundingsChanged proto;
		proto.set_action_type(Asset::SurroundingsChanged_ACTION_TYPE_ACTION_TYPE_MOVE);
		proto.set_entity_id(common_prop.entity_id());
		this->SendProtocol(proto); 
	}
	break;
	
	default:
	{
		std::cout << __func__ << ":No handler, type:" << item.type() << std::endl;
	}
	break;

	}

	return true;
}

void Player::SendMessage(Asset::MsgItem& item)
{
	DispatcherInstance.SendMessage(item);
}	

void Player::SendProtocol(pb::Message& message)
{
	const pb::FieldDescriptor* field = message.GetDescriptor()->FindFieldByName("type_t");
	if (!field) return;
	
	int type_t = field->default_value_enum()->number();
	if (!Asset::META_TYPE_IsValid(type_t)) return;	//如果不合法，不检查会宕线
	
	Asset::Meta meta;
	meta.set_type_t((Asset::META_TYPE)type_t);
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

//玩家心跳周期为10MS，如果该函数返回FALSE则表示掉线
bool Player::Update()
{
	++_heart_count; //心跳

	if (_heart_count % 6000 == 0) //1MIN
	{
		std::cout << "===================Time has gone 1min, i am id:" << this->_ID << std::endl;
	}
	return true;
}
	
int32_t Player::DefaultMethod(pb::Message* message)
{
	const pb::FieldDescriptor* field = message->GetDescriptor()->FindFieldByName("type_t");
	if (!field) 
	{
		std::cout << __func__ << ":Could not found type_t of received message." << std::endl;
		return 1;
	}
	const pb::EnumValueDescriptor* enum_value = message->GetReflection()->GetEnum(*message, field);
	if (!enum_value) return 2;

	const std::string& enum_name = enum_value->name();
	std::cout << __func__ << ":Could not found call back, message type is: " << enum_name.c_str() << std::endl;
	return 0;
}

bool Player::HandleProtocol(int32_t type_t, pb::Message* message)
{
	CallBack& callback = GetMethod(type_t); 
	callback(std::forward<pb::Message*>(message));	
	return true;
}

bool Player::GainItem(int64_t global_item_id)
{
	pb::Message* asset_item = AssetInstance.Get(global_item_id); //此处取出来的必然为合法ITEM.
	if (!asset_item) return false;

	Item* item = new Item(asset_item);
	GainItem(item);
	return true;
}

bool Player::GainItem(Item* item)
{
	if (!item) return false;

	const Asset::Item_CommonProp& common_prop = item->GetCommonProp(); 
	if (!PushBackItem(common_prop.inventory(), item)) return false;
	return true;
}

bool Player::PushBackItem(Asset::INVENTORY_TYPE inventory_type, Item* item)
{
	if (!item) return false;

	const pb::EnumDescriptor* enum_desc = Asset::INVENTORY_TYPE_descriptor();
	if (!enum_desc) return false;

	int inventory_size = enum_desc->value(inventory_type)->options().GetExtension(Asset::inventory_size); //默认格子数

	Asset::Inventories_Inventory* inventory = _stuff.mutable_inventory()->mutable_inventories(inventory_type); 
	if (!inventory) return false;

	if (inventory->items_size() >= inventory_size) return false; //超过包裹最大限制

	auto item_toadd = inventory->mutable_items()->Add();
	item_toadd->CopyFrom(item->GetCommonProp());
	return true;
}

}
