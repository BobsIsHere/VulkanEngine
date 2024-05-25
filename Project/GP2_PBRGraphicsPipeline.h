#pragma once
#pragma once
#include <string>
#include <memory>
#include <vulkanbase/VulkanUtil.h>
#include <vulkanbase/VulkanBase.h>

#include "Vertex.h"
#include "GP2_Mesh.h"
#include "GP2_Shader.h"
#include "GP2_CommandBuffer.h"
#include "GP2_DescriptorPool.h"

using pMesh3D = std::unique_ptr<GP2_Mesh<Vertex3D>>;

template<class UBOPBR>
class GP2_PBRGraphicsPipeline final
{
public:
	//---------------------------
	// Constructors & Destructor
	//---------------------------
	GP2_PBRGraphicsPipeline(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);
	~GP2_PBRGraphicsPipeline() = default;

	//-----------
	// Functions
	//-----------
	void Initialize(const VulkanContext& context);

	void Cleanup();

	void Record(const GP2_CommandBuffer& buffer, VkExtent2D extent, int imageIdx);
	void DrawScene(const GP2_CommandBuffer& buffer);
	void AddMesh(pMesh3D mesh);

	void SetUBO(UBOPBR ubo, size_t uboIndex);

private:
	//-----------
	// Functions
	//-----------
	void CreateGraphicsPipeline();
	VkPushConstantRange CreatePushConstantRange();

	//-----------
	// Variables
	//-----------
	VkDevice m_Device;
	VkRenderPass m_RenderPass;
	VkPipeline m_GraphicsPipeline;
	VkPipelineLayout m_PipelineLayout;

	GP2_Shader<Vertex2D> m_Shader;
	std::vector<pMesh2D> m_pMeshes;
	GP2_DescriptorPool<UBOPBR>* m_pDescriptorPool;
};

template<class UBOPBR>
inline GP2_PBRGraphicsPipeline<UBOPBR>::GP2_PBRGraphicsPipeline(const std::string& vertexShaderFile, const std::string& fragmentShaderFile)
{
}

template<class UBOPBR>
inline void GP2_PBRGraphicsPipeline<UBOPBR>::Initialize(const VulkanContext& context)
{
}

template<class UBOPBR>
inline void GP2_PBRGraphicsPipeline<UBOPBR>::Cleanup()
{
}

template<class UBOPBR>
inline void GP2_PBRGraphicsPipeline<UBOPBR>::Record(const GP2_CommandBuffer& buffer, VkExtent2D extent, int imageIdx)
{
}

template<class UBOPBR>
inline void GP2_PBRGraphicsPipeline<UBOPBR>::DrawScene(const GP2_CommandBuffer& buffer)
{
}

template<class UBOPBR>
inline void GP2_PBRGraphicsPipeline<UBOPBR>::AddMesh(pMesh3D mesh)
{
}

template<class UBOPBR>
inline void GP2_PBRGraphicsPipeline<UBOPBR>::SetUBO(UBOPBR ubo, size_t uboIndex)
{
}

template<class UBOPBR>
inline void GP2_PBRGraphicsPipeline<UBOPBR>::CreateGraphicsPipeline()
{
}

template<class UBOPBR>
inline VkPushConstantRange GP2_PBRGraphicsPipeline<UBOPBR>::CreatePushConstantRange()
{
	return VkPushConstantRange();
}
