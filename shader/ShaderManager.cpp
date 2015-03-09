#include "ShaderManager.h"


ShaderDefines::ShaderDefines()
{
	for (int i = 0; i < d_MaxShaderDefines; ++i)
	{
		auto& d = Defines[i];
		d.defined = 0;
		d.value   = 0;
	}
}
void ShaderDefines::Set(ShaderDefine key, int value)
{
	auto& d = Defines[key];
	d.defined = 1;
	d.value   = value;
}
void ShaderDefines::Remove(ShaderDefine key)
{
	Defines[key].defined = 0;
}
bool ShaderDefines::IsDefined(ShaderDefine key) const
{
	return !!Defines[key].defined;
}
bool ShaderDefines::GetValue(ShaderDefine key)  const
{
	auto& d = Defines[key];
	return d.defined ? d.value : 0;
}
const ShaderDefines::Def& ShaderDefines::operator[](ShaderDefine key) const 
{
	return Defines[key];
}