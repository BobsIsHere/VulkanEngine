# Copy resources folder to output directory
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/resources" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

# Source files
set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")
set(STB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../3rdParty")

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${SHADER_SOURCE_DIR}/*.frag"
    "${SHADER_SOURCE_DIR}/*.vert"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${SHADER_BINARY_DIR}/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} -g ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL}
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
)

set(SOURCES
    "main.cpp"
    "vulkanbase/VulkanBase.h" 
    "vulkanbase/VulkanUtil.h"
    "vulkanbase/VulkanUtil.cpp"
    # Add other source files here, but remove redundant and unnecessary entries
    "labwork/Week01.cpp"
    "labwork/Week02.cpp" 
    "labwork/Week03.cpp" 
    "labwork/Week04.cpp" 
    "labwork/Week05.cpp"
    "labwork/Week06.cpp"
    "GP2_Shader.h"
    "GP2_Shader.cpp" 
    "GP2_CommandPool.h"
    "GP2_CommandPool.cpp"
    "GP2_CommandBuffer.h" 
    "GP2_CommandBuffer.cpp" 
    "GP2_Mesh.h"  
    "GP2_Mesh.cpp"
    "GP2_Buffer.h" 
    "GP2_Buffer.cpp" 
    "Vertex.h" 
    "GP2_UniformBufferObject.h" 
    "GP2_DescriptorPool.h" 
    "GP2_2DGraphicsPipeline.h" 
    "GP2_3DGraphicsPipeline.h" 
    "GP2_PBRGraphicsPipeline.h"  
    "GP2_Texture.h" 
    "GP2_Texture.cpp" 
    "GP2_DepthBuffer.h" 
    "GP2_DepthBuffer.cpp"
    "${STB_DIR}/stb_image.h"
    "GP2_2DMesh.h" "GP2_2DMesh.cpp" "GP2_3DMesh.h" "GP2_3DMesh.cpp"
)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES})
add_dependencies(${PROJECT_NAME} Shaders)

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${STB_DIR})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES} glfw)
