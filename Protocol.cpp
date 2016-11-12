#include "Protocol.h"

namespace Adoter {

ProtocolManager::ProtocolManager() : _parse_sucess(false)
{
	_method = std::bind(&ProtocolManager::DefaultMethod, this, std::placeholders::_1);
}

bool ProtocolManager::Load()
{
	_proto_file_path = "Protocol.proto";
	
	const pb::DescriptorPool* _pool = pb::DescriptorPool::generated_pool();
	const pb::FileDescriptor* _file_descriptor = _pool->FindFileByName(_proto_file_path);

	const pb::EnumDescriptor* gps_type = _file_descriptor->FindEnumTypeByName("MetaType");	
	if (!gps_type) 
		std::cout << __func__ << ":could not found typename:MetaType" << std::endl;

	for (int i = 0; i < _file_descriptor->message_type_count(); ++i)
	{
		const pb::Descriptor* descriptor = _file_descriptor->message_type(i);
		if (!descriptor) return false;
	
		if (descriptor->name() == "Meta") continue;		//Meta不作为协议处理流程一员
			
		const pb::FieldDescriptor* field = descriptor->FindFieldByNumber(1);	//所有MESSAGE的第一个变量必须是类型
		if(!field || field->enum_type() != gps_type) 
		{
			//std::cout << __func__ << ":could not load protocol field:" << field->name() << std::endl;
			continue;
		}

		int type_t = field->default_value_enum()->number();
		if (type_t > real::META_TYPE_C2S_COUNT) continue; 	//只加载C2S的协议处理
		
		const pb::Message* msg = pb::MessageFactory::generated_factory()->GetPrototype(descriptor);
		pb::Message* message = msg->New();
		_messages.emplace(field->default_value_enum()->number(), message);	//合法协议，如果已经存在则忽略，即只加载第一个协议
	}
	std::cout << __func__ << ":加载协议数据成功，加载合法协议数量：" << _messages.size() << std::endl;
	_parse_sucess = true;
	return true;
}

int32_t ProtocolManager::DefaultMethod(pb::Message* message)
{
	const pb::FieldDescriptor* field = message->GetDescriptor()->FindFieldByName("type_t");
	if (!field) 
	{
		std::cout << __func__ << ":could not found type_t of received message." << std::endl;
		return 1;
	}
	int type_t = field->default_value_enum()->number();
	std::cout << __func__ << ":could not found call back, message type is:" << type_t << std::endl;
	return 0;
}

}
