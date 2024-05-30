#pragma once

#include <string>

std::string pathToRootFolder;
std::string pathToExe;

std::string pathToExternalTextures;

std::string pathToTextures;
std::string pathToShaders;
std::string pathToObjects;

std::string pathToSubmarine;
std::string pathToDetachedSubmarine;

std::string pathToTerrain;
std::string pathToWater;

std::string pathToSkyBoxTextures;
std::string pathToSkyBoxShaders;
std::string pathToShadowMappingShaders;
std::string pathToWaterShaders;

bool InitializePaths(std::string strFullExeFileName) {	//Initializes all the paths to the directories of the project

	pathToRootFolder = strFullExeFileName;
	for (size_t i = 0; i < 3; i++) {
		size_t last_slash_index = pathToRootFolder.rfind('\\');
		pathToRootFolder = pathToRootFolder.substr(0, last_slash_index);
	}

	pathToExe = strFullExeFileName;
	for (size_t i = 0; i < 2; i++) {
		size_t last_slash_index = pathToExe.rfind('\\');
		pathToExe = pathToExe.substr(0, last_slash_index);
	}
	pathToExe = pathToExe + "\\Assets";

	pathToExternalTextures = pathToRootFolder + "\\_external\\Textures\\";

	pathToTextures = pathToExe + "\\Textures\\";
	pathToShaders = pathToExe + "\\Shaders\\";
	pathToObjects = pathToExe + "\\Objects\\";

	pathToSubmarine = pathToExe + "\\Objects\\Submarine\\";
	pathToDetachedSubmarine = pathToExe + "\\Objects\\SubmarinePropellor\\";

	pathToTerrain = pathToExe + "\\Objects\\Terrain\\";
	pathToWater = pathToExe + "\\Objects\\Water\\";

	pathToSkyBoxTextures = pathToTextures + "Skybox\\";
	pathToSkyBoxShaders = pathToShaders + "Skybox\\";
	pathToShadowMappingShaders = pathToShaders + "Shadow\\";
	pathToWaterShaders = pathToShaders + "Water\\";

	return true;
}
