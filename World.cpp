#include "World.h"
#include "Asset.h"
#include "Protocol.h"
#include "Scene.h"
#include "Hoster.h"

namespace Adoter
{

World::World()
{
}

World::~World()
{
}

bool World::Load()
{
	//协议初始化：必须最先初始化
	if (!ProtocolInstance.Load()) 
	{
		return false;
	}
	//数据初始化：必须最先初始化
	if (!AssetInstance.Load()) 
	{
		return false;
	}
	//场景初始化：必须最先初始化
	if (!SceneInstance.Load()) 
	{
		return false;
	}
//////////////其他初始化

	//居民、怪物初始化
	if (!HosterInstance.Load()) 
	{
		return false;
	}
	return true;
}

//世界中所有刷新都在此(比如刷怪，拍卖行更新...)，当前周期为50MS.
void World::Update(int32_t diff)
{
	//std::cout << __func__ << " of World:heart_count:" << _heart_count << std::endl;
	++_heart_count;
//刷新居民
	HosterInstance.Update(_heart_count);	
}
	

}
