#include "pch.h"
#include "DirectXRenderer.h"
#include "MeshRepresentation.h"
#include "TransparencyRepresentation.h"
#include "Camera.h"
#include "Texture.h"

namespace dae {

	DirectXRenderer::DirectXRenderer(SDL_Window* pWindow, int windowWidth, int windowHeight, ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext,
		const std::vector<VS_INPUT>& vertices, const std::vector<uint32_t>& indices,
		Texture* pDiffuseTexture, Texture* pNormalTexture, Texture* pSpecularTexture, Texture* pGlossinessTexture,
		Camera* pCamera, const Matrix* pWorldMatrix,
		bool* pUseUniformColor, CullingMode* pCurrentCullingMode)
		// Window
		: m_pWindow(pWindow)
		, m_Width{windowWidth}
		, m_Height{windowHeight}
		// Devices
		, m_pDevice{pDevice}
		, m_pDeviceContext{pDeviceContext}
		// Textures
		, m_pMeshDiffuseTexture{pDiffuseTexture}
		, m_pNormalTexture{pNormalTexture}
		, m_pSpecularTexture{pSpecularTexture}
		, m_pGlossinessTexture{pGlossinessTexture}
		// Camera
		, m_pCamera{pCamera}
		// Togglers
		, m_pUseUniformBGColor{ pUseUniformColor }
	{
		//Initialize DirectX pipeline
		const HRESULT result = InitializeDirectX();
		if (result == S_OK)
		{
			m_IsInitialized = true;
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}

		// Create MeshRepresentation
#pragma region MeshRepresentation

#pragma region Mesh Data
		// Create Textures
		std::vector<ID3D11ShaderResourceView*> pShaderResourceViewVector{};

		// Diffuse
		pShaderResourceViewVector.push_back(m_pMeshDiffuseTexture->GetShaderResourceView());

		// Normal
		pShaderResourceViewVector.push_back(m_pNormalTexture->GetShaderResourceView());

		// Specular
		pShaderResourceViewVector.push_back(m_pSpecularTexture->GetShaderResourceView());

		// Glossiness
		pShaderResourceViewVector.push_back(m_pGlossinessTexture->GetShaderResourceView());

#pragma endregion

		// Create MeshRepresentation
		m_pMeshRepresentation = new MeshRepresentation(m_pDevice, vertices, indices, pShaderResourceViewVector, pWorldMatrix, pCurrentCullingMode);
		m_pMeshRepresentation->SetCamera(m_pCamera);

#pragma endregion

		// Create TransparencyRepresentation
#pragma region TransparencyRepresentation

		// Create Data
		std::vector<VS_INPUT> fireVertices{};
		std::vector<uint32_t> fireIndices{};

		// Create Textures
		Utils::ParseOBJ("Resources/fireFX.obj", fireVertices, fireIndices);

		pShaderResourceViewVector.clear();

		// Diffuse
		std::string texturePath = "Resources/fireFX_diffuse.png";
		m_pTransparencyDiffuseTexture = new Texture(m_pDevice, texturePath.c_str());
		pShaderResourceViewVector.push_back(m_pTransparencyDiffuseTexture->GetShaderResourceView());

		// Only use Position, Color and UV
		std::vector<dae::VS_SIMPLE_INPUT> simpleVertices{};
		for (const auto& currentVertex : fireVertices)
		{
			VS_SIMPLE_INPUT newSimpleVertex{};
			newSimpleVertex.Position = currentVertex.Position;
			newSimpleVertex.Color = currentVertex.Color;
			newSimpleVertex.UV = currentVertex.UV;

			simpleVertices.push_back(newSimpleVertex);
		}

		// Create TransparencyRepresentation
		m_pTransparencyRepresentation = new TransparencyRepresentation(m_pDevice, simpleVertices, fireIndices, pShaderResourceViewVector, pWorldMatrix);
		m_pTransparencyRepresentation->SetCamera(m_pCamera);

#pragma endregion



	}

	DirectXRenderer::~DirectXRenderer()
	{
		delete m_pTransparencyDiffuseTexture;
		m_pTransparencyDiffuseTexture = nullptr;

		if (m_pTransparencyRepresentation)
		{
			delete m_pTransparencyRepresentation;
			m_pTransparencyRepresentation;
		}

		if (m_pMeshRepresentation)
		{
			delete m_pMeshRepresentation;
			m_pMeshRepresentation = nullptr;
		}

		ReleaseResources();
	}

	void DirectXRenderer::Update(const Timer* pTimer)
	{
		m_pMeshRepresentation->Update(pTimer);
		m_pTransparencyRepresentation->Update(pTimer);
	}


	void DirectXRenderer::Render() const
	{
		if (!m_IsInitialized)
			return;

		// 1. CLEAR RTV & DSV
		ColorRGB clearColor{};

		if (*m_pUseUniformBGColor)
		{
			clearColor = ColorRGB{ 0.1f, 0.1f, 0.1f };
		}
		else
		{
			clearColor = ColorRGB{ 0.39f, 0.59f, 0.93f };
		}

		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);


		// 2. SET PIPELINE + INVOKE DRAWCALLS (= RENDER)
		m_pMeshRepresentation->Render(m_pDeviceContext);
		if (m_UseFireFX) m_pTransparencyRepresentation->Render(m_pDeviceContext);


		// 3. PRESENT BACKBUFFER (SWAP)
		m_pSwapChain->Present(0, 0);
	}

	void DirectXRenderer::ToggleFireFX()
	{
		m_UseFireFX = !m_UseFireFX;

		if (m_UseFireFX)
		{
			std::cout << "Fire Effect enabled" << std::endl;
		}
		else
		{
			std::cout << "Fire Effect disabled" << std::endl;
		}
	}
	void DirectXRenderer::ToggleFilterMethods()
	{
		m_pMeshRepresentation->ToggleFilterMethods();
	}

	HRESULT DirectXRenderer::InitializeDirectX()
	{
		// 1. Create Device & DeviceContext
		// ================================

		// Create DXGI Factory
		IDXGIFactory1* pDxgiFactory{ nullptr };

		HRESULT result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));
		if (FAILED(result)) return result;


		// 2. Create SwapChain
		// ===================

		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		// Get the handle (HWND) from the SDL Backbuffer
		SDL_SysWMinfo sysWMInfo{};
		SDL_VERSION(&sysWMInfo.version)
			SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
		swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

		// Create SwapChain
		result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
		if (FAILED(result)) return result;


		// 3. Create DepthStencil (DS) & DepthStencilView (DSV)
		// ====================================================

		// Resource
		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		// View
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDec{};
		depthStencilViewDec.Format = depthStencilDesc.Format;
		depthStencilViewDec.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDec.Texture2D.MipSlice = 0;

		result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
		if (FAILED(result)) return result;

		result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDec, &m_pDepthStencilView);
		if (FAILED(result)) return result;


		// 4. Create RenderTarget (RT) & RenderTargetView (RTV)
		// ====================================================

		// Resource
		result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
		if (FAILED(result)) return result;

		// View
		result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
		if (FAILED(result)) return result;


		// 5. Bind RTV & DSV to Output Merger Stage
		// ========================================

		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);


		// 6. Set Viewport
		// ===============

		D3D11_VIEWPORT viewport{};
		viewport.Width = static_cast<float>(m_Width);
		viewport.Height = static_cast<float>(m_Height);
		viewport.TopLeftX = 0.f;
		viewport.TopLeftY = 0.f;
		viewport.MinDepth = 0.f;
		viewport.MaxDepth = 1.f;

		m_pDeviceContext->RSSetViewports(1, &viewport);


		// Release Resoures
		pDxgiFactory->Release();

		return result;
	}
	void DirectXRenderer::ReleaseResources()
	{
		if (m_pRenderTargetView) m_pRenderTargetView->Release();
		if (m_pRenderTargetBuffer) m_pRenderTargetBuffer->Release();

		if (m_pDepthStencilView) m_pDepthStencilView->Release();
		if (m_pDepthStencilBuffer) m_pDepthStencilBuffer->Release();

		if (m_pSwapChain) m_pSwapChain->Release();
	}
}
