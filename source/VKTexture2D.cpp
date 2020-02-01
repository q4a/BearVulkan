#include "VKPCH.h"
bsize Texture2DCounter = 0;
VKTexture2D::VKTexture2D(bsize Width, bsize Height, bsize Mips, bsize Count, BearTexturePixelFormat PixelFormat, BearTextureUsage typeUsage, void* data)
{


   
    Texture2DCounter++;
    Format = PixelFormat;
    TextureUsage = typeUsage;
    TextureType = TT_Default;
    {
        bear_fill(ImageInfo);
        ImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        ImageInfo.imageType = VK_IMAGE_TYPE_2D;
        ImageInfo.extent.width =static_cast<uint32_t>( Width);
        ImageInfo.extent.height = static_cast<uint32_t>(Height);
        ImageInfo.extent.depth = static_cast<uint32_t>(Count);;
        ImageInfo.mipLevels = static_cast<uint32_t>(Mips);
        ImageInfo.arrayLayers = 1;
        ImageInfo.format = Factory->Translation(PixelFormat);
        ImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        ImageInfo.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        ImageInfo.usage =TextureUsage==TU_STATING ? VK_IMAGE_USAGE_TRANSFER_SRC_BIT| VK_IMAGE_USAGE_TRANSFER_DST_BIT : VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        ImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        ImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        V_CHK(vkCreateImage(Factory->Device, &ImageInfo, nullptr, &Image));
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(Factory->Device, Image, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(Factory->PhysicalDevice,memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        V_CHK(vkAllocateMemory(Factory->Device, &allocInfo, nullptr, &ImageMemory));
        vkBindImageMemory(Factory->Device, Image, ImageMemory, 0);
    }

  
   

    {
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = Image;
        viewInfo.viewType = Count>1? VK_IMAGE_VIEW_TYPE_2D: VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        viewInfo.format = Factory->Translation(PixelFormat);
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = static_cast<uint32_t>(Mips);
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = static_cast<uint32_t>(Count);

        V_CHK(vkCreateImageView(Factory->Device, &viewInfo, nullptr, &ImageView));
    }
    m_buffer = 0;
    switch (TextureUsage)
    {
    case TU_DYNAMIC:
    case TU_STATING:
        AllocBuffer();
        break;
    }
    if (data)
    {
        auto ptr = reinterpret_cast<uint8*>(data);
        for (bsize x = 0; x < Count; x++)
            for (bsize y = 0; y < Mips; y++)
            {
                bsize  size = BearTextureUtils::GetSizeDepth(BearTextureUtils::GetMip(Width, y), BearTextureUtils::GetMip(Height, y), Format);
                bear_copy(Lock(y, x), ptr, size);
                Unlock();
                ptr += size;
            }


    }
}

VKTexture2D::VKTexture2D(bsize Width, bsize Height, BearRenderTargetFormat Format)
{
    Texture2DCounter++;
    RTVFormat = Format;
    m_buffer = 0;
    TextureUsage = TU_STATIC;
    TextureType = TT_RenderTarget;
    bear_fill(ImageInfo);
    ImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ImageInfo.imageType = VK_IMAGE_TYPE_2D;
    ImageInfo.extent.width = static_cast<uint32_t>(Width);
    ImageInfo.extent.height = static_cast<uint32_t>(Height);
    ImageInfo.extent.depth = static_cast<uint32_t>(1);;
    ImageInfo.mipLevels = static_cast<uint32_t>(1);
    ImageInfo.arrayLayers = 1;
    ImageInfo.format = Factory->Translation(RTVFormat);
    ImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    ImageInfo.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    ImageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT| VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    ImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    ImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    V_CHK(vkCreateImage(Factory->Device, &ImageInfo, nullptr, &Image));
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(Factory->Device, Image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(Factory->PhysicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    V_CHK(vkAllocateMemory(Factory->Device, &allocInfo, nullptr, &ImageMemory));
    vkBindImageMemory(Factory->Device, Image, ImageMemory, 0);
    {
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = Image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D ;
        viewInfo.format = Factory->Translation(RTVFormat);
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = static_cast<uint32_t>(1);
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = static_cast<uint32_t>(1);

        V_CHK(vkCreateImageView(Factory->Device, &viewInfo, nullptr, &ImageView));
    }
}

VKTexture2D::VKTexture2D(bsize Width, bsize Height, BearDepthStencilFormat Format)
{
    Texture2DCounter++;
    DSVFormat = Format;
    m_buffer = 0;
    TextureUsage = TU_STATIC;
    TextureType = TT_DepthStencil;
    bear_fill(ImageInfo);
    ImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ImageInfo.imageType = VK_IMAGE_TYPE_2D;
    ImageInfo.extent.width = static_cast<uint32_t>(Width);
    ImageInfo.extent.height = static_cast<uint32_t>(Height);
    ImageInfo.extent.depth = static_cast<uint32_t>(1);;
    ImageInfo.mipLevels = static_cast<uint32_t>(1);
    ImageInfo.arrayLayers = 1;
    ImageInfo.format = Factory->Translation(DSVFormat);
    ImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    ImageInfo.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    ImageInfo.usage =  VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    ImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    ImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    V_CHK(vkCreateImage(Factory->Device, &ImageInfo, nullptr, &Image));
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(Factory->Device, Image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(Factory->PhysicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    V_CHK(vkAllocateMemory(Factory->Device, &allocInfo, nullptr, &ImageMemory));
    vkBindImageMemory(Factory->Device, Image, ImageMemory, 0);
    ImageView = 0;
    {
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = Image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = Factory->Translation(DSVFormat);
        switch (DSVFormat)
        {
        case DSF_DEPTH16:
        case DSF_DEPTH32F:
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT ;
            break;
        default:
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            break;
        }


        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = static_cast<uint32_t>(1);
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = static_cast<uint32_t>(1);

        V_CHK(vkCreateImageView(Factory->Device, &viewInfo, nullptr, &ImageView));
    }

}

void VKTexture2D::Set(VkWriteDescriptorSet* HEAP)
{
    BEAR_ASSERT(ImageView);
    DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    DescriptorImageInfo.imageView = ImageView;
    DescriptorImageInfo.sampler = Factory->DefaultSampler;
    HEAP->descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    HEAP->pImageInfo = &DescriptorImageInfo;
}

VKTexture2D::~VKTexture2D()
{
    Texture2DCounter--;
    if(ImageView)
    vkDestroyImageView(Factory->Device, ImageView, nullptr);
    vkDestroyImage(Factory->Device, Image, nullptr);
    vkFreeMemory(Factory->Device, ImageMemory, nullptr);
    switch (TextureUsage)
    {
    case TU_DYNAMIC:
    case TU_STATING:
        FreeBuffer();
        break;
    }
}


BearTextureType VKTexture2D::GetType()
{
    return TextureType;
}

void* VKTexture2D::Lock(bsize mip, bsize depth)
{
    m_mip = mip;
    m_depth = depth;
    if (TextureType != TT_Default)return 0;
    if (m_buffer)Unlock();
    switch (TextureUsage)
    {
    case TU_STATIC:
        AllocBuffer();
        break;
    case TU_DYNAMIC:
        break;
    case TU_STATING:
        TransitionImageLayout(0,Image, Factory->Translation(Format), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, ImageInfo.mipLevels, ImageInfo.extent.depth);
        CopyImageToBuffer(StagingBuffer, Image, ImageInfo.extent.width, ImageInfo.extent.height, static_cast<uint32_t>(m_mip), static_cast<uint32_t>(m_depth));
        TransitionImageLayout(0, Image, Factory->Translation(Format), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ImageInfo.mipLevels, ImageInfo.extent.depth);

        break;
    default:
        break;
    }
    vkMapMemory(Factory->Device, StagingBufferMemory, 0, BearTextureUtils::GetSizeInMemory(BearTextureUtils::GetMip( ImageInfo.extent.width,mip), BearTextureUtils::GetMip(ImageInfo.extent.height, mip), ImageInfo.mipLevels, Format), 0, &m_buffer);

    return m_buffer;
}

void VKTexture2D::Unlock()
{
    vkUnmapMemory(Factory->Device, StagingBufferMemory);
    switch (TextureUsage)
    {
    case TU_STATIC:
        TransitionImageLayout(0, Image, Factory->Translation(Format), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, ImageInfo.mipLevels, ImageInfo.extent.depth);
        CopyBufferToImage(StagingBuffer, Image, ImageInfo.extent.width, ImageInfo.extent.height, static_cast<uint32_t>(m_mip), static_cast<uint32_t>(m_depth));
        TransitionImageLayout(0, Image, Factory->Translation(Format), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ImageInfo.mipLevels, ImageInfo.extent.depth);
        FreeBuffer();
        break;
    case TU_DYNAMIC:
        TransitionImageLayout(0, Image, Factory->Translation(Format), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, ImageInfo.mipLevels, ImageInfo.extent.depth);
        CopyBufferToImage(StagingBuffer, Image, ImageInfo.extent.width, ImageInfo.extent.height, static_cast<uint32_t>(m_mip), static_cast<uint32_t>(m_depth));
        TransitionImageLayout(0, Image, Factory->Translation(Format), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ImageInfo.mipLevels, ImageInfo.extent.depth);

        break;
    case TU_STATING:
        break;
    default:
        break;
    }
    m_buffer = 0;
  
    
   

 
}

void VKTexture2D::AllocBuffer()
{
    VkDeviceSize imageSize = BearTextureUtils::GetSizeInMemory(ImageInfo.extent.width, ImageInfo.extent.height, ImageInfo.mipLevels, Format) * ImageInfo.extent.depth;
    CreateBuffer(Factory->PhysicalDevice, Factory->Device, imageSize, TextureUsage == TU_STATING ? VK_BUFFER_USAGE_TRANSFER_DST_BIT : VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, StagingBuffer, StagingBufferMemory);
}

void VKTexture2D::FreeBuffer()
{
    vkDestroyBuffer(Factory->Device, StagingBuffer, nullptr);
    vkFreeMemory(Factory->Device, StagingBufferMemory, nullptr);
}
