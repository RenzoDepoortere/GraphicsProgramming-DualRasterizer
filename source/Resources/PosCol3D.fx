// --------
// Matrices
// --------

float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorldMatrix : WORLD;
float4x4 gViewInverseMatrix : VIEWINVERSE;

// ------------
// Shading Maps
// ------------

Texture2D gMeshDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;

Texture2D gTransparentDiffuseMap : DiffuseMap;

// ---------------
// RasterizerState
// ---------------

RasterizerState gTransparencyRasterizerState
{
	Cullmode = none;
	FrontCounterClockwise = false; // default
};


RasterizerState gMeshRasterizerState
{
	Cullmode = back;
	FrontCounterClockwise = false; // default
};

// ----------
// BlendState
// ----------

BlendState gTransparencyBlendState
{
	BlendEnable[0] = true;
	SrcBlend = src_alpha;
	DestBlend = inv_src_alpha;
	BlendOp = add;
	SrcBlendAlpha = zero;
	DestBlendAlpha = zero;
	BlendOpAlpha = add;
	RenderTargetWriteMask[0] = 0x0F;
};

BlendState gMeshBlendState
{
	BlendEnable[0] = false;
};

// -----------------
// DepthStencilState
// -----------------

DepthStencilState gTransparencyDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = zero;
	DepthFunc = less;
	StencilEnable = false;

	// others are redundant because StencilEnable is false (for demo purposes only)
	StencilReadMask = 0x0F;
	StencilWriteMask = 0x0F;

	FrontFaceStencilFunc = always;
	BackFaceStencilFunc = always;

	FrontFaceStencilDepthFail = keep;
	BackFaceStencilDepthFail = keep;

	FrontFaceStencilPass = keep;
	BackFaceStencilPass = keep;

	FrontFaceStencilFail = keep;
	BackFaceStencilFail = keep;
};

DepthStencilState gMeshDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = all;
	DepthFunc = less;
	StencilEnable = false;

	// others are redundant because StencilEnable is false (for demo purposes only)
	StencilReadMask = 0x0F;
	StencilWriteMask = 0x0F;

	FrontFaceStencilFunc = always;
	BackFaceStencilFunc = always;

	FrontFaceStencilDepthFail = keep;
	BackFaceStencilDepthFail = keep;

	FrontFaceStencilPass = keep;
	BackFaceStencilPass = keep;

	FrontFaceStencilFail = keep;
	BackFaceStencilFail = keep;
};

// -----------------------
// Other Shading Variables
// -----------------------

float3 gLightDirection = float3(0.577f, -0.577f, 0.577f);
float3 gLightColor = float3(1.f, 1.f, 1.f);
float gLightIntensity = 7.f;

float gShininess = 25.f;
float3 gAmbient = float3(0.025f, 0.025f, 0.025f);

float gPI = 3.14159265359f;

// --------------------
// Input/Output Structs
// --------------------

struct VS_INPUT
{
	float3 Position : POSITION;
	float3 Color : COLOR;
	float2 TextureUV : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float4 WorldPosition : WORLD_POSITION;
	float3 Color : COLOR;
	float2 TextureUV : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
};


struct VS_SIMPLE_INPUT
{
	float3 Position : POSITION;
	float3 Color : COLOR;
	float2 TextureUV : TEXCOORD;
};

struct VS_SIMPLE_OUTPUT
{
	float4 Position : SV_POSITION;
	float3 Color : COLOR;
	float2 TextureUV : TEXCOORD;
};

// -------------
// SamplerStates
// -------------

SamplerState sampPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap; // or Mirror, Clamp, Border
	AddressV = Wrap; // or Mirror, Clamp, Border
};

SamplerState sampLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap; // or Mirror, Clamp, Border
	AddressV = Wrap; // or Mirror, Clamp, Border
};

SamplerState sampAnisotropic
{
	Filter = ANISOTROPIC;
	AddressU = Wrap; // or Mirror, Clamp, Border
	AddressV = Wrap; // or Mirror, Clamp, Border
};

// -------------
// Vertex Shader
// -------------

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
	output.WorldPosition = mul(float4(input.Position, 1.f), gWorldMatrix);
	output.Color = input.Color;
	output.TextureUV = input.TextureUV;
	output.Normal = mul(normalize(input.Normal), (float3x3) gWorldMatrix);
	output.Tangent = mul(normalize(input.Tangent), (float3x3) gWorldMatrix);

	return output;
}

VS_OUTPUT VS_SIMPLE(VS_SIMPLE_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
	output.Color = input.Color;
	output.TextureUV = input.TextureUV;

	return output;
}

// -----------------
// Shading Functions
// -----------------

float3 calculateDiffuseColor(float lightIntensity, float3 sampledDiffuse)
{
	return (sampledDiffuse * lightIntensity) / gPI;
}

float calculateObservedArea(float3 normal, float3 lightDirection)
{
	float observedArea = dot(normal, -lightDirection);

	if (observedArea < 0)
	{
		return 0;
	}

	return observedArea;
}

float3 calculatePhong(float ks, float exp, float3 lightDirection, float3 viewDirection, float3 normal)
{
	float3 reflect = normalize(lightDirection - 2 * dot(normal, lightDirection) * normal);

	float viewReflectAngle = dot(reflect, viewDirection);
	if (viewReflectAngle < 0)
	{
		return 0;
	}

	float specularReflection = ks * pow(viewReflectAngle, exp);
	return float3(1,1,1) * specularReflection;
}

float4 useNormalMap(float4 sampledNormal, float3 inputNormal, float3 inputTangent)
{
	float3 convertedNormal = (2.f * sampledNormal.xyz) - float3(1.f, 1.f, 1.f);

	float3 binormal = cross(inputNormal, inputTangent);

	float4x4 tangentSpaceAxis;
	tangentSpaceAxis[0] = float4(inputTangent, 0);
	tangentSpaceAxis[1] = float4(binormal, 0);
	tangentSpaceAxis[2] = float4(inputNormal, 0);
	tangentSpaceAxis[3] = float4(0, 0, 0, 0);

	return mul(float4(convertedNormal, 1), tangentSpaceAxis);
}

// -------------
// Pixel Shaders
// -------------

float4 PS_Point(VS_OUTPUT input) : SV_TARGET
{
	// viewDirection
	float3 viewDirection = normalize(input.WorldPosition.xyz - gViewInverseMatrix[3].xyz);

	// Sampled Textures
	float4 diffuseColor = gMeshDiffuseMap.Sample(sampPoint, input.TextureUV);
	float4 specularColor = gSpecularMap.Sample(sampPoint, input.TextureUV);
	float4 glossinessColor = gGlossinessMap.Sample(sampPoint, input.TextureUV);
	float4 normalVector = gNormalMap.Sample(sampPoint, input.TextureUV);

	// Normal map
	float4 desiredNormal = useNormalMap(normalVector, input.Normal, input.Tangent);

	// Shading
	float3 desiredDiffuse = calculateDiffuseColor(gLightIntensity, diffuseColor);
	
	float observedArea = calculateObservedArea(desiredNormal.xyz, gLightDirection);
	
	float3 phong = calculatePhong(specularColor.x, glossinessColor.x * gShininess, gLightDirection, viewDirection, desiredNormal.xyz);
	float maxUnit = max(phong.x, max(phong.y, phong.z));
	if (maxUnit > 1.f)
	{
		phong /= maxUnit;
	}

	// Return
	return (float4(desiredDiffuse, 1) + float4(phong, 1) + float4(gAmbient, 1)) * observedArea;
}

float4 PS_Linear(VS_OUTPUT input) : SV_TARGET
{
	// viewDirection
	float3 viewDirection = normalize(input.WorldPosition.xyz - gViewInverseMatrix[3].xyz);

	// Sampled Textures
	float4 diffuseColor = gMeshDiffuseMap.Sample(sampLinear, input.TextureUV);
	float4 specularColor = gSpecularMap.Sample(sampLinear, input.TextureUV);
	float4 glossinessColor = gGlossinessMap.Sample(sampLinear, input.TextureUV);
	float4 normalVector = gNormalMap.Sample(sampLinear, input.TextureUV);

	// Normal map
	float4 desiredNormal = useNormalMap(normalVector, input.Normal, input.Tangent);

	// Shading
	float3 desiredDiffuse = calculateDiffuseColor(gLightIntensity, diffuseColor);
	float observedArea = calculateObservedArea(desiredNormal.xyz, gLightDirection);
	float3 phong = calculatePhong(specularColor.x, glossinessColor.x * gShininess, gLightDirection, viewDirection, desiredNormal.xyz);
	float maxUnit = max(phong.x, max(phong.y, phong.z));
	if (maxUnit > 1.f)
	{
		phong /= maxUnit;
	}

	// Return
	return (float4(desiredDiffuse, 1) + float4(phong, 1) + float4(gAmbient, 1)) * observedArea;
}

float4 PS_Anisotropic(VS_OUTPUT input) : SV_TARGET
{
	// viewDirection
	float3 viewDirection = normalize(input.WorldPosition.xyz - gViewInverseMatrix[3].xyz);

	// Sampled Textures
	float4 diffuseColor = gMeshDiffuseMap.Sample(sampAnisotropic, input.TextureUV);
	float4 specularColor = gSpecularMap.Sample(sampAnisotropic, input.TextureUV);
	float4 glossinessColor = gGlossinessMap.Sample(sampAnisotropic, input.TextureUV);
	float4 normalVector = gNormalMap.Sample(sampAnisotropic, input.TextureUV);

	// Normal map
	float4 desiredNormal = useNormalMap(normalVector, input.Normal, input.Tangent);

	// Shading
	float3 desiredDiffuse = calculateDiffuseColor(gLightIntensity, diffuseColor);
	float observedArea = calculateObservedArea(desiredNormal.xyz, gLightDirection);
	float3 phong = calculatePhong(specularColor.x, glossinessColor.x * gShininess, gLightDirection, viewDirection, desiredNormal.xyz);
	float maxUnit = max(phong.x, max(phong.y, phong.z));
	if (maxUnit > 1.f)
	{
		phong /= maxUnit;
	}

	// Return
	return (float4(desiredDiffuse, 1) + float4(phong, 1) + float4(gAmbient, 1)) * observedArea;
}

float4 PS_Transparent(VS_OUTPUT input) : SV_TARGET
{
	return gTransparentDiffuseMap.Sample(sampPoint, input.TextureUV);
}

// ----------
// Techniques
// ----------

technique11 PointTechnique
{
	pass P0
	{
		SetRasterizerState(gMeshRasterizerState);
		SetDepthStencilState(gMeshDepthStencilState, 0);
		SetBlendState(gMeshBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_5_0, PS_Point()) );
	}
}

technique11 LinearTechnique
{
	pass P0
	{
		SetRasterizerState(gMeshRasterizerState);
		SetDepthStencilState(gMeshDepthStencilState, 0);
		SetBlendState(gMeshBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Linear()));
	}
}

technique11 AnisotropicTechnique
{
	pass P0
	{
		SetRasterizerState(gMeshRasterizerState);
		SetDepthStencilState(gMeshDepthStencilState, 0);
		SetBlendState(gMeshBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Anisotropic()));
	}
}

technique11 TransparentTechnique
{
	pass P0
	{
		SetRasterizerState(gTransparencyRasterizerState);
		SetDepthStencilState(gTransparencyDepthStencilState, 0);
		SetBlendState(gTransparencyBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS_SIMPLE()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Transparent()));
	}
}