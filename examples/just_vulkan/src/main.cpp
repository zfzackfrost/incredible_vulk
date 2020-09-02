#include <ivulk/core/app.hpp>
#include <ivulk/core/graphics_pipeline.hpp>
#include <ivulk/utils/table_print.hpp>

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <memory>
class SimpleApp : public ivulk::App
{
public:
	SimpleApp(int argc, char* argv[])
		: ivulk::App(argc, argv)
	{ }

protected:
	std::shared_ptr<ivulk::GraphicsPipeline> pipeline;

	virtual void initialize() override {
		pipeline = createVkGraphicsPipeline({
			"shaders/simple.vert.spv",
			"shaders/simple.frag.spv",
		});
		state.vk.pipelines.mainGfx = std::weak_ptr<ivulk::GraphicsPipeline>(pipeline);
	}

	virtual void cleanup() override 
	{
		pipeline.reset();
	}

	virtual void render(std::weak_ptr<ivulk::CommandBuffer> cmdBuffer) override
	{
		if (auto cb = cmdBuffer.lock())
		{
			auto cb0 = cb->getCmdBuffer(0);
			vkCmdBindPipeline(cb0, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipeline());
			vkCmdDraw(cb0, 3, 1, 0, 0);
		}
	}

	virtual void update(float deltaSeconds) override { }

	virtual InitArgs getInitArgs() const override
	{
		return {
			.appName = "Just Vulkan Demo",
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
		std::filesystem::path execPath{state.cmdArgs[0]};
		execPath = std::filesystem::absolute(execPath);
		auto execDir = execPath.parent_path();
		return execDir / "assets";
	}
};

int main(int argc, char* argv[])
{
	try
	{
		SimpleApp {argc, argv}.run();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
