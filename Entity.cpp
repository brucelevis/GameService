#include "Entity.h"
#include "Asset.h"
#include "MessageDispatcher.h"
#include <iostream>

namespace Adoter
{

/*
 * 实体类
 *
 * */
Entity::Entity() 
{ 

}

Entity::~Entity() 
{ 
	EntityInstance.Erase(this->_ID);
}

bool Entity::IsInSight(const Asset::Vector3& pos)
{
	const Asset::Vector3& curr_pos = this->_super_common_prop.position();
	float visibility_range = this->GetSightDistance(); 

	return (visibility_range < fabs(pos.x() - curr_pos.x())) || (visibility_range < fabs(pos.z() - curr_pos.z()));
}

void Entity::OnBirth(int64_t scene_id) 
{
	OnEnterScene(scene_id);
}

void Entity::OnEnterScene(int64_t scene_id/* = 0*/)
{
	_locate_scene = SceneInstance.Get(scene_id);
	if (!_locate_scene) return; //非法的场景 

	SetSceneID(scene_id); //设置当前场景ID
	GetScene()->EnterScene(shared_from_this());
	//向周围玩家发送公共数据
	BroadCastCommonProp(Asset::MSG_TYPE_AOI_ENTER);
}

void Entity::OnDeath()
{
	EntityInstance.Erase(this->_ID);
}
	
void Entity::BroadCastCommonProp(Asset::MSG_TYPE type)       
{
	Asset::MsgItem item; //消息数据
	item.set_type(type);
	item.set_sender(this->_ID);
	item.set_content(this->_super_common_prop.SerializeAsString()); //属性数据
	this->SendNearbyMessage(item); //通知给周围玩家
}

void Entity::OnLeaveScene()
{
	if (!_locate_scene) return; 

	GetScene()->LeaveScene(this);
	//向周围玩家发送公共数据
	Asset::MsgItem item; //消息数据
	item.set_type(Asset::MSG_TYPE_AOI_LEAVE);
	item.set_sender(this->_ID);
	this->SendNearbyMessage(item); //通知给周围玩家
}
	
void Entity::StepMove(const Asset::Vector3& position) 
{ 
	if (!_locate_scene) return;

	this->UpdatePosition(position); //更新自己位置
	GetScene()->UpdatePosition(this, position); //更新场景中的位置：删除之前的位置，增加到新位置
	//向周围玩家发送公共数据，由于移动过程中可能改变等级、状态等数据，所以此处都发
	BroadCastCommonProp(Asset::MSG_TYPE_AOI_MOVE);
} 

bool Entity::HandleMessage(const Asset::MsgItem& item)
{
	switch (item.type())
	{
	//进入视野
	case Asset::MSG_TYPE_AOI_ENTER:
	{
		if (item.sender() == item.receiver()) return false; //这种协议不处理
		Asset::CommonProp common_prop;
		//if (!common_prop.ParseFromString(item.content())) return;
		//if (!IsInSight(common_prop.position())) return;	//如果看不到该实体，则退出
	}
	break;
	//离开视野
	case Asset::MSG_TYPE_AOI_LEAVE:
	{
		if (item.sender() == item.receiver()) return false; //这种协议不处理
		Asset::CommonProp common_prop;
		if (!common_prop.ParseFromString(item.content())) return false;
	}
	break;
	//移动	
	case Asset::MSG_TYPE_AOI_MOVE:
	{
		if (item.sender() == item.receiver()) return false; //这种协议不处理
		Asset::CommonProp common_prop;
		if (!common_prop.ParseFromString(item.content())) return false;
	}
	break;
	
	default:
	{
		std::cout << __func__ << ":No handler, type:" << item.type() << std::endl;
	}
	break;

	}
	
	//std::cout << __func__ << " of Entity call:sender:" << item.sender() << ", receiver:" << item.receiver() << ", type:" << item.type() << std::endl;
	return true;
}

void Entity::SendMessage(Asset::MsgItem& item)
{
	DispatcherInstance.SendMessage(item);
}

void Entity::SendNearbyMessage(Asset::MsgItem& item) 
{
	if (!_locate_scene) return;
	
	//time_t curr = time((time_t*)NULL);
	//std::cout << __func__ << "------------------curr time stat:" << curr << std::endl;
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
	//curr = time((time_t*)NULL);
	//std::cout << __func__ << "------------------curr time end:" << curr << std::endl;
}	

/*
 * 实体管理类
 *
 * */

void EntityManager::Add(std::shared_ptr<Entity> entity)
{
	std::lock_guard<std::mutex> lock(_mutex);
	if (_entities.find(entity->_ID) == _entities.end())
	{
		_entities.emplace(entity->_ID, entity);
		std::cout << "Add entity to manager current size is:" << _entities.size() << ", added id:" << entity->_ID << std::endl;
	}
}

void EntityManager::Emplace(int64_t entity_id, std::shared_ptr<Entity> entity)
{
	try
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_entities.find(entity_id) == _entities.end())
			_entities.emplace(entity_id, entity);
		std::cout << __func__ << ":size:" << _entities.size() << ":id:" << entity->_ID << std::endl;
	}
	catch(std::exception& ex)
	{
		std::cout << __LINE__ << ex.what() << std::endl;
	}
}

std::shared_ptr<Entity> EntityManager::GetEntity(int64_t entity_id)
{
	std::lock_guard<std::mutex> lock(_mutex);
	auto it = _entities.find(entity_id);
	if (it == _entities.end()) return nullptr;
	return it->second;
}
	
bool EntityManager::Has(int64_t entity_id)
{
	auto entity = GetEntity(entity_id);
	return entity != nullptr;
}

void EntityManager::Erase(int64_t entity_id)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_entities.erase(entity_id);
}

void EntityManager::Erase(Entity& entity)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_entities.erase(entity.GetID());
}

}
