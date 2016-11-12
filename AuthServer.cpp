#include "AuthServer.h"
#include "CommonUtil.h"
#include <memory>

namespace GNET
{

void AuthServer::OnAccept(const boost::system::error_code& ec)
{
	std::cout << __func__ << "..." << std::endl;
	
	if (!ec)
	{	
		try
		{
			auto session = std::make_shared<Session>(std::move(_socket));
			_server_sessions.push_back(session);
			session->Start();
		}
		catch (std::exception& e)
		{
			std::cerr << "Exception: " << e.what() << std::endl;
		}
	}
	else
	{
		std::cout << __func__ << ":socket failed." << ec.message() << std::endl;
	}
	StartAccept();	
}

void AuthServer::HandleLogin(Session* session)
{
	real::LoginTicket ticket;
	
	if (CheckAccount()) 
	{
		ticket.set_authentication_state(real::LoginTicket::DONE);
		std::unique_ptr<real::Account> account(new real::Account());
		int64_t tocken = CommonUtil::Random(100000, CommonUtil::Random(100000, 50000000));
		
		std::string ticket = "REAL-" + std::to_string(tocken);	
		this->AddTicket(ticket, account.get());
	}
	else
	{
		ticket.set_authentication_state(real::LoginTicket::UNLAWFUL);
	}
	//SendResponse();
}

void AuthServer::AddTicket(const std::string& ticket, real::Account* account)
{
	std::unique_lock<std::mutex> lock(_ticket_mutex);
	account->set_expiry_time(time(nullptr) + 10);
	if (_valid_tickets.find(ticket) == _valid_tickets.end())
		_valid_tickets.emplace(ticket, std::move(account));
}

}

using namespace GNET;

int main(int argc, const char* argv[])
{
	boost::shared_ptr<AuthServer> pService(new AuthServer(40000, 2));
	
	try 
	{
		if (pService->Start())
		{
			std::cout << __func__ << " start failed" << std::endl;
			return 1;
		}
		
		while(true)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10000));
		}
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
	
	return 0;
}
