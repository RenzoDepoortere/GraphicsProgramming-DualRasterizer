#include "pch.h"
#include "Renderer.h"

#include "DirectXRenderer.h"
#include "SoftwareRenderer.h"

#include "Texture.h"
#include "Camera.h"

#include <iostream>

namespace dae
{
	Renderer::Renderer(SDL_Window* pWindow)
	{
		// Initialize variables
		if (Initialize(pWindow) == false)
		{
			std::cout << "Failed to initialize renderers" << std::endl;
		}
		else m_InitializingSucceeded = true;

		PrintInfo();
	}
	Renderer::~Renderer()
	{
		Delete();
	}

	void Renderer::Update(const Timer* pTimer)
	{
		// When failed initialization, return
		if (m_InitializingSucceeded == false) return;

		// Update camera
		m_pCamera->Update(pTimer);

		// Update worldMatrix
		UpdateWorldMatrix(pTimer);

		// Update directXRenderer
		m_pDirectXRenderer->Update(pTimer);

		// Update softwareRenderer
		m_pSoftwareRenderer->Update(pTimer);
	}
	void Renderer::Render() const
	{
		// When failed initialization, return
		if (m_InitializingSucceeded == false) return;

		if (m_ShowHardware)
		{
			// Render directXRenderer
			m_pDirectXRenderer->Render();
		}
		else
		{
			// Render softwareRenderer
			m_pSoftwareRenderer->Render();
		}
	}

	void Renderer::ToggleRenderer()
	{
		m_ShowHardware = !m_ShowHardware;
		
		if (m_ShowHardware)
		{
			std::cout << "Now using the hardware renderer" << std::endl;
		}
		else
		{
			std::cout << "Now using the software renderer" << std::endl;
		}
	}
	void Renderer::ToggleSpinning()
	{
		m_ShouldSpin = !m_ShouldSpin;

		if (m_ShouldSpin)
		{
			std::cout << "Spinning enabled" << std::endl;
		}
		else
		{
			std::cout << "Spinning disabled" << std::endl;
		}
	}
	void Renderer::ToggleUniformClearColor()
	{
		m_UseClearColorBackground = !m_UseClearColorBackground;

		if (m_UseClearColorBackground)
		{
			std::cout << "Using clear color background" << std::endl;
		}
		else
		{
			std::cout << "Using normal background" << std::endl;
		}
	}
	void Renderer::ToggleCullingMode()
	{
		switch (m_CurrentCullingMode)
		{
		case backFace:
			m_CurrentCullingMode = frontFace;
			std::cout << "Current culling mode is front-face" << std::endl;
			break;

		case frontFace:
			m_CurrentCullingMode = noCulling;
			std::cout << "Current culling mode is no culling" << std::endl;
			break;

		case noCulling:
			m_CurrentCullingMode = backFace;
			std::cout << "Current culling mode is back-face" << std::endl;
			break;

		}
	}

	void Renderer::ToggleFireFX()
	{
		if (m_ShowHardware) m_pDirectXRenderer->ToggleFireFX();
	}
	void Renderer::ToggleFilterMethods()
	{
		if (m_ShowHardware) m_pDirectXRenderer->ToggleFilterMethods();
	}

	void Renderer::ToggleShadingMode()
	{
		if (m_ShowHardware == false) m_pSoftwareRenderer->ToggleShadingMode();
	}
	void Renderer::ToggleNormalMap()
	{
		if (m_ShowHardware == false) m_pSoftwareRenderer->ToggleNormalMap();
	}
	void Renderer::ToggleDepthBuffer()
	{
		if (m_ShowHardware == false) m_pSoftwareRenderer->ToggleFilter();
	}
	void Renderer::ToggleBoundingBox()
	{
		if (m_ShowHardware == false) m_pSoftwareRenderer->ToggleBoundingBox();
	}

	bool Renderer::Initialize(SDL_Window* pWindow)
	{
		// Get windowInfo
		SDL_GetWindowSize(pWindow, &m_WindowWidth, &m_WindowHeight);

		// Create device
		D3D_FEATURE_LEVEL featureLevel{ D3D_FEATURE_LEVEL_11_1 };
		uint32_t createDeviceFlags{ 0 };

		#if defined(DEBUG) || defined(_DEBUG)
			createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
		#endif

		HRESULT result{ D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_HARDWARE,0,createDeviceFlags,&featureLevel,1,D3D11_SDK_VERSION,&m_pDevice,nullptr,&m_pDeviceContext) };
		if (FAILED(result)) return false;

		// Create camera
		const float aspectRatio{ m_WindowWidth / static_cast<float>(m_WindowHeight) };
		m_pCamera = new Camera(aspectRatio, { 0,0,0.f }, 45.f);

		// Get all the modelInfo
		GetModel();

		// Create directXRenderer
		m_pDirectXRenderer = new DirectXRenderer(
			pWindow, m_WindowWidth, m_WindowHeight,
			m_pDevice, m_pDeviceContext,
			m_Vertices, m_Indices,
			m_pDiffuseTexture, m_pNormalTexture, m_pSpecularTexture, m_pGlossinessTexture,
			m_pCamera,
			&m_WorldMatrix, &m_UseClearColorBackground, &m_CurrentCullingMode);

		// Create softwareRenderer
		m_pSoftwareRenderer = new SoftwareRenderer(
			pWindow, m_WindowWidth, m_WindowHeight,
			m_Vertices, m_Indices,
			m_pDiffuseTexture, m_pNormalTexture, m_pSpecularTexture, m_pGlossinessTexture,
			m_pCamera,
			&m_WorldMatrix, &m_UseClearColorBackground, &m_CurrentCullingMode
		);

		return true;
	}
	void Renderer::Delete()
	{
		// Delete directXRenderer
		if (m_pDirectXRenderer)
		{
			delete m_pDirectXRenderer;
			m_pDirectXRenderer = nullptr;
		}

		// Delete softwareRenderer
		if (m_pSoftwareRenderer)
		{
			delete m_pSoftwareRenderer;
			m_pSoftwareRenderer = nullptr;
		}

		// Delete camera
		if (m_pCamera)
		{
			delete m_pCamera;
			m_pCamera = nullptr;
		}	

		// Delete textures
		if (m_pGlossinessTexture)
		{
			delete m_pGlossinessTexture;
			m_pGlossinessTexture = nullptr;
		}

		if (m_pSpecularTexture)
		{
			delete m_pSpecularTexture;
			m_pSpecularTexture = nullptr;
		}

		if (m_pNormalTexture)
		{
			delete m_pNormalTexture;
			m_pNormalTexture = nullptr;
		}

		if (m_pDiffuseTexture)
		{
			delete m_pDiffuseTexture;
			m_pDiffuseTexture = nullptr;
		}

		// Delete devices
		if (m_pDeviceContext)
		{
			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();
		}

		if (m_pDevice) m_pDevice->Release();
	}

	void Renderer::GetModel()
	{
		// Get vertices and indices
		Utils::ParseOBJ("Resources/vehicle.obj", m_Vertices, m_Indices);

		// Create Textures

		// Diffuse
		std::string texturePath{ "Resources/vehicle_diffuse.png" };
		m_pDiffuseTexture = new Texture(m_pDevice, texturePath.c_str());

		// Normal
		texturePath = "Resources/vehicle_normal.png";
		m_pNormalTexture = new Texture(m_pDevice, texturePath.c_str());

		// Specular
		texturePath = "Resources/vehicle_specular.png";
		m_pSpecularTexture = new Texture(m_pDevice, texturePath.c_str());

		// Glossiness
		texturePath = "Resources/vehicle_gloss.png";
		m_pGlossinessTexture = new Texture(m_pDevice, texturePath.c_str());
	}

	void Renderer::PrintInfo()
	{
		std::cout << "Key Binding: Shared" << std::endl;
		std::cout << '\t' << "[F1]" << '\t' << "Toggle Rasterizer Mode (HARDWARE/SOFTWARE)" << std::endl;
		std::cout << '\t' << "[F2]" << '\t' << "Toggle Vehicle Rotation (ON/OFF)" << std::endl;
		std::cout << '\t' << "[F9]" << '\t' << "Cycle CullMode (BACK/FRONT/NONE)" << std::endl;
		std::cout << '\t' << "[F10]" << '\t' << "Toggle Uniform ClearColor (ON/OFF)" << std::endl;
		std::cout << '\t' << "[F11]" << '\t' << "Toggle Print FPS (ON/OFF)" << std::endl;
		std::cout << std::endl;

		std::cout << "Key Binding: Hardware" << std::endl;
		std::cout << '\t' << "[F3]" << '\t' << "Toggle FireFX (ON/OFF)" << std::endl;
		std::cout << '\t' << "[F4]" << '\t' << "Cycle Sampler State (POINT/LINEAR/ANISOTROPIC)" << std::endl;
		std::cout << std::endl;

		std::cout << "Key Binding: Software" << std::endl;
		std::cout << '\t' << "[F5]" << '\t' << "Cycle Shading Mode (COMBINED/OBSERVED_AREA/DIFFUSE/SPECULAR)" << std::endl;
		std::cout << '\t' << "[F6]" << '\t' << "Toggle NormalMap (ON/OFF)" << std::endl;
		std::cout << '\t' << "[F7]" << '\t' << "Toggle DepthBuffer Visualization (ON/OFF)" << std::endl;
		std::cout << '\t' << "[F8]" << '\t' << "Toggle BoundingBox Visualization (ON/OFF)" << std::endl;
		std::cout << std::endl << std::endl << std::endl << std::endl;
	}

	void Renderer::UpdateWorldMatrix(const Timer* pTimer)
	{
		if (m_ShouldSpin) m_AccumulatedTime += pTimer->GetElapsed();

		// Translate Matrix
		const dae::Matrix translationMatrix{ dae::Matrix::CreateTranslation(0.f, 0.f, 50.f) };
		// Rotation Matrix
		const dae::Matrix rotationMatrix{ dae::Matrix::CreateRotationY(m_AccumulatedTime) };
		// Scale Matrix
		const dae::Matrix scaleMatrix{ dae::Matrix::CreateScale(1.f,1.f,1.f) };

		// Change worldMatrix
		m_WorldMatrix = scaleMatrix * rotationMatrix * translationMatrix;
	}
}