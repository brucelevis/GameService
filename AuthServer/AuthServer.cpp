#include "AuthServer.h"
#include <memory>
#include <thread>
#include <iostream>
#include "CommonUtil.h"

namespace Adoter
{

void AuthServer::OnAccept(const boost::system::error_code& ec)
{
}

void AuthServer::HandleLogin(/*Session* session*/)
{
	/*
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
	*/
	//SendResponse();
}

void AuthServer::AddTicket(const std::string& ticket, Auth::Account* account)
{
	std::unique_lock<std::mutex> lock(_ticket_mutex);
	account->set_expiry_time(time(nullptr) + 10);
	if (_valid_tickets.find(ticket) == _valid_tickets.end())
		_valid_tickets.emplace(ticket, std::shared_ptr<Auth::Account>(std::move(account)));
}

bool AuthServer::Start(boost::asio::io_service& io_service)
{
	_bind_ip = "0.0.0.0";
	_bind_port = 80000;
	
	boost::system::error_code ec;
	boost::asio::ip::tcp::resolver resolver(io_service);
	boost::asio::ip::tcp::resolver::iterator end;
	std::string configure_adddress = "127.0.0.1";  
	
	boost::asio::ip::tcp::resolver::query external_address_query(boost::asio::ip::tcp::v4(), configure_adddress, std::to_string(_bind_port));    
	boost::asio::ip::tcp::resolver::iterator end_point = resolver.resolve(external_address_query, ec);

	if (end_point == end || ec) return false;

	_external_address = end_point->endpoint();

	configure_adddress = "127.0.0.1";  
	boost::asio::ip::tcp::resolver::query local_address_query(boost::asio::ip::tcp::v4(), configure_adddress, std::to_string(_bind_port));    
	end_point = resolver.resolve(local_address_query, ec);    
	if (end_point == end || ec) return false;    
	_local_address = end_point->endpoint();

	_ticket_cleanup_timer = new boost::asio::deadline_timer(io_service);	
	_ticket_cleanup_timer->expires_from_now(boost::posix_time::seconds(10));    
	_ticket_cleanup_timer->async_wait(std::bind(&AuthServer::CleanLoginTickets, this, std::placeholders::_1));

	_thread = std::thread(std::bind(&AuthServer::Run, this));

	return true;
}

void AuthServer::Run()
{
	std::cout << "-----------------------" << std::endl;
	boost::asio::io_service io_service;
	boost::asio::ip::tcp::acceptor acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 50000));  
	try  
	{  
		while(!_stopped)
		{
			std::shared_ptr<boost::asio::ip::tcp::socket> socket = std::make_shared<boost::asio::ip::tcp::socket>(io_service);
			acceptor.accept(*socket); 

			std::thread([socket](){
				std::string message = "Hello world."; 
				boost::system::error_code ignored_error;  	
				boost::asio::write(*socket, boost::asio::buffer(message), boost::asio::transfer_all(), ignored_error);
			}).detach();
		}
	}
	catch (std::exception& e)  
	{  
		std::cout << e.what() << std::endl;  
	}  
}

void AuthServer::Stop()
{
	_stopped = true;
	_thread.join(); //释放资源
	_ticket_cleanup_timer->cancel(); //停止定时器
}

void AuthServer::CleanLoginTickets(const boost::system::error_code& error)
{
	if (error) return;
	time_t now = time(nullptr);
	{
		std::unique_lock<std::mutex> lock(_ticket_mutex);
		for (auto it = _valid_tickets.begin(); it != _valid_tickets.end(); ++it)
		{
			if (it->second->expiry_time() < now)	 //超时
				it = _valid_tickets.erase(it);
			else
				++it;
		}
	}
	//每10秒检查一次
	_ticket_cleanup_timer->expires_from_now(boost::posix_time::seconds(10));    
	_ticket_cleanup_timer->async_wait(std::bind(&AuthServer::CleanLoginTickets, this, std::placeholders::_1));
}

}

