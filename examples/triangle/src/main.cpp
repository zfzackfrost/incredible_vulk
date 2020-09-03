#include <ivulk/core/app.hpp>
#include <ivulk/core/graphics_pipeline.hpp>
#include <ivulk/core/vertex.hpp>
#include <ivulk/core/buffer.hpp>
#include <ivulk/utils/table_print.hpp>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>

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

class TriangleApp : public ivulk::App
{
public:
	TriangleApp(int argc, char* argv[])
		: ivulk::App(argc, argv)
	{ }

protected:
	std::shared_ptr<ivulk::GraphicsPipeline> pipeline;
	std::shared_ptr<ivulk::Buffer> vertexBuffer;

	virtual void initialize() override
	{
		pipeline = createVkGraphicsPipeline(
			{
				"shaders/simple.vert.spv",
				"shaders/simple.frag.spv",
			},
			SimpleVertex::getBindingDescription(), SimpleVertex::getAttributeDescriptions());
		state.vk.pipelines.mainGfx = std::weak_ptr<ivulk::GraphicsPipeline>(pipeline);

		const auto sz = sizeof(triangleVerts[0]) * triangleVerts.size();
		vertexBuffer = ivulk::Buffer::create(state.vk.device, {
			.size = sz,
			.usage = ivulk::E_BufferUsage::Vertex,
		});
		vertexBuffer->fillBuffer(triangleVerts.data(), triangleVerts.size());
	}

	virtual void cleanup() override { 
		pipeline.reset();
		vertexBuffer.reset();
	}

	virtual void render(std::weak_ptr<ivulk::CommandBuffer> cmdBuffer) override
	{
		if (auto cb = cmdBuffer.lock())
		{
			auto vb = vertexBuffer->getBuffer();
			auto cb0 = cb->getCmdBuffer(0);
			vkCmdBindPipeline(cb0, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipeline());

			VkBuffer buffers[] = {vb};
			VkDeviceSize offsets[] = {0};
			vkCmdBindVertexBuffers(cb0, 0, 1, buffers, offsets);

			vkCmdDraw(cb0, static_cast<uint32_t>(triangleVerts.size()), 1, 0, 0);
		}
	}

	virtual void update(float deltaSeconds) override { }

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
