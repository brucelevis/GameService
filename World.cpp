#include "World.h"

namespace Adoter
{

World::World()
{
}

World::~World()
{
}

void World::Update(int32_t diff)
{
	_update_time = diff;
}

}
