#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>

// Downloaded from https://registry.khronos.org/OpenGL/index_gl.php and req'd for GL_CLAMP_TO_EDGE.
// Note: `glext.h` does NOT come with GLAD so I had to grab it separately (although I put it in the `libs/GLAD/include/KHR` directory as "best-fit").
#include "KHR/glext.h" 

#include "GLFW/glfw3.h"    
#include "glm/glm.hpp"     
#include "stb/stb_image.h"

using std::string;
using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;

class Utils
{

public:
    // Define degree/radian conversion constants
    inline const static float DEGS_TO_RADS = 3.141592654f / 180.0f;
    inline const static float RADS_TO_DEGS = 180.0f / 3.141592654f;

    // Define our cardinal axes
    inline const static glm::vec3 X_AXIS = glm::vec3(1.0f, 0.0f, 0.0f); // Positive x-axis points directly to the right
    inline const static glm::vec3 Y_AXIS = glm::vec3(0.0f, 1.0f, 0.0f); // Positive y-axis points directly up
    inline const static glm::vec3 Z_AXIS = glm::vec3(0.0f, 0.0f, 1.0f); // Positive z-axis points directly out of the screen

    // Method to load a texture from file and return the texture ID
    //GLuint loadTexture(string filename);
    //GLuint loadTexture(string filenameString, GLenum internalImageFormat = GL_RGBA, GLenum minificationFilter = GL_LINEAR, GLenum magnificationFilter = GL_LINEAR);


    // Function to make the user press enter and then exit the program - called when we hit an unrecoverable error
    // so that debug output can be displayed in the console before we kill the application.
    static void getKeypressThenExit()
    {
        cout << "Press <Enter> to continue... " << endl;
        cin.get();
        exit(-1);
    }

    static double randRange(double minValue, double maxValue)
    {
        return minValue + (((double)rand() / (double)RAND_MAX) * (maxValue - minValue));
    }

    static float randRange(float minValue, float maxValue)
    {
        return minValue + (((float)rand() / (float)RAND_MAX) * (maxValue - minValue));
    }

    static int randRange(int minValue, int maxValue)
    {
        return ((int)rand() % (maxValue - minValue + 1)) + minValue;
    }


    static GLuint loadTexture(string filename, GLenum minificationFilter = GL_LINEAR, GLenum magnificationFilter = GL_LINEAR, bool flipTextureVertically = false, bool verbose = true)
    {
        if (flipTextureVertically)
        {
            // OpenGL uses (0,0) as the bottom-left texture coordinate and (1,1) as the top-right. However, stb provides the image data
            // in the order that the FILE provides it - which goes from TOP-left to BOTTOM-right. As such, it may be necessary to flip
            // flip the image vertically on load.
            stbi_set_flip_vertically_on_load(true);
        }

        const char* filenameCStr = filename.c_str();
        
        // Query the image to make sure it's valid
        int imageWidth, imageHeight, numChannels;
        bool imageOK = stbi_info(filenameCStr, &imageWidth, &imageHeight, &numChannels);
        if (!imageOK)
        {
            cout << "[ERROR] Could not query image: " << stbi_failure_reason << endl;
            return 0;
        }

        // Pick a suitable OpenGL image format for the texture
        GLenum internalImageFormat;
        if (numChannels == 3)
        {
            internalImageFormat = GL_RGB;
            if (verbose) { cout << filename << " internal image format is GL_RGB" << endl; }
        }
        else if (numChannels == 4)
        {
            internalImageFormat = GL_RGBA;
            if (verbose) { cout << filename << " internal image format is GL_RGBA" << endl; }
        }
        else
        {
            cout << "[Error] Image internal format is not RGB or RGBA - aborting! " << endl;
            Utils::getKeypressThenExit();
        }

        // Attempt to load the image into a texture
        unsigned char* textureData = stbi_load(filenameCStr, &imageWidth, &imageHeight, &numChannels, 0);

        /*
        // To get the colour of the pixel at (i,j)
        int i = 264;
        int j = 275;
        unsigned int bytesPerPixel = numChannels; // RGB (so 3 bytes) if we don't have an alpha channel, or RGBA (4 bytes) if we do
        unsigned char* pixelOffset = textureData + (i + imageWidth * j) * bytesPerPixel;
        unsigned char r = pixelOffset[0];
        unsigned char g = pixelOffset[1];
        unsigned char b = pixelOffset[2];
        unsigned char a = numChannels >= 4 ? pixelOffset[3] : 0xff;
        cout << "RGBA at (" << i << "," << j << ")" << "is " << (int)r << ", " << (int)g << ", " << (int)b << ", " << (int)a << endl;

        renderMandelbrot(textureData, imageWidth, imageHeight, numChannels);
        */

        // Generate a texture ID and bind to it
        GLuint tempTextureID;
        glGenTextures(1, &tempTextureID);
        glBindTexture(GL_TEXTURE_2D, tempTextureID);

        // Construct the texture.
        // Note: The 'Data format' is the format of the image data as provided by the image library. FreeImage decodes images into
        // BGR/BGRA format, but we want to work with it in the more common RGBA format, so we specify the 'Internal format' as such.
        glTexImage2D(GL_TEXTURE_2D, // Type of texture
                                 0, // Mipmap level (0 being the top level i.e. full size)
               internalImageFormat, // Internal format (typically RGBA)
                        imageWidth, // Width of the texture
                       imageHeight, // Height of the texture,
                                 0, // Border in pixels
               internalImageFormat, // Data format
                  GL_UNSIGNED_BYTE, // Type of texture data
                      textureData); // The data to use for this texture

        // Specify our minification and magnification filters
        // TODO: Set GL_TEXTURE_WRAP_S and GL_TEXTURE_WRAP_T if you want. GL_CLAMP_TO_BORDER, GL_CLAMP_TO_EDGE or GL_REPEAT would do it.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minificationFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magnificationFilter);

        // Free the image data now now that we have it as a texture
        stbi_image_free(textureData);

        // Unbind the texture & return the ID for use w/ OpenGL
        glBindTexture(GL_TEXTURE_2D, 0);
        return tempTextureID;
    }

    // Method to return the memory address of a given texture at the (i,j) position given the image width and number of channels (i.e. RGB->3 or RGBA->4).
    static unsigned char* getPixelOffsetAddress(unsigned char* textureData, int i, int j, int imageWidth, int numChannels)
    {
        return textureData + (i + imageWidth * j) * numChannels;
    }

    static void renderMandelbrot(unsigned char* textureData, int textureWidth, int textureHeight, int numChannels)
    {
        // Entire standard mandelbrot
        /*
        double regionWidth = 2.5;
        double regionHeight = 2.0;
        double centreX = 0.35935;
        double centreY = -0.091;
        */

        double regionWidth = 0.25;
		double regionHeight = 0.20;
		double centreX = 0.3;
		double centreY = 0.0;

        // Set up some coefficients to colourise our fractal based on its 'escape time' (i.e. iteration)
        //Random random = new Random();

        //static bool initialised = false;

        //static int colours[256];

        /*
        if (!initialised)
        {
            for (int loop = 0; loop < 256; loop++)
            {
                colours[loop] = gdImageColorAllocate(imageFile, loop, loop, loop);
            }
            initialised = true;
        }
        */

        // How many times to iterate over our Mandelbrot calculation before aborting
        static const unsigned char MAX_ITERATIONS = 255;

        // Specify ARGB encoding, 8-bits per channel
        //Bitmap.Config bitmapConf = Bitmap.Config.ARGB_8888;

        // Create a mutable bitmap (i.e. one we can modify!) of the specified size
        //Bitmap bmp = Bitmap.createBitmap( (int)bitmapWidth, (int)bitmapHeight, bitmapConf);

        int counter = 0;
        int maxCounterValue = (int)textureWidth * (int)textureHeight;

        double x0, y0, x, y, xtemp;
        unsigned char iteration = 0;

        // Loop through every horizontal row of pixels...
        for (int i = 0; i < textureWidth; i++)
        {
            // Loop through every vertical column of pixels
            for (int j = 0; j < textureHeight; j++)
            {
                x0 = centreX - (regionWidth / 2) + regionWidth * ((double)i / textureWidth);
                y0 = centreY - (regionHeight / 2) + regionHeight * ((double)j / textureHeight);

                // Initialise for this pixel's calculations
                x = y = 0.0;
                iteration = 0;

                // Perform the Mandelbrot calculations
                while ((x * x + y * y < 4) && (iteration < MAX_ITERATIONS))
                {
                    xtemp = x * x - y * y + x0;
                    y = 2.0 * x * y + y0;
                    x = xtemp;
                    ++iteration;
                }

                // Set that colour for the current pixel

                auto pixelOffset = getPixelOffsetAddress(textureData, i, j, textureWidth, numChannels);

                *pixelOffset     = iteration; // Red
                *(pixelOffset+1) = iteration; // Green
                *(pixelOffset+2) = iteration; // Blue
                *(pixelOffset+3) = 255;       // Alpha

                //gdImageSetPixel(imageFile, i, j, colours[iteration]);


            } // End of bitmapHeight loop

        } // End of bitmapWidth loop

        // Finished? Return the bitmap!
       // return imageFile;
    }

    // Templated function convert a string into a float
    template<typename T>
    float stringToFloat(const std::string& numberAsString)
    {
        float number;
        std::stringstream stream(numberAsString);
        stream >> number;
        if (stream.fail())
        {
            cerr << "Failed to convert the following to a number: " << numberAsString << endl;
            return 0.0f;
        }
        return number;
    }

    // Templated function to convert various types of data to string
    template<class T>
    static std::string toString(const T& t)
    {
        std::ostringstream stream;
        stream << t;
        return stream.str();
   }

};
#endif // UTILS_HPP
