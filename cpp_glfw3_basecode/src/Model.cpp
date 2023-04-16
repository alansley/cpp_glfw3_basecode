#include "Model.h"

// Private method to initialise or re-initialise a model
void Model::initModel()
{
	// Free model vector memory if required
	if (vertices      != 0) { delete vertices;      }
	if (normals       != 0) { delete normals;       }
	if (normalIndices != 0) { delete normalIndices; }
	if (texCoords     != 0) { delete texCoords;     }
	if (faces         != 0) { delete faces;         }

	// Free model data memory if required
	if (vertexData   != 0)  { delete vertexData;    }
	if (normalData   != 0)  { delete normalData;    }
	if (texCoordData != 0)  { delete texCoordData;  }
	if (faceData     != 0)  { delete faceData;      }

	// Create new vectors
	vertices      = new vector<vec3>(); // Vector of vertex data
	normals       = new vector<vec3>(); // Vector of normal data
	normalIndices = new vector<vec3>(); // Vector of normal indices to deal with f v1//n1 v2//n2 v3//n3 data
	texCoords     = new vector<vec3>(); // Vector of texture coordinates
	faces         = new vector<vec3>(); // Vector of faces, consists of 3 vertex indices

	// Our vectors of attributes are initially empty
	numVertices      = 0;
	numNormals       = 0;
	numNormalIndices = 0;
	numTexCoords     = 0;
	numFaces         = 0;

	// Model data pointers initially point at nothing
	vertexData   = 0;
	normalData   = 0;
	texCoordData = 0;
	faceData     = 0;
}

// Simple helper functions to determine information about our model
bool Model::hasVertices()      { return numVertices      > 0; }
bool Model::hasFaces()         { return numFaces         > 0; }
bool Model::hasNormals()       { return numNormals       > 0; }
bool Model::hasNormalIndices() { return numNormalIndices > 0; }

// Constructor
Model::Model(DrawingMethod theDrawingMethod)
{
	drawingMethod = theDrawingMethod;
	initModel();
}

// Constructor which also loads a model
Model::Model(string filename, DrawingMethod theDrawingMethod)
{
	drawingMethod = theDrawingMethod;
	initModel();
	load(filename);
}

// Copy constructor to create deep copies of model objects
// Note: We only deep copy the vertexData and normalData arrays, not all the vectors!
// Usage: Model myNewModel(someExistingModel); // Easy as!
Model::Model(Model &source)
{
	initModel();

	numVertices      = source.numVertices;
	numNormals       = source.numNormals;
	numNormalIndices = source.numNormalIndices;
	numTexCoords     = source.numTexCoords;
	numFaces         = source.numFaces;
	drawingMethod    = source.drawingMethod;

    int tvertDataSize = source.getVertexDataSizeBytes();
    int tnumVertices = tvertDataSize / (3 * sizeof(GLfloat));
    cout << "vertexData size is: " << tvertDataSize << endl;
    cout << "num vertices is: " << tnumVertices << endl;

    int tnormDataSize = source.getNormalDataSizeBytes();
    int tnumNormals = tnormDataSize / (3 * sizeof(GLfloat));
    cout << "vertexData size is: " << tnormDataSize << endl;
    cout << "num normals is: " << tnumNormals << endl;


	// If we have vertex data then allocate space for it and copy the data from the source model
	if (numVertices > 0)
	{
		vertexData = new float[numVertices * 3];
		memcpy(vertexData, source.getVertexData(), source.getVertexDataSizeBytes() );
	}
	else
	{
		cout << "Model created using copy constructor has 0 vertices!" << endl;
		exit(-1);
	}

	// If we have normal data then allocate space for it and copy the data from the source model
	if (numNormals > 0)
	{
		normalData = new float[numNormals * 3];
		memcpy(normalData, source.getNormalData(), source.getNormalDataSizeBytes() );
	}
	else
	{
		cout << "Model created using copy constructor has 0 normals!" << endl;
		exit(-1);
	}

	cout << "Model successfully created through copy constructor." << endl;
}


// Destructor - frees our allocated pointer memory
Model::~Model()
{
	// Because vectors are quirky we have to clear the vector, shrink it and THEN delete it to free the memory used.
	if (vertices      != 0) { vertices->clear();      vertices->shrink_to_fit();      delete vertices;      }
	if (normals       != 0) { normals->clear();       normals->shrink_to_fit();       delete normals;       }
	if (normalIndices != 0) { normalIndices->clear(); normalIndices->shrink_to_fit(); delete normalIndices; }
	if (texCoords     != 0) { texCoords->clear();     texCoords->shrink_to_fit();     delete texCoords;     }
	if (faces         != 0) { faces->clear();         faces->shrink_to_fit();         delete faces;         }

	// Free model data memory if required
	if (vertexData   != 0)  { delete[] vertexData;   }
	if (normalData   != 0)  { delete[] normalData;   }
	if (texCoordData != 0)  { delete[] texCoordData; }
	if (faceData     != 0)  { delete[] faceData;     }
}

// Method to load a model
void Model::load(string filename)
{
	// Re-initialise our model
	initModel();

	// Load the model file
	bool modelLoadedCleanly = readModelFile(filename);

	if (modelLoadedCleanly)
	{
		cout << "\n----- Model: " << filename << " loaded cleanly ----- " << endl;
	}
	else
	{
		cout << "Model " << filename << " load attempt produced errors =/" << endl;
	}

	if ( hasVertices() )
	{
		cout << "Vertex count: " << getNumVertices() << endl;
	}
	else
	{
		cout << "Model has no vertices - exiting." << endl;
		exit(-1);
	}

	if ( hasFaces() )         { cout << "Face count: " << getNumFaces() << endl;                 }
	if ( hasNormals() )	      { cout << "Normal count: " << getNumNormals() << endl;             }
	if ( hasNormalIndices() ) {	cout << "Normal index count: " << getNumNormalIndices() << endl; }

	// Transfer the loaded data in our vectors to the data arrays
	setupData();
}


// Method to read through the model file adding all vertices, faces and normals to our
// vertices, faces and normals vectors.
// Note: This does NOT transfer the data into our vertexData, faceData or normalData arrays!
//       That must be done as a separate step by calling setupData() after building up the
//       vectors with this method!
// Also: This method does not decrement the face number of normal index by 1 (because .OBJ
//       files start their counts at 1) to put them in a range starting from 0, that job
//       is done in the setupData() method performed after calling this method!
bool Model::readModelFile(string filename)
{
	bool loadedCleanly = true;

	// Open the model file to read from
	ifstream file( filename.c_str() );

	if (!file.good() )
	{
		std::cerr << "Failed to open model file: " << filename << endl;
		exit(-1);
	}

	// Read the file line by line, extracting data into the correct arrays
	string tempLine;
	int lineCount = 0;
	while ( file.good() )
	{
		getline(file, tempLine);        // Read a line from the file

		lineCount++;

		// If the line isn't empty, process it...
		if (tempLine.length() > 1)
		{
			istringstream iss(tempLine);    // Convert the string into a stringstream for easy separation

			vector<string> tokens;          // Create a vector of strings to hold the separate elements of the line

			// Actually perform the separation of elements and place them in the tokens vector
			copy( istream_iterator<string>(iss), istream_iterator<string>(), back_inserter< vector<string> >(tokens) );

			// Start at the beginning of the vector of tokens
			vector<string>::iterator iter = tokens.begin();

			// If the first token is "v", then we're dealing with vertex data
			if (*iter == "v")
			{
				// As long as there's 4 tokens on the line...
				if (tokens.size() == 4)
				{
					// ...get them as floats ...
					iter++;
					float x = atof( (*iter).c_str() );

					iter++;
					float y = atof( (*iter).c_str() );

					iter++;
					float z = atof( (*iter).c_str() );

					// ... and push them into the vertices vector ...
					vertices->push_back( vec3(x, y, z) );

					// .. then increase our vertex count.
					numVertices++;
				}
				else // If we got vertex data without 3 components - whine!
				{
					loadedCleanly = false;
					cout << "Found vertex data with wrong component count at line number: " << lineCount << " - Skipping!" << endl;
				}

			} // End of vertex line parsing

			// If the first token is "vn", then we're dealing with normal data
			if (*iter == "vn")
			{
				// As long as there's 4 tokens on the line...
				if (tokens.size() == 4)
				{
					// ...get them as floats ...
					iter++;
					float normalX = atof( (*iter).c_str() );

					iter++;
					float normalY = atof( (*iter).c_str() );

					iter++;
					float normalZ = atof( (*iter).c_str() );

					// ... and push them into the normals vector ...
					normals->push_back( vec3(normalX, normalY, normalZ) );

					// .. then increase our normal count.
					numNormals++;
				}
				else // If we got vertex data without 3 components - whine!
				{
					loadedCleanly = false;
					cout << "Found normal data with wrong component count at line number: " << lineCount << " - Skipping!" << endl;
				}

			} // End of vertex line parsing


			// If the first token is "f", then we're dealing with faces
			if (*iter == "f")
			{
				// Check if there's a double-slash in the line
				size_t pos = tempLine.find("//");

				// As long as there's four tokens on the line and they don't contain a "//"...
				if ( (tokens.size() == 4) && (pos == string::npos) )
				{
					// ...get the face vertices as unsigned ints ...
					iter++;
					GLuint v1  = atoi( (*iter).c_str() );

					iter++;
					GLuint v2 = atoi( (*iter).c_str() );

					iter++;
					GLuint v3 = atoi( (*iter).c_str() );

					// ... and push them into the faces vector ...
					faces->push_back( vec3(v1, v2, v3) );

					// .. then increase our face count.
					numFaces++;
				}
				else if ( (tokens.size() == 4) && (pos != string::npos) ) // 4 tokens and found vertex//normal notation?
				{
					string token, faceToken, normalToken;

					// Get the 1st of three tokens as a string
					iter++;
					token = *iter;

					// Find where the double-slash starts in that token
					size_t faceEndPos = token.find("//");

					// Put sub-string from the start to the beginning of the double-slash into our subToken string
					string faceToken1 = token.substr(0, faceEndPos);
					//cout << "Got faceToken: " << faceToken1 << endl;

					// Mark the start of our next subtoken
					size_t nextTokenStartPos = faceEndPos + 2;

					// Copy from first character after the "//" to the end
					string normalToken1 = token.substr(nextTokenStartPos);

					// Get the 2nd of three tokens as a string
					iter++;
					token = *iter;

					// Find where the double-slash starts in that token
					faceEndPos = token.find("//");

					// Put sub-string from the start to the beginning of the double-slash into our subToken string
					string faceToken2 = token.substr(0, faceEndPos);

					// Mark the start of our next subtoken
					nextTokenStartPos = faceEndPos + 2;

					// Copy from first character after the "//" to the end
					string normalToken2 = token.substr(nextTokenStartPos);

					// Get the 3rd of three tokens as a string
					iter++;
					token = *iter;

					// Find where the double-slash starts in that token
					faceEndPos = token.find("//");

					// Put sub-string from the start to the beginning of the double-slash into our subToken string
					string faceToken3 = token.substr(0, faceEndPos);

					// Mark the start of our next subtoken
					nextTokenStartPos = faceEndPos + 2;

					// Copy from first character after the "//" to the end
					string normalToken3 = token.substr(nextTokenStartPos);

					// Finally, add the normal index to the normal indices vector and increment the count
					normalIndices->push_back( vec3( atoi( normalToken1.c_str() ), atoi( normalToken2.c_str() ), atoi(normalToken3.c_str() ) ) );
					numNormalIndices++;

					// And add the face vertex number to the faces vector and increment the count
					faces->push_back( vec3( atoi( faceToken1.c_str() ), atoi( faceToken2.c_str() ), atoi( faceToken3.c_str() ) ) );
					numFaces++;
				}
				else // If we got face data without 3 components - whine!
				{
					loadedCleanly = false;
					cout << "Found face data with wrong component count at line number: " << lineCount << " - Skipping!" << endl;

				}

			} // End of face line parsing

		} // End of line parsing section

	} // End of file parsing section

	// Close the file
	file.close();

	// Return our boolean flag to say whether we loaded the model cleanly or not
	return loadedCleanly;
}

// Returns true if v1 can be considered equal to v2
bool Model::is_near(float v1, float v2, float threshold) { return fabs(v1 - v2) <= threshold; }

// TODO: THIS CAN BE DELETED - IT'S NOT EVEN USED!------------------------------------------------------------------------------------------------------
// Searches through all already-exported vertices for a similar one.
// Similar = same position + same UVs + same normal
int Model::getSimilarVertexIndex(vec3 &in_vertex, vec3 &in_normal, float threshold)
{
    int foundMatch = -1;

    // Lame linear search
    for ( unsigned int i = 0; i < numVertices * 3; i += 3 )
    {
        if (is_near( in_vertex.x , vertexData[i],   threshold) &&
            is_near( in_vertex.y , vertexData[i+1], threshold) &&
            is_near( in_vertex.z , vertexData[i+2], threshold) &&
            is_near( in_normal.x , normalData[i],   threshold) &&
            is_near( in_normal.y , normalData[i+1], threshold) &&
            is_near( in_normal.z , normalData[i+2], threshold)
        )
        {
            foundMatch = i;
        }
    }
    return foundMatch;
}

// Method to setup our plain arrays of floats for OpenGL to work with
// NOTE: If drawing as arrays, we CANNOT have array size mismatches!
void Model::setupData()
{
	if (drawingMethod == DRAWING_AS_ARRAYS)
	{
		cout << "Setting up model data to draw as: Arrays." << endl;

		if ( ( hasVertices() ) && ( hasFaces() ) && ( !hasNormals() ) )
		{
			cout << "Model has vertices and faces, but no normals. Normals will be generated." << endl;

			// Allocate enough space for our vertexData and normalData arrays
			vertexData = new float[numFaces * 3 * 3];
			normalData = new float[numFaces * 3 * 3];

			// Create the vertexData and normalData arrays from the vector of faces
			// Note: We generate the face normals ourselves
			int vertexCount = 0;
			int normalCount = 0;
			for (vector<vec3>::iterator faceIter = faces->begin(); faceIter != faces->end(); faceIter++)
			{
				// Get the numbers of the three vertices that this face is comprised of
				GLuint firstVertexNum  = (GLuint)(*faceIter).x;
				GLuint secondVertexNum = (GLuint)(*faceIter).y;
				GLuint thirdVertexNum  = (GLuint)(*faceIter).z;

				// Now that we have the vertex numbers, we need to get the actual vertices
				// Note: We subtract 1 from the number of the vertex because faces start at
				//       face number 1 in the .oBJ format, while in our code the first vertex
				//       will be at location zero (i.e. vertices.begin() )
				vector<vec3>::iterator vertIter = vertices->begin();

				vec3 faceVert1 = vec3( (*(vertIter + firstVertexNum  - 1) ) );
				vec3 faceVert2 = vec3( (*(vertIter + secondVertexNum - 1) ) );
				vec3 faceVert3 = vec3( (*(vertIter + thirdVertexNum  - 1) ) );

				// Now that we have the 3 vertices, we need to calculate the normal of the face
				// formed by these vertices

				// Convert this vertex data into a pure form
				vec3 v1 = vec3( faceVert2 - faceVert1 );
				vec3 v2 = vec3( faceVert3 - faceVert1 );

				// Generate the normal as the cross product and normalize it
				vec3 normal = vec3( glm::cross(v1, v2) );
				vec3 normalizedNormal = glm::normalize(normal);

				// Put the vertex data into our vertexData array
				vertexData[vertexCount++] = faceVert1.x;
				vertexData[vertexCount++] = faceVert1.y;
				vertexData[vertexCount++] = faceVert1.z;
				vertexData[vertexCount++] = faceVert2.x;
				vertexData[vertexCount++] = faceVert2.y;
				vertexData[vertexCount++] = faceVert2.z;
				vertexData[vertexCount++] = faceVert3.x;
				vertexData[vertexCount++] = faceVert3.y;
				vertexData[vertexCount++] = faceVert3.z;

				// Put the normal data into our normalData array
				// Note: we put the same normal into the normalData array for each of the
				// 3 vertices comprising the face! That's as much as we can do without per-vertex normals!
				normalData[normalCount++] = normalizedNormal.x;
				normalData[normalCount++] = normalizedNormal.y;
				normalData[normalCount++] = normalizedNormal.z;
				normalData[normalCount++] = normalizedNormal.x;
				normalData[normalCount++] = normalizedNormal.y;
				normalData[normalCount++] = normalizedNormal.z;
				normalData[normalCount++] = normalizedNormal.x;
				normalData[normalCount++] = normalizedNormal.y;
				normalData[normalCount++] = normalizedNormal.z;

			} // End of loop iterating over the model faces

			numVertices = vertexCount / 3;
			numNormals  = normalCount / 3;

			cout << "Number of vertices in data array: " << numVertices << " (" << getVertexDataSizeBytes() << " bytes)" << endl;
			cout << "Number of normals  in data array: " << numNormals  << " (" << getNormalDataSizeBytes() << " bytes)" << endl;
		}
		// If we have vertices and faces and normals
        else if ( ( hasVertices() ) && ( hasFaces() ) && ( hasNormals() ) && ( hasNormalIndices() ) )
		{
			cout << "Model has vertices, faces, normals & normal indices. Transferring data." << endl;

			// Allocate enough space for our vertexData and normalData arrays
			vertexData = new float[numFaces * 3 * 3];
			normalData = new float[numFaces * 3 * 3];

			// Create the vertexData and normalData arrays from the vector of faces
			// Note: We generate the face normals ourselves
			int vertexCount = 0;
			int normalCount = 0;
			for (vector<vec3>::iterator faceIter = faces->begin(); faceIter != faces->end(); faceIter++)
			{
				// Get the numbers of the three vertices that this face is comprised of
				GLuint firstVertexNum  = (GLuint)(*faceIter).x;
				GLuint secondVertexNum = (GLuint)(*faceIter).y;
				GLuint thirdVertexNum  = (GLuint)(*faceIter).z;

				// Now that we have the vertex numbers, we need to get the actual vertices
				// Note: We subtract 1 from the number of the vertex because faces start at
				//       face number 1 in the .oBJ format, while in our code the first vertex
				//       will be at location zero (i.e. vertices->begin() )
				vector<vec3>::iterator vertIter = vertices->begin();

				vec3 faceVert1 = vec3( (*(vertIter + firstVertexNum  - 1) ) );
				vec3 faceVert2 = vec3( (*(vertIter + secondVertexNum - 1) ) );
				vec3 faceVert3 = vec3( (*(vertIter + thirdVertexNum  - 1) ) );

				// Put the vertex data into our vertexData array
				vertexData[vertexCount++] = faceVert1.x;
				vertexData[vertexCount++] = faceVert1.y;
				vertexData[vertexCount++] = faceVert1.z;
				vertexData[vertexCount++] = faceVert2.x;
				vertexData[vertexCount++] = faceVert2.y;
				vertexData[vertexCount++] = faceVert2.z;
				vertexData[vertexCount++] = faceVert3.x;
				vertexData[vertexCount++] = faceVert3.y;
				vertexData[vertexCount++] = faceVert3.z;
			}

            // We have to take a separate pass through the normalIndices to construct the normalData array
			for (vector<vec3>::iterator normalIndexIter = normalIndices->begin(); normalIndexIter != normalIndices->end(); normalIndexIter++)
			{
			    // Get the numbers of the three normals that this face uses
				GLuint firstNormalNum  = (GLuint)(*normalIndexIter).x;
				GLuint secondNormalNum = (GLuint)(*normalIndexIter).y;
				GLuint thirdNormalNum  = (GLuint)(*normalIndexIter).z;

                // Now that we have the normal index numbers, we need to get the actual normals
				// Note: We subtract 1 from the number of the normal because normals start at
				//       number 1 in the .obJ format, while in our code the first vertex
				//       will be at location zero (i.e. normals->begin() )
				vector<vec3>::iterator normalIter = normals->begin();

				vec3 normal1 = vec3( (*(normalIter + firstNormalNum  - 1) ) );
				vec3 normal2 = vec3( (*(normalIter + secondNormalNum - 1) ) );
				vec3 normal3 = vec3( (*(normalIter + thirdNormalNum  - 1) ) );

				// Put the normal data into our normalData array
				normalData[normalCount++] = normal1.x;
				normalData[normalCount++] = normal1.y;
				normalData[normalCount++] = normal1.z;

				normalData[normalCount++] = normal2.x;
				normalData[normalCount++] = normal2.y;
				normalData[normalCount++] = normal2.z;

				normalData[normalCount++] = normal3.x;
				normalData[normalCount++] = normal3.y;
				normalData[normalCount++] = normal3.z;

			} // End of loop iterating over the model faces

			numVertices = vertexCount / 3;
			numNormals  = normalCount / 3;

			cout << "Number of vertices in data array: " << numVertices << " (" << getVertexDataSizeBytes() << " bytes)" << endl;
			cout << "Number of normals  in data array: " << numNormals  << " (" << getNormalDataSizeBytes() << " bytes)" << endl;
		}

		// If we ONLY have vertex data, then transfer just that...
		else if ( ( hasVertices() ) && ( !hasFaces() ) && ( !hasNormals() ) )
		{
			cout << "The model has no faces or normals. Transferring vertex data." << endl;

			vertexData = new float[numVertices * 3]; //float[numFaces * 3 ( 3];

			// Transfer all vertices from the vertices vector to the vertexData array
			int vertexCount = 0;
			for (vector<vec3>::iterator vertexIter = vertices->begin(); vertexIter != vertices->end(); vertexIter++)
			{
				vertexData[vertexCount++] = (*vertexIter).x;
				vertexData[vertexCount++] = (*vertexIter).y;
				vertexData[vertexCount++] = (*vertexIter).z;
			}

			// Divde by 3 (remember there's 3 values per vertex, x/y/z) to get our true vertex count
			numVertices = vertexCount / 3;

			// Print a summary of the vertex data
			cout << "Number of vertices in data array: " << numVertices << " (" << getVertexDataSizeBytes() << " bytes)" << endl;
		}
		else
		{
            cout << "Can't load this model." << endl;
            cout << "Got vertices?: " << hasVertices() << endl;
            cout << "Has normals? : " << hasNormals()  << endl;
            cout << "etc..." << endl;
        }
    }
    else // If we're not DRAWING_AS_ARRAYS then we must be DRAWING_AS_ELEMENTS (no duplicates in data arrays)...
	{
        cout << "Setting up model data to draw as: Elements." << endl;

		// Transfer the vertex data from the vector of vec3s to our vertexData array of floats
		if ( hasVertices() && hasFaces() )
		{
            vertexData = new float[numFaces * 3 * 3];
            normalData = new float[numFaces * 3 * 3];

            // Create the vertexData and normalData arrays from the vector of faces
			// Note: We generate the face normals ourselves
			int vertexCount = 0;
			int normalCount = 0;
			for (vector<vec3>::iterator faceIter = faces->begin(); faceIter != faces->end(); faceIter++)
			{
				// Get the numbers of the three vertices that this face is comprised of
				GLuint firstVertexNum  = (GLuint)(*faceIter).x;
				GLuint secondVertexNum = (GLuint)(*faceIter).y;
				GLuint thirdVertexNum  = (GLuint)(*faceIter).z;

				// Now that we have the vertex numbers, we need to get the actual vertices
				// Note: We subtract 1 from the number of the vertex because faces start at
				//       face number 1 in the .oBJ format, while in our code the first vertex
				//       will be at location zero (i.e. vertices.begin() )
				vector<vec3>::iterator vertIter = vertices->begin();

				vec3 faceVert1 = vec3( (*(vertIter + firstVertexNum  - 1) ) );
				vec3 faceVert2 = vec3( (*(vertIter + secondVertexNum - 1) ) );
				vec3 faceVert3 = vec3( (*(vertIter + thirdVertexNum  - 1) ) );

				// Now that we have the 3 vertices, we need to calculate the normal of the face
				// formed by these vertices

				// Convert this vertex data into a pure form
				vec3 v1 = vec3( faceVert2 - faceVert1 );
				vec3 v2 = vec3( faceVert3 - faceVert1 );

				// Generate the normal as the cross product and normalize it
				vec3 normal = vec3( glm::cross(v1, v2) );
				vec3 normalizedNormal = glm::normalize(normal);

				// Put the vertex data into our vertexData array
				vertexData[vertexCount++] = faceVert1.x;
				vertexData[vertexCount++] = faceVert1.y;
				vertexData[vertexCount++] = faceVert1.z;
				vertexData[vertexCount++] = faceVert2.x;
				vertexData[vertexCount++] = faceVert2.y;
				vertexData[vertexCount++] = faceVert2.z;
				vertexData[vertexCount++] = faceVert3.x;
				vertexData[vertexCount++] = faceVert3.y;
				vertexData[vertexCount++] = faceVert3.z;

				// Put the normal data into our normalData array
				// Note: we put the same normal into the normalData array for each of the
				// 3 vertices comprising the face! That's as much as we can do without per-vertex normals!
				normalData[normalCount++] = normalizedNormal.x;
				normalData[normalCount++] = normalizedNormal.y;
				normalData[normalCount++] = normalizedNormal.z;
				normalData[normalCount++] = normalizedNormal.x;
				normalData[normalCount++] = normalizedNormal.y;
				normalData[normalCount++] = normalizedNormal.z;
				normalData[normalCount++] = normalizedNormal.x;
				normalData[normalCount++] = normalizedNormal.y;
				normalData[normalCount++] = normalizedNormal.z;

			} // End of loop iterating over the model faces

			numVertices = vertexCount / 3;
			numNormals  = normalCount / 3;

			cout << "Number of vertices in data array: " << numVertices << " (" << getVertexDataSizeBytes() << " bytes)" << endl;
			cout << "Number of normals  in data array: " << numNormals  << " (" << getNormalDataSizeBytes() << " bytes)" << endl;
		}
		else
		{
			cout << "User elected to draw as elements, but no vertex data found. Exiting." << endl;
			exit(-1);
		}

		// Transfer the vertex data from the vector of vec3s to our vertexData array of floats
		if ( hasFaces() )
		{
			// Allocate storage for our face data (3 vertex numbers per face)
			faceData = new GLuint[numFaces * 3];

			int faceCount = 0;
			for (vector<vec3>::iterator iter = faces->begin(); iter != faces->end(); iter++)
			{
				// IMPORTANT: Face numbering in a .OBJ file starts at 1 and not 0! So to map this
				//            to our array (which starts at element 0) we need to subtract one
				//            from every face! i.e. I have a triangle, in the obj file it says
				//            we're using vertices 1, 2 and 3 for the triangle --- but in my
				//            faceData array this maps to elements 0, 1 and 2!
				faceData[faceCount++] = ((*iter).x) - 1; // Put the x component in the vertexData array and move to next value
				faceData[faceCount++] = ((*iter).y) - 1; // Put the y component in the vertexData array and move to next value
				faceData[faceCount++] = ((*iter).z) - 1; // Put the z component in the vertexData array and move to next value
			}

			numFaces = faceCount / 3;
		}
		else
		{
			cout << "User elected to draw as elements, but no face data found. Exiting." << endl;
			exit(-1);
		}

	} // End of drawing as elements section

} // End of setupData method

// A method to print out the vector of vertices
void Model::printVertices()
{
	for (vector<vec3>::iterator i = vertices->begin(); i != vertices->end(); i++)
	{
		cout << "Vertex x: " << (*i).x << "\t" << "y: " << (*i).y << "\t" "z: " << (*i).z << endl;
	}
}

// A method to print out the vector of faces
void Model::printFaces()
{
	for (vector<vec3>::iterator i = faces->begin(); i != faces->end(); i++)
	{
		cout << "Face - v1: " << (*i).x << "\t" << "v2: " << (*i).y << "\t" "v3: " << (*i).z << endl;
	}
}

// A method to print out the vector of normals
void Model::printNormals()
{
	for (vector<vec3>::iterator i = normals->begin(); i != normals->end(); i++)
	{
		cout << "Normal x: " << (*i).x << "\t" << "Normal y: " << (*i).y << "\t" "Normal z: " << (*i).z << endl;
	}
}

// A method to print out the normalData array
void Model::printNormalData()
{
	for (GLuint loop = 0; loop < numNormals * 3; loop += 3 )
	{
		cout << "Normal " << (loop / 3) << " is " << normalData[loop] << "\t" << normalData[loop+1] << "\t" << normalData[loop+2] << endl;
	}
}

// A method to print out the normalData array
void Model::printVertexData()
{
	for (GLuint loop = 0; loop < numVertices * 3; loop += 3 )
	{
		cout << "Vertex " << (loop / 3) << " is " << vertexData[loop] << "\t" << vertexData[loop+1] << "\t" << vertexData[loop+2] << endl;
	}
}

// Getters
GLvoid* Model::getVertexData()                 { return vertexData;                            }
GLvoid* Model::getNormalData()                 { return normalData;                            }
GLvoid* Model::getNormalIndexData()            { return normalIndexData;                       }
GLvoid* Model::getFaceData()                   { return faceData;                              }
GLuint  Model::getVertexDataSizeBytes()        { return numVertices * 3 * sizeof(GLfloat);     }
GLuint  Model::getNormalDataSizeBytes()        { return numNormals * 3 * sizeof(GLfloat);      }
GLuint  Model::getNormalIndexDataSizeBytes()   { return numNormalIndices * 3 * sizeof(GLuint); }
GLuint  Model::getFaceDataSizeBytes()          { return numFaces * 3 * sizeof(GLuint);         }
GLuint  Model::getNumVertices()                { return numVertices;                           }
GLuint  Model::getNumNormals()                 { return numNormals;                            }
GLuint  Model::getNumNormalIndices()           { return numNormalIndices;                      }
GLuint  Model::getNumFaces()                   { return numFaces;                              }
GLuint  Model::getFaceElementCount()           { return numFaces * 3;                          }
Model::DrawingMethod Model::getDrawingMethod() { return drawingMethod;                         }

// Method to scale the size of a model uniformly
void Model::scale(float scale)
{
	for (GLuint loop = 0; loop < numVertices * 3; /* We change the loop inside itself! */)
	{
		vertexData[loop++] *= scale;
		vertexData[loop++] *= scale;
		vertexData[loop++] *= scale;
	}
}

// Method to scale the size of a model on separate axes
void Model::scale(float xScale, float yScale, float zScale)
{
	for (GLuint loop = 0; loop < numVertices * 3; /* We change the loop inside itself! */)
	{
		vertexData[loop++] *= xScale;
		vertexData[loop++] *= yScale;
		vertexData[loop++] *= zScale;
	}
}
