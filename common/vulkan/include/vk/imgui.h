
#include <vulkan/vulkan_core.h>
#include <engine.h>

namespace vk
{

    class ImGui
    {
    public:

        void Init() override;
        void Destroy() override;

        void Update(core::seconds dt) override;
        void Render();

        void CleanupSwapChain();
        void CreateSwapChain();

    private:
        VkDescriptorPool descriptorPool_;
    };
}
