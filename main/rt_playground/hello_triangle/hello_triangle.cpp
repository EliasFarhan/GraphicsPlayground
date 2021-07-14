#include <SDL_main.h>
#include <vulkan/vulkan.h>
#include <glm/mat4x4.hpp>

#include "rt/engine.h"

class HelloTriangle : public core::Program
{
public:
    void Init() override;
    void Update(core::seconds dt) override;
    void Destroy() override;
    void OnEvent(SDL_Event& event) override;
    void DrawImGui() override;
private:
    // Holds data for a ray tracing scratch buffer that is used as a temporary storage
    struct RayTracingScratchBuffer
    {
        std::uint64_t deviceAddress = 0;
        VkBuffer handle = VK_NULL_HANDLE;
		VmaAllocation memory;
    };

    // Ray tracing acceleration structure
    struct AccelerationStructure
    {
        VkAccelerationStructureKHR handle;
        uint64_t deviceAddress = 0;
        VkDeviceMemory memory;
        VkBuffer buffer;
    };

    struct StorageImage {
        VmaAllocation memory;
        VkImage image;
        VkImageView view;
        VkFormat format;
    } storageImage_;

    struct UniformData {
        glm::mat4 viewInverse;
        glm::mat4 projInverse;
    } uniformData_;

	AccelerationStructure bottomLevelAS_{};
	AccelerationStructure topLevelAS_{};


    void CreateAccelerationStructureBuffer(const AccelerationStructure& accelerationStructure, const VkAccelerationStructureBuildSizesInfoKHR& accelerationStructureBuildSizesInfo);
    RayTracingScratchBuffer CreateScratchBuffer(VkDeviceSize buildScratchSize);
    void DeleteScratchBuffer(RayTracingScratchBuffer& scratchBuffer);
    bool CreateBottomLevelAccelerationStructure();
    bool CreateTopLevelAccelerationStructure();
    bool CreateStorageImage();
    bool CreateUniformBuffer();
    bool CreateRayTracingPipeline();
    bool CreateShaderBindingTable();
    bool CreateDescriptorSets();
    bool BuildCommandBuffers();

	rt::Buffer vertexBuffer_;
	rt::Buffer indexBuffer_;
	rt::Buffer transformBuffer_;
	std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups_{};
	rt::Buffer raygenShaderBindingTable_;
	rt::Buffer missShaderBindingTable_;
	rt::Buffer hitShaderBindingTable_;
	std::uint32_t indexCount_ = 0;


	VkPipeline pipeline_;
	VkPipelineLayout pipelineLayout_;
	VkDescriptorSet descriptorSet_;
	VkDescriptorSetLayout descriptorSetLayout_;

    bool initResult_ = true;


	PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR;
	PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
	PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR;
	PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;
	PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
	PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
	PFN_vkBuildAccelerationStructuresKHR vkBuildAccelerationStructuresKHR;
	PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;
	PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR;
	PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR;
    
};

void HelloTriangle::CreateAccelerationStructureBuffer(const AccelerationStructure& accelerationStructure,
    const VkAccelerationStructureBuildSizesInfoKHR& accelerationStructureBuildSizesInfo)
{
}

HelloTriangle::RayTracingScratchBuffer HelloTriangle::CreateScratchBuffer(VkDeviceSize buildScratchSize)
{
	return {};
}

void HelloTriangle::DeleteScratchBuffer(RayTracingScratchBuffer& scratchBuffer)
{
}

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
	auto& engine = rt::Engine::GetInstance();
	// Setup vertices for a single triangle
	struct Vertex {
		std::array<float,3> pos;
	};
	std::vector<Vertex> vertices = {
		{ {  1.0f,  1.0f, 0.0f } },
		{ { -1.0f,  1.0f, 0.0f } },
		{ {  0.0f, -1.0f, 0.0f } }
	};

	// Setup indices
	std::vector<uint32_t> indices = { 0, 1, 2 };
	indexCount_ = static_cast<uint32_t>(indices.size());

	// Setup identity transform matrix
	VkTransformMatrixKHR transformMatrix = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f
	};

	// Create buffers
	// For the sake of simplicity we won't stage the vertex data to the GPU memory
	// Vertex buffer
	engine.CreateBuffer(
		vertices.size() * sizeof(Vertex),
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		vertexBuffer_.buffer,
		vertexBuffer_.allocation);
	
	// Index buffer
	engine.CreateBuffer(
		indices.size() * sizeof(uint32_t),
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		indexBuffer_.buffer,
		indexBuffer_.allocation);
	// Transform buffer
	engine.CreateBuffer(
		sizeof(VkTransformMatrixKHR),
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		transformBuffer_.buffer,
		transformBuffer_.allocation);

	VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
	VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};
	VkDeviceOrHostAddressConstKHR transformBufferDeviceAddress{};

	vertexBufferDeviceAddress.deviceAddress = engine.GetBufferDeviceAddress(vertexBuffer_.buffer);
	indexBufferDeviceAddress.deviceAddress = engine.GetBufferDeviceAddress(indexBuffer_.buffer);
	transformBufferDeviceAddress.deviceAddress = engine.GetBufferDeviceAddress(transformBuffer_.buffer);

	// Build
	VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
	accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
	accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
	accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
	accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
	accelerationStructureGeometry.geometry.triangles.vertexData = vertexBufferDeviceAddress;
	accelerationStructureGeometry.geometry.triangles.maxVertex = 3;
	accelerationStructureGeometry.geometry.triangles.vertexStride = sizeof(Vertex);
	accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
	accelerationStructureGeometry.geometry.triangles.indexData = indexBufferDeviceAddress;
	accelerationStructureGeometry.geometry.triangles.transformData.deviceAddress = 0;
	accelerationStructureGeometry.geometry.triangles.transformData.hostAddress = nullptr;
	accelerationStructureGeometry.geometry.triangles.transformData = transformBufferDeviceAddress;

	// Get size info
	VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
	accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	accelerationStructureBuildGeometryInfo.geometryCount = 1;
	accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;

	const uint32_t numTriangles = 1;
	VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
	accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	vkGetAccelerationStructureBuildSizesKHR(
		engine.GetDevice(),
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&accelerationStructureBuildGeometryInfo,
		&numTriangles,
		&accelerationStructureBuildSizesInfo);

	CreateAccelerationStructureBuffer(bottomLevelAS_, accelerationStructureBuildSizesInfo);

	VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
	accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	accelerationStructureCreateInfo.buffer = bottomLevelAS_.buffer;
	accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
	accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	vkCreateAccelerationStructureKHR(engine.GetDevice(), &accelerationStructureCreateInfo, nullptr, &bottomLevelAS_.handle);

	// Create a small scratch buffer used during build of the bottom level acceleration structure
	RayTracingScratchBuffer scratchBuffer = CreateScratchBuffer(accelerationStructureBuildSizesInfo.buildScratchSize);

	VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
	accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	accelerationBuildGeometryInfo.dstAccelerationStructure = bottomLevelAS_.handle;
	accelerationBuildGeometryInfo.geometryCount = 1;
	accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
	accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

	VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
	accelerationStructureBuildRangeInfo.primitiveCount = numTriangles;
	accelerationStructureBuildRangeInfo.primitiveOffset = 0;
	accelerationStructureBuildRangeInfo.firstVertex = 0;
	accelerationStructureBuildRangeInfo.transformOffset = 0;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

	// Build the acceleration structure on the device via a one-time command buffer submission
	// Some implementations may support acceleration structure building on the host (VkPhysicalDeviceAccelerationStructureFeaturesKHR->accelerationStructureHostCommands), but we prefer device builds
	VkCommandBuffer commandBuffer = engine.BeginSingleTimeCommands();//(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
	vkCmdBuildAccelerationStructuresKHR(
		commandBuffer,
		1,
		&accelerationBuildGeometryInfo,
		accelerationBuildStructureRangeInfos.data());
	engine.EndSingleTimeCommands(commandBuffer);
	VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
	accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
	accelerationDeviceAddressInfo.accelerationStructure = bottomLevelAS_.handle;
	bottomLevelAS_.deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(engine.GetDevice(), &accelerationDeviceAddressInfo);

	DeleteScratchBuffer(scratchBuffer);
    return true;
}

bool HelloTriangle::CreateTopLevelAccelerationStructure()
{
    return true;
}

bool HelloTriangle::CreateStorageImage()
{
    auto& engine = rt::Engine::GetInstance();
    const auto [width, height] = engine.GetSize();
    const auto& swapchain = engine.GetSwapChain();
    engine.CreateImage(width, height, swapchain.imageFormat, VK_IMAGE_TILING_OPTIMAL,
                       VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, storageImage_.image, storageImage_.memory);
    VkImageViewCreateInfo colorImageView{};
    colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    colorImageView.format = swapchain.imageFormat;
    colorImageView.subresourceRange = {};
    colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    colorImageView.subresourceRange.baseMipLevel = 0;
    colorImageView.subresourceRange.levelCount = 1;
    colorImageView.subresourceRange.baseArrayLayer = 0;
    colorImageView.subresourceRange.layerCount = 1;
    colorImageView.image = storageImage_.image;
    vkCreateImageView(engine.GetDevice(), &colorImageView, nullptr, &storageImage_.view);
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
	auto& engine =rt::Engine::GetInstance();
	auto device = engine.GetDevice();
	vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(device, "vkGetBufferDeviceAddressKHR"));
	vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructuresKHR"));
	vkBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device, "vkBuildAccelerationStructuresKHR"));
	vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureKHR"));
	vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR"));
	vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(device, "vkGetAccelerationStructureBuildSizesKHR"));
	vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(device, "vkGetAccelerationStructureDeviceAddressKHR"));
	vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(device, "vkCmdTraceRaysKHR"));
	vkGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupHandlesKHR"));
	vkCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(device, "vkCreateRayTracingPipelinesKHR"));

	return;
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
    HelloTriangle helloTriangle;
    rt::Engine engine(helloTriangle);
    engine.Run();
    return 0;
}
