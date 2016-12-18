#pragma once

#include <memory>
#include <functional>
#include "Entity.h"
#include "P_Header.h"
#include "WorldSession.h"
#include "MessageDispatcher.h"
#include "Item.h"
#include "Asset.h"

namespace Adoter
{
namespace pb = google::protobuf;

/*
 *
 * 玩家
 *
 * */
class Player : public Entity
{
	typedef std::function<int32_t(pb::Message*)> CallBack;
	std::unordered_map<int32_t, CallBack>  _callbacks;	//每个协议的回调函数，不要传入引用
private:
	std::shared_ptr<WorldSession> _session;	//网络连接
	CallBack _method;
	
	Asset::Player _stuff; //玩家数据
	int64_t _heart_count = 0; //心跳次数
public:
	Player();

	Player(int64_t player_id, std::shared_ptr<WorldSession> session);
	//协议处理默认调用函数
	int32_t DefaultMethod(pb::Message*);
	
	void AddHandler(Asset::META_TYPE message_type, CallBack callback)
	{
		if (_callbacks.find(message_type) != _callbacks.end()) return;
		_callbacks.emplace(message_type, callback);
	}

	CallBack& GetMethod(int32_t message_type)
	{
		auto it = _callbacks.find(message_type);
		if (it == _callbacks.end()) return _method;
		return it->second;
	}

//////////////////////////////////////属性接口
	//获取当前视野，玩家真正的视野范围
	float GetCurretSightDistance() { return this->_stuff.provisional_prop().sight_distance(); }
	//修改视野(比如，有些技能遮挡视野)
	void ModifySightDistance(float off) { this->_stuff.mutable_provisional_prop()->set_sight_distance(this->_stuff.provisional_prop().sight_distance() + off); }
	///////////////////////////////////////////////////////////////////////
	//获取玩家数据结构
	//
	//所有CLASS_NAME + Asset::CLASS_NAME组合的结构设计，建议都定义该函数
	///////////////////////////////////////////////////////////////////////
	const Asset::Player& Get() { return this->_stuff; }
//////////////////////////////////////客户端操作响应接口
	//进入场景
	virtual int32_t CmdEnterScene(pb::Message* message);
	//玩家登录
	virtual int32_t CmdLogin(pb::Message* message);
	//玩家移动
	virtual int32_t CmdMove(pb::Message* message);
///////1.重载基类函数
	//离开场景
	virtual int32_t LeaveScene(pb::Message* message);
	//消息处理
	virtual bool HandleMessage(const Asset::MsgItem& item) override;
	virtual void SendMessage(Asset::MsgItem& item) override; 
	//协议处理(Protocol Buffer)
	virtual bool HandleProtocol(int32_t type_t, pb::Message* message) override; 
	virtual void SendProtocol(pb::Message& message) override; 
	virtual void SendNearbyProtocol(pb::Message& message) override; //向周围玩家发送消息数据
	//加载数据	
	virtual int32_t Load() override;
	//保存数据
	virtual int32_t Save() override;
	//是否为玩家
	virtual bool IsPlayer() override { return true; }
	//设置实体类型
	virtual void SetEntityType(Asset::ENTITY_TYPE entity_type) { 
		this->_super_common_prop.set_entity_type(entity_type);  //基类
		this->_stuff.mutable_common_prop()->set_entity_type(entity_type);  //玩家
	}
///////2.自定义函数
	//玩家登出
	virtual int32_t Logout(pb::Message* message);
	//进入游戏
	virtual int32_t OnEnterGame();
	//同步玩家数据到客户端
	virtual void SendPlayer();
	//玩家心跳周期为10MS，如果该函数返回FALSE则表示掉线
	virtual bool Update();
public:
	//获取所有包裹
	const Asset::Inventories& GetInventories()
	{
		return _stuff.inventory();
	}

	//获取指定包裹
	const Asset::Inventories_Inventory& GetInventory(Asset::INVENTORY_TYPE type)
	{
		return _stuff.inventory().inventories(type);
	}	
	
	//获取物品
	bool GainItem(Item* item);
	bool GainItem(int64_t global_item_id);
	//存放物品
	bool PushBackItem(Asset::INVENTORY_TYPE inventory_type, Item* item);

	const std::shared_ptr<WorldSession> GetSession()
	{
		return _session;
	}
};

}
