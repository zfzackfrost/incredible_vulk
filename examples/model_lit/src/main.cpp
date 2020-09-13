#include <ivulk/glm.hpp>

#include <ivulk/core/app.hpp>
#include <ivulk/core/buffer.hpp>
#include <ivulk/core/graphics_pipeline.hpp>
#include <ivulk/core/image.hpp>
#include <ivulk/core/sampler.hpp>
#include <ivulk/core/texture.hpp>
#include <ivulk/core/uniform_buffer.hpp>
#include <ivulk/core/vertex.hpp>
#include <ivulk/render/transform.hpp>

#include <ivulk/render/model/static_model.hpp>
#include <ivulk/render/renderable_instance.hpp>
#include <ivulk/render/scene.hpp>
#include <ivulk/render/standard_shader.hpp>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>

using namespace ivulk;

struct PointLight
{
    LAYOUT_VEC3 glm::vec3 position = glm::vec3(0, 0, 0);
    LAYOUT_VEC3 glm::vec3 color    = glm::vec3(1.0f, 1.0f, 1.0f);

    LAYOUT_FLOAT float constant  = 1.0f;
    LAYOUT_FLOAT float linear    = 0.22f;
    LAYOUT_FLOAT float quadratic = 0.2f;
};
struct DirectionLight
{
    LAYOUT_VEC3 glm::vec3 direction;
    LAYOUT_VEC3 glm::vec3 color;
};
struct LightingUBOData
{
    LAYOUT_STRUCT PointLight pointLights[4];
    LAYOUT_STRUCT DirectionLight dirLights[4];

    LAYOUT_INT int32_t pointLightCount = 0;
    LAYOUT_INT int32_t dirLightCount   = 0;

    LAYOUT_VEC3 glm::vec3 viewPos;
};

class ModelLitApp : public App
{
public:
    ModelLitApp(int argc, char* argv[])
        : App(argc, argv)
    {
        elapsedTime     = 0.0f;
        timeSinceStatus = std::numeric_limits<float>::max();
        clearColor      = glm::vec4(0, 0, 0, 1);
    }

protected:
    void createWoodPipeline()
    {
        GraphicsPipelineInfo createInfo {
			.vertex = StaticMeshVertex::getPipelineInfo(),
			.shaderPath = {
				.vert = "shaders/sphere.vert.spv",
				.frag = "shaders/sphere.frag.spv",
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
				.textureBindings = {
					{
						.image = woodDiffuseTex,
						.sampler = sampler,
						.binding = 4u,
					},
					{
						.image = woodSpecularTex,
						.sampler = sampler,
						.binding = 5u,
					},
					{
						.image = woodNormalTex,
						.sampler = sampler,
						.binding = 6u,
					},
				}
			},
		};
        if (woodPipeline)
        {
            woodPipeline->recreate(createInfo);
        }
        else
        {
            woodPipeline = GraphicsPipeline::create(state.vk.device, createInfo);
        }
    }
    virtual void initialize(bool swapchainOnly) override
    {
        if (!swapchainOnly)
        {
            sampler = Sampler::create(state.vk.device, {});
            woodDiffuseTex  = Image::create(state.vk.device,
                                           {
                                               .load = {
                                                   .bEnable = true,
                                                   .path    = "textures/Wood/Wood_diffuse512.png",
                                               },
                                           });
            woodSpecularTex = Image::create(state.vk.device,
											{
												.load = {
													.bEnable = true,
													.path    = "textures/Wood/Wood_specular256.png",
													.bSrgb   = false,
												},
											});
            woodNormalTex   = Image::create(state.vk.device,
					{
						.load = {
						.bEnable = true,
						.path    = "textures/Wood/Wood_normal512.png",
						.bSrgb   = false,
						},
					});
        }
        uboMatrices = UniformBufferObject::create(state.vk.device, {.size = sizeof(MatricesUBO)});
        uboLighting = UniformBufferObject::create(state.vk.device, {.size = sizeof(LightingUBOData)});

        createWoodPipeline();
        state.vk.pipelines.mainGfx = std::weak_ptr<GraphicsPipeline>(woodPipeline);

        // Skip anything that doesn't depend on the swapchain, if requested
        if (swapchainOnly)
            return;

        sphereModel = StaticModel::load("models/unitsphere.fbx");

        EventManager::addCallback(E_EventType::KeyDown,
                                  std::bind(&ModelLitApp::escapeKeyQuit, this, std::placeholders::_1));

        scene   = Scene::create();
        sphere1 = scene->addRenderable(
            RenderableInstance::create(sphereModel, _priority = 0, _pipeline = woodPipeline));
        sphere2 = scene->addRenderable(
            RenderableInstance::create(sphereModel,
                                       _priority = 0,
                                       _pipeline = woodPipeline));
    }

    void escapeKeyQuit(Event evt)
    {
        auto e = evt.assumeKeyEvent();
        if (e.bRepeat)
            return;
        if (e.keycode != E_KeyCode::KeyEsc)
            return;
        quit();
    }
    void dirKeys(Event evt)
    {
        auto e = evt.assumeKeyEvent();
        if (e.bRepeat)
            return;
        auto mult = e.bIsDown ? 1.0f : -1.0f;

        if (e.keycode == E_KeyCode::KeyDownArrow || e.keycode == E_KeyCode::KeyS)
            dirKeysInput += glm::vec2(0, -1) * mult;
        if (e.keycode == E_KeyCode::KeyUpArrow || e.keycode == E_KeyCode::KeyW)
            dirKeysInput += glm::vec2(0, +1) * mult;
        if (e.keycode == E_KeyCode::KeyLeftArrow || e.keycode == E_KeyCode::KeyA)
            dirKeysInput += glm::vec2(-1, 0) * mult;
        if (e.keycode == E_KeyCode::KeyRightArrow || e.keycode == E_KeyCode::KeyD)
            dirKeysInput += glm::vec2(+1, 0) * mult;
    }

    virtual void cleanup(bool swapchainOnly) override
    {

        // Skip anything that doesn't depend on the swapchain, if requested
        if (swapchainOnly)
            return;

        scene.reset();
        sphereModel.reset();
        uboMatrices.reset();
        uboLighting.reset();
        woodDiffuseTex.reset();
        woodNormalTex.reset();
        woodSpecularTex.reset();
        sampler.reset();
        woodPipeline.reset();
    }

    virtual void render(CommandBuffers::Ref cmdBuffer) override
    {
        if (auto cb = cmdBuffer.lock())
        {
            cb->clearAttachments(woodPipeline, clearColor);
            scene->render(cb);
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

        auto _sphere1 = sphere1.lock();
        if (!_sphere1)
            return;
        auto _sphere2 = sphere2.lock();
        if (!_sphere2)
            return;

        float rotRate = glm::sin((elapsedTime / glm::two_over_pi<float>()) * glm::two_pi<float>()) * 0.5
                        + 0.5;
        rotRate              = glm::radians(glm::mix(100.0f, 200.0f, rotRate));
        glm::vec3 deltaEuler = glm::vec3(deltaSeconds * rotRate);
        deltaEuler.x         = 0.0f;
        _sphere1->transform.rotation *= glm::quat(deltaEuler);
        _sphere1->transform.translate.x = glm::sin((elapsedTime / 2.0f) * glm::two_pi<float>());
        
        _sphere2->transform.translate.x = -1;
        _sphere2->transform.translate.y = -1.5;
        _sphere2->transform.translate.z = glm::sin((elapsedTime / 2.0f) * glm::two_pi<float>());

        // ================= Matrices ================== //

        float aspect = static_cast<float>(state.vk.swapChain.extent.width)
                       / static_cast<float>(state.vk.swapChain.extent.height);

        viewXform     = viewXform.withLookAt(viewPos, {0, 0, 0});
        matrices.view = viewXform.viewMatrix();

        matrices.proj = glm::perspective(glm::radians(45.0f), aspect, 0.0001f, 1000.0f);
        matrices.proj[1][1] *= -1.0f;

        uboMatrices->setUniforms(matrices);

        // ================== Lights =================== //

        lighting.dirLights[0].color     = glm::vec3(0.6);
        lighting.dirLights[0].direction = glm::normalize(glm::vec3(0.1, 0.1, -1));
        lighting.dirLightCount          = 1;

        lighting.pointLights[0].color    = glm::vec3(0.5);
        lighting.pointLights[0].position = glm::vec3(0, 2, 0.1);
        lighting.pointLightCount         = 1;

        lighting.viewPos = viewPos;

        uboLighting->setUniforms(lighting);
    }

    virtual InitArgs getInitArgs() const override
    {
        return {
			.appName = "Lit Sphere Demo",
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

    Transform viewXform;

    StaticModel::Ptr sphereModel;

    GraphicsPipeline::Ptr woodPipeline;
    Image::Ptr woodDiffuseTex;
    Image::Ptr woodSpecularTex;
    Image::Ptr woodNormalTex;
    Sampler::Ptr sampler;

    Scene::Ptr scene;
    RenderableInstance::Ref sphere1;
    RenderableInstance::Ref sphere2;

    UniformBufferObject::Ptr uboMatrices;
    MatricesUBO matrices;

    UniformBufferObject::Ptr uboLighting;
    LightingUBOData lighting;

    glm::vec4 clearColor;

    float elapsedTime;
    float timeSinceStatus;

    glm::vec2 dirKeysInput;
    glm::vec3 viewPos = {4, 2, 2};
};

int main(int argc, char* argv[])
{
    try
    {
        ModelLitApp {argc, argv}.run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
