#include "pch.h"
#include "Effect.h"


Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetFile)
	: BaseEffect::BaseEffect(pDevice, assetFile)
{
	// Get GPU Variables
	GetVariables();
}

Effect::~Effect()
{
	DeleteVariables();
}

ID3DX11EffectTechnique* Effect::GetTechnique(dae::FilterMethod filterMethod) const
{
	switch (filterMethod)
	{
	case dae::Point:
		return m_pPointTechnique;
		break;

	case dae::Linear:
		return m_pLinearTechnique;
		break;

	case dae::Anisotropic:
		return m_pAnisotropicTechnique;
		break;
	}

	return {};
}

#pragma region Setters
void Effect::SetWorldMatrix(const dae::Matrix& worldMatrix)
{
	if (m_pMatWorldVariable)
	{
		m_pMatWorldVariable->SetMatrix(reinterpret_cast<const float*>(&worldMatrix));
	}
}
void Effect::SetViewInverseMatrix(const dae::Matrix& viewInverseMatrix)
{
	if (m_pMatViewInverseMatrix)
	{
		m_pMatViewInverseMatrix->SetMatrix(reinterpret_cast<const float*>(&viewInverseMatrix));
	}
}

void Effect::SetDiffuseMap(ID3D11ShaderResourceView* pShaderResourceView)
{
	if (m_pDiffuseMapVariable)
	{
		m_pDiffuseMapVariable->SetResource(pShaderResourceView);
	}
}
void Effect::SetNormalMap(ID3D11ShaderResourceView* pShaderResourceView)
{
	if (m_pNormalMapVariable)
	{
		m_pNormalMapVariable->SetResource(pShaderResourceView);
	}
}
void Effect::SetSpecularMap(ID3D11ShaderResourceView* pShaderResourceView)
{
	if (m_pSpecularMapVariable)
	{
		m_pSpecularMapVariable->SetResource(pShaderResourceView);
	}
}
void Effect::SetGlossinessMap(ID3D11ShaderResourceView* pShaderResourceView)
{
	if (m_pGlossinessMapVariable)
	{
		m_pGlossinessMapVariable->SetResource(pShaderResourceView);
	}
}
#pragma endregion

void Effect::GetVariables()
{
	// Get Techniques
	m_pPointTechnique = m_pEffect->GetTechniqueByName("PointTechnique");
	if (!m_pPointTechnique->IsValid())
	{
		std::wcout << L"Point technique not valid" << '\n';
	}

	m_pLinearTechnique = m_pEffect->GetTechniqueByName("LinearTechnique");
	if (!m_pLinearTechnique->IsValid())
	{
		std::wcout << L"Linear technique not valid" << '\n';
	}

	m_pAnisotropicTechnique = m_pEffect->GetTechniqueByName("AnisotropicTechnique");
	if (!m_pAnisotropicTechnique->IsValid())
	{
		std::wcout << L"Anisotropic technique not valid" << '\n';
	}

	// Get Matrices
	m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
	if (!m_pMatWorldVariable->IsValid())
	{
		std::wcout << L"m_pMatWorldVariable not valid!" << '\n';
	}

	m_pMatViewInverseMatrix = m_pEffect->GetVariableByName("gViewInverseMatrix")->AsMatrix();
	if (!m_pMatViewInverseMatrix->IsValid())
	{
		std::wcout << L"m_pMatViewInverseMatrix not valid!" << '\n';
	}

	// Get Shading Maps
	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gMeshDiffuseMap")->AsShaderResource();
	if (!m_pDiffuseMapVariable->IsValid())
	{
		std::wcout << L"m_pDiffuseMapVariable not valid!" << '\n';
	}

	m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
	if (!m_pNormalMapVariable->IsValid())
	{
		std::wcout << L"m_pNormalMapVariable not valid!" << '\n';
	}

	m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
	if (!m_pSpecularMapVariable->IsValid())
	{
		std::wcout << L"m_pSpecularMapVariable not valid!" << '\n';
	}

	m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
	if (!m_pGlossinessMapVariable->IsValid())
	{
		std::wcout << L"m_pGlossinessMapVariable not valid!" << '\n';
	}
}

void Effect::DeleteVariables()
{
	if (m_pGlossinessMapVariable)
	{
		m_pGlossinessMapVariable->Release();
		m_pGlossinessMapVariable = nullptr;
	}

	if (m_pSpecularMapVariable)
	{
		m_pSpecularMapVariable->Release();
		m_pSpecularMapVariable = nullptr;
	}

	if (m_pNormalMapVariable)
	{
		m_pNormalMapVariable->Release();
		m_pNormalMapVariable = nullptr;
	}

	if (m_pDiffuseMapVariable)
	{
		m_pDiffuseMapVariable->Release();
		m_pDiffuseMapVariable = nullptr;
	}

	if (m_pMatViewInverseMatrix)
	{
		m_pMatViewInverseMatrix->Release();
		m_pMatViewInverseMatrix = nullptr;
	}

	if (m_pMatWorldVariable)
	{
		m_pMatWorldVariable->Release();
		m_pMatWorldVariable = nullptr;
	}

	if (m_pPointTechnique)
	{
		m_pPointTechnique->Release();
		m_pPointTechnique = nullptr;
	}

	if (m_pLinearTechnique)
	{
		m_pLinearTechnique->Release();
		m_pLinearTechnique = nullptr;
	}

	if (m_pAnisotropicTechnique)
	{
		m_pAnisotropicTechnique->Release();
		m_pAnisotropicTechnique = nullptr;
	}
}