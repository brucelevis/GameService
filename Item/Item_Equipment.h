#pragma once

#include "Item.h"
#include "ProtoHeader.h"

namespace Adoter
{

class Item_Equipment : public Item
{
private:
	real::ItemEquipment stuff_extra;
public:
	Item_Equipment();
	~Item_Equipment();
	
	explicit Item_Equipment(real::Item_Equipment* stuff);
	
	virtual bool CanUse() override;     
	virtual int32_t Use() override; 
};

}
