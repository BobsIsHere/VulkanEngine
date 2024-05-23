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
class PBRGraphicsPipeline final
{
public:
	//---------------------------
	// Constructors & Destructor
	//---------------------------
	PBRGraphicsPipeline(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);
	~PBRGraphicsPipeline() = default;

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
inline PBRGraphicsPipeline<UBOPBR>::PBRGraphicsPipeline(const std::string& vertexShaderFile, const std::string& fragmentShaderFile)
{
}

template<class UBOPBR>
inline void PBRGraphicsPipeline<UBOPBR>::Initialize(const VulkanContext& context)
{
}

template<class UBOPBR>
inline void PBRGraphicsPipeline<UBOPBR>::Cleanup()
{
}

template<class UBOPBR>
inline void PBRGraphicsPipeline<UBOPBR>::Record(const GP2_CommandBuffer& buffer, VkExtent2D extent, int imageIdx)
{
}

template<class UBOPBR>
inline void PBRGraphicsPipeline<UBOPBR>::DrawScene(const GP2_CommandBuffer& buffer)
{
}

template<class UBOPBR>
inline void PBRGraphicsPipeline<UBOPBR>::AddMesh(pMesh3D mesh)
{
}

template<class UBOPBR>
inline void PBRGraphicsPipeline<UBOPBR>::SetUBO(UBOPBR ubo, size_t uboIndex)
{
}

template<class UBOPBR>
inline void PBRGraphicsPipeline<UBOPBR>::CreateGraphicsPipeline()
{
}

template<class UBOPBR>
inline VkPushConstantRange PBRGraphicsPipeline<UBOPBR>::CreatePushConstantRange()
{
	return VkPushConstantRange();
}
