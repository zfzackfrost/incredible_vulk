#include <ivulk/core/app.hpp>
#include <ivulk/core/buffer.hpp>
#include <ivulk/core/graphics_pipeline.hpp>
#include <ivulk/core/image.hpp>
#include <ivulk/core/sampler.hpp>
#include <ivulk/core/texture.hpp>
#include <ivulk/core/vertex.hpp>

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
	((glm::vec2, texCoords, 1))
);
// clang-format on

const std::vector<SimpleVertex> verts = {
	SimpleVertex {.pos = {-0.5f, -0.5f}, .texCoords = {0.0f, 0.0f}},
	SimpleVertex {.pos = {0.5f, -0.5f}, .texCoords = {1.0f, 0.0f}},
	SimpleVertex {.pos = {0.5f, 0.5f}, .texCoords = {1.0f, 1.0f}},
	SimpleVertex {.pos = {-0.5f, 0.5f}, .texCoords = {0.0f, 1.0f}},
};

const std::vector<uint32_t> indices = {2, 1, 0, 0, 3, 2};

class RectangleTexApp : public App
{
public:
	RectangleTexApp(int argc, char* argv[])
		: App(argc, argv)
	{
		frameNum        = 0;
		elapsedTime     = 0.0f;
		timeSinceStatus = std::numeric_limits<float>::max();
		clearColorA     = {1, 0.65, 0, 1};
		clearColorB     = {0, 0.355, 1, 1};
		clearColor      = clearColorA;
	}

protected:
	void createIndexBuffer()
	{
		const auto sz      = sizeof(indices[0]) * indices.size();
		auto stagingBuffer = Buffer::create(state.vk.device,
											{
												.size       = sz,
												.usage      = E_BufferUsage::TransferSrc,
												.memoryMode = E_MemoryMode::CpuToGpu,
											});
		stagingBuffer->fillBuffer(indices.data(), sz, indices.size());

		indexBuffer = Buffer::create(state.vk.device,
									 {
										 .size       = sz,
										 .usage      = E_BufferUsage::TransferDst | E_BufferUsage::Index,
										 .memoryMode = E_MemoryMode::GpuOnly,
									 });
		indexBuffer->copyFromBuffer(stagingBuffer, sz);
	}

	void createVertexBuffer()
	{
		const auto sz      = sizeof(verts[0]) * verts.size();
		auto stagingBuffer = Buffer::create(state.vk.device,
											{
												.size       = sz,
												.usage      = E_BufferUsage::TransferSrc,
												.memoryMode = E_MemoryMode::CpuToGpu,
											});
		stagingBuffer->fillBuffer(verts.data(), sz, verts.size());

		vertexBuffer = Buffer::create(state.vk.device,
									  {
										  .size       = sz,
										  .usage      = E_BufferUsage::TransferDst | E_BufferUsage::Vertex,
										  .memoryMode = E_MemoryMode::GpuOnly,
									  });
		vertexBuffer->copyFromBuffer(stagingBuffer, sz);
	}

	virtual void initialize(bool swapchainOnly) override
	{
		if (!swapchainOnly)
		{
			tex = Image::create(state.vk.device,
					{.load = {.bEnable = true, .path = "textures/forest.png"}});

			sampler = Sampler::create(state.vk.device, {});
		}

		pipeline = createGraphicsPipeline(
			{
				"shaders/texture.vert.spv",
				"shaders/texture.frag.spv",
			},
			SimpleVertex::getBindingDescription(),
			SimpleVertex::getAttributeDescriptions(),
			{}, {{
				.image = tex,
				.sampler = sampler,
				.binding = 1u,
			}});
		state.vk.pipelines.mainGfx = std::weak_ptr<GraphicsPipeline>(pipeline);

		// Skip anything that doesn't depend on the swapchain, if requested
		if (swapchainOnly)
			return;

		createVertexBuffer();
		createIndexBuffer();

	}

	virtual void cleanup(bool swapchainOnly) override
	{
		pipeline.reset();

		// Skip anything that doesn't depend on the swapchain, if requested
		if (swapchainOnly)
			return;

		vertexBuffer.reset();
		indexBuffer.reset();
		tex.reset();
		sampler.reset();
	}

	virtual void render(CommandBuffers::Ref cmdBuffer) override
	{
		if (auto cb = cmdBuffer.lock())
		{
			cb->clearAttachments(pipeline, clearColor);
			cb->bindPipeline(pipeline);
			cb->draw(_vertexBuffer = vertexBuffer, _indexBuffer = indexBuffer);
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
		float alpha            = glm::sin((elapsedTime / period) * glm::two_pi<float>());

		alpha = alpha * 0.5 + 0.5;

		clearColor = glm::mix(clearColorA, clearColorB, alpha);

		frameNum++;
	}

	virtual InitArgs getInitArgs() const override
	{
		return {
			.appName = "Textured Rectangle Demo",
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

	virtual boost::filesystem::path getAssetsDir() override
	{
		boost::filesystem::path execPath {state.cmdArgs[0]};
		execPath     = boost::filesystem::absolute(boost::filesystem::system_complete(execPath));
		auto execDir = execPath.parent_path();
		return execDir / "assets";
	}

	GraphicsPipeline::Ptr pipeline;
	Buffer::Ptr vertexBuffer;
	Buffer::Ptr indexBuffer;
	Image::Ptr tex;
	Sampler::Ptr sampler;

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
		RectangleTexApp {argc, argv}.run();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
