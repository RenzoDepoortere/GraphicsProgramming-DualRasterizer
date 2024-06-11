#include "pch.h"
#include "Texture.h"

Texture::Texture(ID3D11Device* pDevice, const char* fileName)
{
	LoadTexture(pDevice, fileName);
}

Texture::~Texture()
{
	ReleaseResource();
}

ID3D11ShaderResourceView* Texture::GetShaderResourceView() const
{
	return m_pShaderResourceView;
}

dae::ColorRGB Texture::Sample(const dae::Vector2& uv) const
{
	//TODO
	//Sample the correct texel for the given uv
	const int textureWidth{ m_pSurface->w };
	const int textureHeight{ m_pSurface->h };

	const dae::Vector2 convertedUV{ (float)int(uv.x * textureWidth),(float)int(uv.y * textureHeight) };

	const size_t surfacePixelIndex{ static_cast<size_t>(convertedUV.y * textureWidth + convertedUV.x) };
	const auto desiredPixel{ m_pSurfacePixels[surfacePixelIndex] };

	Uint8 redValue{};
	Uint8 blueValue{};
	Uint8 greenValue{};

	SDL_GetRGB(desiredPixel, m_pSurface->format, &redValue, &blueValue, &greenValue);

	dae::ColorRGB desiredColor{ static_cast<float>(redValue), static_cast<float>(blueValue), static_cast<float>(greenValue) };
	desiredColor.r /= 255.f;
	desiredColor.g /= 255.f;
	desiredColor.b /= 255.f;

	return desiredColor;
}

void Texture::LoadTexture(ID3D11Device* pDevice, const char* fileName)
{
	// Load File
	SDL_Surface* pSurface{ IMG_Load(fileName) };
	if (pSurface == NULL)
	{
		std::cout << "Failed to Load File" << '\n';
		return;
	}

	m_pSurface = pSurface;
	m_pSurfacePixels = (uint32_t*)pSurface->pixels;

	// Create Texture
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = pSurface->w;
	desc.Height = pSurface->h;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = pSurface->pixels;
	initData.SysMemPitch = static_cast<UINT>(pSurface->pitch);
	initData.SysMemSlicePitch = static_cast<UINT>(pSurface->h * pSurface->pitch);

	HRESULT result = pDevice->CreateTexture2D(&desc, &initData, reinterpret_cast<ID3D11Texture2D**>(&m_pTexture));

	if (FAILED(result))
	{
		std::cout << "Failed to Create Texture2D" << '\n';
		return;
	}

	// Create ResourceView
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	shaderResourceViewDesc.Format = format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	result = pDevice->CreateShaderResourceView(m_pTexture, &shaderResourceViewDesc, &m_pShaderResourceView);

	if (FAILED(result))
	{
		std::cout << "Failed to Create ShaderResourceView" << '\n';
		return;
	}
}

void Texture::ReleaseResource()
{
	if (m_pShaderResourceView)
	{
		m_pShaderResourceView->Release();
		m_pShaderResourceView = nullptr;
	}

	if (m_pTexture)
	{
		m_pTexture->Release();
		m_pTexture = nullptr;
	}

	if (m_pSurface)
	{
		SDL_FreeSurface(m_pSurface);
		m_pSurface = nullptr;
	}
}