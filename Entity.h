#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>
#include "P_Header.h"
#include "Scene.h"

namespace Adoter
{

/*
 * 类说明:
 *
 * 游戏中所有实体的基类
 *
 * 说明：有的游戏中也被称为Object、Actor、Creature...
 *
 * */
namespace pb = google::protobuf;

class Scene;

class Entity : public std::enable_shared_from_this<Entity>
{
public:
	int64_t _ID; //所有实体的唯一识别不能重复
	Scene* _locate_scene; //实体所在场景
	real::CommonProp _common_prop; //公共属性
public:
	Entity(); 
	virtual ~Entity(); 
	
	virtual int64_t GetID();

	virtual void EnterScene() { }
	virtual void LeaveScene() { }
	virtual void StepMove(real::Vector3& position) { } 
	virtual void UpdatePosition(real::Vector3& position) { _common_prop.mutable_position()->CopyFrom(position); }
	virtual void UpdatePosition(float x, float z) //不用Y坐标
	{ 
		real::Vector3 position; 
		position.set_x(x);
		position.set_z(z);
		UpdatePosition(position);
	}
	virtual float GetX() { return _common_prop.position().x(); }
	virtual float GetY() { return _common_prop.position().y(); }
	virtual float GetZ() { return _common_prop.position().z(); }
	virtual Scene* GetScene() { return _locate_scene; }	
	virtual int32_t GetSceneID() { return _common_prop.scene_id(); }	
	virtual void SetScene(Scene* scene) { _locate_scene = scene; }	
	virtual int32_t Load() { return 0; }
	virtual int32_t Save() { return 0; }
	
	//消息处理
	virtual void HandleMessage(real::MsgItem& item) { } 
	virtual void SendMessage(real::MsgItem& item) { }
	virtual void SendNearbyMessage(real::MsgItem& message) { }
	//协议处理(Protocol Buffer)
	virtual void HandleProtocol(pb::Message& message) { } 
	virtual void SendProtocol(pb::Message& message) { }
	virtual void SendNearbyProtocol(pb::Message& message) { }
};

class EntityManager : public std::enable_shared_from_this<EntityManager>
{
private:
	std::mutex _mutex;
	std::unordered_map<int64_t, std::shared_ptr<Entity>&> _entities;
public:
	static EntityManager& Instance()
	{
		static EntityManager _instance;
		return _instance;
	}

	void Erase(int64_t id);
	void Erase(Entity& entity);
	void Add(std::shared_ptr<Entity> entity);
	void Emplace(int64_t entity_id, std::shared_ptr<Entity> entity);
	std::shared_ptr<Entity> GetEntity(int64_t id);
};

#define EntityInstance EntityManager::Instance()

}
