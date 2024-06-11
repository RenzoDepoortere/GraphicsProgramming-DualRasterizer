#pragma once
#include "Utils.h"
#include "DataTypes.h"

class MeshRepresentation;
class TransparencyRepresentation;
class Camera;
class Texture;

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class DirectXRenderer final
	{
	public:
		DirectXRenderer(SDL_Window* pWindow, int windowWidth, int windowHeight, ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext,
						const std::vector<VS_INPUT>& vertices, const std::vector<uint32_t>& indices,
						Texture* pDiffuseTexture, Texture* pNormalTexture, Texture* pSpecularTexture, Texture* pGlossinessTexture,
						Camera* pCamera, const Matrix* pWorldMatrix, 
						bool* pUseUniformColor, CullingMode* pCurrentCullingMode);
		~DirectXRenderer();

		DirectXRenderer(const DirectXRenderer&) = delete;
		DirectXRenderer(DirectXRenderer&&) noexcept = delete;
		DirectXRenderer& operator=(const DirectXRenderer&) = delete;
		DirectXRenderer& operator=(DirectXRenderer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render() const;

		void ToggleFireFX();
		void ToggleFilterMethods();

	private:
		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		bool m_IsInitialized{ false };

		//DIRECTX
		HRESULT InitializeDirectX();
		
		ID3D11Device* m_pDevice{ nullptr };
		ID3D11DeviceContext* m_pDeviceContext{ nullptr };
		
		IDXGISwapChain* m_pSwapChain{ nullptr };
		
		ID3D11Texture2D* m_pDepthStencilBuffer{ nullptr };
		ID3D11DepthStencilView* m_pDepthStencilView{ nullptr };
		
		ID3D11Resource* m_pRenderTargetBuffer{ nullptr };
		ID3D11RenderTargetView* m_pRenderTargetView{ nullptr };

		//OTHER
		MeshRepresentation* m_pMeshRepresentation{ nullptr };
		TransparencyRepresentation* m_pTransparencyRepresentation{ nullptr };

		Camera* m_pCamera{ nullptr };

		Texture* m_pMeshDiffuseTexture{ nullptr };
		Texture* m_pNormalTexture{ nullptr };
		Texture* m_pSpecularTexture{ nullptr };
		Texture* m_pGlossinessTexture{ nullptr };

		Texture* m_pTransparencyDiffuseTexture{ nullptr };

		bool* m_pUseUniformBGColor{ nullptr };
		bool m_UseFireFX{ true };

		// HELPER
		// ======

		void ReleaseResources();
	};
}
