@echo off
SETLOCAL EnableDelayedExpansion
SET BLENDER_PATH=%APPDATA%\Blender Foundation\Blender\3.5\scripts\addons\CosmicRenderEngine\
IF NOT EXIST "%BLENDER_PATH%" MKDIR "%BLENDER_PATH%"
FOR /F %%G IN (BuildList.txt) DO COPY "..\CosmicRenderEngine\%%G" "%BLENDER_PATH%%%G"
