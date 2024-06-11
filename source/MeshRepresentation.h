#pragma once
#include "DataTypes.h"

class Effect;
class Camera;

class MeshRepresentation final
{
public:
	// Contructor and Destructor
	explicit MeshRepresentation(ID3D11Device* pDevice,
								const std::vector<dae::VS_INPUT>& vertexData, const std::vector<uint32_t>& indexData,
								const std::vector<ID3D11ShaderResourceView*>& pShaderResourceViewVector,
								const dae::Matrix* pWorldMatrix, dae::CullingMode* pCullingMode);
	~MeshRepresentation();

	// Rule Of Five
	MeshRepresentation(const MeshRepresentation&) = delete;
	MeshRepresentation(MeshRepresentation&&) noexcept = delete;
	MeshRepresentation& operator=(const MeshRepresentation&) = delete;
	MeshRepresentation& operator=(MeshRepresentation&&) noexcept = delete;

	// Public Functions
	void Update(const dae::Timer* pTimer);
	void Render(ID3D11DeviceContext* pDeviceContext);

	void ToggleFilterMethods();

	void SetCamera(Camera* pCamera);

private:

	// DIRECTX
	Effect* m_pEffect{ nullptr };
	ID3DX11EffectTechnique* m_pPointTechnique{ nullptr };
	ID3DX11EffectTechnique* m_pLinearTechnique{ nullptr };
	ID3DX11EffectTechnique* m_pAnisotropicTechnique{ nullptr };

	ID3D11InputLayout* m_pInputLayout{ nullptr };
	ID3D11Buffer* m_pVertexBuffer{ nullptr };
	ID3D11Buffer* m_pIndexBuffer{ nullptr };

	uint32_t m_NumIndices{};

	ID3D11RasterizerState* m_pBackFaceRasterizer{ nullptr };
	ID3D11RasterizerState* m_pFrontFaceRasterizer{ nullptr };
	ID3D11RasterizerState* m_pNoCullingRasterizer{ nullptr };

	// OTHER
	Camera* m_pCamera{ nullptr };

	const dae::Matrix* m_pWorldMatrix{ nullptr };
	dae::CullingMode* m_pCurrentCullingMode{ nullptr };
	
	dae::FilterMethod m_CurrentFilterMethod{ dae::FilterMethod::Point };
	float m_AccumulatedTime{};

	// HELPER
	// ======

	void Initialize(ID3D11Device* pDevice, const std::vector<dae::VS_INPUT>& vertexData, const std::vector<uint32_t>& indexData);
	void ReleaseResources();

	void UpdateWorldViewProjectionMatrix(const dae::Timer* pTimer);
};

