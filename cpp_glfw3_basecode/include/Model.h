/***
File          : Model.h
Version       : 0.8
Author        : Al Lansley
Original Date : 26/08/2013
Last Update   : 20/09/2018 - fixed some valgrind memory leaks where I was calling 'delete' instead of 'delete[]' to release data arrays in the destructor.
Purpose: .OBJ format model loader. Handles vertices, normals, normal indices and faces, does not (at present) handle texture coordinates.

         Notes:
         - The .obj files must be set up to use triangles, not quads.

         - Models may have:
            - vertices only,
            - vertices and normals,
            - vertices and normals and faces,
            - vertices and normals and faces and normal indices.

         - If a model has vertices and faces (no normal data) then we calculate normals as a cross-product of the two vectors forming each triangle.

         - This class keeps all the data read from the .obj file in vectors of vec3's called vertices, normals, normalIndices, texCoords, and faces.

         - Once the data from the vectors has been transferred into the vertexData/normalData/faceData arrays then we no longer need the vectors.

         - There are TWO ways to load a model:
                DRAWING_AS_ARRAYS   - Where the data arrays are the expanded versions which can contain duplicates based on the face/index data, and
                DRAWING_AS_ELEMENTS - Where the data arrays DO NOT contain duplicates.

         - When you load a model, you must specify whether you're DRAWING_AS_ARRAYS or DRAWING_AS_ELEMENTS so that the data arrays are populated correctly.
***/

#ifndef MODEL_H
#define MODEL_H

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <cstring>

#ifndef __glad_h_
    #include <glad/glad.h>
#endif

#include "glm/glm.hpp"

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::ifstream;
using std::istringstream;
using std::istream_iterator;
using std::back_inserter;

using glm::vec3;
using glm::vec4;
using glm::mat4;

class Model
{
    public:
        // This enumeration controls whether our data arrays contain duplicates (DRAWING_AS_ARRAYS) or do not (DRAWING_AS_ELEMENTS)
        enum DrawingMethod { DRAWING_AS_ARRAYS = 0, DRAWING_AS_ELEMENTS = 1 };

        // Standard constructor
        Model(DrawingMethod theDrawingMethod);

        // Constructor which also loads a model
        Model(string filename, DrawingMethod theDrawingMethod);

        // Copy constructor to create deep copies of model objects
        // Note: We only deep copy the vertexData and normalData arrays, not all the vectors!
        Model(Model &source);

        // Destructor - frees our allocated pointer memory
        ~Model();

        // Method to load a model
        void load(string filename);

        // Method to read through the model file adding all vertices, faces and normals to our
        // vertices, faces and normals vectors.
        bool readModelFile(string filename);

        // Method to setup our data arrays of floats for OpenGL to work with
        void setupData();

        // Methods to print our data arrays for debugging purposes
        void printVertices();
        void printVertexData();
        void printFaces();
        void printNormals();
        void printNormalData();

        // Simple checker methods to determine information about our model
        bool hasVertices();
        bool hasFaces();
        bool hasNormals();
        bool hasNormalIndices();

        // Getter methods
        GLvoid* getVertexData();
        GLvoid* getNormalData();
        GLvoid* getNormalIndexData();
        GLvoid* getFaceData();

        GLuint  getVertexDataSizeBytes();
        GLuint  getNormalDataSizeBytes();
        GLuint  getNormalIndexDataSizeBytes();
        GLuint  getFaceDataSizeBytes();

        GLuint  getNumVertices();
        GLuint  getNumNormals();
        GLuint  getNumNormalIndices();
        GLuint  getNumFaces();
        GLuint  getFaceElementCount();

        DrawingMethod getDrawingMethod();

        // Method to scale the size of a model uniformly
        void scale(float scale);

        // Method to scale the size of a model on separate axes
        void scale(float xScale, float yScale, float zScale);

        // Compare two floats to be within a threshold, or not
        bool is_near(float v1, float v2, float threshold);

        int getSimilarVertexIndex(vec3 &in_vertex, vec3 &in_normal, float threshold);

    private:
        // Pointers to vectors of vec3's to store the data read from the model file
        // Note: If we don't assign these to nullptr as we declare them then when attempting to load a model they aren't 0, so we try to free them in Model::initModel() which causes a segfault in Linux
        vector<vec3> *vertices      = nullptr;
        vector<vec3> *normals       = nullptr;
        vector<vec3> *normalIndices = nullptr;
        vector<vec3> *texCoords     = nullptr;
        vector<vec3> *faces         = nullptr;

        // Pointers to arrays of floats to store expanded data made up from the vertices and faces
        GLfloat *vertexData      = nullptr;
        GLfloat *normalData      = nullptr;
        GLfloat *normalIndexData = nullptr;
        GLfloat *texCoordData    = nullptr;
        GLuint  *faceData        = nullptr;

        // Counters to keep track of how many vertices, normals, normal indices, texture coordinates and faces
        GLuint numVertices;
        GLuint numNormals;
        GLuint numNormalIndices;
        GLuint numTexCoords;
        GLuint numFaces;

        // Whether we should populate the data arrays to draw as arrays or elements (controls whether vertex data is duplicated)
        DrawingMethod drawingMethod;

        // Private method to initialise or re-initialise a model
        void initModel();

}; // End of Model class

#endif // MODEL_H
