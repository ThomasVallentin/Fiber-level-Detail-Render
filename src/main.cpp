#include "SimulationEngine.h"
#include "WrapDeformer.h"
#include "SelfShadows.h"
#include "ShadowMap.h"

#include "Base/Window.h"
#include "Base/Event.h"
#include "Base/Resolver.h"
#include "Base/Shader.h"
#include "Base/Framebuffer.h"
#include "Base/VertexArray.h"
#include "Base/Camera.h"
#include "Base/Profiler.h"
#include "Base/bccReader.h"
#include "Base/Mesh.h"
#include "Base/Math.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtx/string_cast.hpp>

#include <imgui.h>

#include <iostream>


#define SHADOW_MAP_TEXTURE_UNIT 0
#define SELF_SHADOWS_TEXTURE_UNIT 1


// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;

// Camera controls :
// - Pan    : Alt + Middle click + Mouse move
// - Rotate : Alt + Left click   + Mouse move
// - Zoom   : Alt + Right click  + Mouse move (horizontal)
Camera camera;
glm::vec2 mousePos;

// Timing
float deltaTime = 0.0f;
float prevTime = 0.0f;

// Fibers generation parameters
int plyCount = 3;
int fibersCount = 64;
int fibersDivisionCount = 4;
float plyRadius = 0.1f;
glm::vec2 fiberRadius = {0.1f, 0.2f};
float fiberRotation = 1.0f;
glm::vec3 fiberColor = glm::vec3(0.8f);
// Rendering parameters
bool showFibers = true;
bool showClothMesh = false;

bool useAmbientOcclusion = true;

bool useShadowMapping = true;
bool useSelfShadows = true;

float shadowMapThickness = 0.15f;
float selfShadowRotation = 0.0f;

// Lighting parameters
glm::vec3 initLightDirection = glm::normalize(glm::vec3(0.5f, -0.5f, -0.5f));
float lightRotation = 0.0f;
bool animateLightRotation = false;
glm::vec3 backgroundColor = {0.2f, 0.3f, 0.3f};

// Simulation parameters
bool enableSimulation = false;
float fe = 100.0;
float h = 1.0 / fe;


int main(int argc, char *argv[])
{
    auto& resolver = Resolver::Init(fs::weakly_canonical(argv[0])
                                    .parent_path()
                                    .parent_path());

    auto window = Window({SCR_WIDTH, SCR_HEIGHT, "Fiber-Level Detail Render"});
    auto eventCallback = [&](Event* event) {
        switch (event->GetType()) 
        {
            case EventType::WindowResized: {
                auto resizeEvent = dynamic_cast<WindowResizedEvent*>(event);
                glViewport(0, 0, resizeEvent->GetWidth(), resizeEvent->GetHeight());
                camera.SetViewportSize(resizeEvent->GetWidth(), resizeEvent->GetHeight());
                break;
            }
        }

        camera.OnEvent(event);
    };
    window.SetEventCallback(eventCallback);  // Define the event callback of the application

    // Initialize profiler
    auto& profiler = Profiler::Init(window);
    std::vector<ProfilingScopeData> profilingScopes;

    // Read curves from the BCC file and send them to OpenGL
    std::string filename = "resources/fiber.bcc";
    if(argc > 1){
        filename = argv[1];
    }

    fs::path filePath = resolver.Resolve(filename);
    std::vector<fs::path> availableFiles = ListBCCFiles(resolver.Resolve("resources"));

    std::vector<glm::vec3> fibersVertices;
    std::vector<uint32_t> fibersIndices;
    LoadBCCFile(filePath, fibersVertices, fibersIndices);
    VertexArrayPtr fibersVertexArray = LoadBCCToOpenGL(fibersVertices, fibersIndices);
    VertexBufferPtr fibersVertexBuffer = fibersVertexArray->GetVertexBuffers()[0];
    IndexBufferPtr fibersIndexBuffer = fibersVertexArray->GetIndexBuffer();

    glPatchParameteri(GL_PATCH_VERTICES, 4);

    Shader fiberShader(resolver.Resolve("src/shaders/fibers.vs.glsl").c_str(), 
                       resolver.Resolve("src/shaders/fibers.fs.glsl").c_str(),
                       resolver.Resolve("src/shaders/fibers.gs.glsl").c_str(),
                       resolver.Resolve("src/shaders/fibers.tsc.glsl").c_str(),
                       resolver.Resolve("src/shaders/fibers.tse.glsl").c_str());

    // Generate a fabric mesh used to deform the fibers
    std::vector<Vertex> clothVertices;
    std::vector<uint32_t> clothIndices;
    Mesh::BuildPlane(22.0f, 15.0f, 60, 40, clothVertices, clothIndices);
    auto clothVertexBuffer = VertexBuffer::Create(clothVertices.data(), 
                                                  clothVertices.size() * sizeof(Vertex));
    clothVertexBuffer->SetLayout({{"Position",  3, GL_FLOAT, false},
                                  {"Normal",    3, GL_FLOAT, false},
                                  {"TexCoord",  2, GL_FLOAT, false}});
    auto clothIndexBuffer = IndexBuffer::Create(clothIndices.data(), clothIndices.size()); 
    auto clothVertexArray = VertexArray::Create();
    clothVertexArray->Bind();
    clothVertexArray->AddVertexBuffer(clothVertexBuffer);
    clothVertexArray->SetIndexBuffer(clothIndexBuffer);
    clothVertexArray->Unbind();

    // Initialize the simulation engine
    SimulationEngine engine;
    InitClothFromMesh(engine, clothVertices, 60, 40, fe);

    // Initialize the deformer that will wrap the fibers vertices to the simulated mesh
    WrapDeformer wrap;

    // Shadow mapping
    DirectionalLight directional(initLightDirection, {0.8f, 0.8f, 0.8f});
    ShadowMap shadowMap(4096);
    
    Shader lambertShader(resolver.Resolve("src/shaders/default3D.vs.glsl").c_str(), 
                         resolver.Resolve("src/shaders/lambert.fs.glsl").c_str());

    // Self Shadows
    SelfShadowsSettings selfShadowsSettings{512, 16, (uint32_t)plyCount, plyRadius};
    std::shared_ptr<Texture3D> selfShadowsTexture = SelfShadows::GenerateTexture(selfShadowsSettings);    

    glViewport(0, 0, window.GetWidth(), window.GetHeight());
    while (!window.ShouldClose()) {
        // Making a copy of the profiler data to display it in the UI (so that we display the previous frame stats)
        profilingScopes = profiler.GetScopes();
        profiler.Clear();

        float currentTime = static_cast<float>(glfwGetTime());
        deltaTime = currentTime - prevTime;
        prevTime = currentTime;
        
        camera.Update();
 
        if (enableSimulation && (showFibers || showClothMesh))
        {
            // Mesh animation
            const ProfilingScope scope("Mesh animation");  
            
            massSpringGravityWindSolver(engine, h);
            for (int i = 0 ; i < engine.particles.size() ; ++i)
            {
                clothVertices[i].position = engine.particles[i].position;
            }
            Mesh::GenerateNormals(clothVertices, clothIndices);

            clothVertexBuffer->Bind();
            clothVertexBuffer->SetData(clothVertices.data(), 
                                        clothVertices.size() * sizeof(Vertex));
            clothVertexBuffer->Unbind();
        }

            
        if (wrap.IsInitialized() && showFibers)
        {
            // Fibers deformation
            const ProfilingScope scope("Fibers deformation");  

            wrap.Deform(fibersVertices, clothVertices, clothIndices);
            fibersVertexBuffer->Bind();
            fibersVertexBuffer->SetData(fibersVertices.data(), 
                                        fibersVertices.size() * sizeof(glm::vec3));
            fibersVertexBuffer->Unbind();
        }

        glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        {
            // OpenGL Rendering
            const ProfilingScope scope("Render commands");  

            glm::mat4 projMatrix = camera.GetProjectionMatrix();
            glm::mat4 viewMatrix = camera.GetViewMatrix();
            glm::mat4 viewInverseMatrix = glm::inverse(camera.GetViewMatrix());
            glm::mat4 modelMatrix = glm::mat4(1.0f);

            // Update light position
            // directional.SetDirection(camera.GetForwardDirection());
            if (animateLightRotation)
                lightRotation += deltaTime * 25.0f - (lightRotation > 180.0f) * 360.0f;
            directional.SetDirection(glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightRotation), {0.0f, 1.0f, 0.0f}) * glm::vec4(initLightDirection, 1.0f)));

            // Render the shadow map
            if (useShadowMapping)
            {    
                shadowMap.Begin(directional.GetViewMatrix(), directional.GetProjectionMatrix(), shadowMapThickness);
                {
                    // Render all the objects that cast shadows here
                    fibersVertexArray->Bind();
                    glEnable(GL_CULL_FACE);
                    glCullFace(GL_BACK);
                    glDrawElements(GL_PATCHES, fibersIndexBuffer->GetCount(), GL_UNSIGNED_INT, nullptr);
                    glDisable(GL_CULL_FACE);
                    fibersVertexArray->Unbind();
                }
                shadowMap.End();
            }
            else
            {
                shadowMap.Clear();
            }

            if (showFibers)
            {
                fiberShader.use();
                fiberShader.setMat4("uProjMatrix", projMatrix);
                fiberShader.setMat4("uViewMatrix", viewMatrix);
                fiberShader.setMat4("uModelMatrix", modelMatrix);
            
                fibersVertexArray->Bind();

                fiberShader.setInt("uPlyCount", plyCount);
                fiberShader.setInt("uTessLineCount", fibersCount);
                fiberShader.setInt("uTessSubdivisionCount", fibersDivisionCount);

                fiberShader.setFloat("R_ply", plyRadius);
                fiberShader.setFloat("Rmin", fiberRadius.x);
                fiberShader.setFloat("Rmax", fiberRadius.y);
                fiberShader.setFloat("theta", fiberRotation);
                fiberShader.setFloat("s", 2.0f);  // length of rotation
                fiberShader.setFloat("eN", 1.0f); // ellipse scaling factor along Normal
                fiberShader.setFloat("eB", 1.0f); // ellipse scaling factor along Bitangent

                fiberShader.setFloat("R[0]", 0.20f); // distance from fiber i to ply center
                fiberShader.setFloat("R[1]", 0.25f); // distance from fiber i to ply center
                fiberShader.setFloat("R[2]", 0.30f); // distance from fiber i to ply center
                fiberShader.setFloat("R[3]", 0.35f); // distance from fiber i to ply center

                fiberShader.setVec3("uLightDirection", glm::vec3(viewMatrix * glm::vec4(directional.GetDirection(), 0.0)));

                // Fragment related uniforms
                fiberShader.setBool("uUseAmbientOcclusion", useAmbientOcclusion); // distance from fiber i to ply center
                fiberShader.setVec3("fiberColor",fiberColor);
                if (useShadowMapping)
                {
                    fiberShader.setMat4("uViewToLightMatrix", directional.GetProjectionMatrix() * directional.GetViewMatrix() * viewInverseMatrix);
                    shadowMap.GetTexture()->Attach(SHADOW_MAP_TEXTURE_UNIT);
                    fiberShader.setInt("uShadowMap", SHADOW_MAP_TEXTURE_UNIT);
                }
                else 
                {
                    fiberShader.setMat4("uViewToLightMatrix", glm::mat4(0.0));
                    Texture2D::ClearUnit(SHADOW_MAP_TEXTURE_UNIT);
                    fiberShader.setInt("uShadowMap", SHADOW_MAP_TEXTURE_UNIT);
                }

                if (useSelfShadows)
                {
                    selfShadowsTexture->Attach(SELF_SHADOWS_TEXTURE_UNIT);
                    fiberShader.setInt("uSelfShadowsTexture", SELF_SHADOWS_TEXTURE_UNIT);
                    fiberShader.setFloat("uSelfShadowRotation", selfShadowRotation);
                }
                else
                {
                    Texture3D::ClearUnit(SELF_SHADOWS_TEXTURE_UNIT);
                    fiberShader.setInt("uSelfShadowsTexture", SELF_SHADOWS_TEXTURE_UNIT);
                }

                glDrawElements(GL_PATCHES, fibersIndexBuffer->GetCount(), GL_UNSIGNED_INT, nullptr);
                fibersVertexArray->Unbind();
            }

            if (showClothMesh)
            {
                lambertShader.use();
                lambertShader.setMat4("uModelMatrix", glm::mat4(1.0f));
                lambertShader.setMat4("uViewMatrix", viewMatrix);
                lambertShader.setMat4("uProjMatrix", projMatrix);
                lambertShader.setInt("uReceiveShadows", false);

                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                clothVertexArray->Bind();
                glDrawElements(GL_TRIANGLES, clothIndexBuffer->GetCount(), GL_UNSIGNED_INT, nullptr);
                clothVertexArray->Unbind();
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
        }

        {
            const ProfilingScope scope("UI Rendering");  

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            auto& io = ImGui::GetIO();

            ImGui::Begin("Control panel", nullptr);
            {
                ImGui::PushItemWidth(100.0f);

                if (ImGui::CollapsingHeader("Profiling", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    indentedLabel("FPS :");
                    ImGui::SameLine();
                    ImGui::Text("%.1f (%.3fms)", io.Framerate, 1000.0f / io.Framerate);
                
                    for (const auto& scope : profilingScopes)
                    {
                        float elapsedTime = scope.duration * 1000.0;
                        indentedLabel((scope.name + " :").c_str());
                        ImGui::SameLine();
                        ImGui::BeginDisabled();
                        ImGui::DragFloat((std::string("##") + scope.name + "TimeDrag").c_str(), &elapsedTime, 1.0f, 0.0f, 0.0f, "%.3fms");
                        ImGui::EndDisabled();
                    }
                    
                    ImGui::Spacing();
                }
                
                if (ImGui::CollapsingHeader("Fiber generation settings", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    indentedLabel("File name :");
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth((ImGui::GetWindowContentRegionWidth() - ImGui::GetCursorPosX()) * 0.75f);
                    if (ImGui::BeginCombo("##FileChoiceCombo", filePath.filename().c_str()))
                    {
                        for (const auto& path : availableFiles)
                        {
                            if (ImGui::Selectable(path.filename().c_str(), filePath.filename() == path.filename()))
                            {
                                filePath = path;
                                LoadBCCFile(filePath, fibersVertices, fibersIndices);
                                fibersVertexBuffer->Bind();
                                fibersVertexBuffer->SetData(fibersVertices.data(), 
                                                            fibersVertices.size() * sizeof(glm::vec3));
                                fibersVertexBuffer->Unbind();
                                fibersIndexBuffer->Bind();
                                fibersIndexBuffer->SetData(fibersIndices.data(), fibersIndices.size());
                                fibersIndexBuffer->Unbind();
                                if (wrap.IsInitialized())
                                {
                                    wrap.Initialize(fibersVertices, clothVertices, clothIndices);
                                }
                            }
                        }

                        ImGui::EndCombo();
                    }

                    indentedLabel("Ply count :");
                    ImGui::SameLine();
                    if (ImGui::DragInt("##PlyCountDrag", &plyCount, 0.1f, 0, 10, 
                                       plyCount > 1 ? "%d ply" : "%d plies"))

                    {
                        selfShadowsSettings.plyCount = plyCount;
                        selfShadowsTexture = SelfShadows::GenerateTexture(selfShadowsSettings);    
                    }

                    indentedLabel("Fibers count :");
                    ImGui::SameLine();
                    if (ImGui::DragInt("##FibersCountDrag", &fibersCount, 0.1f, plyCount, 64, 
                                   fibersCount > 1 ? "%d fibers" : "%d fiber"))
                    {
                        fibersCount = std::max(plyCount, fibersCount);
                    }

                    indentedLabel("Fibers divisions :");
                    ImGui::SameLine();
                    if (ImGui::DragInt("##FibersDivisonDrag", &fibersDivisionCount, 0.1f, 2, 64, 
                                       fibersDivisionCount > 1 ? "%d divisions" : "%d division"))
                    {
                        fibersDivisionCount = std::max(2, fibersDivisionCount);
                    }

                    indentedLabel("Ply radius :");
                    ImGui::SameLine();
                    ImGui::DragFloat("##PlyRadiusDrag", &plyRadius, 0.01f, 0.0f, 5.0f, "%.2f", ImGuiSliderFlags_Logarithmic);

                    indentedLabel("Fibers radius :");
                    ImGui::SameLine();
                    ImGui::DragFloat2("##FibersRadiusDrag", &fiberRadius.x, 0.01f, 0.0f, 5.0f, "%.2f", ImGuiSliderFlags_Logarithmic);

                    indentedLabel("Fibers rotation :");
                    ImGui::SameLine();
                    ImGui::DragFloat("##FibersRotationDrag", &fiberRotation, 0.01f, -5.0f, 5.0f, "%.2f");
                }

                if (ImGui::CollapsingHeader("Render settings", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    indentedLabel("Show fibers :");
                    ImGui::SameLine();
                    ImGui::Checkbox("##ShowFibersCB", &showFibers);

                    indentedLabel("Ambient occlusion :");
                    ImGui::SameLine();
                    ImGui::Checkbox("##UseAmbientOcclusion", &useAmbientOcclusion);

                    indentedLabel("Self Shadows :");
                    ImGui::SameLine();
                    ImGui::Checkbox("##UseSelfShadows", &useSelfShadows);

                    indentedLabel("Shadow Mapping :");
                    ImGui::SameLine();
                    ImGui::Checkbox("##UseShadowMapping", &useShadowMapping);
                
                    ImGui::BeginDisabled(!useShadowMapping);
                    indentedLabel("Shadow Map Thickess :");
                    ImGui::SameLine();
                    ImGui::DragFloat("##ShadowMapThicknessSlider", &shadowMapThickness, 0.001f, 0.0f, 1.0);
                    ImGui::EndDisabled();

                    indentedLabel("Background Color :");
                    ImGui::SameLine();
                    ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth() - ImGui::GetCursorPosX());
                    ImGui::ColorEdit3("##BackgroundColorSlider", &backgroundColor.r, ImGuiColorEditFlags_Float);
                    ImGui::PopItemWidth();

                    indentedLabel("Fibers Color :");
                    ImGui::SameLine();
                    ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth() - ImGui::GetCursorPosX());
                    ImGui::ColorEdit3("##FiberColorSlider", &fiberColor.r, ImGuiColorEditFlags_Float);
                    ImGui::PopItemWidth();

                    ImGui::Spacing();
                }

                if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    indentedLabel("Light Orientation :");
                    ImGui::SameLine();
                    if (ImGui::DragFloat("##LightOrientation", &lightRotation, 0.5f, -180.1f, 180.1f, "%.1f"))
                    {
                        if (lightRotation < -180.0f) lightRotation += 360.0f;
                        else if (lightRotation > 180.0f)  lightRotation -= 360.0f;
                    }
                    
                    indentedLabel("Animated light :");
                    ImGui::SameLine();
                    ImGui::Checkbox("##AnimatedLightCheckBox", &animateLightRotation);

                    ImGui::Spacing();
                }

                if (ImGui::CollapsingHeader("Simulation", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    indentedLabel("Enable simulation :");
                    ImGui::SameLine();
                    if (ImGui::Checkbox("##EnableSimulationCB", &enableSimulation))
                    {
                        if (!wrap.IsInitialized())
                            wrap.Initialize(fibersVertices, clothVertices, clothIndices);
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Reset##Simulation"))
                    {
                        Mesh::BuildPlane(22.0f, 15.0f, 60, 40, clothVertices, clothIndices);
                        InitClothFromMesh(engine, clothVertices, 60, 40, fe);
                    }

                    indentedLabel("Show simulation mesh :");
                    ImGui::SameLine();
                    ImGui::Checkbox("##ShowSimulationMeshCB", &showClothMesh);

                    indentedLabel("Smoothing iterations :");
                    ImGui::SameLine();
                    int iterations = wrap.GetSmoothIterations();
                    if (ImGui::DragInt("##SmoothIterationDrag", &iterations, 0.1f, 0, 10, "%d steps"))
                        wrap.SetSmoothIterations(iterations);

                    ImGui::Spacing();
                }

                ImGui::PopItemWidth();
                ImGui::End();
            }

            // Render ImGui items
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        {
            // It's the SwapBuffers that actually compute the rendering, not the calls to glXXX commands
            // recording its computation time
            const ProfilingScope scope("OpenGL Rendering");  
            window.Update();
        }
    }

    return 0;
}
