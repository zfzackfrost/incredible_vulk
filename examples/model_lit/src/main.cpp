#include <ivulk/glm.hpp>

#include <ivulk/core/app.hpp>
#include <ivulk/core/buffer.hpp>
#include <ivulk/core/graphics_pipeline.hpp>
#include <ivulk/core/image.hpp>
#include <ivulk/core/sampler.hpp>
#include <ivulk/core/texture.hpp>
#include <ivulk/core/uniform_buffer.hpp>
#include <ivulk/core/vertex.hpp>
#include <ivulk/render/renderer.hpp>
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

class ModelLitApp : public App
{
public:
    ModelLitApp(int argc, char* argv[]) // NOLINT
        : App(argc, argv)
    {
        elapsedTime     = 0.0f;
        timeSinceStatus = std::numeric_limits<float>::max();
        clearColor      = glm::vec4(0, 0, 0, 1);
    }

protected:
    void loadTextures()
    {
        dirtyMetal.albedo    = Image::create(state.vk.device,
                                          {

                                              .load = {
                                                  .bEnable = true,
                                                  .path    = "textures/DirtyMetal/albedo.png",
                                                  .bSrgb   = true,
                                              }});
        dirtyMetal.metallic  = Image::create(state.vk.device,
                                            {

                                                .load = {
                                                    .bEnable = true,
                                                    .path    = "textures/DirtyMetal/metallic.png",
                                                    .bSrgb   = false,
                                                }});
        dirtyMetal.roughness = Image::create(state.vk.device,
                                             {

                                                 .load = {
                                                     .bEnable = true,
                                                     .path    = "textures/DirtyMetal/roughness.png",
                                                     .bSrgb   = false,
                                                 }});
        dirtyMetal.normal    = Image::create(state.vk.device,
                                          {

                                              .load = {
                                                  .bEnable = true,
                                                  .path    = "textures/DirtyMetal/normal.png",
                                                  .bSrgb   = false,
                                              }});


    }

    void createOffscreen()
    {
        offscreen.color = Image::create(state.vk.device, {
            .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
            .format = VK_FORMAT_R8G8B8A8_SNORM,
            .extent = {
                .width = state.vk.swapChain.extent.width,
                .height = state.vk.swapChain.extent.height,
                .depth = 1,
            },
        });
        offscreen.depth = Image::create(state.vk.device, {
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
            .format = VK_FORMAT_D32_SFLOAT,
            .extent = {
                .width = state.vk.swapChain.extent.width,
                .height = state.vk.swapChain.extent.height,
                .depth = 1,
            },
        });
    }
    void createDirtyMetalPipeline()
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
						.ubo     = uboScene,
						.binding = 1u,
					},
				},
                .textureBindings = {
                    {
                        .image = dirtyMetal.albedo,
                        .sampler = sampler,
                        .binding = 4,
                    },
                    {
                        .image = dirtyMetal.normal,
                        .sampler = sampler,
                        .binding = 5u,
                    },
                    {
                        .image = dirtyMetal.metallic,
                        .sampler = sampler,
                        .binding = 6u,
                    },
                    {
                        .image = dirtyMetal.roughness,
                        .sampler = sampler,
                        .binding = 7u,
                    },
                },
			},
		};
        auto& pipeline = dirtyMetal.pipeline;
        if (pipeline)
        {
            pipeline->recreate(createInfo);
        }
        else
        {
            pipeline = GraphicsPipeline::create(state.vk.device, createInfo);
        }
    }
    void initialize(bool swapchainOnly) override
    {
        if (!swapchainOnly)
        {
            loadTextures();
            sampler  = Sampler::create(state.vk.device, {});
            renderer = Renderer::create<Renderer>(this);
        }
        uboMatrices = UniformBufferObject::create(state.vk.device, {.size = sizeof(MatricesUBO)});
        uboScene    = UniformBufferObject::create(state.vk.device, {.size = sizeof(SceneUBOData)});

        createOffscreen();
        createDirtyMetalPipeline();

        state.vk.pipelines.mainGfx = dirtyMetal.pipeline;
        renderer->renderSwapchain();
        // renderer->renderOffscreen({
                    // .renderContext = dirtyMetal.pipeline,
                    // .attachments = {
                        // offscreen.color,
                        // offscreen.depth,
                    // },
                    // .width = state.vk.swapChain.extent.width,
                    // .height = state.vk.swapChain.extent.height,
            // });
        // Skip anything that doesn't depend on the swapchain, if requested
        if (swapchainOnly)
            return;

        renderer->activate();

        sphereModel = StaticModel::load("models/unitsphere.fbx");

        EventManager::addCallback(E_EventType::KeyDown, [this](Event evt) { escapeKeyQuit(evt); });
        std::vector<GraphicsPipeline::Ref> spherePipelines = {dirtyMetal.pipeline};
        scene                                              = Scene::create();
        sphere1                                            = scene->addRenderable(
            RenderableInstance::create(sphereModel, _priority = 0, _pipelines = spherePipelines));
        sphere2 = scene->addRenderable(
            RenderableInstance::create(sphereModel, _priority = 0, _pipelines = spherePipelines));
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

    void cleanup(bool swapchainOnly) override
    {

        // Skip anything that doesn't depend on the swapchain, if requested
        if (swapchainOnly)
            return;

        scene.reset();
        sphereModel.reset();
        uboMatrices.reset();
        uboScene.reset();
        sampler.reset();

        // Don't destroy pipelines when recreating swapchain
        // ... instead destroy at the end of the session and
        //     use the `recreate` method of pipelines when
        //     recreating the swapchain.
        dirtyMetal.pipeline.reset();
    }

    void render(CommandBuffers::Ref cmdBuffer) override
    {
        if (auto cb = cmdBuffer.lock())
        {
            cb->clearAttachments(dirtyMetal.pipeline, {.color = clearColor});
            scene->render(cb);
        }
    }

    void update(float deltaSeconds) override
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
        rotRate         = glm::radians(glm::mix(100.0f, 200.0f, rotRate));
        auto deltaEuler = glm::vec3(deltaSeconds * rotRate);
        deltaEuler.x    = 0.0f;
        // _sphere1->transform.rotation *= glm::quat(deltaEuler);
        // _sphere1->transform.translate.x = meter {glm::sin((elapsedTime / 2.0f) * glm::two_pi<float>())};

        // float theta                     = (elapsedTime / 2.5f) * glm::two_pi<float>();
        // _sphere2->transform.translate.x = meter {glm::cos(theta)} + _sphere1->transform.translate.x;
        // _sphere2->transform.translate.y = meter {glm::sin(theta)} + _sphere1->transform.translate.y;
        // _sphere2->transform.translate.z = _sphere1->transform.translate.z;
        _sphere2->transform.scale = glm::vec3(0.35);

        // ================= Matrices ================== //

        float aspect = static_cast<float>(state.vk.swapChain.extent.width)
                       / static_cast<float>(state.vk.swapChain.extent.height);

        viewXform     = viewXform.withLookAt(viewPos, {{}, {}, {}});
        matrices.view = viewXform.viewMatrix();

        matrices.proj = glm::perspective(glm::radians(45.0f), aspect, 0.0001f, 1000.0f);
        matrices.proj[1][1] *= -1.0f;

        uboMatrices->setUniforms(matrices);

        // ================== Lights =================== //

        sceneData.dirLights[0].color     = glm::vec3(0.6);
        sceneData.dirLights[0].direction = glm::normalize(glm::vec3(0.1, 0.1, -1));
        sceneData.dirLightCount          = 0;

        sceneData.pointLights[0].color       = glm::vec3(4);
        sceneData.pointLights[0].position    = glm::vec3(0, 2, 0.1);
        sceneData.pointLights[0].attenuation = PointLightAttenuation::fromRadius(2.0f);
        sceneData.pointLightCount            = 1;

        sceneData.viewPosition = viewPos.toVec();

        uboScene->setUniforms(sceneData);
    }

    [[nodiscard]] InitArgs getInitArgs() const override
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

    int32_t rateDeviceSuitability(vk::PhysicalDevice device) override
    {
        auto deviceProperties = device.getProperties();
        // auto deviceFeatures = device.getFeatures();
        int32_t result = 0;
        if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
            result += 1000;
        return result;
    }

    boost::filesystem::path getAssetsDir() override
    {
        boost::filesystem::path execPath {state.cmdArgs[0]};
        execPath = boost::filesystem::absolute(boost::filesystem::system_complete(execPath));

        auto execDir = execPath.parent_path();
        return execDir / "assets";
    }

    Transform viewXform;

    struct
    {
        Image::Ptr albedo;
        Image::Ptr metallic;
        Image::Ptr normal;
        Image::Ptr roughness;
        GraphicsPipeline::Ptr pipeline;
    } dirtyMetal;

    struct
    {
        Image::Ptr color;
        Image::Ptr depth;
    } offscreen;

    StaticModel::Ptr sphereModel;

    Sampler::Ptr sampler;

    Scene::Ptr scene;
    RenderableInstance::Ref sphere1;
    RenderableInstance::Ref sphere2;

    UniformBufferObject::Ptr uboMatrices;
    MatricesUBO matrices;

    UniformBufferObject::Ptr uboScene;
    SceneUBOData sceneData;

    Renderer::Ptr renderer;

    glm::vec4 clearColor;

    float elapsedTime;
    float timeSinceStatus;

    glm::vec2 dirKeysInput;
    Position viewPos = {meter {4}, meter {4}, meter {2}};
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
