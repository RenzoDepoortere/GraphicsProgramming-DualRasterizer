#pragma once
#include "Math.h"
#include "Vector2.h"
#include "pch.h"

namespace dae
{
	// -- Enums -- //
	// ==============

	enum PrimitiveTopology
	{
		TriangleList,
		TriangleStrip
	};

	enum CullingMode
	{
		backFace, frontFace, noCulling
	};

	enum FilterMethod
	{
		Point, Linear, Anisotropic
	};

	// -- Structs -- //
	// ================

	struct VS_INPUT
	{
		Vector3 Position{};
		Vector3 Color{};
		Vector2 UV{};
		Vector3 normal{};
		Vector3 tangent{};
		Vector3 viewDirection{};
	};

	struct VS_SIMPLE_INPUT
	{
		Vector3 Position{};
		Vector3 Color{};
		Vector2 UV{};
	};

	struct VS_OUPUT
	{
		Vector4 Position{};
		ColorRGB Color{};
		Vector2 UV{};
		Vector3 normal{};
		Vector3 tangent{};
		Vector3 viewDirection{};
	};

	struct Mesh
	{
		std::vector<VS_INPUT> vertices{};
		std::vector<uint32_t> indices{};
		PrimitiveTopology primitiveTopology{ PrimitiveTopology::TriangleStrip };

		std::vector<VS_OUPUT> vertices_out{};
		Matrix worldMatrix{};
	};
}