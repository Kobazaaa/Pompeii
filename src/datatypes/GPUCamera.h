#ifndef CAMERA_GPU_H
#define CAMERA_GPU_H

// -- Math Includes --
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_RADIANS
#include <variant>
#include <glm/glm.hpp>

namespace pompeii
{
	struct ManualExposureSettings
	{
		float aperture;
		float shutterSpeed;
		float iso;
	};
	struct AutoExposureSettings
	{
		float minLogLum;
		float logLumRange;
	};
	struct alignas(16) CameraData
	{
		glm::mat4 view;
		glm::mat4 proj;

		ManualExposureSettings manualExposureSettings;
		AutoExposureSettings autoExposureSettings;
		bool autoExposure;
		bool _padding[11];
	};
}

#endif // CAMERA_GPU_H
