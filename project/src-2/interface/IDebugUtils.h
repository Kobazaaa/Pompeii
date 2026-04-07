#ifndef POMPEII_DEBUG_UTILITIES_INTERFACE_H
#define POMPEII_DEBUG_UTILITIES_INTERFACE_H

// -- Standard Library
#include <string>

// -- Math Includes
#include "glm/vec4.hpp"

// -- Forward Declarations
namespace pompeii
{
	class IShader;
	class ICommandPool;
	class IPipeline;
	class IImage;
	class IBuffer;
	class ICommandBuffer;
}

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    IDebugMessenger
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class IDebugUtils
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		virtual ~IDebugUtils() = default;

		virtual void Initialize() = 0;
		virtual void Shutdown() = 0;

		//--------------------------------------------------
		//    Functionality
		//--------------------------------------------------
		virtual void BeginLabel(const ICommandBuffer& cmd, const std::string& name, const glm::vec4& color = {1.f, 1.f, 1.f, 1.f}) = 0;
		virtual void EndLabel(const ICommandBuffer& cmd) = 0;
		virtual void InsertLabel(const ICommandBuffer& cmd, const std::string& name, const glm::vec4& color = { 1.f, 1.f, 1.f, 1.f }) = 0;

		virtual void SetName(const std::string& name, const IBuffer& buffer) const = 0;
		virtual void SetName(const std::string& name, const IImage& image) const = 0;
		virtual void SetName(const std::string& name, const IPipeline& pipeline) const = 0;
		virtual void SetName(const std::string& name, const ICommandBuffer& cmdBuf) const = 0;
		virtual void SetName(const std::string& name, const ICommandPool& pool) const = 0;
		virtual void SetName(const std::string& name, const IShader& shader) const = 0;
	};
}

#endif // POMPEII_DEBUG_UTILITIES_INTERFACE_H
