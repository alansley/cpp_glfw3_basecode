#ifndef CAMERA_H
#define CAMERA_H

#include <glm/ext/scalar_constants.hpp>

#include "GLFW/glfw3.h" // Include GL Framework. Note: This pulls in GL.h for us.
#include "glm/glm.hpp"  // Include GL Mathematics library
#include "glm/gtx/string_cast.hpp"

using glm::vec3;

// Class to act as a camera that can respond to mouse movements to look around and
// keypresses to move forward/back/left/right relative to the direction we're facing.
class Camera
{
private:
	// Core camera attributes
	vec3 position, rotation, speed;

	// How fast the camera moves when a key is held down, and how sensitive it is to pitch (up/down) and yaw (left/right) movement
	float movementSpeedFactor, pitchSensitivity, yawSensitivity;

	// Window midpoint locations
	int windowMidX, windowMidY;

	// Boolean flags to keep track of which movement keys are being held
	bool holdingForward, holdingBackward, holdingLeftStrafe, holdingRightStrafe, holdingAscend, holdingDescend;

	// Inline function to convert degrees to radians
	// Note: Although we suggest that the compiler should inline the method, it'll
	// make up its own mind about whether this actually occurs.
	inline float toRads(const float &angleDegs) const {	return angleDegs * DEGS_TO_RADS; }

public:
	// Conversion factors for radians and degrees
	static inline const float DEGS_TO_RADS = glm::pi<float>() / 180.0f;
	static inline const float RADS_TO_DEGS = 180.0f / glm::pi<float>();

	// Constructor
	Camera(vec3 initialPosition, vec3 initialRotation, GLint windowWidth, GLint windowHeight);

	//  Specify our keyboard handling method
	void handleKeypress(GLint key, GLint action);

	// Keyboard and mouse handlers
	void handleMouseMove(GLFWwindow *window, double mouseX, double mouseY);

	// Method to convert an angle in degress to radians
	//const float toRads(const float &angleInDegrees) const;

	// Method to move the camera based on the current direction in a frame-rate independent manner 
	void move(double deltaTime);

	// --------------------------------- Inline methods ----------------------------------------------
	// Note: C++ methods declared in headers get 'inlined' if the compiler deems it a good idea

	// Window midpoint setter
	inline void updateWindowMidpoint(GLsizei windowWidth, GLsizei windowHeight)
	{
		windowMidX = windowWidth  / 2;
		windowMidY = windowHeight / 2;
	}

	// Pitch and yaw sensitivity getters and setters
	float getPitchSensitivity() const      { return pitchSensitivity;  }
	void  setPitchSensitivity(float value) { pitchSensitivity = value; }
	float getYawSensitivity() const        { return yawSensitivity;    }
	void  setYawSensitivity(float value)   { yawSensitivity   = value; }

	// Position setters and getters
	void  setLocation(vec3 l)  { position = l;      }
	vec3  getLocation()  const { return position;   }
	float getXLocation() const { return position.x; }
	float getYLocation() const { return position.y; }
	float getZLocation() const { return position.z; }

	// Rotation setters and getters
	void  setRotationDegs(vec3 eulerDegs)  { rotation = eulerDegs; }
	vec3  getRotationDegs()  const         { return rotation;      }
	float getXRotationDegs() const         { return rotation.x;	   }
	float getYRotationDegs() const         { return rotation.y;    }
	float getZRotationDegs() const         { return rotation.z;    }

	vec3  getRotationRads() const  { return rotation * DEGS_TO_RADS;   }
	float getXRotationRads() const { return rotation.x * DEGS_TO_RADS; }
	float getYRotationRads() const { return rotation.y * DEGS_TO_RADS; }
	float getZRotationRads() const { return rotation.z * DEGS_TO_RADS; }
};

#endif // CAMERA_H
