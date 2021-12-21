# CMakeList.txt: MaterialEditor 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。

# 需要最小的CMake版本
cmake_minimum_required (VERSION 3.8)
# 工程的名称，会作为MSVS的Workspace的名字
project (MaterialEditor)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)
list(APPEND CMAKE_CXX_FLAGS "-std=c++11")


# 全局变量：CMAKE_SOURCE_DIR CMake的起始目录，即源码的根目录
# 全局变量：PROJECT_NAME 工程的名称
# 全局变量：PROJECT_SOURCE_DIR 工程的源码根目录的完整路径






#添加读取资源绝对路径
#==========================================================================================================
configure_file(configuration/filesystem.cpp.in ${CMAKE_SOURCE_DIR}/includes/core/filesystem.cpp)
configure_file(configuration/filesystem.h.in ${CMAKE_SOURCE_DIR}/includes/core/filesystem.h)
#==========================================================================================================






#添加glfw glm assimp等头文件和库
#==========================================================================================================
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# find the required packages
find_package(GLM REQUIRED)
message(STATUS "GLM included at ${GLM_INCLUDE_DIR}")
find_package(GLFW3 REQUIRED)
message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")
find_package(ASSIMP REQUIRED)
message(STATUS "Found ASSIMP in ${ASSIMP_INCLUDE_DIR}")

if(WIN32)
  set(LIBS glfw3 opengl32 assimp freetype)
endif(WIN32)
#==========================================================================================================




#添加需要链接的库文件目录LINK_DIRECTORIES
#==========================================================================================================
link_directories(${CMAKE_SOURCE_DIR}/lib)
# 添加头文件目录INCLUDE_DIRECTORIES
include_directories(${CMAKE_SOURCE_DIR}/includes)
#添加glad库
add_library(GLAD "src/glad.c")
set(LIBS ${LIBS} GLAD)
#添加stb库
add_library(STB_IMAGE "src/stb_image.cpp")
set(LIBS ${LIBS} STB_IMAGE)
#==========================================================================================================




#添加编译文件
#==========================================================================================================
macro(makeLink src dest target)
  add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E create_symlink ${src} ${dest}  DEPENDS  ${dest} COMMENT "mklink ${src} -> ${dest}")
endmacro()


# 将源代码添加到此项目的可执行文件
file(GLOB SOURCES
	"src/*.h"
	"src/*.cpp"
	"src/*.c"
)
file(GLOB MATERIALSHADER
	"src/shader/*.glsl"
)
file(GLOB IMGUI
	"includes/imgui/*.h"
	"includes/imgui/*.cpp"	
)

file(GLOB CORE
	"includes/core/*.h"
	"includes/core/*.cpp"
)
set(NAME "MaterialEditor")
source_group(imgui FILES ${IMGUI})
source_group(core FILES ${CORE})
source_group(shader FILES ${MATERIALSHADER})
add_executable (${NAME} ${SOURCES} ${MATERIALSHADER} ${IMGUI} ${CORE})
target_link_libraries(${NAME} ${LIBS})

#输出结果到bin文件夹中
if(WIN32)
	set_target_properties(${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
	set_target_properties(${NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/Debug")
endif(WIN32)

#==========================================================================================================






# 把shader和assimp的工具copy到build目录下
#==========================================================================================================
file(GLOB SHADERS
	"src/shader/*.glsl"
)
foreach(SHADER ${SHADERS})
	if(WIN32)
        add_custom_command(TARGET ${NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${SHADER} $<TARGET_FILE_DIR:${NAME}>)
	endif(WIN32)
endforeach(SHADER)

file(GLOB ASSIMPDLL
	"dlls/assimp-vc140-mt.dll"
)
if(WIN32)
	add_custom_command(TARGET ${NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${ASSIMPDLL} $<TARGET_FILE_DIR:${NAME}>)
endif(WIN32)

#==========================================================================================================





