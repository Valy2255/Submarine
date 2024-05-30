#pragma once

#include <vec3.hpp>
#include <mat4x4.hpp>

#include <GL/glew.h>

const GLfloat ZOOM = 45.0f;
const float SPEED = 7.0f;

enum ECameraMovementType
{
	UNKNOWN,
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN,
	SHIFT,
	SPACE
};

class Camera
{

private:

	// Default camera values
	const float zNEAR = 0.1f;
	const float zFAR = 1000.f;
	const float YAW = -90.0f;
	const float PITCH = 0.0f;
	const float FOV = 45.0f;


public:

	glm::vec3 startPosition;

	Camera(const int width, const int height, const glm::vec3& position);

	void Set(const int width, const int height, const glm::vec3& position);
	void SetPosition(const glm::vec3& position);
	void SetYaw(const float yaw);
	void SetPitch(const float pitch);

	void Reset(const int width, const int height);
	void Reshape(int windowWidth, int windowHeight);

	const glm::mat4 GetViewMatrix() const;
	const glm::vec3 GetPosition() const;
	const glm::mat4 GetProjectionMatrix() const;

	void ProcessKeyboard(ECameraMovementType direction, float deltaTime);
	void MouseControl(float xPos, float yPos);
	void ProcessMouseScroll(float yOffset);

	GLfloat GetZoom();

	void ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);

private:

	void UpdateCameraVectors();

protected:

	const float cameraSpeedFactor = 2.5f;
	//const float cameraSpeedFactor = 100.f;
	const float mouseSensitivity = 0.1f;

	// Perspective properties
	float zNear;
	float zFar;
	float FoVy;
	int width;
	int height;
	bool isPerspective;

	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 right;
	glm::vec3 up;
	glm::vec3 worldUp;

	GLfloat m_zoom;

	// Euler Angles
	float yaw;
	float pitch;

	bool bFirstMouseMove = true;
	float lastX = 0.f, lastY = 0.f;

};