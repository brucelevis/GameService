#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <boost/asio.hpp>
#include "Socket.h"

namespace Adoter
{

using namespace google::protobuf;

/*
 * 类说明：
 *
 * 会话：每个玩家和服务器保持的一个连接，称为会话.
 *
 * 说明：有的游戏也称为连接(Connection).
 *
 * */

class Session : public Socket<Session>
{
public:
	typedef std::function<int32_t(Message*)> CallBack;
public:
	virtual ~Session() { }
	Session(boost::asio::ip::tcp::socket&& socket);
	Session(Session const& right) = delete;    
	Session& operator=(Session const& right) = delete;
	
	virtual void Start() override;
	bool Update() override { return true; }
	void InitializeHandler(const boost::system::error_code error, const std::size_t bytes_transferred);
};

class SessionManager : public SocketManager<Session> 
{
	typedef SocketManager<Session> SuperSocketManager;
private:
	std::mutex _mutex;
	std::vector<std::shared_ptr<Session>> _list; //定时清理断开的会话
	std::unordered_map<int64_t, std::shared_ptr<Session>> _sessions; //根据玩家状态变化处理	
public:
	static SessionManager& Instance()
	{
		static SessionManager _instance;
		return _instance;
	}

	size_t GetCount();
	void Add(std::shared_ptr<Session> session);
	void Emplace(int64_t player_id, std::shared_ptr<Session> session);
	bool StartNetwork(boost::asio::io_service& io_service, const std::string& bind_ip, int32_t port, int thread_count = 1) override;
protected:        
	NetworkThread<Session>* CreateThreads() const override;
private:        
	static void OnSocketAccept(tcp::socket&& socket, int32_t thread_index);
};

#define SessionInstance SessionManager::Instance()

}
