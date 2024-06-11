#pragma once
#include "DataTypes.h"

class TransparencyEffect;
class Camera;

class TransparencyRepresentation
{
public:
	// Contructor and Destructor
	explicit TransparencyRepresentation(ID3D11Device* pDevice,
										const std::vector<dae::VS_SIMPLE_INPUT>& vertexData, const std::vector<uint32_t>& indexData,
										const std::vector<ID3D11ShaderResourceView*>& pShaderResourceViewVector,
										const dae::Matrix* pWorldMatrix);
	~TransparencyRepresentation();

	// Rule Of Five
	TransparencyRepresentation(const TransparencyRepresentation&) = delete;
	TransparencyRepresentation(TransparencyRepresentation&&) noexcept = delete;
	TransparencyRepresentation& operator=(const TransparencyRepresentation&) = delete;
	TransparencyRepresentation& operator=(TransparencyRepresentation&&) noexcept = delete;

	// Public Functions
	void Update(const dae::Timer* pTimer);
	void Render(ID3D11DeviceContext* pDeviceContext);

	void SetCamera(Camera* pCamera);


private:

	// DIRECTX
	TransparencyEffect* m_pEffect{ nullptr };
	ID3DX11EffectTechnique* m_pTechnique{ nullptr };

	ID3D11InputLayout* m_pInputLayout{ nullptr };
	ID3D11Buffer* m_pVertexBuffer{ nullptr };
	ID3D11Buffer* m_pIndexBuffer{ nullptr };

	uint32_t m_NumIndices{};

	// OTHER
	Camera* m_pCamera;
	float m_AccumulatedTime{};

	const dae::Matrix* m_pWorldMatrix{ nullptr };

	// Helper
	void Initialize(ID3D11Device* pDevice, const std::vector<dae::VS_SIMPLE_INPUT>& vertexData, const std::vector<uint32_t>& indexData);

	void UpdateWorldViewProjectionMatrix(const dae::Timer* pTimer);
};

