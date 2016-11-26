#include "Asset.h"

namespace Adoter {

AssetManager::AssetManager() : _parse_sucess(false)
{
}

bool AssetManager::Load()
{
	_proto_file_path = "CommonConfig.proto";
	
	const pb::DescriptorPool* _pool = pb::DescriptorPool::generated_pool();
	if (!_pool) return false;

	const pb::FileDescriptor* _file_descriptor = _pool->FindFileByName(_proto_file_path);
	if (!_file_descriptor) return false;

	const pb::EnumDescriptor* gps_type = _file_descriptor->FindEnumTypeByName("ConfigType");	
	if (!gps_type) std::cout << __func__ << ":could not found typename:ConfigType" << std::endl;

	for (int i = 0; i < _file_descriptor->message_type_count(); ++i)
	{
		const pb::Descriptor* descriptor = _file_descriptor->message_type(i);
		if (!descriptor) return false;
	
		const pb::FieldDescriptor* field = descriptor->FindFieldByNumber(1);	//所有MESSAGE的第一个变量必须是类型
		if(!field || field->enum_type() != gps_type) continue;

		const pb::Message* msg = pb::MessageFactory::generated_factory()->GetPrototype(descriptor);
		pb::Message* message = msg->New();
		if (_messages.find(field->default_value_enum()->number()) == _messages.end())
			_messages.emplace(field->default_value_enum()->number(), message);	//合法协议，如果已经存在则忽略，即只加载第一个协议
		else
		       std::cout << "配置类型重复：" << field->default_value_enum()->number() << std::endl;
	}
	std::cout << __func__ << ":加载配置数据成功，加载合法配置数量：" << _messages.size() << std::endl;
	_parse_sucess = true;
	return true;
}

}
