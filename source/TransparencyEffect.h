#pragma once
#include "BaseEffect.h"

class TransparencyEffect final : public BaseEffect
{
public:
	// Contructor and Destructor
	explicit TransparencyEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
	virtual ~TransparencyEffect() override;

	// Rule Of Five
	TransparencyEffect(const TransparencyEffect&) = delete;
	TransparencyEffect(TransparencyEffect&&) noexcept = delete;
	TransparencyEffect& operator=(const TransparencyEffect&) = delete;
	TransparencyEffect& operator=(TransparencyEffect&&) noexcept = delete;

	// Public Functions
	ID3DX11EffectTechnique* GetTechnique() const;

	void SetDiffuseMap(ID3D11ShaderResourceView* pShaderResourceView);

private:

	// Technique
	ID3DX11EffectTechnique* m_pTechnique{ nullptr };

	// Shading Maps
	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{ nullptr };
};

