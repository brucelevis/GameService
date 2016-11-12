#pragma once

#include <memory>
#include <string>
#include <iostream>
#include <functional>
#include <unordered_map>
#include "ProtoHeader.h"

namespace Adoter {

namespace pb = google::protobuf;

/*
 * 类功能：
 * 
 * 自动注册(.proto)文件中的所有合法配置;
 *
 * */

class CollocationManager : public std::enable_shared_from_this<CollocationManager>
{
private:
	bool _parse_sucess;
	std::string _proto_file_path;
	
	std::unordered_map<int32_t, pb::Message*>  _messages;
public:
	CollocationManager();

	static CollocationManager& Instance()
	{
		static CollocationManager _instance;
		return _instance;
	}
	
	pb::Message* GetMessage(int32_t message_type)
	{
		auto it = _messages.find(message_type);
		if (it == _messages.end()) return nullptr;
		return it->second;
	}
	
	bool Load();
};

#define CollocationInstance CollocationManager::Instance()

}
