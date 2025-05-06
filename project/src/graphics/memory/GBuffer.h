#ifndef G_BUFFER_H
#define G_BUFFER_H

// -- Pompeii Includes --
#include "Image.h"

namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  GBuffer	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class GBuffer
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		void Initialize(const Context& context, VkExtent2D size);
		void Destroy(const Context& context);
		void Resize(const Context& context, VkExtent2D size);

		void TransitionBufferWriting(const CommandBuffer& commandBuffer);
		void TransitionBufferSampling(const CommandBuffer& commandBuffer);

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		// This data is only valid if the GBuffer images have been transitioned with TransitionBufferWriting
		const std::vector<VkRenderingAttachmentInfo>& GetRenderingAttachments() const;
		uint32_t GetAttachmentCount() const;
		std::vector<VkFormat> GetAllFormats() const;
		VkExtent2D GetExtent() const;

		// -- Images --
		const Image& GetAlbedoOpacityImage() const;
		const Image& GetNormalImage() const;
		const Image& GetWorldPosImage() const;
		const Image& GetSpecularityImage() const;

	private:
		// -- Images --
		Image m_Albedo_Opacity;
		Image m_Normal;
		Image m_WorldPos;
		Image m_Specularity;
		std::vector<Image*> m_vAllImages;
		void CreateImage(const Context& context, Image& image, VkExtent2D size, VkFormat format, const char* pName);

		// -- Data --
		void AddRenderingAttachment(const Image& image);
		std::vector<VkRenderingAttachmentInfo> m_vRenderingAttachments{};
		VkExtent2D m_Extent{};
	};
}
#endif // G_BUFFER_H