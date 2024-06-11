#include "pch.h"
#include "BaseEffect.h"

BaseEffect::BaseEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	// Load Effect
	m_pEffect = LoadEffect(pDevice, assetFile);

	// Get Matrix
	m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
	if (!m_pMatWorldViewProjVariable->IsValid())
	{
		std::wcout << L"m_pMatWorldViewProjVariable not valid!" << '\n';
	}
}

BaseEffect::~BaseEffect()
{
	if (m_pMatWorldViewProjVariable)
	{
		m_pMatWorldViewProjVariable->Release();
		m_pMatWorldViewProjVariable = nullptr;
	}

	if (m_pEffect)
	{
		m_pEffect->Release();
		m_pEffect = nullptr;
	}
}

ID3DX11Effect* BaseEffect::GetEffect() const
{
	return m_pEffect;
}

void BaseEffect::SetWorldViewProjectionMatrix(const dae::Matrix& worldViewProjectionMatrix)
{
	if (m_pMatWorldViewProjVariable)
	{
		m_pMatWorldViewProjVariable->SetMatrix(reinterpret_cast<const float*>(&worldViewProjectionMatrix));
	}
}

ID3DX11Effect* BaseEffect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	HRESULT result;
	ID3D10Blob* pErrorBlob{ nullptr };
	ID3DX11Effect* pEffect;

	DWORD shaderFlags{ 0 };
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	result = D3DX11CompileEffectFromFile(assetFile.c_str(),
		nullptr,
		nullptr,
		shaderFlags,
		0,
		pDevice,
		&pEffect,
		&pErrorBlob);

	if (FAILED(result))
	{
		if (pErrorBlob != nullptr)
		{
			const char* pErrors = static_cast<char*>(pErrorBlob->GetBufferPointer());

			std::wstringstream ss;
			for (unsigned int i{ 0 }; i < pErrorBlob->GetBufferSize(); i++)
			{
				ss << pErrors[i];
			}

			OutputDebugStringW(ss.str().c_str());
			pErrorBlob->Release();
			pErrorBlob = nullptr;

			std::wcout << ss.str() << '\n';
		}
		else
		{
			std::wstringstream ss;
			ss << "EffectLoader: Failed to CreateEffectFromFile! \nPath: " << assetFile;
			std::wcout << ss.str() << '\n';

			return nullptr;
		}
	}

	return pEffect;
}
