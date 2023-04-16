#include "Camera.h"

// Constructor
Camera::Camera(vec3 initialLocation, const vec3 initialRotationDegs, const GLint windowWidth, const GLint windowHeight)
{
	// Set position, rotation and speed values to zero
	position = initialLocation;
	rotation = initialRotationDegs;
	speed    = vec3(0.0f);

	// How fast we move (higher values mean we move and strafe faster)
	movementSpeedFactor = 200.0f;

	pitchSensitivity = 0.15f; // Mouse sensitivity when looking up and down
	yawSensitivity   = 0.15f; // Mouse sensitivity when looking left and right

	// Holding any keys down?
	holdingForward     = false;
	holdingBackward    = false;
	holdingLeftStrafe  = false;
	holdingRightStrafe = false;
	holdingAscend      = false;
	holdingDescend     = false;

	// Calculate the midpoint of the window
	updateWindowMidpoint(windowWidth, windowHeight);
}

// Method to toggle our movement flags based on key presses
void Camera::handleKeypress(GLint key, GLint action)
{
	// If a key is PRESSED toggle the relevant key-press flag
	// Note: The GLFW_REPEAT check helps us to not lose keypresses when holding keys down for more than a few seconds.
	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		switch (key)
		{
			case GLFW_KEY_W:
				holdingForward = true;
				break;
			case GLFW_KEY_S:
				holdingBackward = true;
				break;
			case GLFW_KEY_A:
				holdingLeftStrafe = true;
				break;
			case GLFW_KEY_D:
				holdingRightStrafe = true;
				break;
			case GLFW_KEY_E:
				holdingAscend = true;
				break;
			case GLFW_KEY_Q:
				holdingDescend = true;
				break;
			default:
				// Do nothing...
				break;
		}
	}
	else // If a key is released, toggle the relevant flag
	{
		switch (key)
		{
			case GLFW_KEY_W:
				holdingForward = false;
				break;
			case GLFW_KEY_S:
				holdingBackward = false;
				break;
			case GLFW_KEY_A:
				holdingLeftStrafe = false;
				break;
			case GLFW_KEY_D:
				holdingRightStrafe = false;
				break;
			case GLFW_KEY_E:
				holdingAscend = false;
				break;
			case GLFW_KEY_Q:
				holdingDescend = false;
				break;
			default:
				// Do nothing...
				break;

		} // End of switch block

	} // End of else GLFW_RELEASE block
}

// Method to deal with mouse position changes
void Camera::handleMouseMove(GLFWwindow *window, double mouseX, double mouseY)
{
	// Calculate our horizontal and vertical mouse movement from middle of the window
	float horizMouseMovement = (mouseX - windowMidX) * yawSensitivity;
	float vertMouseMovement  = (mouseY - windowMidY) * pitchSensitivity;

	// Apply the mouse movement to our rotation vector. The vertical (look up and down
    // i.e. pitch) rotation is applied on the X axis, and the horizontal (look left and
    // i.e. yaw) rotation is applied on the Y Axis.
	rotation.x += vertMouseMovement;
	rotation.y += horizMouseMovement;

	// Limit looking up to vertically up
	if (rotation.x < -90.0f) { rotation.x = -90.0f; }

	// Limit looking down to vertically down
	if (rotation.x >  90.0f) { rotation.x = 90.0f;  }

	// If you prefer to keep the angles in the range -180 to +180 use this code and comment out the 0 to 360 code below.
	//
	// Looking left and right. Keep the angles in the range -180.0f (anticlockwise turn looking behind) to 180.0f (clockwise turn looking behind)
	/*
	if (rotation.y < -180.0f) { rotation.y += 360.0f; }
	if (rotation.y > 180.0f)  { rotation.y -= 360.0f; }
	*/

	// Looking left and right - keep angles in the range 0.0 to 360.0
	// 0 degrees is looking directly down the negative Z axis "North", 90 degrees is "East", 180 degrees is "South", 270 degrees is "West"
	// We can also do this so that our 360 degrees goes -180 through +180 and it works the same, but it's probably best to keep our
	// range to 0 through 360 instead of -180 through +180.
	if (rotation.y < 0.0f)   { rotation.y += 360.0f; }
	if (rotation.y > 360.0f) { rotation.y -= 360.0f; }

	// Reset the mouse position to the centre of the window each frame
	glfwSetCursorPos(window, windowMidX, windowMidY);
}

// Method to calculate which direction we need to move the camera and by what amount
void Camera::move(double deltaTime)
{
	// Vector to break up our movement into components along the X, Y and Z axis
	vec3 movement(0.0f, 0.0f, 0.0f);

	// Get the sine and cosine of our x and y axis rotation (specified in radians)
	float sinXRot = sin(rotation.x * DEGS_TO_RADS);
	float cosXRot = cos(rotation.x * DEGS_TO_RADS);

	float sinYRot = sin(rotation.y * DEGS_TO_RADS);
	float cosYRot = cos(rotation.y * DEGS_TO_RADS);
		
    // This cancels out moving on the Z axis when we're looking up or down
	float pitchLimitFactor = cosXRot;

    // Apply relevant movement to the camera location
	if (holdingForward)	    { movement += vec3(sinYRot * pitchLimitFactor, -sinXRot, -cosYRot * pitchLimitFactor);	}
	if (holdingBackward)    { movement -= vec3(sinYRot * pitchLimitFactor, -sinXRot, -cosYRot * pitchLimitFactor);	}

	if (holdingLeftStrafe)  { movement -= vec3(cosYRot, 0.0f, sinYRot); }
	if (holdingRightStrafe)	{ movement += vec3(cosYRot, 0.0f, sinYRot); }	

	// TODO: Pure world-space ascend/descend only... would be better to take the camera rotation into account.
	if (holdingAscend)  { movement += vec3(0.0f, 1.0f, 0.0f); }
	if (holdingDescend) { movement -= vec3(0.0f, 1.0f, 0.0f); }

	// Normalise our movement vector, but ONLY if it's non-zero! Normalising a vector of zero length
	// leads to the new vector having a length of NaN (Not a Number) because of the divide by zero.
	// Note: Using `> 0.0f` rather than '!= 0.0f' avoids the compiler float comparison warning - we could
	// instead use `> std::epsilon()` from the limit.h header, but I think that would be overkill.
	// Further reading: http://www.cplusplus.com/reference/limits/numeric_limits/
	vec3 normalizedMovement(0.0f, 0.0f, 0.0f);
	if (glm::length(movement) > 0.0f) { normalizedMovement = glm::normalize(movement); }

	// Calculate our value to keep the movement the same speed regardless of the frame rate...
	double framerateIndependentFactor = movementSpeedFactor * deltaTime;

	// .. and then apply it to our movement vector.
	normalizedMovement *= framerateIndependentFactor;

	// Finally, apply the movement to our camera location
	position += normalizedMovement;
}
