#include <ivulk/core/app.hpp>
#include <ivulk/core/buffer.hpp>
#include <ivulk/core/graphics_pipeline.hpp>
#include <ivulk/core/texture.hpp>
#include <ivulk/core/uniform_buffer.hpp>
#include <ivulk/core/vertex.hpp>

#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>

#include <glm/gtc/constants.hpp>

using namespace ivulk;

struct UboData
{
    LAYOUT_VEC3 glm::vec3 tint;
    struct LAYOUT_STRUCT
    {
        LAYOUT_MAT4 glm::mat4 model = glm::mat4(1);
        LAYOUT_MAT4 glm::mat4 view  = glm::mat4(1);
        LAYOUT_MAT4 glm::mat4 proj  = glm::mat4(1);
    } matrices;
};

// clang-format off
IVULK_VERTEX_STRUCT(SimpleVertex, 
	((glm::vec3, pos, 0))
	((glm::vec2, texCoords, 1))
	((glm::vec3, color, 2))
);
// clang-format on

const std::vector<SimpleVertex> verts = {
    // Top square
    {.pos = {-0.5f, -0.5f, 0.0f}, .texCoords = {0, 0}, .color = {1.0f, 0.0f, 0.0f}},
    {.pos = {0.5f, -0.5f, 0.0f}, .texCoords = {1, 0}, .color = {0.0f, 1.0f, 0.0f}},
    {.pos = {0.5f, 0.5f, 0.0f}, .texCoords = {1, 1}, .color = {0.0f, 0.0f, 1.0f}},
    {.pos = {-0.5f, 0.5f, 0.0f}, .texCoords = {0, 1}, .color = {1.0f, 1.0f, 1.0f}},

    // Bottom square
    {.pos = {-0.5f, -0.5f, -0.5f}, .texCoords = {0, 0}, .color = {1.0f, 0.0f, 0.0f}},
    {.pos = {0.5f, -0.5f, -0.5f}, .texCoords = {1, 0}, .color = {0.0f, 1.0f, 0.0f}},
    {.pos = {0.5f, 0.5f, -0.5f}, .texCoords = {1, 1}, .color = {0.0f, 0.0f, 1.0f}},
    {.pos = {-0.5f, 0.5f, -0.5f}, .texCoords = {0, 1}, .color = {1.0f, 1.0f, 1.0f}},
};

// clang-format off
const std::vector<uint32_t> indices = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4,
};
// clang-format on

class Squares3DApp : public App
{
public:
    Squares3DApp(int argc, char* argv[])
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
            tex = Image::create(state.vk.device, {.load = {.bEnable = true, .path = "textures/forest.png"}});
            sampler = Sampler::create(state.vk.device, {});
        }
        ubo = UniformBufferObject::create(state.vk.device, {.size = sizeof(UboData)});

        pipeline = GraphicsPipeline::create(state.vk.device, {
			.vertex = SimpleVertex::getPipelineInfo(),
			.shaderPath = {
				.vert = "shaders/texture_3d.vert.spv",
				.frag = "shaders/texture_3d.frag.spv",
			},
			.descriptor = {
				.uboBindings = {
					{
						.ubo = ubo,
						.binding = 0u,
					},
				},
				.textureBindings = {
					{
						.image = tex,
						.sampler = sampler,
						.binding = 1u,
					},
				},
			},
		});
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
        ubo.reset();

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

        // ================= Matrices ================== //
        float aspect = static_cast<float>(state.vk.swapChain.extent.width)
                       / static_cast<float>(state.vk.swapChain.extent.height);
        uboData.matrices.proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 10.0f);
        uboData.matrices.proj[1][1] *= -1;
        uboData.matrices.view  = glm::lookAt(glm::vec3(2.0f), glm::vec3(0), glm::vec3(0, 0, 1));
        uboData.matrices.model = glm::mat4(1.0f);
        uboData.tint           = glm::vec3(1.0);
        ubo->setUniforms(uboData);
    }

    virtual InitArgs getInitArgs() const override
    {
        return {
			.appName = "3D Squares Demo",
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

    UboData uboData;
    UniformBufferObject::Ptr ubo;

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
        Squares3DApp {argc, argv}.run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
