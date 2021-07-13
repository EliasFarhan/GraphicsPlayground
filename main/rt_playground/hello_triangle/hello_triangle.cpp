#include <SDL_main.h>

#include "rt/engine.h"

class HelloTriangle : core::Program
{
public:
    void Init() override;
    void Update(core::seconds dt) override;
    void Destroy() override;
    void OnEvent(SDL_Event& event) override;
    void DrawImGui() override;
private:
    bool CreateBottomLevelAccelerationStructure();
    bool CreateTopLevelAccelerationStructure();
    bool CreateStorageImage();
    bool CreateUniformBuffer();
    bool CreateRayTracingPipeline();
    bool CreateShaderBindingTable();
    bool CreateDescriptorSets();
    bool BuildCommandBuffers();

    bool initResult_ = true;
};

void HelloTriangle::Update(core::seconds dt)
{
}

void HelloTriangle::Destroy()
{
}

void HelloTriangle::OnEvent(SDL_Event& event)
{
}

void HelloTriangle::DrawImGui()
{
}

bool HelloTriangle::CreateBottomLevelAccelerationStructure()
{
    return true;
}

bool HelloTriangle::CreateTopLevelAccelerationStructure()
{
    return true;
}

bool HelloTriangle::CreateStorageImage()
{
    return true;
}

bool HelloTriangle::CreateUniformBuffer()
{
    return true;
}

bool HelloTriangle::CreateRayTracingPipeline()
{
    return true;
}

bool HelloTriangle::CreateShaderBindingTable()
{
    return true;
}

bool HelloTriangle::CreateDescriptorSets()
{
    return true;
}

bool HelloTriangle::BuildCommandBuffers()
{
    return true;
}

void HelloTriangle::Init()
{
	// Create the acceleration structures used to render the ray traced scene
	initResult_ = initResult_ && CreateBottomLevelAccelerationStructure();
	initResult_ = initResult_ && CreateTopLevelAccelerationStructure();

	initResult_ = initResult_ && CreateStorageImage();
	initResult_ = initResult_ && CreateUniformBuffer();
	initResult_ = initResult_ && CreateRayTracingPipeline();
	initResult_ = initResult_ && CreateShaderBindingTable();
	initResult_ = initResult_ && CreateDescriptorSets();
	initResult_ = initResult_ && BuildCommandBuffers();
}


int main(int argc, char** argv)
{
    rt::Engine engine;
    engine.Run();
    return 0;
}
