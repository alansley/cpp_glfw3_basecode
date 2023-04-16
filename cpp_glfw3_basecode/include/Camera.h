#ifndef CAMERA_H
#define CAMERA_H

#include "GLFW/glfw3.h" // Include GL Framework. Note: This pulls in GL.h for us.
#include "glm/glm.hpp"  // Include GL Mathematics library
#include "glm/gtx/string_cast.hpp"
#include <glm/ext/scalar_constants.hpp>

using glm::vec3;

// Class to act as a camera that responds to mouse movement to look around and keypresses to move around
class Camera
{
private:
	// Core camera attributes
	vec3 position, rotation, speed;

	// How fast the camera moves when a key is held down, and how sensitive it is to pitch (up/down) and yaw (left/right) movement
	float movementSpeedFactor, pitchSensitivity, yawSensitivity;

	// Window midpoint locations
	int windowMidX{}, windowMidY{};

	// Bools to keep track of which movement keys are being held
	bool holdingForward, holdingBackward, holdingLeftStrafe, holdingRightStrafe, holdingAscend, holdingDescend;

	// Inline function to convert between degrees and radians
	// Note: Although we suggest that the compiler should inline the method, it'll make up its own mind about whether this actually occurs.
	[[nodiscard]] inline float toRads(const float &angleDegs) const {	return angleDegs * DEGS_TO_RADS; }
	[[nodiscard]] inline float toDegs(const float& angleRads) const { return angleRads * RADS_TO_DEGS; }

public:
	// Conversion factors for radians and degrees
	static inline const float DEGS_TO_RADS = glm::pi<float>() / 180.0f;
	static inline const float RADS_TO_DEGS = 180.0f / glm::pi<float>();

	// Constructor
	Camera(vec3 initialLocation, vec3 initialRotationDegs, GLint windowWidth, GLint windowHeight);

	//  Specify our keyboard handling method
	void handleKeypress(GLint key, GLint action);

	// Keyboard and mouse handlers
	void handleMouseMove(GLFWwindow *window, double mouseX, double mouseY);

	// Method to move the camera based on the current direction in a frame-rate independent manner 
	void move(double deltaTime);

	// ---------- Inline Methods ----------
	// Note: C++ methods declared in headers get 'inlined' if the compiler deems it a good idea.
	// Also: We don't have to specify the `inline` keyword in modern C++.

	// Window midpoint setter
	void updateWindowMidpoint(GLsizei windowWidth, GLsizei windowHeight)
	{
		windowMidX = windowWidth  / 2;
		windowMidY = windowHeight / 2;
	}

	// Pitch and yaw sensitivity getters and setters
	[[nodiscard]] float getPitchSensitivity() const { return pitchSensitivity;  }
	void setPitchSensitivity(float value)           { pitchSensitivity = value; }
	[[nodiscard]] float getYawSensitivity() const   { return yawSensitivity;    }
	void setYawSensitivity(float value)             { yawSensitivity   = value; }

	// Position setters and getters
	void  setPosition(vec3 newPosition)      { position = newPosition; }
	[[nodiscard]] vec3  getPosition()  const { return position;        }
	[[nodiscard]] float getXPosition() const { return position.x;      }
	[[nodiscard]] float getYPosition() const { return position.y;      }
	[[nodiscard]] float getZPosition() const { return position.z;      }

	// Rotation setters and getters
	void  setRotationDegs(vec3 eulerDegs)        { rotation = eulerDegs; }
	[[nodiscard]] vec3  getRotationDegs()  const { return rotation;      }
	[[nodiscard]] float getXRotationDegs() const { return rotation.x;	 }
	[[nodiscard]] float getYRotationDegs() const { return rotation.y;    }
	[[nodiscard]] float getZRotationDegs() const { return rotation.z;    }

	[[nodiscard]] vec3  getRotationRads()  const { return rotation * DEGS_TO_RADS;   }
	[[nodiscard]] float getXRotationRads() const { return rotation.x * DEGS_TO_RADS; }
	[[nodiscard]] float getYRotationRads() const { return rotation.y * DEGS_TO_RADS; }
	[[nodiscard]] float getZRotationRads() const { return rotation.z * DEGS_TO_RADS; }
};

#endif // CAMERA_H
