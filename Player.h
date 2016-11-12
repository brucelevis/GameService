#pragma once

#include <memory>
#include <functional>
#include "Entity.h"
#include "P_Header.h"
#include "WorldSession.h"
#include "MessageDispatcher.h"

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
	std::shared_ptr<WorldSession> _session;	
private:
	real::Player _stuff;
public:
	Player();
	Player(int64_t player_id, std::shared_ptr<WorldSession> session);
	int32_t GetLevel() { return this->_common_prop.level(); }
	int32_t GetGender() { return this->_common_prop.gender(); }

	virtual int32_t Load() override;
	virtual int32_t Save() override;

	int32_t Login();
	int32_t Logout();
	
	virtual void EnterScene() override;
	virtual void LeaveScene() override;
	virtual void StepMove(real::Vector3& position) override;
	virtual void HandleMessage(real::MsgItem& item) override;
	virtual void SendMessage(real::MsgItem& item) override; 
	virtual void SendNearbyMessage(real::MsgItem& item) override;
	virtual void HandleProtocol(pb::Message& message) override {} 
	virtual void SendProtocol(pb::Message& message) override; 
	virtual void SendNearbyProtocol(pb::Message& message) override;
public:
	const real::Inventories& GetInventories()
	{
		return _stuff.inventory();
	}

	const real::Inventories_Inventory& GetInventory(real::InventoryType type)
	{
		return _stuff.inventory().inventories(type);
	}	
	
	const std::shared_ptr<WorldSession> GetSession()
	{
		return _session;
	}
};

}
