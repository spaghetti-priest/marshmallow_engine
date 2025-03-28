@echo off
set SHADER_DIR= C:/marshmallow/shaders
C:/VulkanSDK/1.4.309.0/Bin/glslc.exe %SHADER_DIR%/shader.vert -o %SHADER_DIR%/vert.spv
C:/VulkanSDK/1.4.309.0/Bin/glslc.exe %SHADER_DIR%/shader.frag -o %SHADER_DIR%/frag.spv