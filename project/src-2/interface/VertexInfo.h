#ifndef POMPEII_VERTEX_INFO_H
#define POMPEII_VERTEX_INFO_H

// -- Standard Library --
#include <vector>

// -- Pompeii Includes --
#include "PompeiiEnums.h"

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    VertexLayout
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct VertexBindingDesc
	{
		uint32_t binding = 0;
		uint32_t stride = 0;
		VertexInputRate inputRate = VertexInputRate::Vertex;

		bool operator==(const VertexBindingDesc&) const = default;
	};
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    VertexLayout
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct VertexAttributeDesc
	{
		uint32_t location = 0;
		uint32_t binding = 0;
		VertexFormat format;
		uint32_t offset = 0;

		bool operator==(const VertexAttributeDesc&) const = default;
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    VertexLayout
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct VertexLayout
	{
		std::vector<VertexBindingDesc> bindings;
		std::vector<VertexAttributeDesc> attributes;

		bool operator==(const VertexLayout&) const = default;
	};
}

#endif // POMPEII_VERTEX_INFO_H
