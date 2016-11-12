#include "WorldSession.h"
#include "Player.h"

namespace Adoter
{

WorldSession::WorldSession(boost::asio::ip::tcp::socket&& socket) : Socket(std::move(socket))
{
}

void WorldSession::InitializeHandler(const boost::system::error_code error, const std::size_t bytes_transferred)
{
	try
	{
		std::cout << "------------------------------" << _socket.remote_endpoint().address() << std::endl;

		if (error)
		{
			Close();
			std::cout << "Remote client disconnect, RemoteIp:" << _socket.remote_endpoint().address() << std::endl;
			return;
		}
		else
		{
			/*
			{
				real::Login login;
				login.set_type_t(real::META_TYPE_C2S_LOGIN);
				login.mutable_account()->set_username("zhenyunheng@zulong.com");
				login.mutable_account()->set_password("123456");

				real::Meta smeta;
				smeta.set_type_t(real::META_TYPE_C2S_LOGIN);
				smeta.set_stuff(login.SerializeAsString());
	
				std::string content = smeta.SerializeAsString();
				AsyncSend(content.c_str(), content.size());
			}
			*/
			real::Meta meta;
			bool result = meta.ParseFromArray(_buffer.data(), bytes_transferred);
			if (!result) 
			{
				std::cout << __func__ << ":Meta parse error." << std::endl;	
				Close();
				return;		//非法协议
			}
			
			google::protobuf::Message* message = ProtocolInstance.GetMessage(meta.type_t());	
			if (!message) 
			{
				std::cout << __func__ << ":Could not found message of type:" << meta.type_t() << std::endl;
				Close();
				return;		//非法协议
			}
			
			result = message->ParseFromString(meta.stuff());
			if (!result) 
			{
				std::cout << __func__ << ":Messge parse error." << std::endl;	
				Close();
				return;		//非法协议
			}
			/*
			//由于是登录协议，需要加载数据，因此此处特殊处理
			if (real::META_TYPE_C2S_LOGIN == meta.type_t())
			{	
				const real::Login* login = dynamic_cast<real::Login*>(message);
				if (!login) 
				{
					std::cout << __func__ << ":Login parse error." << std::endl;	
					return;		//非法协议
				}
				std::cout << __func__ << ":模拟登陆." << std::endl;	
				for (int i = 0; i < 1; ++i)
				{
					//std::shared_ptr<Player> player(new Player(login->player_id(), shared_from_this()));
					//usleep(100);
					std::shared_ptr<Player> player(new Player(10001, shared_from_this()));
					player->SetScene(SceneInstance.GetScene(1000).get());
					if (!player->Login())	//登录成功
					{
						//EntityInstance.Add(player);	//实体通用管理类
						EntityInstance.Emplace(10001, player);	//实体通用管理类
					}
				}
			}
			else
			*/
			{
				//其他协议的调用规则
				CallBack callback = ProtocolInstance.GetMethod(meta.type_t()); 
				callback(message);	
			}
		}
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
		return;
	}
	//递归持续接收	
	AsyncReceiveWithCallback(&WorldSession::InitializeHandler);
}

void WorldSession::Start()
{
	AsyncReceiveWithCallback(&WorldSession::InitializeHandler);
}

void WorldSessionManager::Add(std::shared_ptr<WorldSession> session)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_list.push_back(session);
}	

void WorldSessionManager::Emplace(int64_t player_id, std::shared_ptr<WorldSession> session)
{
	std::lock_guard<std::mutex> lock(_mutex);
	if (_sessions.find(player_id) == _sessions.end())
		_sessions.emplace(player_id, session);
}

size_t WorldSessionManager::GetCount()
{
	std::lock_guard<std::mutex> lock(_mutex);
	return _list.size();
}

NetworkThread<WorldSession>* WorldSessionManager::CreateThreads() const
{    
	return new NetworkThread<WorldSession>[GetNetworkThreadCount()];
}

void WorldSessionManager::OnSocketAccept(tcp::socket&& socket, int32_t thread_index)
{    
	WorldSessionInstance.OnSocketOpen(std::forward<tcp::socket>(socket), thread_index);
}

bool WorldSessionManager::StartNetwork(boost::asio::io_service& io_service, const std::string& bind_ip, int32_t port, int thread_count)
{
	if (!SuperSocketManager::StartNetwork(io_service, bind_ip, port, thread_count)) return false;
	_acceptor->SetSocketFactory(std::bind(&SuperSocketManager::GetSocketForAccept, this));    
	_acceptor->AsyncAcceptWithCallback<&OnSocketAccept>();    
	return true;
}

}
