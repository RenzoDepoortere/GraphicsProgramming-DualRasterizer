#include "pch.h"
#include "TransparencyEffect.h"

TransparencyEffect::TransparencyEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
	: BaseEffect::BaseEffect(pDevice,assetFile)
{
	// Get Technique
	m_pTechnique = m_pEffect->GetTechniqueByName("TransparentTechnique");
	if (!m_pTechnique->IsValid())
	{
		std::wcout << L"Transparent technique not valid" << '\n';
	}

	// Get DiffuseMap
	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gTransparentDiffuseMap")->AsShaderResource();
	if (!m_pDiffuseMapVariable->IsValid())
	{
		std::wcout << L"m_pDiffuseMapVariable not valid!" << '\n';
	}
}

TransparencyEffect::~TransparencyEffect()
{
	if (m_pDiffuseMapVariable)
	{
		m_pDiffuseMapVariable->Release();
		m_pDiffuseMapVariable = nullptr;
	}

	if (m_pTechnique)
	{
		m_pTechnique->Release();
		m_pTechnique = nullptr;
	}
}

ID3DX11EffectTechnique* TransparencyEffect::GetTechnique() const
{
	return m_pTechnique;
}

void TransparencyEffect::SetDiffuseMap(ID3D11ShaderResourceView* pShaderResourceView)
{
	if (m_pDiffuseMapVariable)
	{
		m_pDiffuseMapVariable->SetResource(pShaderResourceView);
	}
}