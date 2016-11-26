#pragma once

#include <unordered_map>
#include <memory>
#include <unordered_set>
#include "Entity.h"

//玩家视野
#define DISTANCE 20

namespace Adoter
{

class Entity;

class Scene : public std::enable_shared_from_this<Scene>
{
private:
	int64_t _ID;
	std::unordered_map<int32_t, std::unordered_set<int64_t>> _x_entities; //X轴上实体
	std::unordered_map<int32_t, std::unordered_set<int64_t>> _z_entities; //Z轴上实体
public:
	~Scene() { }
	Scene() { }
	Scene(int64_t ID) : _ID(ID)  { }
	int64_t GetID() { return this->_ID; }
public:
	void EnterScene(Entity* entity);
	void UpdatePosition(Entity* entity, Asset::Vector3& position);	//更新玩家在场景中的位置
	void LeaveScene(Entity* entity);
	int32_t GetNearByEntities(Entity* entity, std::vector<int64_t>& entities); //获取附近指定类型的实体
};

class SceneManager : public std::enable_shared_from_this<SceneManager>
{
	std::unordered_map<int64_t, std::shared_ptr<Scene>> _scenes; 
public:
	static SceneManager& Instance()
	{
		static SceneManager _instance;
		return _instance;
	}
	
	void Add(std::shared_ptr<Scene> scene) { _scenes.emplace(scene->GetID(), scene); }
	void Emplace(int64_t scene_id, std::shared_ptr<Scene> scene) { _scenes.emplace(scene_id, scene); }
	std::shared_ptr<Scene> GetScene(int64_t scene_id) 
	{
		auto it = _scenes.find(scene_id); 
		if (it == _scenes.end()) return nullptr;
		return it->second;
	}
};

#define SceneInstance SceneManager::Instance()

}
