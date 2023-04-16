#ifndef SHADER_PROGRAM_HPP
#define SHADER_PROGRAM_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <list>

#ifndef __glad_h_
	#include "glad/glad.h"
#endif

#include "Utils.hpp"

// Save some typing
using std::cout;
using std::endl;
using std::fstream;
using std::map;
using std::string;
using std::stringstream;
using std::list;
using std::pair;

// Custom typedefs to make it easy to store and work with a list of shader pairs.
//
// Note: Each ShaderPair contains the type of shader (GL_VERTEX_SHADER etc) and the id of the shader.
//       The ShaderPairList is a list of each shader pair in our shader program.
typedef pair<GLenum, GLuint> ShaderPair;
typedef list<ShaderPair> ShaderPairList;

class ShaderProgram
{
private:
    // Class DEBUG flag - if set to false then, errors aside, we'll run completely silently
	static const bool DEBUG = true;

    // We'll use an enum to differentiate between shaders and shader programs when querying the info log
	enum class ShaderObjectType { SHADER, PROGRAM };

    // Shader program and individual shader Ids
	GLuint programId;
	GLuint vertexShaderId;
	GLuint fragmentShaderId;
	GLuint tessEvaluationShaderId;
	GLuint tessControlShaderId;

	// A human-friendly description of the shader program
	string shaderProgramName;

    // How many shaders are attached to the shader program
	GLuint shaderCount;

	// Map of attributes and their binding locations
	map<string,int> attributeMap;

	// Map of uniforms and their binding locations
	map<string,int> uniformMap;

	// List of shader pairs - each pair has the type of shader as 'first' and the shaderId as 'second'
	ShaderPairList shaderPairList;

    // Has this shader program been initialised?
	bool initialised;

public:

    // Constructor
	ShaderProgram(string name)
	{
	    shaderProgramName = name;
	    if (DEBUG) { cout << "\n----- Shader Program: " << name << " -----" << endl; }

		// We start in a non-initialised state - calling initFromFiles() or initFromStrings() will initialise us
		initialised = false;

		// Generate a unique Id / handle for the shader program.
		// Note: We MUST have a valid rendering context before generating the programId or we'll segfault!
		programId = glCreateProgram();

		// Initially, we have zero shaders attached to the program
		shaderCount = 0;
	}

	// Destructor
	~ShaderProgram()
	{
		// Delete the shader program from the graphics card memory to free all the resources it's been using
		glDeleteProgram(programId);
	}

	// Method to compile a shader of a given type
	GLuint addShader(GLenum shaderType, string shaderSource)
	{
		string shaderTypeString;
		switch (shaderType)
		{
			case GL_VERTEX_SHADER:
				shaderTypeString = "GL_VERTEX_SHADER";
				break;
			case GL_FRAGMENT_SHADER:
				shaderTypeString = "GL_FRAGMENT_SHADER";
				break;
            case GL_TESS_CONTROL_SHADER:
				shaderTypeString = "GL_TESS_CONTROL_SHADER";
				break;
            case GL_TESS_EVALUATION_SHADER:
				shaderTypeString = "GL_TESS_EVALUATION_SHADER";
				break;
			case GL_GEOMETRY_SHADER:
				cout << "[ERROR] Geometry shaders are unsupported at this time." << endl;
				Utils::getKeypressThenExit();
				break;
            default:
				cout << "[ERROR] Bad shader type enum in addShader." << endl;
				Utils::getKeypressThenExit();
				break;
		}

		// Generate a shader id
		// Note: Shader id will be non-zero if successfully created.
		GLuint shaderId = glCreateShader(shaderType);
		if (shaderId == 0)
		{
			// Get and display the shader log
			cout << "[ERROR] Could not create shader of type " << shaderTypeString << ": " << getInfoLog(ShaderObjectType::SHADER, shaderId) << endl;
			Utils::getKeypressThenExit();
		}

		// Get the source string as a pointer to an array of characters
		const char *shaderSourceChars = shaderSource.c_str();

		// Attach the GLSL source code to the shader
		// Note: The 2nd arg (`count` is how many elements are in the array of shader source code we're providing. As we're
		// just giving a single shader we'll say that it's 1).
		// Also: The pointer to an array of source chars will be null terminated, so we don't need to specify the source array length (4th arg) and can just use NULL.
		glShaderSource(shaderId, 1, &shaderSourceChars, NULL);

		// Compile the shader
		glCompileShader(shaderId);

		// Check the compilation status and report any errors
		GLint shaderStatus;
		glGetShaderiv(shaderId, GL_COMPILE_STATUS, &shaderStatus);
		if (shaderStatus == GL_FALSE)
		{
			cout << shaderTypeString << " compilation failed: " << getInfoLog(ShaderObjectType::SHADER, shaderId) << endl;
			Utils::getKeypressThenExit();
		}
		else // All good!
		{
			if (DEBUG) { cout << "[OK] " << shaderTypeString << " shader compilation successful." << endl; }

			// Add the map of shader-type to shader-id to the list of shader maps
			ShaderPair tempPair(shaderType, shaderId);
			ShaderPairList::iterator it = shaderPairList.end();
			shaderPairList.insert(it, tempPair);
		}

		++shaderCount;

        // Assuming everything went well, return the shader id
		return shaderId;
	}

	// Method to compile/attach/link/verify the shaders.
	// Note: Rather than returning a boolean as a success/fail status we'll just consider
	// a failure here to be an unrecoverable error and abort on failure.
	void initialise()
	{
	    // Attach all added shaders
	    for (ShaderPairList::iterator it = shaderPairList.begin(); it != shaderPairList.end(); ++it)
        {
            ShaderPair tempPair = *it;
            //GLenum shaderType = tempPair.first;
            GLuint shaderId   = tempPair.second;
            glAttachShader(programId, shaderId);
        }

		// Link the shader program - details are placed in the program info log
		glLinkProgram(programId);

		// Once the shader program has the shaders attached and linked, the shaders are no longer required.
		// If the linking failed, then we're going to abort anyway so we still detach the shaders.
		for (ShaderPairList::iterator it = shaderPairList.begin(); it != shaderPairList.end(); ++it)
        {
            ShaderPair tempPair = *it;
            //GLenum shaderType = tempPair.first;
            GLuint shaderId = tempPair.second;
            glDetachShader(programId, shaderId);
        }

		// Check the link status - zero indicates success.
		GLint programLinkSuccess;
		glGetProgramiv(programId, GL_LINK_STATUS, &programLinkSuccess);
		if (programLinkSuccess == GL_TRUE)
		{
			if (DEBUG) { cout << "[OK] Shader program link successful." << endl; }
		}
		else // Get the program info log, display it, and return false to indicate failure
        {
            cout << "[ERROR] Shader program link failed: " << getInfoLog(ShaderObjectType::PROGRAM, programId) << endl;
			Utils::getKeypressThenExit();
		}

		// Validate the program
		glValidateProgram(programId);

		// Check the validation status - GL_TRUE indicates success.
		GLint programValidatationStatus;
		glGetProgramiv(programId, GL_VALIDATE_STATUS, &programValidatationStatus);
		if (programValidatationStatus == GL_TRUE)
		{
			if (DEBUG) { cout << "[OK] Shader program validation successful." << endl; }
		}
		else // Get the program info log, display it, and bail
		{
			cout << "[ERROR] Shader program validation failed. Reason: " << getInfoLog(ShaderObjectType::PROGRAM, programId) << endl;
			Utils::getKeypressThenExit();
		}

		if (DEBUG)
		{
			// Spit the active attributes
			GLint activeAttributeCount;
			glGetProgramiv(programId, GL_ACTIVE_ATTRIBUTES, &activeAttributeCount);
			cout << "Shader program active attributes: " << activeAttributeCount << endl;

			// Spit the active uniforms
			GLint activeUniformCount;
			glGetProgramiv(programId, GL_ACTIVE_UNIFORMS, &activeUniformCount);
			cout << "Shader program active uniforms: " << activeUniformCount << endl;
		}

		// Finally, the shader program is initialised
		initialised = true;
	}

	// Method to load the shader source code from a file
	string static loadShaderFromFile(const string filename)
	{
	    // Create an input filestream and attempt to open the specified file
		std::ifstream file( filename.c_str() );

        // If we couldn't open the file we'll bail out
		if ( !file.good() )
		{
			cout << "Failed to open file: " << filename << endl;
			Utils::getKeypressThenExit();
		}

		// Otherwise, create a string stream, dump the contents of the file into it, close the file & return its contents as a string
		stringstream stream;
		stream << file.rdbuf();
		file.close();
		return stream.str();
	}

	string getInfoLog(ShaderObjectType type, int id)
	{
	    // Get the length of the log in bytes
		GLint infoLogLength;
		if (type == ShaderObjectType::SHADER)
		{
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
			//cout << "Got shader log length of: " << infoLogLength << endl;
		}
		else // type must be ShaderObjectType::PROGRAM
		{
			glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
			//cout << "Got program log length of: " << infoLogLength << endl;
		}

		// Now get the log itself
		GLchar *infoLog = new GLchar[infoLogLength + 1];
		if (type == ShaderObjectType::SHADER)
		{
			glGetShaderInfoLog(id, infoLogLength, &infoLogLength, infoLog);
		}
		else // type must be ShaderObjectType::PROGRAM
		{
			glGetProgramInfoLog(id, infoLogLength, &infoLogLength, infoLog);
		}

		// Convert the info log to a string, delete the char array and return the info log as a string
		string infoLogString(infoLog);
		delete[] infoLog;
		return infoLogString;
	}

	// Method to enable the shader program - we'll suggest this for inlining
	inline void use()
	{
	    // Sanity check that we're initialised and ready to go...
	    if (initialised)
        {
            glUseProgram(programId);
        }
        else
        {
            cout << "[ERROR] Shader program " << programId << " not initialised!" << endl;
			Utils::getKeypressThenExit();
        }
	}

	// Method to disable the shader - we'll also suggest this for inlining
	inline void disable() { glUseProgram(0); }

	// Method to return the bound location of a named attribute.
	// Note: Be careful in the shader that you actually USE the attribute - non-used attributes can get automatically stripped!
	GLuint attribute(const string attributeName) { return attributeMap[attributeName]; }

	// Method to return the bound location of a named uniform.
	// Note: Be careful in the shader that you actually USE the uniform - non-used uniforms can get automatically stripped!
	GLuint uniform(const string uniformName) { return uniformMap[uniformName]; }

	// Method to add an attribute to the shader and return the bound location
	int bindAttribute(const string attributeName)
	{
	    // Add the attribute location value for the attributeName key
		attributeMap[attributeName] = glGetAttribLocation( programId, attributeName.c_str() );

		// Check to ensure that the shader contains an attribute with this name
		if (attributeMap[attributeName] == -1)
		{
			cout << "Could not add attribute: " << attributeName << " - location returned -1!" << endl;
			Utils::getKeypressThenExit();
		}
		else // Valid attribute location? Inform user if we're in debug mode.
		{
		    if (DEBUG)
            {
                cout << "Attribute: " << attributeName << " bound to location: " << attributeMap[attributeName] << endl;
            }
		}

        // Return the attribute location
		return attributeMap[attributeName];
	}

	// Method to add a uniform to the shader and return the bound location
	int bindUniform(const string uniformName)
	{
	    // Add the uniform location value for the uniformName key
		uniformMap[uniformName] = glGetUniformLocation( programId, uniformName.c_str() );

		// Check to ensure that the shader contains a uniform with this name
		if (uniformMap[uniformName] == -1)
		{
			cout << "Could not add uniform: " << uniformName << " - location returned -1!" << endl;
			Utils::getKeypressThenExit();
		}
		else // Valid uniform location? Inform user if we're in debug mode.
		{
		    if (DEBUG)
            {
                cout << "Uniform: " << uniformName << " bound to location: " << uniformMap[uniformName] << endl;
            }
		}

        // Return the uniform location
		return uniformMap[uniformName];
	}

	GLuint getProgramID() { return programId; }

};

#endif // SHADER_PROGRAM_HPP
