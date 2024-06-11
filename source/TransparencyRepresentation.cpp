#include "pch.h"
#include "TransparencyRepresentation.h"
#include "TransparencyEffect.h"
#include "Camera.h"

TransparencyRepresentation::TransparencyRepresentation(ID3D11Device* pDevice,
														const std::vector<dae::VS_SIMPLE_INPUT>& vertexData, const std::vector<uint32_t>& indexData,
														const std::vector<ID3D11ShaderResourceView*>& pShaderResourceViewVector,
														const dae::Matrix* pWorldMatrix)
	: m_pWorldMatrix{ pWorldMatrix }
{
	Initialize(pDevice, vertexData, indexData);

	// Set Shading Maps
	m_pEffect->SetDiffuseMap(pShaderResourceViewVector[0]);
}

TransparencyRepresentation::~TransparencyRepresentation()
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

	delete m_pEffect;
	m_pEffect = nullptr;
}

void TransparencyRepresentation::Update(const dae::Timer* pTimer)
{
	m_pCamera->Update(pTimer);
	UpdateWorldViewProjectionMatrix(pTimer);
}

void TransparencyRepresentation::Render(ID3D11DeviceContext* pDeviceContext)
{
	// 1. Set Primitive Topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 2. Set Input Layout
	pDeviceContext->IASetInputLayout(m_pInputLayout);

	// 3. Set VertexBuffer
	constexpr UINT stride{ sizeof(dae::VS_SIMPLE_INPUT) };
	constexpr UINT offset{ 0 };

	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	// 4. Set IndexBuffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// 5. Draw
	D3DX11_TECHNIQUE_DESC techDesc{};
	m_pTechnique->GetDesc(&techDesc);

	for (UINT p{ 0 }; p < techDesc.Passes; ++p)
	{
		m_pTechnique->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
	}
}

void TransparencyRepresentation::SetCamera(Camera* pCamera)
{
	m_pCamera = pCamera;
}

void TransparencyRepresentation::Initialize(ID3D11Device* pDevice, const std::vector<dae::VS_SIMPLE_INPUT>& vertexData, const std::vector<uint32_t>& indexData)
{
	// Initializing Variables
	m_pEffect = new TransparencyEffect(pDevice, L"Resources/PosCol3D.fx");
	m_pTechnique = m_pEffect->GetTechnique();

	// Create Vertex Layout
	static constexpr uint32_t numElements{ 3 };
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

	// Create Input Layout
	D3DX11_PASS_DESC passDesc{};
	m_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

	HRESULT result
	{ pDevice->CreateInputLayout(
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
	bd.ByteWidth = sizeof(dae::VS_SIMPLE_INPUT) * static_cast<uint32_t>(vertexData.size());
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
}

void TransparencyRepresentation::UpdateWorldViewProjectionMatrix(const dae::Timer* pTimer)
{
	const dae::Matrix viewMatrix{ m_pCamera->GetInvViewMatrix() };
	const dae::Matrix projectionMatrix{ m_pCamera->GetProjectionMatrix() };

	const dae::Matrix worldViewProjectionMatrix{ *m_pWorldMatrix * viewMatrix * projectionMatrix };
	m_pEffect->SetWorldViewProjectionMatrix(worldViewProjectionMatrix);
}