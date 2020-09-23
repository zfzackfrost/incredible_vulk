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
#include <ivulk/render/renderer.hpp>

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
    void createSpherePipeline(bool top)
    {
        GraphicsPipelineInfo createInfo {
			.vertex = StaticMeshVertex::getPipelineInfo(),
			.shaderPath = {
				.vert = "shaders/sphere.vert.spv",
				.frag = (top) ? "shaders/sphere_top.frag.spv" : "shaders/sphere_bottom.frag.spv",
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
			},
		};
        auto& pipeline = (top) ? topPipeline : bottomPipeline;
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
            sampler = Sampler::create(state.vk.device, {});
        }
        uboMatrices = UniformBufferObject::create(state.vk.device, {.size = sizeof(MatricesUBO)});
        uboScene    = UniformBufferObject::create(state.vk.device, {.size = sizeof(SceneUBOData)});

        createSpherePipeline(true);
        createSpherePipeline(false);

        state.vk.pipelines.mainGfx = topPipeline;

        // Skip anything that doesn't depend on the swapchain, if requested
        if (swapchainOnly)
            return;

        renderer = Renderer::create<Renderer>(this);
        renderer->activate();

        sphereModel = StaticModel::load("models/unitsphere_splitmat.fbx");

        EventManager::addCallback(E_EventType::KeyDown, [this](Event evt) { escapeKeyQuit(evt); });
        std::vector<GraphicsPipeline::Ref> spherePipelines = {
            topPipeline,
            bottomPipeline,
        };
        scene   = Scene::create();
        sphere1 = scene->addRenderable(
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
        topPipeline.reset();
        bottomPipeline.reset();
    }

    void render(CommandBuffers::Ref cmdBuffer) override
    {
        if (auto cb = cmdBuffer.lock())
        {
            cb->clearAttachments(topPipeline, {.color = clearColor});
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
        _sphere1->transform.rotation *= glm::quat(deltaEuler);
        _sphere1->transform.translate.x = meter {glm::sin((elapsedTime / 2.0f) * glm::two_pi<float>())};

        float theta                     = (elapsedTime / 2.5f) * glm::two_pi<float>();
        _sphere2->transform.translate.x = meter {glm::cos(theta)} + _sphere1->transform.translate.x;
        _sphere2->transform.translate.y = meter {glm::sin(theta)} + _sphere1->transform.translate.y;
        _sphere2->transform.translate.z = _sphere1->transform.translate.z;
        _sphere2->transform.scale       = glm::vec3(0.35);

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

        sceneData.pointLights[0].color       = glm::vec3(0.5);
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

    StaticModel::Ptr sphereModel;

    GraphicsPipeline::Ptr topPipeline, bottomPipeline;
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
