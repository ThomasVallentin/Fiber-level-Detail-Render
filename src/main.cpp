#include "ParticleSystem.h"
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



#define SHADOW_MAP_TEXTURE_UNIT 0
#define SELF_SHADOWS_TEXTURE_UNIT 1



// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;

// Camera controls :
// - Pan    : Alt + Middle click + Mouse move
// - Rotate : Alt + Left click   + Mouse move
// - Zoom   : Alt + Right click  + Mouse move (horizontal)
Camera camera;
glm::vec2 mousePos;

// Timing
float deltaTime = 0.0f;
float prevTime = 0.0f;

// Simulation parameters
float fe = 100.0;
float h = 1.0 / fe;

// Rendering parameters
bool showFibers = true;
bool showClothMesh = false;
bool showFloor = false;

bool useAmbientOcclusion = true;

bool useShadowMapping = true;
bool useSelfShadows = true;

float shadowMapThickness = 0.15f;

glm::vec3 initLightDirection = glm::normalize(glm::vec3(0.5f, -0.5f, -0.5f));
float lightRotation = 0.0f;
bool animateLightRotation = false;

float selfShadowRotation = 0.0f;

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

    // Read curves from the BCC file and send them to OpenGL
    std::vector<std::vector<glm::vec3>> closedFibersCP;
    std::vector<std::vector<glm::vec3>> openFibersCP;
    readBCC(resolver.Resolve("resources/fiber.bcc"), closedFibersCP, openFibersCP);

    // Merge all the curves into a single vector to draw all of them in a single drawcall
    std::vector<glm::vec3> controlPoints;
    for (const auto& fiber : closedFibersCP)
    {
        for (const auto& cPoints : fiber)
            controlPoints.push_back(cPoints);
        controlPoints.push_back(fiber.front());
    }
    for (const auto& fiber : openFibersCP)
        for (const auto& cPoints : fiber)
            controlPoints.push_back(cPoints);

    // Send the data to OpenGL
    uint32_t fibersVertexCount = controlPoints.size();
    auto fibersVertexBuffer = VertexBuffer::Create(controlPoints.data(), 
                                                   fibersVertexCount * sizeof(glm::vec3));
    fibersVertexBuffer->SetLayout({{"Position",  3, GL_FLOAT, false}});

    std::vector<uint32_t> indices;
    for (size_t i = 0 ; i < fibersVertexCount - 3 ; i++)
    {
        indices.push_back(i);
        indices.push_back(i+1);
        indices.push_back(i+2);
        indices.push_back(i+3);
    }
    auto fibersIndexBuffer = IndexBuffer::Create(indices.data(), 
                                                 indices.size());

    auto fibersVertexArray = VertexArray::Create();
    fibersVertexArray->Bind();
    fibersVertexArray->AddVertexBuffer(fibersVertexBuffer);
    fibersVertexArray->SetIndexBuffer(fibersIndexBuffer);
    fibersVertexArray->Unbind();
    
    glPatchParameteri(GL_PATCH_VERTICES, 4);

    Shader fiberShader(resolver.Resolve("src/shaders/fibers.vs.glsl").c_str(), 
                       resolver.Resolve("src/shaders/fibers.fs.glsl").c_str(),
                       resolver.Resolve("src/shaders/fibers.gs.glsl").c_str(),
                       resolver.Resolve("src/shaders/fibers.tsc.glsl").c_str(),
                       resolver.Resolve("src/shaders/fibers.tse.glsl").c_str());

    // Fabric mesh used to deform the fibers
    std::vector<Vertex> clothVertices;

    std::vector<uint32_t> clothIndices;
    Mesh::BuildPlane(22.0f, 15.0f, 60, 40, clothVertices, clothIndices);    
    ParticleSystem partSys;
    InitClothFromMesh(partSys, clothVertices, 60, 40, fe);

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

    WrapDeformer wrap;
    wrap.Initialize(controlPoints, clothVertices, clothIndices);

    // Shadow mapping
    DirectionalLight directional(initLightDirection, {0.8f, 0.8f, 0.8f});
    ShadowMap shadowMap(4096);
    
    Shader blitChannelShader(resolver.Resolve("src/shaders/utility/fullScreen.vs.glsl").c_str(), 
                             resolver.Resolve("src/shaders/utility/blitTextureChannel.fs.glsl").c_str());

    // Simple floor to visualize the casted shadows
    std::vector<Vertex> floorVertices = {{{-10.0, 0.0, -10.0}, {0.0, 1.0, 0.0}, {0.0, 0.0}},
                                         {{-10.0, 0.0,  10.0}, {0.0, 1.0, 0.0}, {0.0, 1.0}},
                                         {{ 10.0, 0.0,  10.0}, {0.0, 1.0, 0.0}, {1.0, 1.0}},
                                         {{ 10.0, 0.0, -10.0}, {0.0, 1.0, 0.0}, {1.0, 0.0}}};
    std::vector<uint32_t> floorIndices = {0, 1, 2, 2, 3, 0};
    auto floorVertexBuffer = VertexBuffer::Create(floorVertices.data(), 
                                                  floorVertices.size() * sizeof(Vertex));
    floorVertexBuffer->SetLayout({{"Position",  3, GL_FLOAT, false},
                                  {"Normal",    3, GL_FLOAT, false},
                                  {"TexCoord",  2, GL_FLOAT, false}});
    auto floorIndexBuffer = IndexBuffer::Create(floorIndices.data(), floorIndices.size()); 
    auto floorVertexArray = VertexArray::Create();
    floorVertexArray->Bind();
    floorVertexArray->AddVertexBuffer(floorVertexBuffer);
    floorVertexArray->SetIndexBuffer(floorIndexBuffer);
    floorVertexArray->Unbind();

    Shader lambertShader(resolver.Resolve("src/shaders/default3D.vs.glsl").c_str(), 
                       resolver.Resolve("src/shaders/lambert.fs.glsl").c_str());

    // Self Shadows
    SelfShadowsSettings selfShadowsSettings{512, 16};
    std::shared_ptr<Texture3D> selfShadowsTexture = SelfShadows::GenerateTexture(selfShadowsSettings);    

    // Shader slice3DShader(resolver.Resolve("src/shaders/utility/fullScreen.vs.glsl").c_str(),
    //                      resolver.Resolve("src/shaders/utility/3DTextureSlice.fs.glsl").c_str());

    glViewport(0, 0, window.GetWidth(), window.GetHeight());
    auto& profiler = Profiler::Init(window);
    std::vector<ProfilingScopeData> profilingScopes;
    while (!window.ShouldClose()) {
        // Making a copy of the profiler data to display it in the UI (so that we display the previous frame stats)
        profilingScopes = profiler.GetScopes();
        profiler.Clear();

        float currentTime = static_cast<float>(glfwGetTime());
        deltaTime = currentTime - prevTime;
        prevTime = currentTime;
        
        camera.Update();
 
        {
            // Mesh animation
            const ProfilingScope scope("Mesh animation");  

            if (showFibers || showClothMesh)
            {
                massSpringGravityWindSolver(partSys, h);
                for (int i = 0 ; i < partSys.particles.size() ; ++i)
                {
                    clothVertices[i].position = partSys.particles[i].position;
                }
                Mesh::GenerateNormals(clothVertices, clothIndices);

                clothVertexBuffer->Bind();
                clothVertexBuffer->SetData(clothVertices.data(), 
                                            clothVertices.size() * sizeof(Vertex));
                clothVertexBuffer->Unbind();
            }
        }

        {
            // Fibers deformation
            const ProfilingScope scope("Fibers deformation");  
            
            if (showFibers)
            {
                wrap.Deform(controlPoints, clothVertices, clothIndices);
                fibersVertexBuffer->Bind();
                fibersVertexBuffer->SetData(controlPoints.data(), 
                                            fibersVertexCount * sizeof(glm::vec3));
                fibersVertexBuffer->Unbind();
            }
        }  

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
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

                fiberShader.setFloat("R_ply", 0.1f);
                fiberShader.setFloat("Rmin", 0.1f);
                fiberShader.setFloat("Rmax", 0.2f);
                fiberShader.setFloat("theta", 1.0f);
                fiberShader.setFloat("s", 2.0f);  // length of rotation
                fiberShader.setFloat("eN", 1.0f); // ellipse scaling factor along Normal
                fiberShader.setFloat("eB", 1.0f); // ellipse scaling factor along Bitangent

                fiberShader.setFloat("R[0]", 0.20f); // distance from fiber i to ply center
                fiberShader.setFloat("R[1]", 0.25f); // distance from fiber i to ply center
                fiberShader.setFloat("R[2]", 0.30f); // distance from fiber i to ply center
                fiberShader.setFloat("R[3]", 0.35f); // distance from fiber i to ply center

                fiberShader.setInt("uTessLineCount", 64); // distance from fiber i to ply center
                fiberShader.setInt("uTessSubdivisionCount", 4);
                fiberShader.setVec3("uLightDirection", glm::vec3(viewMatrix * glm::vec4(directional.GetDirection(), 0.0)));

                // Fragment related uniforms
                fiberShader.setBool("uUseAmbientOcclusion", useAmbientOcclusion); // distance from fiber i to ply center
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
            }
            // // Render slices of selfShadow to screen
            // glViewport(0, 0, 512, 512);
            // slice3DShader.use();
            // selfShadowsTexture->Attach(0);
            // slice3DShader.setInt("uInputTexture", 0);
            // slice3DShader.setFloat("uDepth", std::sin(currentTime) * 0.5 + 0.5);
            // glBindVertexArray(dummyVAO);
            // glDrawArrays(GL_TRIANGLES, 0, 3);
            // glBindVertexArray(0);

            // directional.SetDirection(camera.GetForwardDirection());


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
        // // Blit shadow map to the screen
        // glViewport(0, 0, 512, 512);
        // blitChannelShader.use();
        // shadowMap.GetTexture()->Attach(0);
        // blitChannelShader.setInt("uInput", 0);
        // glBindVertexArray(dummyVAO);
        // glDrawArrays(GL_TRIANGLES, 0, 3);
        // glBindVertexArray(0);

        {
            const ProfilingScope scope("UI Rendering");  

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            auto& io = ImGui::GetIO();
            ImGui::Begin("Control panel", nullptr);
            {
                if (ImGui::CollapsingHeader("Stats", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    indentedLabel("FPS:");
                    ImGui::SameLine();
                    ImGui::Text("%.1f (%.3fms)", io.Framerate, 1000.0f / io.Framerate);
                
                    ImGui::Text("Profiling:");
                    for (const auto& scope : profilingScopes)
                    {
                        float elapsedTime = scope.duration * 1000.0;
                        indentedLabel((scope.name + " :").c_str());
                        ImGui::SameLine();
                        ImGui::BeginDisabled();
                        ImGui::PushItemWidth(100.0f);
                        ImGui::DragFloat((std::string("##") + scope.name + "TimeDrag").c_str(), &elapsedTime, 1.0f, 0.0f, 0.0f, "%.3fms");
                        ImGui::EndDisabled();
                    }
                }

                ImGui::Spacing();
                
                if (ImGui::CollapsingHeader("Rendering parameters", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    indentedLabel("Show fibers :");
                    ImGui::SameLine();
                    ImGui::Checkbox("##ShowFibersCB", &showFibers);

                    indentedLabel("Show cloth mesh :");
                    ImGui::SameLine();
                    ImGui::Checkbox("##ShowClothMeshCB", &showClothMesh);

                    indentedLabel("Show floor :");
                    ImGui::SameLine();
                    ImGui::Checkbox("##ShowFloorCB", &showFloor);
                
                    indentedLabel("Ambient occlusion:");
                    ImGui::SameLine();
                    ImGui::Checkbox("##UseAmbientOcclusion", &useAmbientOcclusion);

                    indentedLabel("Shadow Mapping:");
                    ImGui::SameLine();
                    ImGui::Checkbox("##UseShadowMapping", &useShadowMapping);
                

                    indentedLabel("Shadow Map Thickess:");
                    ImGui::SameLine();
                    ImGui::DragFloat("##ShadowMapThicknessSlider", &shadowMapThickness, 0.001f, 0.0f, 1.0);

                    indentedLabel("Self Shadows:");
                    ImGui::SameLine();
                    ImGui::Checkbox("##UseSelfShadows", &useSelfShadows);

                    ImGui::BeginDisabled(!useSelfShadows);
                    indentedLabel("selfShadowRotation:");
                    ImGui::SameLine();
                    ImGui::DragFloat("##selfShadowRotation", &selfShadowRotation, 0.05f, -180.0f, 180.0f);
                    ImGui::EndDisabled();

                    indentedLabel("Light Orientation:");
                    ImGui::SameLine();
                    ImGui::DragFloat("##LightOrientation", &lightRotation, 0.5f, -180.0f, 180.0f, "%.1f");
                    
                    indentedLabel("Animated light:");
                    ImGui::SameLine();
                    ImGui::Checkbox("##AnimatedLightCheckBox", &animateLightRotation);
                }
            }
            ImGui::End();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        {
            // It's the SwapBuffers that actually compute the rendering, not the calls to glXXX commands
            const ProfilingScope scope("OpenGL Rendering");  
            window.Update();
        }
    }

    return 0;
}
