#pragma once

namespace ShaderSources
{

const char* shaderColor = 
"cbuffer MatrixProjection : register(b0)\
{\
	matrix projectionMatrix;\
}\
\
cbuffer MatrixView : register(b1)\
{\
	matrix viewMatrix;\
}\
\
cbuffer MatrixWorld : register(b2)\
{\
	matrix worldMatrix;\
}\
\
cbuffer Color : register(b3)\
{\
	float4 _color;\
}\
\
Texture2D shaderTexture;\
SamplerState SampleType;\
\
\
struct VertexInputType\
{\
	float4 position : POSITION;\
};\
\
struct PixelInputType\
{\
float4 position : SV_POSITION; \
}; \
\
PixelInputType VShader(VertexInputType input)\
{\
PixelInputType output; \
\
input.position.w = 1.0f; \
\
matrix mvp = mul(projectionMatrix, mul(viewMatrix, worldMatrix)); \
output.position = mul(mvp, input.position); \
\
return output; \
}\
\
float4 PShader(PixelInputType input) : SV_Target\
{\
return _color; \
}";

//==============================================================================================================================================
//==============================================================================================================================================
//==============================================================================================================================================

const char* shaderLineAnimation = 
"cbuffer MatrixProjection : register(b0)\
{\
	matrix projectionMatrix;\
}\
\
cbuffer MatrixView : register(b1)\
{\
	matrix viewMatrix;\
}\
\
cbuffer MatrixWorld : register(b2)\
{\
	matrix worldMatrix;\
}\
\
cbuffer Animation : register(b3)\
{\
	float animationMinY;\
	float animationHeight;\
	float maxAnimationOffset;\
	float startScale;\
}\
\
Texture2D shaderTexture;\
SamplerState SampleType;\
\
\
struct VertexInputType\
{\
	float4 position : POSITION;\
	float3 normal : NORMAL;\
	float4 color : COLOR;\
	float2 tex : TEXCOORD0;\
};\
\
struct GeometryInputType\
{\
	float4 position : POSITION;\
	float3 normal : NORMAL;\
	matrix mvp : TEXCOORD1;\
	float4 color : COLOR;\
};\
\
struct PixelInputType\
{\
	float4 position : SV_POSITION;\
	float4 color : COLOR;\
};\
\
GeometryInputType VShader(VertexInputType input)\
{\
	GeometryInputType output;\
\
	output.color = input.color;\
	output.normal = input.normal;\
\
	output.mvp = mul(projectionMatrix, mul(viewMatrix, worldMatrix));\
	output.position = input.position;\
\
	return output;\
}\
\
[maxvertexcount(3)]\
void GShader(line GeometryInputType input[2], inout LineStream<PixelInputType> OutputStream)\
{\
	PixelInputType output;\
\
	float4 p = (input[0].position + input[1].position) / 1;\
\
	float state = (p.y - animationMinY) / animationHeight;\
	if (state < 0) state = 0;\
	if (state > 1) state = 1;\
	float4 offset = float4(normalize(input[0].normal + input[1].normal), 0) * lerp(maxAnimationOffset, 0, state);\
\
	output.color = input[0].color;\
	output.position = input[0].position;\
	output.position = p + (input[0].position - p) * lerp(startScale, 1, state) + offset;\
	output.position.w = 1;\
	output.position = mul(input[0].mvp, output.position);\
	OutputStream.Append(output);\
\
	output.color = input[1].color;\
	output.position = input[1].position;\
	output.position = p + (input[1].position - p) * lerp(startScale, 1, state) + offset;\
	output.position.w = 1;\
	output.position = mul(input[1].mvp, output.position);\
	OutputStream.Append(output);\
}\
\
float4 PShader(PixelInputType input) : SV_Target\
{\
	return shaderTexture.Sample(SampleType, float2(input.color.x, 0));\
}";

//==============================================================================================================================================
//==============================================================================================================================================
//==============================================================================================================================================

const char* shaderSimple = 
"cbuffer MatrixProjection : register(b0)\
{\
	matrix projectionMatrix;\
}\
\
cbuffer MatrixView : register(b1)\
{\
	matrix viewMatrix;\
}\
\
cbuffer MatrixWorld : register(b2)\
{\
	matrix worldMatrix;\
}\
\
Texture2D shaderTexture;\
SamplerState SampleType;\
\
\
struct VertexInputType\
{\
	float4 position : POSITION;\
	float2 tex : TEXCOORD0;\
};\
\
struct PixelInputType\
{\
	float4 position : SV_POSITION;\
	float2 tex : TEXCOORD0;\
};\
\
PixelInputType VShader(VertexInputType input)\
{\
	PixelInputType output;\
\
	input.position.w = 1.0f;\
\
	matrix mvp = mul(projectionMatrix, mul(viewMatrix, worldMatrix));\
	output.position = mul(mvp, input.position);\
\
	output.tex = input.tex;\
\
	return output;\
}\
\
float4 PShader(PixelInputType input) : SV_Target\
{\
	return shaderTexture.Sample(SampleType, input.tex);\
}";

//==============================================================================================================================================
//==============================================================================================================================================
//==============================================================================================================================================

const char* shaderText =
"cbuffer MatrixProjection : register(b0)\
{\
	matrix projectionMatrix;\
}\
\
cbuffer MatrixView : register(b1)\
{\
	matrix viewMatrix;\
}\
\
cbuffer MatrixWorld : register(b2)\
{\
	matrix worldMatrix;\
}\
\
cbuffer Color : register(b3)\
{\
	float4 color;\
}\
\
Texture2D shaderTexture;\
SamplerState SampleType;\
\
\
struct VertexInputType\
{\
	float4 position : POSITION;\
	float2 tex : TEXCOORD0;\
};\
\
struct PixelInputType\
{\
	float4 position : SV_POSITION;\
	float2 tex : TEXCOORD0;\
};\
\
PixelInputType VShader(VertexInputType input)\
{\
	PixelInputType output;\
\
	input.position.w = 1.0f;\
\
	matrix mvp = mul(projectionMatrix, mul(viewMatrix, worldMatrix));\
	output.position = mul(mvp, input.position);\
\
	output.tex = input.tex;\
\
	return output;\
}\
\
float4 PShader(PixelInputType input) : SV_Target\
{\
	return shaderTexture.Sample(SampleType, input.tex) * color;\
}";

//==============================================================================================================================================
//==============================================================================================================================================
//==============================================================================================================================================

const char* shaderTriangleAnimation =
"cbuffer MatrixProjection : register(b0)\
{\
	matrix projectionMatrix;\
}\
\
cbuffer MatrixView : register(b1)\
{\
	matrix viewMatrix;\
}\
\
cbuffer MatrixWorld : register(b2)\
{\
	matrix worldMatrix;\
}\
\
cbuffer Animation : register(b3)\
{\
	float animationMinY;\
	float animationHeight;\
	float maxAnimationOffset;\
	float startScale;\
}\
\
Texture2D shaderTexture;\
SamplerState SampleType;\
\
\
struct VertexInputType\
{\
	float4 position : POSITION;\
	float3 normal : NORMAL;\
	float4 color : COLOR;\
	float2 tex : TEXCOORD0;\
};\
\
struct GeometryInputType\
{\
	float4 position : POSITION;\
	float3 normal : NORMAL;\
	matrix mvp : TEXCOORD1;\
	float4 color : COLOR;\
};\
\
struct PixelInputType\
{\
	float4 position : SV_POSITION;\
	float4 color : COLOR;\
};\
\
GeometryInputType VShader(VertexInputType input)\
{\
	GeometryInputType output;\
\
	output.color = input.color;\
	output.normal = input.normal;\
\
	output.mvp = mul(projectionMatrix, mul(viewMatrix, worldMatrix));\
	output.position = input.position;\
\
	return output;\
}\
\
[maxvertexcount(3)]\
void GShader(triangle GeometryInputType input[3], inout TriangleStream<PixelInputType> OutputStream)\
{\
	PixelInputType output;\
\
	float4 p = (input[0].position + input[1].position + input[2].position) / 3;\
\
	float state = (p.y - animationMinY) / animationHeight;\
	if (state < 0) state = 0;\
	if (state > 1) state = 1;\
	float4 offset = float4(normalize(input[0].normal + input[1].normal + input[2].normal), 0) * lerp(maxAnimationOffset, 0, state);\
\
	output.color = input[0].color;\
	output.position = input[0].position;\
	output.position = p + (input[0].position - p) * lerp(startScale, 1, state) + offset;\
	output.position.w = 1;\
	output.position = mul(input[0].mvp, output.position);\
	OutputStream.Append(output);\
\
	output.color = input[1].color;\
	output.position = input[1].position;\
	output.position = p + (input[1].position - p) * lerp(startScale, 1, state) + offset;\
	output.position.w = 1;\
	output.position = mul(input[1].mvp, output.position);\
	OutputStream.Append(output);\
\
	output.color = input[2].color;\
	output.position = input[2].position;\
	output.position = p + (input[2].position - p) * lerp(startScale, 1, state) + offset;\
	output.position.w = 1;\
	output.position = mul(input[2].mvp, output.position);\
	OutputStream.Append(output);\
}\
\
float4 PShader(PixelInputType input) : SV_Target\
{\
	return shaderTexture.Sample(SampleType, float2(input.color.x, 0));\
}";

}