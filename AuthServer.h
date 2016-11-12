/*
 * 认证服务器
 *
 * 主要用来生成TOCKEN
 *
 * 账号认证，其本质是一个WEB服务器
 *
 * */
#include <atomic>
#include <mutex>
#include <string>
#include "Server.h"

namespace GNET
{

class AuthServer : public Service 
{
private:
	std::mutex _ticket_mutex;    
	boost::asio::deadline_timer* _ticket_cleanup_timer;
	std::unordered_map<std::string, real::Account*> _valid_tickets;    
	
	std::vector<std::shared_ptr<Session>> _client_sessions;	 
	std::vector<std::shared_ptr<Session>> _server_sessions;	//游戏服务器连接会话
	std::vector<real::ServerList_Server> _server_list; //游戏服务器列表
public:
	~AuthServer() {}
	AuthServer(const int port, const int thread_nums = 5) : Service(port, thread_nums) { }
	virtual void Initialize() override { std::cout << __func__ << "----" << __LINE__ << std::endl; }
	virtual void OnAccept(const boost::system::error_code& ec) override;
public:
	void HandleLogin(Session* session);
	void AddTicket(const std::string& ticket, real::Account* account);
	bool CheckAccount() { return true; }
};

}
