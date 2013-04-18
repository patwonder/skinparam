/*-----------------------------------------------------------------//
// April 10, 2005												   //
//                                Copyright ? 2005  Justin Walsh //
//																   //
//  This code is Released Under the GNU GPL                        //
//       http://www.gnu.org/copyleft/gpl.html                      //
//-----------------------------------------------------------------*/

#include <fstream>
#include <sstream>
#include "ObjLoader.h"

using namespace Utils;
using namespace std;

namespace Utils {
	inline float atoff(const char* a) {
		return (float)atof(a);
	}
} // namespace Utils

/*-----------------------------------------------------------------//
//		  				ObjModel Class                             //
//                                                                 */
	ObjModel::ObjModel()  {
	}

	ObjModel::~ObjModel()  {
	}

/*                                                                 //
//-----------------------------------------------------------------*/



/*-----------------------------------------------------------------//
//		  			     ObjLoader Class                           //
//                                                                 */

	ObjLoader::ObjLoader()  {
		fileName = NULL;
		theObj = NULL;
	}

	ObjLoader::~ObjLoader()  {
		FreeObj();
	}

	void ObjLoader::FreeObj(void)  {
		if(fileName != NULL) delete fileName;
		if(theObj != NULL)   delete theObj;
		fileName = NULL;
		theObj = NULL;
	}

	ObjModel* ObjLoader::ReturnObj(void)  {
		ObjModel* ret = theObj;
		theObj = NULL;
		return ret;
	}

	ObjLoader::ObjLoader(string file)  {
		fileName = new string(file);
		theObj = new ObjModel();
		ReadData();
	}

	void ObjLoader::LoadObj(string file)  {
		FreeObj();
		fileName = new string(file);
		theObj = new ObjModel();
		ReadData();
	}

	void ObjLoader::ReadData(void)  {
		ifstream input(fileName->c_str());
		string buffer;

		//make sure we got the file opened up ok...
		if( !input.is_open() )
			return;

		//hear are the counters...
		theObj->Vertices.push_back(ObjVertex());
		theObj->Normals.push_back(ObjNormal());
		theObj->TexCoords.push_back(ObjTexCoord());

		ObjPart currentPart = { "", 0, 0 };
		//get the hard data, and load up our arrays...
		//read one line at a time of the file...
		while( !input.eof() )  {
			getline(input, buffer);
			istringstream line(buffer);
			string cmd;
			string f1, f2, f3;

			// skip empty lines
			if (buffer.size() == 0) continue;

			// skip comments
			if (buffer[0] == '#') continue;

			// parse command
			line >> cmd;
			
			if(cmd == "vn")  {
				line >> f1 >> f2 >> f3;
				theObj->Normals.push_back(ObjNormal(atoff(f1.c_str()), atoff(f2.c_str()), atoff(f3.c_str())).normalize());
                //sscanf(buffer.c_str(), "vn %f %f %f", theObj->NormalArray[nC].X, 
				//					   theObj->NormalArray[nC].Y, theObj->NormalArray[nC].Z);
			}				
			else if(cmd == "vt")  {
				line >> f1 >> f2;
				ObjTexCoord texCoord = { atoff(f1.c_str()), atoff(f2.c_str()) };
				theObj->TexCoords.push_back(texCoord);
				//sscanf(buffer.c_str(), "vt %f %f", theObj->TexCoordArray[tC].U, 
				//					   theObj->TexCoordArray[tC].V);
			}				
			else if(cmd == "v")  {
				line >> f1 >> f2 >> f3;
				theObj->Vertices.push_back(ObjVertex(atoff(f1.c_str()), atoff(f2.c_str()), atoff(f3.c_str())));
				//sscanf(buffer.c_str(), "v %f %f %f", theObj->VertexArray[vC].X, 
				//					   theObj->VertexArray[vC].Y, theObj->VertexArray[vC].Z);
			}
			else if(cmd == "f")  {
				line >> f1 >> f2 >> f3;

				ObjTriangle triangle;
				
				int sPos = 0;
				int ePos = sPos;
				string temp;
				ePos = f1.find_first_of("/");
				//we have a line with the format of "f %d/%d/%d %d/%d/%d %d/%d/%d"
				if(ePos != string::npos)  {
					temp = f1.substr(sPos, ePos - sPos);
					triangle.Vertex[0] = atoi(temp.c_str());

					sPos = ePos+1;
					ePos = f1.find("/", sPos);
					temp = f1.substr(sPos, ePos - sPos);
					triangle.TexCoord[0] = atoi(temp.c_str());

					sPos = ePos+1;
					ePos = f1.length();
					temp = f1.substr(sPos, ePos - sPos);
					triangle.Normal[0] = atoi(temp.c_str());
				}

				sPos = 0;
				ePos = f2.find_first_of("/");
				//we have a line with the format of "f %d/%d/%d %d/%d/%d %d/%d/%d"
				if(ePos != string::npos)  {
					temp = f2.substr(sPos, ePos - sPos);
					triangle.Vertex[1] = atoi(temp.c_str());

					sPos = ePos + 1;
					ePos = f2.find("/", sPos);
					temp = f2.substr(sPos, ePos - sPos);
					triangle.TexCoord[1] = atoi(temp.c_str());

					sPos = ePos + 1;
					ePos = f2.length();
					temp = f2.substr(sPos, ePos - sPos);
					triangle.Normal[1] = atoi(temp.c_str());
				}

				sPos = 0;
				ePos = f3.find_first_of("/");
				//we have a line with the format of "f %d/%d/%d %d/%d/%d %d/%d/%d"
				if(ePos != string::npos)  {
					temp = f3.substr(sPos, ePos - sPos);
					triangle.Vertex[2] = atoi(temp.c_str());

					sPos = ePos + 1;
					ePos = f3.find("/", sPos);
					temp = f3.substr(sPos, ePos - sPos);
					triangle.TexCoord[2] = atoi(temp.c_str());

					sPos = ePos + 1;
					ePos = f3.length();
					temp = f3.substr(sPos, ePos - sPos);
					triangle.Normal[2] = atoi(temp.c_str());
				}
				theObj->Triangles.push_back(triangle);
				//sscanf(buffer.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d",
                //          theObj->TriangleArray[fC].Vertex[0], theObj->TriangleArray[fC].TexCoord[0], theObj->TriangleArray[fC].Normal[0],
				//     	  theObj->TriangleArray[fC].Vertex[1], theObj->TriangleArray[fC].TexCoord[1], theObj->TriangleArray[fC].Normal[1],
				//		  theObj->TriangleArray[fC].Vertex[2], theObj->TriangleArray[fC].TexCoord[2], theObj->TriangleArray[fC].Normal[2]);
			} else if (cmd == "usemtl") {
				line >> f1;
				if (theObj->Triangles.size() > (size_t)currentPart.VertexIdxMin) {
					// only save new part if previous one actually consists of any triangle
					currentPart.VertexIdxMax = theObj->Triangles.size();
					theObj->Parts.push_back(currentPart);
				}
				currentPart.MaterialName = f1;
				currentPart.VertexIdxMin = theObj->Triangles.size();
			} else if (cmd == "mtllib") {
				line >> f1;
				ReadMtl("model\\" + f1);
			}
		}
		// don't forget the last part of the object
		if (theObj->Triangles.size() > (size_t)currentPart.VertexIdxMin) {
			// only save new part if previous one actually consists of any triangle
			currentPart.VertexIdxMax = theObj->Triangles.size();
			theObj->Parts.push_back(currentPart);
		}
		//all should be good
		input.close();
	}

	void ObjLoader::ReadMtl(string filename) {
		ifstream input(filename.c_str());
		string buffer;

		//make sure we got the file opened up ok...
		if( !input.is_open() )
			return;

		ObjMaterial* current = NULL;

		//get the hard data, and load up our arrays...
		//read one line at a time of the file...
		while( !input.eof() )  {
			getline(input, buffer);
			istringstream line(buffer);
			string cmd;
			string f1, f2, f3;

			// skip empty lines
			if (buffer.size() == 0) continue;

			// skip comments
			if (buffer[0] == '#') continue;

			// parse command
			line >> cmd;

			if (cmd == "newmtl") {
				line >> f1;
				theObj->Materials[f1] = ObjMaterial();
				current = &theObj->Materials[f1];
				memset(current, 0, sizeof(ObjMaterial));
			} else {
				if (current) {
					if (cmd == "Ke") {
						line >> f1 >> f2 >> f3;
						current->Emission[0] = atoff(f1.c_str());
						current->Emission[1] = atoff(f2.c_str());
						current->Emission[2] = atoff(f3.c_str());
					} else if (cmd == "Ka") {
						line >> f1 >> f2 >> f3;
						current->Ambient[0] = atoff(f1.c_str());
						current->Ambient[1] = atoff(f2.c_str());
						current->Ambient[2] = atoff(f3.c_str());
					} else if (cmd == "Kd") {
						line >> f1 >> f2 >> f3;
						current->Diffuse[0] = atoff(f1.c_str());
						current->Diffuse[1] = atoff(f2.c_str());
						current->Diffuse[2] = atoff(f3.c_str());
					} else if (cmd == "Ks") {
						line >> f1 >> f2 >> f3;
						current->Specular[0] = atoff(f1.c_str());
						current->Specular[1] = atoff(f2.c_str());
						current->Specular[2] = atoff(f3.c_str());
					} else if (cmd == "Ns") {
						line >> f1;
						current->Shininess = atoff(f1.c_str());
					}
				}
			}
		}
		input.close();
	}

/*                                                                 //
//-----------------------------------------------------------------*/
