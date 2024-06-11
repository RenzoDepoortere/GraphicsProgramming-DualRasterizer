#pragma once
#include <SDL_surface.h>
#include <string>
#include "ColorRGB.h"

class Texture final
{
public:
	// Constructor and Destructor
	explicit Texture(ID3D11Device* pDevice, const char* fileName);
	~Texture();

	// Rule Of Five
	Texture(const Texture&) = delete;
	Texture(Texture&&) noexcept = delete;
	Texture& operator=(const Texture&) = delete;
	Texture& operator=(Texture&&) noexcept = delete;

	// Public
	ID3D11ShaderResourceView* GetShaderResourceView() const;
	dae::ColorRGB Sample(const dae::Vector2& uv) const;

private:
	ID3D11Resource* m_pTexture{ nullptr };
	ID3D11ShaderResourceView* m_pShaderResourceView{ nullptr };

	SDL_Surface* m_pSurface{ nullptr };
	uint32_t* m_pSurfacePixels{ nullptr };

	// HELPER
	void LoadTexture(ID3D11Device* pDevice, const char* fileName);
	void ReleaseResource();
};

