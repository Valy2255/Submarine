// YellowSubmarine.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <stdlib.h> // necesare pentru citirea shaderStencilTesting-elor

#include <GL/glew.h>

#define GLM_FORCE_CTOR_INIT
#include <GLM.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <glfw3.h>

#include "Camera.h"
#include "Model.h"
#include "Shader.h"
#include "Input.h"
#include "Paths.h"

#include "Skybox.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <chrono>

#pragma comment (lib, "glfw3dll.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "OpenGL32.lib")
#pragma comment (lib, "assimp-vc140-mt.lib")


bool DrawSkybox(Shader shaderSkybox, glm::mat4& view, glm::mat4& projection) {
	// ** SKYBOX **

	// cubes
	glBindVertexArray(cubeMapVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	// draw skybox as last
	glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
	shaderSkybox.Use();

	view = glm::mat4(glm::mat3(pCamera->GetViewMatrix())); // remove translation from the view matrix

	shaderSkybox.SetMat4("view", view);
	shaderSkybox.SetMat4("projection", projection);

	// skybox cube
	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS); // set depth function back to default
	// ** SKYBOX **

	return true;
}

bool DrawObject(Shader shaderModel, Model objectModel, glm::mat4& view, glm::mat4& projection, float scaleFactor) {
	// ** MODEL **
	shaderModel.Use();

	view = pCamera->GetViewMatrix();

	shaderModel.SetMat4("view", view);
	shaderModel.SetMat4("projection", projection);



	// Draw the loaded model
	glm::mat4 model;
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // Move to scene centre
	model = glm::scale(model, glm::vec3(scaleFactor, scaleFactor, scaleFactor));	// Scale model
	//model = glm::rotate(model, degrees, glm::vec3(0.0f, 1.0f, 0.0f));
	shaderModel.SetMat4("model", model);
	objectModel.Draw(shaderModel);
	// ** MODEL **

	return true;
}

bool DrawAndRotateSubmarineObject(Shader shaderModel, Model objectModel, glm::mat4& view, glm::mat4& projection, float scaleFactor, float time, float speedFactor, float xPos, float yPos, float zPos, float yaw, float pitch) {
	// ** MODEL **
	shaderModel.Use();

	/*glm::vec3 cameraPosition = glm::vec3(xPos, yPos, zPos) + glm::vec3(0.0f,1.0f,1.5f);
	pCamera->SetPosition(cameraPosition);
	pCamera->SetYaw(yaw);
	pCamera->SetPitch(pitch);*/

	view = pCamera->GetViewMatrix();

	shaderModel.SetMat4("view", view);
	shaderModel.SetMat4("projection", projection);

	// Draw the loaded model
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // Move to scene centre
	model = glm::scale(model, glm::vec3(scaleFactor, scaleFactor, scaleFactor));	// Scale model

	model = glm::translate(model, glm::vec3(xPos, yPos, zPos));
	model = glm::rotate(model, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));

	float degrees = 90 + glfwGetTime() / speedFactor;
	//model = glm::rotate(
	//	model,
	//	time * glm::radians(180.0f) * speedFactor,
	//	glm::vec3(0.0f, 0.0f, 1.0f)
	//);

	//model = glm::rotate(model, degrees, glm::vec3(0.0f, 1.0f, 0.0f));
	shaderModel.SetMat4("model", model);
	objectModel.Draw(shaderModel);
	// ** MODEL **

	return true;
}

bool BuildDepthMapVBO(unsigned int& depthMap, unsigned int& depthMapFBO) {

	// configure depth map FBO
	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture

	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}



bool RenderSceneWithLight(Shader &shadowMappingDepthShader, Shader &shadowMappingShader, Model &terrain, Model &water, unsigned int depthMap, unsigned int depthMapFBO, glm::vec3 lightPos, glm::mat4& view, glm::mat4& projection, float modifyDiffuseStrength, float modifyAmbientStrength, float modifySpecularStrength)
{
	// Render settings
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set up light-space transformation matrix
	glm::mat4 lightProjection, lightView, lightSpaceMatrix;
	float near_plane = 1.0f, far_plane = 25.0f;  // Adjusted far_plane for better depth range
	lightProjection = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, near_plane, far_plane);
	lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0, 1, 0));
	lightSpaceMatrix = lightProjection * lightView;

	// Render depth of scene to texture (from light's perspective)
	shadowMappingDepthShader.Use();
	shadowMappingDepthShader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	DrawObject(shadowMappingDepthShader, terrain, lightView, lightProjection, 0.2f);
	DrawObject(shadowMappingDepthShader, water, lightView, lightProjection, 0.2f);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Render scene as normal using the generated depth/shadow map
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shadowMappingShader.Use();
	shadowMappingShader.SetMat4("projection", projection);
	shadowMappingShader.SetMat4("view", view);
	shadowMappingShader.SetFloat("diffuseStrength", modifyDiffuseStrength);
	shadowMappingShader.SetFloat("ambientStrength", modifyAmbientStrength);
	shadowMappingShader.SetFloat("specularStrength", modifySpecularStrength);
	shadowMappingDepthShader.SetVec3("viewPos", pCamera->GetPosition());
	shadowMappingShader.SetVec3("lightPos", lightPos);
	shadowMappingShader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glDisable(GL_CULL_FACE);
	DrawObject(shadowMappingShader, terrain, view, projection, 0.2f);
	DrawObject(shadowMappingShader, water, view, projection, 0.2f);

	return true;
}


bool InitializeWindow(GLFWwindow*& window) {
	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "SubmarineProject", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewInit();

	// configure global opengl state
	glEnable(GL_DEPTH_TEST);

	return true;
}

int main(int argc, char** argv) {

	auto t_start = std::chrono::high_resolution_clock::now();
	bool dayOrNight = true;
	float modifyDiffuseStrength = 0.5f;
	float modifyAmbientStrength = 0.5f;
	float modifySpecularStrength = 0.5f;
	
	GLFWwindow* window;
	InitializeWindow(window);

	// Build and compile shaders
	std::string strFullExeFileName = argv[0];
	InitializePaths(strFullExeFileName);

	Shader shadowMappingShader(pathToShadowMappingShaders + "ShadowMapping.vs", pathToShadowMappingShaders + "ShadowMapping.fs");
	Shader shadowMappingDepthShader(pathToShadowMappingShaders + "ShadowMappingDepth.vs", pathToShadowMappingShaders + "ShadowMappingDepth.fs");

	std::string pathToObjectShaders(pathToShaders + "Objects\\");
	Shader shaderModel(pathToObjectShaders + "modelLoading.vs", pathToObjectShaders + "modelLoading.frag");

	// Load models
	std::string pathSub = pathToDetachedSubmarine + "sub_obj.obj";
	std::string pathTerrain = pathToTerrain + "terrain.obj";
	std::string pathWater = pathToWater + "water.obj";

	const char* submarine = pathSub.c_str();
	Model submarineModel((GLchar*)submarine);

	const char* terrain = pathTerrain.c_str();
	Model terrainModel((GLchar*)terrain);

	// configure depth map FBO
// -----------------------
	constexpr unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// shader configuration
	// --------------------
	shadowMappingShader.Use();
	shadowMappingShader.SetInt("diffuseTexture", 0);
	shadowMappingShader.SetInt("shadowMap", 1);

	// lighting info
	glm::vec3 lightPos(-2.0f, 15.0f, -1.0f);

	glEnable(GL_CULL_FACE);

	glm::mat4 projection = glm::perspective(pCamera->GetZoom(), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

	// ** SKYBOX **
	Shader shaderCubeMap(pathToSkyBoxShaders + "cubemaps.vs", pathToSkyBoxShaders + "cubemaps.fs");
	Shader shaderSkybox(pathToSkyBoxShaders + "skybox.vs", pathToSkyBoxShaders + "skybox.fs");
	buildSkybox(shaderCubeMap, shaderSkybox, pathToTextures);
	// ** SKYBOX **

	// ** WATER **
	Shader shaderWater(pathToWaterShaders + "water.vs", pathToWaterShaders + "water.fs");

	const char* water = pathWater.c_str();
	Model waterModel((GLchar*)water);
	// ** WATER **

	// ** RENDER LOOP **

	float xPos = 0.0f;
	float yPos = 0.0f;
	float zPos = 0.0f;
	float yaw = -90.0f;
	float pitch = 0.0f;

	while (!glfwWindowShouldClose(window)) {

		// per-frame time logic
		double currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		processInput(window, xPos, yPos, zPos, yaw, pitch);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 view = pCamera->GetViewMatrix();
		RenderSceneWithLight(shadowMappingDepthShader, shadowMappingShader, waterModel, terrainModel, depthMap, depthMapFBO, lightPos, view, projection, modifyDiffuseStrength, modifyAmbientStrength, modifySpecularStrength);

		DrawSkybox(shaderSkybox, view, projection);

		// ** MODEL **
		//DrawObject(shaderModel, submarineModel, view, projection, 0.5f);

		auto t_now = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

		DrawAndRotateSubmarineObject(shaderModel, submarineModel, view, projection, 0.5f, time, 0.1f, xPos, yPos, zPos, yaw, pitch);

		/*DrawObject(shaderModel, terrainModel, view, projection, 0.2f);
		DrawObject(shaderModel, waterModel, view, projection, 0.2f);*/
		// ** MODEL **

		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		{
			dayOrNight = !dayOrNight;
			buildSkybox(shaderCubeMap, shaderSkybox, pathToTextures, dayOrNight);
		}
		if (glfwGetKey(window, GLFW_KEY_KP_1) == GLFW_PRESS)
		{
			modifyAmbientStrength += 0.1f;
		}
		if (glfwGetKey(window, GLFW_KEY_KP_2) == GLFW_PRESS)
		{
			modifyAmbientStrength -= 0.1f;
		}
		if (glfwGetKey(window, GLFW_KEY_KP_3) == GLFW_PRESS)
		{
			modifyDiffuseStrength += 0.1f;
		}
		if (glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS)
		{
			modifyDiffuseStrength -= 0.1f;
		}
		if (glfwGetKey(window, GLFW_KEY_KP_5) == GLFW_PRESS)
		{
			modifySpecularStrength += 0.1f;
		}
		if (glfwGetKey(window, GLFW_KEY_KP_6) == GLFW_PRESS)
		{
			modifySpecularStrength -= 0.1f;
		}
		if (glfwGetKey(window, GLFW_KEY_KP_0) == GLFW_PRESS)
		{
			std::cout << "Ambient: " << modifyAmbientStrength << " " << "Diffuse: " << modifyDiffuseStrength << "Specular: " << modifySpecularStrength << std::endl;
		}

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	// ** RENDER LOOP **

	// optional: de-allocate all resources once they've outlived their purpose:
	delete pCamera;

	glfwTerminate();
	return 0;
}