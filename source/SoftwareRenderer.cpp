//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "pch.h"
#include "SoftwareRenderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Texture.h"
#include "Utils.h"
#include "BRDFs.h"

// Printing
#include <iostream>

using namespace dae;

SoftwareRenderer::SoftwareRenderer(SDL_Window* pWindow, int windowWidth, int windowHeight,
									const std::vector<VS_INPUT>& vertices, const std::vector<uint32_t>& indices,
									Texture* pDiffuseTexture, Texture* pNormalTexture, Texture* pSpecularTexture, Texture* pGlossinessTexture,
									Camera* pCamera,
									const Matrix* pWorldMatrix, bool* pUseClearColorBackground, CullingMode* pCurrentCullingMode)
	// Window
	: m_pWindow(pWindow)
	, m_Width{ windowWidth }
	, m_Height{ windowHeight }
	// Textures
	, m_pDiffuseTexture{ pDiffuseTexture }
	, m_pNormalTexture{ pNormalTexture }
	, m_pSpecularTexture{ pSpecularTexture }
	, m_pGlossinessTexture{ pGlossinessTexture }
	// Camera
	, m_pCamera{ pCamera }
	// Togglers
	, m_pWorldMatrix{ pWorldMatrix }
	, m_pUseClearColorBackground{ pUseClearColorBackground }
	, m_pCurrentCullingMode{ pCurrentCullingMode }
{
	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	// Create BufferArray and initialize all with maxFloat value
	m_pDepthBufferPixels = new float[m_Width * m_Height];
	std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);

	// Define mesh
	std::vector<Mesh> meshes_world{};
	meshes_world.push_back(Mesh{ vertices,indices,PrimitiveTopology::TriangleList });
	m_Meshes = meshes_world;
}

SoftwareRenderer::~SoftwareRenderer()
{
	delete[] m_pDepthBufferPixels;
}

void SoftwareRenderer::Update(const Timer* pTimer)
{
		// Change worldMatrix
		for (auto& currentMesh : m_Meshes)
		{
			currentMesh.worldMatrix = *m_pWorldMatrix;
		}
}

void SoftwareRenderer::Render()
{
	//@START
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);

	//Clear depthBuffer

	// Define backgroundColor
	Vector3 backgroundColor{ 0.39f, 0.39f, 0.39f };
	if (*m_pUseClearColorBackground) backgroundColor = Vector3{ 0.1f, 0.1f, 0.1f };

	SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format,
		static_cast<Uint8>(backgroundColor.x * 255),
		static_cast<Uint8>(backgroundColor.y * 255),
		static_cast<Uint8>(backgroundColor.z * 255)));

	// Refill depthBuffer
	std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);

	// For every mesh
	for (size_t idx{}; idx < m_Meshes.size(); ++idx)
	{
		const Mesh currentMesh{ m_Meshes[idx] };
		const bool usingStripTopology{ currentMesh.primitiveTopology == PrimitiveTopology::TriangleStrip };

		//////////////////////
		// -- PROJECTION -- //
		//////////////////////

		// Transform model-space vertices to NDC-space vertices
		std::vector<VS_OUPUT> vertexOut{};
		const Matrix worldMatrix{ m_Meshes[idx].worldMatrix };

		VertexTransformationFunction(currentMesh.vertices, vertexOut, worldMatrix);


		/////////////////////////////
		// -- PRIMITIVE TOPOLGY -- //
		/////////////////////////////

		// For-loop counting depends on primitiveTopolgy
		size_t idxAddition{ 3 };
		size_t indicesSizeLimit{ 0 };
		if (usingStripTopology)
		{
			idxAddition = 1;
			indicesSizeLimit = 2;
		}


		// For every triangle
		for (size_t idx{}; idx < currentMesh.indices.size() - indicesSizeLimit; idx += idxAddition)
		{
			// VertexIndices
			const int firstIndex{ (int)currentMesh.indices[idx] };
			int secondIndex{ (int)currentMesh.indices[idx + 1] };
			int thirdIndex{ (int)currentMesh.indices[idx + 2] };

			// Swap second and third index with triangleStrip
			const bool triangleIsOdd{ idx % 2 == 1 };
			if (usingStripTopology && triangleIsOdd)
			{
				std::swap(secondIndex, thirdIndex);
			}


			// Normal Vertices
			std::vector<VS_OUPUT> normalVertices{};
			normalVertices.push_back(vertexOut[firstIndex]);
			normalVertices.push_back(vertexOut[secondIndex]);
			normalVertices.push_back(vertexOut[thirdIndex]);


			////////////////////////
			// -- Optimization -- //
			////////////////////////

			// Check if not insideFrustum
			bool isInsideFrustum{ true };
			for (const auto& vertex : normalVertices)
			{
				const bool xInsideFrustum{ -1.f <= vertex.Position.x && vertex.Position.x <= 1.f };
				const bool yInsideFrustum{ -1.f <= vertex.Position.y && vertex.Position.y <= 1.f };
				const bool zInsideFrustum{ 0.f <= vertex.Position.z && vertex.Position.z <= 1.f };

				const bool currentInsideFrustum{ xInsideFrustum && yInsideFrustum && zInsideFrustum };
				if (!currentInsideFrustum)
				{
					isInsideFrustum = false;
					break;
				}
			}

			// Else, don't show
			if (!isInsideFrustum)
			{
				continue;
			}


			/////////////////////////
			// -- RASTERIZATION -- //
			/////////////////////////


			// NDC-space to raster-space
			std::vector<VS_OUPUT> rasterVertices{};
			for (size_t idx{}; idx < normalVertices.size(); ++idx)
			{
				VS_OUPUT newVertex{ normalVertices[idx] };
				newVertex.Position.x = ((newVertex.Position.x + 1) / 2) * m_Width;
				newVertex.Position.y = ((1 - newVertex.Position.y) / 2) * m_Height;

				rasterVertices.push_back(newVertex);
			}


			// RasterVertices
			const Vector2 rasterVector0{ rasterVertices[0].Position.x,rasterVertices[0].Position.y };
			const Vector2 rasterVector1{ rasterVertices[1].Position.x,rasterVertices[1].Position.y };
			const Vector2 rasterVector2{ rasterVertices[2].Position.x,rasterVertices[2].Position.y };


			////////////////////////
			// -- BOUNDING BOX -- //
			////////////////////////

			// TopRight
			Vector2 topRightPoint{};
			topRightPoint.x = std::max(std::max(rasterVector0.x, rasterVector1.x), rasterVector2.x);
			topRightPoint.y = std::max(std::max(rasterVector0.y, rasterVector1.y), rasterVector2.y);

			// Limit point to screenBoundaries
			topRightPoint.x = Clamp(topRightPoint.x, 0.f, static_cast<float>(m_Width));
			topRightPoint.y = Clamp(topRightPoint.y, 0.f, static_cast<float>(m_Height));


			// BottomLeft
			Vector2 bottomLeftPoint{};
			bottomLeftPoint.x = std::min(std::min(rasterVector0.x, rasterVector1.x), rasterVector2.x);
			bottomLeftPoint.y = std::min(std::min(rasterVector0.y, rasterVector1.y), rasterVector2.y);

			// Limit point to screenBoundaries
			bottomLeftPoint.x = Clamp(bottomLeftPoint.x, 0.f, static_cast<float>(m_Width));
			bottomLeftPoint.y = Clamp(bottomLeftPoint.y, 0.f, static_cast<float>(m_Height));


			// For every pixel
			for (int py{ static_cast<int>(bottomLeftPoint.y - 1) }; py < static_cast<int>(topRightPoint.y + 1); ++py)
			{
				for (int px{ static_cast<int>(bottomLeftPoint.x - 1) }; px < static_cast<int>(topRightPoint.x + 1); ++px)
				{
					const int pixelIndex{ py * m_Width + px };
					const Vector2 pixelPos{ static_cast<float>(px), static_cast<float>(py) };

					ColorRGB finalColor{};

					// If should show boundingBoxes, skip calculation
					if (m_ShowBoundingBoxes)
					{
						finalColor = ColorRGB{ 1,1,1 };

						//Update Color in Buffer
						finalColor.MaxToOne();

						m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
							static_cast<uint8_t>(finalColor.r * 255),
							static_cast<uint8_t>(finalColor.g * 255),
							static_cast<uint8_t>(finalColor.b * 255));

						continue;
					}

					///////////////////
					// -- Weights -- //
					///////////////////

					const Vector2 firstEdge{ rasterVector1 - rasterVector0 };
					const Vector2 secondEdge{ rasterVector2 - rasterVector1 };
					const Vector2 thirdEdge{ rasterVector0 - rasterVector2 };

					const float totalParallelogramArea{ Vector2::Cross(firstEdge,-thirdEdge) };

					const float W0{ Vector2::Cross(secondEdge, pixelPos - rasterVector1) / totalParallelogramArea };
					const float W1{ Vector2::Cross(thirdEdge , pixelPos - rasterVector2) / totalParallelogramArea };
					const float W2{ Vector2::Cross(firstEdge , pixelPos - rasterVector0) / totalParallelogramArea };

					// Culling
					bool shouldRender{ false };
					switch (*m_pCurrentCullingMode)
					{
					case backFace:
						shouldRender = 0 < totalParallelogramArea;
						break;

					case frontFace:
						shouldRender = totalParallelogramArea <= 0;
						break;

					case noCulling:
						shouldRender = true;
						break;
					}

					// Check if the pixel is in it
					const bool pixelInsideTriangleWeight{W0 > 0 && W1 > 0 && W2 > 0};
					if (pixelInsideTriangleWeight && shouldRender)
					{


						///////////////////
						// -- Z Depth -- //
						///////////////////

						const float firstZDepth{ rasterVertices[0].Position.z };
						const float secondZDepth{ rasterVertices[1].Position.z };
						const float thirdZDepth{ rasterVertices[2].Position.z };

						const float interpolatedZDepth{ 1 / ((1 / firstZDepth) * W0 + (1 / secondZDepth) * W1 + (1 / thirdZDepth) * W2) };

						// Depth test
						const bool isCloserThenDepthBuffer{ interpolatedZDepth < m_pDepthBufferPixels[pixelIndex] };
						if (isCloserThenDepthBuffer)
						{
							m_pDepthBufferPixels[pixelIndex] = interpolatedZDepth;


							//////////////
							// -- UV -- //
							//////////////

							const Vector2 firstUV{ rasterVertices[0].UV };
							const Vector2 secondUV{ rasterVertices[1].UV };
							const Vector2 thirdUV{ rasterVertices[2].UV };

							// W Depth
							const float firstWDepth{ rasterVertices[0].Position.w };
							const float secondWDepth{ rasterVertices[1].Position.w };
							const float thirdWDepth{ rasterVertices[2].Position.w };

							const float interpolatedWDepth{ 1 / ((1 / firstWDepth) * W0 + (1 / secondWDepth) * W1 + (1 / thirdWDepth) * W2) };

							// Interpolate UV
							const Vector2 interpolatedUV{ ((firstUV / firstWDepth) * W0 + (secondUV / secondWDepth) * W1 + (thirdUV / thirdWDepth) * W2) * interpolatedWDepth };
							const ColorRGB uvColor{ m_pDiffuseTexture->Sample(interpolatedUV) };


							///////////////////
							// -- Shading -- //
							///////////////////

							Vector3 desiredNormal{};

							// Interpolate Normal
							const Vector3 firstNormal{ rasterVertices[0].normal };
							const Vector3 secondNormal{ rasterVertices[1].normal };
							const Vector3 thirdNormal{ rasterVertices[2].normal };

							const Vector3 interpolatedNormal{ ((firstNormal / firstWDepth) * W0 + (secondNormal / secondWDepth) * W1 + (thirdNormal / thirdWDepth) * W2) * interpolatedWDepth };
							desiredNormal = interpolatedNormal;

							// Interpolate Tangent
							const Vector3 firstTangent{ rasterVertices[0].tangent };
							const Vector3 secondTangent{ rasterVertices[1].tangent };
							const Vector3 thirdTangent{ rasterVertices[2].tangent };

							const Vector3 interpolatedTangent{ ((firstTangent / firstWDepth) * W0 + (secondTangent / secondWDepth) * W1 + (thirdTangent / thirdWDepth) * W2) * interpolatedWDepth };

							// Tangent space transformation matrix
							if (m_UseNormalMap)
							{
								// Sample normal
								const ColorRGB normalColor{ m_pNormalTexture->Sample(interpolatedUV) };
								Vector3 sampledNormal{ normalColor.r, normalColor.g, normalColor.b };
								sampledNormal = 2.f * sampledNormal - Vector3{ 1.f, 1.f, 1.f };

								// Create tangentSpaceAxis
								const Vector3 binormal{ Vector3::Cross(interpolatedNormal,interpolatedTangent) };
								Matrix tangentSpaceAxis{};

								tangentSpaceAxis[0] = { interpolatedTangent, 0 };
								tangentSpaceAxis[1] = { binormal,0 };
								tangentSpaceAxis[2] = { interpolatedNormal,0 };
								tangentSpaceAxis[3] = { 0,0,0,0 };

								// Multiply sampledNormal with matrix
								desiredNormal = tangentSpaceAxis.TransformVector(sampledNormal);
							}

							// Interpolate viewDirection
							const Vector3 cameraOrigin{ m_pCamera->GetOrigin() };

							const Vector3 firstViewDirection{ (Vector3{rasterVertices[0].Position.x, rasterVertices[0].Position.y, rasterVertices[0].Position.z} - cameraOrigin).Normalized() };
							const Vector3 secondViewDirection{ (Vector3{rasterVertices[1].Position.x, rasterVertices[1].Position.y, rasterVertices[1].Position.z} - cameraOrigin).Normalized() };
							const Vector3 thirdViewDirection{ (Vector3{rasterVertices[2].Position.x, rasterVertices[2].Position.y, rasterVertices[2].Position.z} - cameraOrigin).Normalized() };

							const Vector3 interpolatedViewDirection{ ((firstViewDirection / firstWDepth) * W0 + (secondViewDirection / secondWDepth) * W1 + (thirdViewDirection / thirdWDepth) * W2) * interpolatedWDepth };

							// Collecting all interpolations
							VS_OUPUT shadingVertex{};
							shadingVertex.Position = Vector4{ pixelPos.x,pixelPos.y,interpolatedZDepth,interpolatedWDepth };
							shadingVertex.Color = uvColor;
							shadingVertex.UV = interpolatedUV;
							shadingVertex.normal = desiredNormal;
							shadingVertex.tangent = interpolatedTangent;
							shadingVertex.viewDirection = interpolatedViewDirection;

							// Actual shading
							const ColorRGB shadedColor{ PixelShading(shadingVertex) };


							//////////////////////
							// -- Show Color -- //
							//////////////////////

							// Switch between showing finalColor and depthBuffer
							if (!m_ShowDepthBuffer)
							{
								finalColor = shadedColor;
							}
							else
							{
								const float remapValue{ InverseLerp(.985f,1.f,interpolatedZDepth) };
								const ColorRGB depthBufferColor{ remapValue, remapValue, remapValue };

								finalColor = depthBufferColor;
							}


							//Update Color in Buffer
							finalColor.MaxToOne();

							m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
								static_cast<uint8_t>(finalColor.r * 255),
								static_cast<uint8_t>(finalColor.g * 255),
								static_cast<uint8_t>(finalColor.b * 255));
						}
					}
				}
			}
		}
	}

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void SoftwareRenderer::VertexTransformationFunction(const std::vector<VS_INPUT>& vertices_in, std::vector<VS_OUPUT>& vertices_out, const Matrix& worldMatrix) const
{
	vertices_out.reserve(vertices_in.size());

	const Matrix cameraInvViewMatrix{ m_pCamera->GetInvViewMatrix() };
	const Matrix cameraProjectionMatrix{ m_pCamera->GetProjectionMatrix() };

	const Matrix viewMatrix{ cameraInvViewMatrix };
	const Matrix projectionMatrix{ cameraProjectionMatrix };
	const Matrix worldViewProjectionMatrix{ worldMatrix * viewMatrix * projectionMatrix };

	for (const auto& currentVertex : vertices_in)
	{
		Vector4 transformedPosition{ currentVertex.Position, 0 };
		transformedPosition = worldViewProjectionMatrix.TransformPoint(transformedPosition);;

		// Perspective Divide
		transformedPosition.x /= transformedPosition.w;
		transformedPosition.y /= transformedPosition.w;
		transformedPosition.z /= transformedPosition.w;

		// Put in vertexOut
		VS_OUPUT tempVertex{};
		tempVertex.Position = transformedPosition;
		tempVertex.Color = ColorRGB{ currentVertex.Color.x, currentVertex.Color.y,currentVertex.Color.z };
		tempVertex.UV = currentVertex.UV;

		tempVertex.normal = worldMatrix.TransformVector(currentVertex.normal);
		tempVertex.tangent = worldMatrix.TransformVector(currentVertex.tangent);

		vertices_out.emplace_back(tempVertex);
	}
}

bool SoftwareRenderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}

void SoftwareRenderer::ToggleFilter()
{
	m_ShowDepthBuffer = !m_ShowDepthBuffer;

	if (m_ShowDepthBuffer)
	{
		std::cout << "Enabled depth-buffer" << std::endl;
	}
	else
	{
		std::cout << "Disabled depth-buffer" << std::endl;
	}
}
void SoftwareRenderer::ToggleShadingMode()
{
	switch (m_CurrentShadingMode)
	{
	case shadingModes::ObservedArea:
		m_CurrentShadingMode = shadingModes::Diffuse;
		std::cout << "Current shadingMode is diffuse" << std::endl;
		break;

	case shadingModes::Diffuse:
		m_CurrentShadingMode = shadingModes::Specular;
		std::cout << "Current shadingMode is specular" << std::endl;
		break;

	case shadingModes::Specular:
		m_CurrentShadingMode = shadingModes::Combined;
		std::cout << "Current shadingMode is combined" << std::endl;
		break;

	case shadingModes::Combined:
		m_CurrentShadingMode = shadingModes::ObservedArea;
		std::cout << "Current shadingMode is observedArea" << std::endl;
		break;
	}
}
void SoftwareRenderer::ToggleNormalMap()
{
	m_UseNormalMap = !m_UseNormalMap;

	if (m_UseNormalMap)
	{
		std::cout << "normalMap on!" << '\n';
	}
	else
	{
		std::cout << "normalMap off!" << '\n';
	}
}
void SoftwareRenderer::ToggleBoundingBox()
{
	m_ShowBoundingBoxes = !m_ShowBoundingBoxes;

	if (m_ShowBoundingBoxes)
	{
		std::cout << "Enabled bounding-boxes visualization" << std::endl;
	}
	else
	{
		std::cout << "Disabled bounding-boxes visualization" << std::endl;
	}
}

bool SoftwareRenderer::IsValueBetweenBoundaries(float value, float minBound, float maxBound) const
{
	if (minBound <= value && value <= maxBound)
	{
		return true;
	}

	return false;
}

ColorRGB SoftwareRenderer::PixelShading(const VS_OUPUT& vertex) const
{
	// Light
	const Vector3 lightDirection{ 0.577f, -0.577f, 0.577f };
	const float lightIntensity{ 7.f };
	const ColorRGB lightColor{ 1,1,1 };

	// Other settings
	const float shininess{ 25.f };
	const ColorRGB ambient{ 0.025f, 0.025f, 0.025f };

	// Maps
	const ColorRGB sampledSpecularColor{ m_pSpecularTexture->Sample(vertex.UV) };
	const Vector3 sampledSpecularVector{ sampledSpecularColor.r,sampledSpecularColor.g,sampledSpecularColor.b };

	const ColorRGB glossinessColor{ m_pGlossinessTexture->Sample(vertex.UV) };
	const Vector3 glossinessVector{ glossinessColor.r,glossinessColor.g,glossinessColor.b };


	// Calculate ObservedArea --> lighted area
	float observedArea{ Vector3::Dot(vertex.normal,-lightDirection) };
	if (observedArea < 0) return{};

	// Calculate Diffuse --> Diffuse + AO
	const ColorRGB diffuseColor{ BRDF::Lambert(lightIntensity, vertex.Color) };

	//// Calculate Radiance --> light intensity
	//const ColorRGB radiance{ lightColor * lightIntensity };

	// Calculate Phong --> Specular
	ColorRGB specular{ BRDF::Phong(sampledSpecularVector.x, glossinessVector.x * shininess, -lightDirection, vertex.viewDirection, vertex.normal) };
	specular.MaxToOne();

	// Switch between modes
	switch (m_CurrentShadingMode)
	{
	case shadingModes::ObservedArea:
		return observedArea * ColorRGB{ 1,1,1 };
		break;

	case shadingModes::Diffuse:
		return diffuseColor;
		break;

	case shadingModes::Specular:
		return specular;
		break;

	case shadingModes::Combined:
		return (diffuseColor + specular + ambient) * observedArea;
		break;

	default:
		return {};
	}
}