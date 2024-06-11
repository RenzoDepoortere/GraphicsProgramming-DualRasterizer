#pragma once

#include <cstdint>
#include <vector>

#include "Camera.h"
#include "DataTypes.h"

struct SDL_Window;
struct SDL_Surface;

class Texture;

namespace dae
{
	struct Mesh;
	class Timer;
	class Scene;

	class SoftwareRenderer final
	{
	public:
		SoftwareRenderer(SDL_Window* pWindow, int windowWidth, int windowHeight,
						const std::vector<VS_INPUT>& vertices, const std::vector<uint32_t>& indices,
						Texture* pDiffuseTexture, Texture* pNormalTexture, Texture* pSpecularTexture, Texture* pGlossinessTexture,
						Camera* pCamera,
						const Matrix* pWorldMatrix, bool* pUseClearColorBackground, CullingMode* pCurrentCullingMode);
		~SoftwareRenderer();

		SoftwareRenderer(const SoftwareRenderer&) = delete;
		SoftwareRenderer(SoftwareRenderer&&) noexcept = delete;
		SoftwareRenderer& operator=(const SoftwareRenderer&) = delete;
		SoftwareRenderer& operator=(SoftwareRenderer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render();

		bool SaveBufferToImage() const;

		void ToggleFilter();
		void ToggleShadingMode();
		void ToggleNormalMap();
		void ToggleBoundingBox();

	private:
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		float* m_pDepthBufferPixels;

		Camera* m_pCamera{ nullptr };

		std::vector<VS_INPUT> m_TrianglesVertices;
		std::vector<Mesh> m_Meshes;

		Texture* m_pDiffuseTexture{ nullptr };
		Texture* m_pNormalTexture{ nullptr };
		Texture* m_pSpecularTexture{ nullptr };
		Texture* m_pGlossinessTexture{ nullptr };

		int m_Width{};
		int m_Height{};

		const Matrix* m_pWorldMatrix{ nullptr };
		bool* m_pUseClearColorBackground{ nullptr };
		CullingMode* m_pCurrentCullingMode{ nullptr };

		bool m_ShowDepthBuffer{ false };
		bool m_UseNormalMap{ true };
		bool m_ShowBoundingBoxes{ false };

		float m_AccumulatedTime{};

		enum class shadingModes
		{
			ObservedArea,
			Diffuse,
			Specular,
			Combined
		};
		shadingModes m_CurrentShadingMode{ shadingModes::Combined };

		//Function that transforms the vertices from the mesh from World space to Screen space
		void VertexTransformationFunction(const std::vector<VS_INPUT>& vertices_in, std::vector<VS_OUPUT>& vertices_out, const Matrix& worldMatrix) const; //W1 Version

		// HELPERS
		bool IsValueBetweenBoundaries(float value, float minBound = 0.0f, float maxBound = 1.0f) const;
		ColorRGB PixelShading(const VS_OUPUT& vertex) const;
	};
}
