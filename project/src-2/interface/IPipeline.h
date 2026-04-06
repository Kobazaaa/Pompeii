#ifndef POMPEII_PIPELINE_INTERFACE_H
#define POMPEII_PIPELINE_INTERFACE_H

// -- Pompeii Includes --
#include "PompeiiEnums.h"
#include "IShader.h"
#include "VertexInfo.h"

// -- Standard Library --
#include <filesystem>

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    Pipeline Description
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct PipelineDesc
	{
		std::vector<ImageFormat> colorAttachmentFormats;

		IShader* vertexShader;
		IShader* fragmentShader;

		VertexLayout vertexLayout;

		CullMode cullMode = CullMode::Back;
		FrontFace frontFace = FrontFace::Clockwise;
		PolygonMode polygonMode = PolygonMode::Fill;
		PrimitiveTopology primitiveTopology = PrimitiveTopology::TriangleList;

		bool depthRead = true;
		bool depthWrite = true;
		ImageFormat depthFormat = ImageFormat::Depth32F;
		DepthCompareOp depthCompareOp = DepthCompareOp::Less;

		bool operator==(const PipelineDesc& other) const = default;
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    Pipeline Hasher
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct PipelineDescHasher
	{
		static void HashCombine(size_t& seed, size_t hash)
		{
			seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		size_t operator()(const PipelineDesc& desc) const
		{
			size_t hash = 0;

			for (const auto& format : desc.colorAttachmentFormats)
			{
				HashCombine(hash, std::hash<int>()(static_cast<int>(format)));
			}

			HashCombine(hash, std::hash<IShader*>()(desc.vertexShader));
			HashCombine(hash, std::hash<IShader*>()(desc.fragmentShader));

			for (const auto& [binding, stride, inputRate] : desc.vertexLayout.bindings)
			{
				HashCombine(hash, binding);
				HashCombine(hash, stride);
				HashCombine(hash, static_cast<int>(inputRate));
			}
			for (const auto& [location, binding, format, offset] : desc.vertexLayout.attributes)
			{
				HashCombine(hash, location);
				HashCombine(hash, binding);
				HashCombine(hash, static_cast<int>(format));
				HashCombine(hash, offset);
			}

			HashCombine(hash, std::hash<int>()(static_cast<int>(desc.cullMode)));
			HashCombine(hash, std::hash<int>()(static_cast<int>(desc.frontFace)));
			HashCombine(hash, std::hash<int>()(static_cast<int>(desc.polygonMode)));
			HashCombine(hash, std::hash<int>()(static_cast<int>(desc.primitiveTopology)));
			HashCombine(hash, std::hash<int>()(static_cast<int>(desc.depthFormat)));
			HashCombine(hash, std::hash<int>()(static_cast<int>(desc.depthCompareOp)));

			HashCombine(hash, std::hash<bool>()(desc.depthRead));
			HashCombine(hash, std::hash<bool>()(desc.depthWrite));

			return hash;
		}
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    IPipeline
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class IPipeline
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		virtual ~IPipeline() = default;

	private:
	};
}

#endif // POMPEII_PIPELINE_INTERFACE_H
