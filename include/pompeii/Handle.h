#ifndef HANDLE_H
#define HANDLE_H

// -- Standard Library --
#include <cstdint>
#include <limits>

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Handle
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	template <typename Tag>
	struct Handle
	{
		static constexpr uint32_t INVALID_INDEX = std::numeric_limits<uint32_t>::max();

		uint32_t index		{ INVALID_INDEX };
		uint32_t generation	{ 0 };

		constexpr bool IsValid() const noexcept { return index != INVALID_INDEX; }
		constexpr bool operator==(const Handle& other) const noexcept = default;
	};

	namespace tags
	{
		struct MeshTag {};
		struct TextureTag {};
		struct MaterialTag {};
		struct BufferTag {};
		struct LightTag {};
	}

	using MeshHandle		= Handle<tags::MeshTag>;
	using TextureHandle		= Handle<tags::TextureTag>;
	using MaterialHandle	= Handle<tags::MaterialTag>;
	using BufferHandle		= Handle<tags::BufferTag>;
	using LightHandle		= Handle<tags::LightTag>;
}

#endif // HANDLE_H
