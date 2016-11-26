#include "Entity.h"
#include <iostream>

namespace Adoter
{

Entity::Entity() 
{ 

}

Entity::~Entity() 
{ 
	EntityInstance.Erase(this->_ID);
}

int64_t Entity::GetID()	
{ 
	return this->_ID; 
}

void EntityManager::Add(std::shared_ptr<Entity> entity)
{
	std::lock_guard<std::mutex> lock(_mutex);
	if (_entities.find(entity->_ID) == _entities.end())
		_entities.emplace(entity->_ID, entity);
	std::cout << __func__ << ":size:" << _entities.size() << ":id:" << entity->_ID << std::endl;
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

std::shared_ptr<Entity> EntityManager::GetEntity(int64_t id)
{
	std::lock_guard<std::mutex> lock(_mutex);
	auto it = _entities.find(id);
	if (it == _entities.end()) return nullptr;
	return it->second;
}

void EntityManager::Erase(int64_t id)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_entities.erase(id);
}

void EntityManager::Erase(Entity& entity)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_entities.erase(entity.GetID());
}

}
