#include "pch.h"
#include "MeshRepresentation.h"
#include "Effect.h"
#include "Camera.h"

MeshRepresentation::MeshRepresentation(ID3D11Device* pDevice,
										const std::vector<dae::VS_INPUT>& vertexData, const std::vector<uint32_t>& indexData,
										const std::vector<ID3D11ShaderResourceView*>& pShaderResourceViewVector,
										const dae::Matrix* pWorldMatrix, dae::CullingMode* pCullingMode)
	: m_pWorldMatrix{pWorldMatrix}
	, m_pCurrentCullingMode{ pCullingMode }
{	
	Initialize(pDevice,vertexData,indexData);

	// Set Shading Maps
	m_pEffect->SetDiffuseMap(pShaderResourceViewVector[0]);
	m_pEffect->SetNormalMap(pShaderResourceViewVector[1]);
	m_pEffect->SetSpecularMap(pShaderResourceViewVector[2]);
	m_pEffect->SetGlossinessMap(pShaderResourceViewVector[3]);
}

MeshRepresentation::~MeshRepresentation()
{
	ReleaseResources();
}

void MeshRepresentation::Update(const dae::Timer* pTimer)
{
	UpdateWorldViewProjectionMatrix(pTimer);
}

void MeshRepresentation::Render(ID3D11DeviceContext* pDeviceContext)
{
	// 1. Set Primitive Topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 2. Set Input Layout
	pDeviceContext->IASetInputLayout(m_pInputLayout);

	// 3. Set VertexBuffer
	constexpr UINT stride{ sizeof(dae::VS_INPUT) };
	constexpr UINT offset{ 0 };

	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	// 4. Set IndexBuffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// 5. Draw
	D3DX11_TECHNIQUE_DESC techDesc{};

	// Override rasterizerState
	ID3D11RasterizerState* pDrawRasterizerState{ nullptr };

	switch (*m_pCurrentCullingMode)
	{
	case dae::backFace:
		pDrawRasterizerState = m_pBackFaceRasterizer;
		break;

	case dae::frontFace:
		pDrawRasterizerState = m_pFrontFaceRasterizer;
		break;

	case dae::noCulling:
		pDrawRasterizerState = m_pNoCullingRasterizer;
		break;
	}


	// Toggle between different FilterMethods
	switch (m_CurrentFilterMethod)
	{
	case dae::Point:
		m_pPointTechnique->GetDesc(&techDesc);

		for (UINT p{ 0 }; p < techDesc.Passes; ++p)
		{
			m_pPointTechnique->GetPassByIndex(p)->Apply(0, pDeviceContext);
			pDeviceContext->RSSetState(pDrawRasterizerState);
			pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
		}

		break;
	case dae::Linear:
		m_pLinearTechnique->GetDesc(&techDesc);

		for (UINT p{ 0 }; p < techDesc.Passes; ++p)
		{
			m_pLinearTechnique->GetPassByIndex(p)->Apply(0, pDeviceContext);
			pDeviceContext->RSSetState(pDrawRasterizerState);
			pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
		}

		break;
	case dae::Anisotropic:
		m_pAnisotropicTechnique->GetDesc(&techDesc);

		for (UINT p{ 0 }; p < techDesc.Passes; ++p)
		{
			m_pAnisotropicTechnique->GetPassByIndex(p)->Apply(0, pDeviceContext);
			pDeviceContext->RSSetState(pDrawRasterizerState);
			pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
		}

		break;
	}
}

void MeshRepresentation::ToggleFilterMethods()
{
	switch (m_CurrentFilterMethod)
	{
	case dae::FilterMethod::Point:
		m_CurrentFilterMethod = dae::FilterMethod::Linear;
		std::cout << "Now using linear filter method" << std::endl;
		break;

	case dae::FilterMethod::Linear:
		m_CurrentFilterMethod = dae::FilterMethod::Anisotropic;
		std::cout << "Now using anisotropic filter method" << std::endl;
		break;

	case dae::FilterMethod::Anisotropic:
		m_CurrentFilterMethod = dae::FilterMethod::Point;
		std::cout << "Now using point filter method" << std::endl;
		break;
	}
}

void MeshRepresentation::SetCamera(Camera* pCamera)
{
	m_pCamera = pCamera;
}

void MeshRepresentation::Initialize(ID3D11Device* pDevice, const std::vector<dae::VS_INPUT>& vertexData, const std::vector<uint32_t>& indexData)
{
	// Initializing Variables
	m_pEffect = new Effect(pDevice, L"Resources/PosCol3D.fx");

	m_pPointTechnique = m_pEffect->GetTechnique(dae::FilterMethod::Point);
	m_pLinearTechnique = m_pEffect->GetTechnique(dae::FilterMethod::Linear);
	m_pAnisotropicTechnique = m_pEffect->GetTechnique(dae::FilterMethod::Anisotropic);

	// Create Vertex Layout
	static constexpr uint32_t numElements{ 5 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "COLOR";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 12;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "TEXCOORD";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "NORMAL";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[4].SemanticName = "TANGENT";
	vertexDesc[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[4].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	vertexDesc[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	// Create Input Layout
	D3DX11_PASS_DESC passDesc{};
	m_pPointTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

	HRESULT result
	{	pDevice->CreateInputLayout(
		vertexDesc,
		numElements,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&m_pInputLayout)
	};

	if (FAILED(result))
	{
		return;
	}


	// Create vertex buffer
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(dae::VS_INPUT) * static_cast<uint32_t>(vertexData.size());
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = vertexData.data();

	result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
	if (FAILED(result))
	{
		return;
	}

	// Create index buffer
	m_NumIndices = static_cast<uint32_t>(indexData.size());

	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	initData.pSysMem = indexData.data();

	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
	if (FAILED(result))
	{
		return;
	}


	// Create rasterizerStates
	D3D11_RASTERIZER_DESC currentRasterizer{};

	currentRasterizer.FillMode = D3D11_FILL_SOLID;
	currentRasterizer.FrontCounterClockwise = false;
	currentRasterizer.DepthBias = 0;
	currentRasterizer.SlopeScaledDepthBias = 0.f;
	currentRasterizer.DepthBiasClamp = 0.f;
	currentRasterizer.DepthClipEnable = true;
	currentRasterizer.ScissorEnable = false;
	currentRasterizer.MultisampleEnable = false;
	currentRasterizer.AntialiasedLineEnable = false;

	// backFace
	currentRasterizer.CullMode = D3D11_CULL_BACK;

	result = pDevice->CreateRasterizerState(&currentRasterizer, &m_pBackFaceRasterizer);
	if (FAILED(result))
	{
		return;
	}

	// frontFace
	currentRasterizer.CullMode = D3D11_CULL_FRONT;

	result = pDevice->CreateRasterizerState(&currentRasterizer, &m_pFrontFaceRasterizer);
	if (FAILED(result))
	{
		return;
	}

	// no culling
	currentRasterizer.CullMode = D3D11_CULL_NONE;

	result = pDevice->CreateRasterizerState(&currentRasterizer, &m_pNoCullingRasterizer);
	if (FAILED(result))
	{
		return;
	}
}

void MeshRepresentation::ReleaseResources()
{
	if (m_pInputLayout)
	{
		m_pInputLayout->Release();
		m_pInputLayout = nullptr;
	}

	if (m_pVertexBuffer)
	{
		m_pVertexBuffer->Release();
		m_pVertexBuffer = nullptr;
	}

	if (m_pIndexBuffer)
	{
		m_pIndexBuffer->Release();
		m_pIndexBuffer = nullptr;
	}

	if (m_pBackFaceRasterizer)
	{
		m_pBackFaceRasterizer->Release();
		m_pBackFaceRasterizer = nullptr;
	}

	if (m_pFrontFaceRasterizer)
	{
		m_pFrontFaceRasterizer->Release();
		m_pFrontFaceRasterizer = nullptr;
	}

	if (m_pNoCullingRasterizer)
	{
		m_pNoCullingRasterizer->Release();
		m_pNoCullingRasterizer = nullptr;
	}

	delete m_pEffect;
	m_pEffect = nullptr;
}

void MeshRepresentation::UpdateWorldViewProjectionMatrix(const dae::Timer* pTimer)
{
	m_pEffect->SetWorldMatrix(*m_pWorldMatrix);

	// Set WorldViewProjectionMatrix
	const dae::Matrix viewMatrix{ m_pCamera->GetInvViewMatrix() };
	m_pEffect->SetViewInverseMatrix(viewMatrix);

	const dae::Matrix projectionMatrix{ m_pCamera->GetProjectionMatrix() };
	
	const dae::Matrix worldViewProjectionMatrix{ *m_pWorldMatrix * viewMatrix * projectionMatrix };
	m_pEffect->SetWorldViewProjectionMatrix(worldViewProjectionMatrix);
}