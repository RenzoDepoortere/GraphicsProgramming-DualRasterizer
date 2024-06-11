#pragma once
#include "DataTypes.h"
#include "Utils.h"

class Camera;
class Texture;

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class DirectXRenderer;
	class SoftwareRenderer;

	class Renderer final
	{
	public:
		// Constructor and Destructor
		explicit Renderer(SDL_Window* pWindow);
		~Renderer();

		// Rule of Five
		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		// Public functions
		void Update(const Timer* pTimer);
		void Render() const;

		void ToggleRenderer();
		void ToggleSpinning();
		void ToggleUniformClearColor();
		void ToggleCullingMode();

		void ToggleFireFX();
		void ToggleFilterMethods();

		void ToggleShadingMode();
		void ToggleNormalMap();
		void ToggleDepthBuffer();
		void ToggleBoundingBox();

	private:
		// Member Variables
		// ----------------

		// Window
		int m_WindowWidth{};
		int m_WindowHeight{};

		// DirectX
		ID3D11Device* m_pDevice{ nullptr };
		ID3D11DeviceContext* m_pDeviceContext{ nullptr };

		// Renderers
		bool m_InitializingSucceeded{ false };
		DirectXRenderer* m_pDirectXRenderer{ nullptr };
		SoftwareRenderer* m_pSoftwareRenderer{ nullptr };

		// Camera
		Camera* m_pCamera{ nullptr };

		// WorldMatrix
		Matrix m_WorldMatrix{};
		float m_AccumulatedTime{};

		// ModelData
		std::vector<VS_INPUT> m_Vertices{};
		std::vector<uint32_t> m_Indices{};

		// Textures
		Texture* m_pDiffuseTexture{ nullptr };
		Texture* m_pNormalTexture{ nullptr };
		Texture* m_pSpecularTexture{ nullptr };
		Texture* m_pGlossinessTexture{ nullptr };

		// Togglers
		bool m_ShowHardware{ true };
		bool m_ShouldSpin{ false };
		bool m_UseClearColorBackground{ false };
		CullingMode m_CurrentCullingMode{ CullingMode::backFace };

		// Member Functions
		// ----------------
		bool Initialize(SDL_Window* pWindow);
		void Delete();

		void GetModel();

		void PrintInfo();

		void UpdateWorldMatrix(const Timer* pTimer);
	};
}