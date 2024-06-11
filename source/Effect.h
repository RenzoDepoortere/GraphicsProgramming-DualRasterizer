#pragma once
#include "BaseEffect.h"

class Effect final : public BaseEffect
{
public:

	// Contructor and Destructor
	explicit Effect(ID3D11Device* pDevice, const std::wstring& assetFile);
	virtual ~Effect() override;

	// Rule Of Five
	Effect(const Effect&) = delete;
	Effect(Effect&&) noexcept = delete;
	Effect& operator=(const Effect&) = delete;
	Effect& operator=(Effect&&) noexcept = delete;

	// Public Functions
	ID3DX11EffectTechnique* GetTechnique(dae::FilterMethod filterMethod) const;

	void SetWorldMatrix(const dae::Matrix& worldMatrix);
	void SetViewInverseMatrix(const dae::Matrix& viewInverseMatrix);
	
	void SetDiffuseMap(ID3D11ShaderResourceView* pShaderResourceView);
	void SetNormalMap(ID3D11ShaderResourceView* pShaderResourceView);
	void SetSpecularMap(ID3D11ShaderResourceView* pShaderResourceView);
	void SetGlossinessMap(ID3D11ShaderResourceView* pShaderResourceView);

private:

	// Techniques
	ID3DX11EffectTechnique* m_pPointTechnique{ nullptr };
	ID3DX11EffectTechnique* m_pLinearTechnique{ nullptr };
	ID3DX11EffectTechnique* m_pAnisotropicTechnique{ nullptr };

	// Matrices
	ID3DX11EffectMatrixVariable* m_pMatWorldVariable{ nullptr };
	ID3DX11EffectMatrixVariable* m_pMatViewInverseMatrix{ nullptr };

	// Shading Maps
	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{ nullptr };
	ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable{ nullptr };
	ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable{ nullptr };
	ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable{ nullptr };

	// HELPER
	// ======
	void GetVariables();
	void DeleteVariables();
};

