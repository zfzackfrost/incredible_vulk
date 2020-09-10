#include <ivulk/glm.hpp>

#include <ivulk/core/app.hpp>
#include <ivulk/core/buffer.hpp>
#include <ivulk/core/graphics_pipeline.hpp>
#include <ivulk/core/image.hpp>
#include <ivulk/core/sampler.hpp>
#include <ivulk/core/texture.hpp>
#include <ivulk/core/uniform_buffer.hpp>
#include <ivulk/core/vertex.hpp>

#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>

#include <ivulk/core/model/static_model.hpp>

using namespace ivulk;

struct MatricesUBOData
{
	LAYOUT_MAT4 glm::mat4 model = glm::mat4(1.0f);
	LAYOUT_MAT4 glm::mat4 view  = glm::mat4(1.0f);
	LAYOUT_MAT4 glm::mat4 proj  = glm::mat4(1.0f);
};

struct LAYOUT_STRUCT PointLight
{
	LAYOUT_VEC3 glm::vec3 position;
	LAYOUT_VEC3 glm::vec3 color;
};
struct LAYOUT_STRUCT DirectionLight
{
	LAYOUT_VEC3 glm::vec3 direction;
	LAYOUT_VEC3 glm::vec3 color;
};
struct LightingUBOData
{
	PointLight pointLights[4];
	LAYOUT_INT int32_t pointLightCount = 0;
	DirectionLight dirLights[4];
	LAYOUT_INT int32_t dirLightCount = 0;
	LAYOUT_VEC3 glm::vec3 viewPos;
};

class ModelSpinApp : public App
{
public:
	ModelSpinApp(int argc, char* argv[])
		: App(argc, argv)
	{
		elapsedTime     = 0.0f;
		timeSinceStatus = std::numeric_limits<float>::max();
		clearColorA     = {1, 0.65, 0, 1};
		clearColorB     = {0, 0.355, 1, 1};
		clearColor      = clearColorA;
	}

protected:

	virtual void initialize(bool swapchainOnly) override
	{
		if (!swapchainOnly)
		{
			sampler           = Sampler::create(state.vk.device, {});
			crateBaseColorTex = Image::create(state.vk.device,
											  {
												  .load {
													  .bEnable = true,
													  .path    = "textures/Crate/Crate_basecolor.png",
												  },
											  });
		}
		uboMatrices = UniformBufferObject::create(state.vk.device, {.size = sizeof(MatricesUBOData)});
		uboLighting = UniformBufferObject::create(state.vk.device, {.size = sizeof(LightingUBOData)});

		pipeline = GraphicsPipeline::create(state.vk.device, {
			.vertex = StaticMeshVertex::getPipelineInfo(),
			.shaderPath = {
				.vert = "shaders/crate.vert.spv",
				.frag = "shaders/crate.frag.spv",
			},
			.descriptor = {
				.uboBindings = {
					{
						.ubo     = uboMatrices,
						.binding = 0u,
					},
					{
						.ubo     = uboLighting,
						.binding = 1u,
					},
				},
			},
		});
		state.vk.pipelines.mainGfx = std::weak_ptr<GraphicsPipeline>(pipeline);

		// Skip anything that doesn't depend on the swapchain, if requested
		if (swapchainOnly)
			return;

		model = StaticModel::load("models/crate.fbx");
	}

	virtual void cleanup(bool swapchainOnly) override
	{
		pipeline.reset();

		// Skip anything that doesn't depend on the swapchain, if requested
		if (swapchainOnly)
			return;

		model.reset();
		uboMatrices.reset();
		uboLighting.reset();
		crateBaseColorTex.reset();
		sampler.reset();
	}

	virtual void render(CommandBuffers::Ref cmdBuffer) override
	{
		if (auto cb = cmdBuffer.lock())
		{
			cb->clearAttachments(pipeline, clearColor);
			cb->bindPipeline(pipeline);
			model->render(cb);
		}
	}

	virtual void update(float deltaSeconds) override
	{
		elapsedTime += deltaSeconds;
		timeSinceStatus += deltaSeconds;

		// Print stats every 5 seconds
		if (timeSinceStatus >= 5.0f)
		{
			std::cout << "Frame Delta: " << deltaSeconds << " seconds" << std::endl;
			timeSinceStatus = 0.0f;
		}

		// ================= Matrices ================== //

		float aspect = static_cast<float>(state.vk.swapChain.extent.width)
					   / static_cast<float>(state.vk.swapChain.extent.height);

		matrices.model = glm::rotate(glm::mat4(matrices.model),
											 deltaSeconds * glm::radians(180.0f),
											 glm::vec3(0.0f, 0.0f, 1.0f));

		matrices.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0), glm::vec3(0, 0, 1));

		matrices.proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 10.0f);
		matrices.proj[1][1] *= -1.0f;

		uboMatrices->setUniforms(matrices);

		// ================== Lights =================== //

		glm::vec3 lightOrigin;
		{
			constexpr float radius = 3.5f;
			constexpr float lightCirclePeriod = 2.0f;
			float theta = (elapsedTime / lightCirclePeriod) * glm::two_pi<float>();
			lightOrigin.x = glm::cos(theta) * radius;
			lightOrigin.y = glm::sin(theta) * radius;
			lightOrigin.z = 2.45f;
		}
		lighting.dirLights[0].color = glm::vec3(1);
		lighting.dirLights[0].direction = glm::normalize(glm::vec3(0.1, 0.1, 1));
		lighting.dirLightCount = 1;
		lighting.viewPos = glm::vec3(2.0f, 2.0f, 2.0f);

		uboLighting->setUniforms(lighting);

		
	}

	virtual InitArgs getInitArgs() const override
	{
		return {
			.appName = "Spinning Rectangle Demo",
			.bDebugPrint = true,
			.window = {
				.width = 800,
				.height = 600,
				.bResizable = true,
			},
			.vk = {
				.bEnableValidation = true
			}
		};
	}

	virtual int32_t rateDeviceSuitability(VkPhysicalDevice device) override
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		int32_t result = 0;
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			result += 1000;
		return result;
	}

	virtual boost::filesystem::path getAssetsDir() override
	{
		boost::filesystem::path execPath {state.cmdArgs[0]};
		execPath = boost::filesystem::absolute(boost::filesystem::system_complete(execPath));

		auto execDir = execPath.parent_path();
		return execDir / "assets";
	}

	GraphicsPipeline::Ptr pipeline;
	StaticModel::Ptr model;

	Image::Ptr crateBaseColorTex;
	Sampler::Ptr sampler;

	UniformBufferObject::Ptr uboMatrices;
	MatricesUBOData matrices;
	
	UniformBufferObject::Ptr uboLighting;
	LightingUBOData lighting;

	glm::vec4 clearColor;
	glm::vec4 clearColorA;
	glm::vec4 clearColorB;

	float elapsedTime;
	float timeSinceStatus;

};

int main(int argc, char* argv[])
{
	try
	{
		ModelSpinApp {argc, argv}.run();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
