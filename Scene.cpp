#include "Scene.h"
#include <vector>
#include <algorithm>

namespace Adoter
{

void Scene::EnterScene(Entity* entity)
{
	if (!entity) return;
	_x_entities[entity->GetX()].emplace(entity->GetID());
	_z_entities[entity->GetX()].emplace(entity->GetID());
}

void Scene::UpdatePosition(Entity* entity, real::Vector3& position)
{
	if (!entity) return;
	_x_entities[entity->GetX()].erase(entity->GetID());
	_z_entities[entity->GetZ()].erase(entity->GetID());
	_x_entities[position.x()].emplace(entity->GetID());
	_z_entities[position.z()].emplace(entity->GetID());
}

void Scene::LeaveScene(Entity* entity)
{
	if (!entity) return;
	_x_entities[entity->GetX()].erase(entity->GetID());
	_z_entities[entity->GetZ()].erase(entity->GetID());
}

int32_t Scene::GetNearByEntities(Entity* entity, std::vector<int64_t>& entities)
{
	if (!entity) return 0; //没有实体
	
	int32_t x = entity->GetX();
	int32_t z = entity->GetZ();
	std::unordered_set<int64_t> x_entities;
	std::unordered_set<int64_t> z_entities;
	for (int32_t xi = x - DISTANCE; xi <= x + DISTANCE; ++xi)
	{
		x_entities.insert(_x_entities[xi].begin(), _x_entities[xi].end());
	}
	for (int32_t zi = z - DISTANCE; zi <= z + DISTANCE; ++zi)
	{
		z_entities.insert(_z_entities[zi].begin(), _z_entities[zi].end());
	}
	std::set_intersection(x_entities.begin(), x_entities.end(), z_entities.begin(), z_entities.end(), std::back_inserter(entities));
	return entities.size();
}

}
