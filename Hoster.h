#pragma once

#include <memory>
#include <functional>
#include "Entity.h"
#include "Asset.h"
#include "MessageDispatcher.h"

namespace Adoter
{
namespace pb = google::protobuf;

/*
 * 居民
 *
 * */
class Hoster : public Entity
{
private:
	Asset::Hoster _stuff;
public:
	~Hoster() { }
	Hoster() { }
	Hoster(int64_t hoster_id);
	Hoster(Asset::Hoster& hoster);
	Hoster(Asset::Hoster* hoster);

	virtual bool HandleMessage(const Asset::MsgItem& item) override;

	bool Update(int32_t heart_count);
	//所有CLASS_NAME + Asset::CLASS_NAME组合的结构设计，建议都定义该函数
	const Asset::Hoster& Get() { return this->_stuff; } 
	const Asset::CommonProp& GetHumanProp() { return this->_stuff.human_prop(); } //获取基础属性，比如级别、性别...
};

/*
 * 居民管理
 *
 * */
class HosterManager
{
private:
	std::unordered_set<std::shared_ptr<Hoster>> _hosters;
public:
	static HosterManager& Instance()
	{
		static HosterManager _instance;
		return _instance;
	}
	
	//加载HOSTER数据
	bool Load();
	//定时刷新HOSTER
	bool Update(int32_t diff);
};

#define HosterInstance HosterManager::Instance()

}
