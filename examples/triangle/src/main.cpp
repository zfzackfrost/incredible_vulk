#include <ivulk/core/app.hpp>
#include <ivulk/core/buffer.hpp>
#include <ivulk/core/graphics_pipeline.hpp>
#include <ivulk/core/vertex.hpp>
#include <ivulk/utils/table_print.hpp>

#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>

#include <glm/gtc/constants.hpp>

using namespace ivulk;

// clang-format off
IVULK_VERTEX_STRUCT(SimpleVertex, 
	((glm::vec2, pos, 0))
	((glm::vec3, color, 1))
);
// clang-format on

const std::vector<SimpleVertex> triangleVerts = {
	// First triangle
	{
		.pos = {-0.5f, -0.5f},
		.color = {1.0f, 0.0f, 0.0f},
	},
	{
		.pos = {0.5f, 0.5f},
		.color = {1.0f, 1.0f, 0.0f},
	},
	{
		.pos = {-0.5f, 0.5f},
		.color = {0.0f, 0.0f, 1.0f},
	},
	// Second triangle
	{
		.pos = {-0.5f, -0.5f},
		.color = {1.0f, 0.0f, 0.0f},
	},
	{
		.pos = {0.5f, -0.5f},
		.color = {1.0f, 1.0f, 1.0f},
	},
	{
		.pos = {0.5f, 0.5f},
		.color = {1.0f, 1.0f, 0.0f},
	},
};

class TriangleApp : public App
{
public:
	TriangleApp(int argc, char* argv[])
		: App(argc, argv)
	{
		frameNum = 0;
		elapsedTime = 0.0f;
		timeSinceStatus = std::numeric_limits<float>::max();
		clearColorA = {1, 1, 0, 1};
		clearColorB = {0, 0.075, 1, 1};
		clearColor = clearColorA;
	}

protected:
	virtual void initialize(bool swapchainOnly) override
	{
		pipeline = createVkGraphicsPipeline(
			{
				"shaders/simple.vert.spv",
				"shaders/simple.frag.spv",
			},
			SimpleVertex::getBindingDescription(), SimpleVertex::getAttributeDescriptions());
		state.vk.pipelines.mainGfx = std::weak_ptr<GraphicsPipeline>(pipeline);

		// Skip anything that doesn't depend on the swapchain, if requested
		if (swapchainOnly) return;

		const auto sz = sizeof(triangleVerts[0]) * triangleVerts.size();
		vertexBuffer = Buffer::create(state.vk.device,
									  {
										  .size = sz,
										  .usage = E_BufferUsage::Vertex,
									  });
		vertexBuffer->fillBuffer(triangleVerts.data(), triangleVerts.size());
	}

	virtual void cleanup(bool swapchainOnly) override
	{
		pipeline.reset();

		// Skip anything that doesn't depend on the swapchain, if requested
		if (swapchainOnly) return;

		vertexBuffer.reset();
	}

	virtual void render(CommandBuffers::Ref cmdBuffer) override
	{
		if (auto cb = cmdBuffer.lock())
		{
			cb->clearAttachments(pipeline, clearColor);
			cb->bindPipeline(pipeline);
			cb->draw(_buffer = vertexBuffer);
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

		constexpr float period = 5.0f;
		float alpha = glm::sin((elapsedTime / period) * glm::two_pi<float>());

		alpha = alpha * 0.5 + 0.5;

		clearColor = glm::mix(clearColorA, clearColorB, alpha);

		frameNum++;
	}

	virtual InitArgs getInitArgs() const override
	{
		return {
			.appName = "Triangle Demo",
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
		result += deviceProperties.limits.maxImageDimension2D;
		if (deviceFeatures.tessellationShader)
			result += 350;
		return result;
	}

	virtual std::filesystem::path getAssetsDir() override
	{
		std::filesystem::path execPath {state.cmdArgs[0]};
		execPath = std::filesystem::absolute(execPath);
		auto execDir = execPath.parent_path();
		return execDir / "assets";
	}

	std::shared_ptr<GraphicsPipeline> pipeline;
	std::shared_ptr<Buffer> vertexBuffer;

	std::size_t frameNum;

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
		TriangleApp {argc, argv}.run();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
