#pragma once
#include "Matrix.h"
#include "DataTypes.h"

class BaseEffect
{
public:
	// Contructor and Destructor
	BaseEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
	virtual ~BaseEffect();

	// Rule Of Five
	BaseEffect(const BaseEffect&) = delete;
	BaseEffect(BaseEffect&&) noexcept = delete;
	BaseEffect& operator=(const BaseEffect&) = delete;
	BaseEffect& operator=(BaseEffect&&) noexcept = delete;

	// Public Functions
	ID3DX11Effect* GetEffect() const;

	void SetWorldViewProjectionMatrix(const dae::Matrix& worldViewProjectionMatrix);

protected:

	ID3DX11Effect* m_pEffect{ nullptr };
	ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable{ nullptr };

private:
	// Helper
	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
};

