#pragma once

#include <glfw3.h>

#include "Camera.h"

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
const unsigned int SHADOW_WIDTH = 4096;
const unsigned int SHADOW_HEIGHT = 4096;
const float maxHeight = 2.5f;

// timing
double deltaTime = 0.0f;    // time between current frame and last frame
double lastFrame = 0.0f;

Camera* pCamera = new Camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(-2.0f, 0.0f, -1.0f));

void processInput(GLFWwindow* window, float& xPos, float& yPos, float& zPos, float& yaw, float& pitch) {	// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		pCamera->ProcessKeyboard(FORWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		pCamera->ProcessKeyboard(BACKWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		pCamera->ProcessKeyboard(LEFT, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		pCamera->ProcessKeyboard(RIGHT, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		pCamera->ProcessKeyboard(UP, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		pCamera->ProcessKeyboard(DOWN, (float)deltaTime);

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		pCamera->Reset(width, height);
	}


	const float speed = 0.01f;
	const float rotationSpeed = 0.08f;

    glm::vec3 forwardDirection,rightDirection;

    if (yPos >= maxHeight)
    {
        pitch -= 0.1f;
        if (pitch <= 0)
            yPos -= 0.1f;

    }
    else if (yPos <= -5.0f)
    {
        pitch += 0.1f;
        if (pitch >= 0)
            yPos += 0.1f;
    }
    else
    {
        forwardDirection = (glm::normalize(glm::vec3(
            -cos(glm::radians(pitch)) * sin(glm::radians(yaw)),
            sin(glm::radians(pitch)),
            -cos(glm::radians(pitch)) * cos(glm::radians(yaw))
        )));
        rightDirection = glm::cross(forwardDirection, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    // Move forward
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        xPos += forwardDirection.x * speed;
        yPos += forwardDirection.y * speed;
        zPos += forwardDirection.z * speed;
		pCamera->ProcessKeyboard(FORWARD, (float)(speed * 0.2));
		
        // Check if yPos exceeds the maximum height, if so, set it to the maximum height
        if (yPos > maxHeight)
            yPos = maxHeight;
    }

    // Move backward
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
        xPos -= forwardDirection.x * speed;
        yPos -= forwardDirection.y * speed;
        zPos -= forwardDirection.z * speed;
		pCamera->ProcessKeyboard(BACKWARD, (float)(speed * 0.2));

        // Ensure yPos doesn't go below zero or outside the water
        if (yPos < -5.0f)
            yPos = -5.0f;
    }
    // Strafe right
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
        xPos += rightDirection.x * speed;
        yPos += rightDirection.y * speed;
        zPos += rightDirection.z * speed;
		pCamera->ProcessKeyboard(RIGHT, (float)(speed*0.2));
    }
    // Strafe left
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        xPos -= rightDirection.x * speed;
        yPos -= rightDirection.y * speed;
        zPos -= rightDirection.z * speed;
		pCamera->ProcessKeyboard(LEFT, (float)(speed * 0.2));
    }

    // Handle yaw (turn left/right) and pitch (look up/down)
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        yaw += rotationSpeed;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        yaw -= rotationSpeed;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        pitch += rotationSpeed;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        pitch -= rotationSpeed;

    if (yPos > maxHeight)
        yPos = maxHeight;
    else if (yPos < -5.0f)
        yPos = -5.0f;
    
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {	// glfw: whenever the window size changed (by OS or user resize) this callback function executes
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	pCamera->Reshape(width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	pCamera->MouseControl((float)xpos, (float)ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yOffset) {
	pCamera->ProcessMouseScroll((float)yOffset);
}

// might not be necessary
bool RotateLight = true;
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_L && action == GLFW_PRESS) {
		RotateLight = true;
	}
	if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		RotateLight = false;
	}
}