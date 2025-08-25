#ifndef CAMERA_GPU_H
#define CAMERA_GPU_H

// -- Math Includes --
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

namespace pompeii
{
	struct ExposureSettings
	{
		float aperture;
		float shutterSpeed;
		float iso;
	};
	struct alignas(16) CameraData
	{
		glm::mat4 view;
		glm::mat4 proj;

		ExposureSettings exposureSettings;
		bool autoExposure;
	};
}

#endif // CAMERA_GPU_H
