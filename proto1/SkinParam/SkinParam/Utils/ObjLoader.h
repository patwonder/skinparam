/*-----------------------------------------------------------------//
// April 10, 2005                                                    //
//                                Copyright ? 2005  Justin Walsh     //
//                                                                   //
//  This code is Released Under the GNU GPL                          //
//       http://www.gnu.org/copyleft/gpl.html                        //
//                                                                   //
//  Contributor(s):                                                  //
//    Yifan Wu (patwonder@163.com)  (May 6th, 2013)                  //
//-----------------------------------------------------------------*/

#pragma once

#include <string>
#include <map>
#include <vector>
#include "FVector.h"

namespace Utils {

/*-----------------------------------------------------------------//
//Here we have our data types our loader class will need and use...//
//                                                                 */
	struct ObjVertex : public FVector {
		ObjVertex() { }
		ObjVertex(float x, float y, float z)
			: FVector(x, y, z) { }
		ObjVertex(const FVector& v) : FVector(v) { }
	};

	struct ObjNormal : public FVector {
		ObjNormal() { }
		ObjNormal(float x, float y, float z)
			: FVector(x, y, z) { }
		ObjNormal(const FVector& v) : FVector(v) { }
	};

	struct ObjTangent : public FVector {
		ObjTangent() { }
		ObjTangent(float x, float y, float z)
			: FVector(x, y, z) { }
		ObjTangent(const FVector& v) : FVector(v) { }
	};

	struct ObjBinormal : public FVector {
		ObjBinormal() { }
		ObjBinormal(float x, float y, float z)
			: FVector(x, y, z) { }
		ObjBinormal(const FVector& v) : FVector(v) { }
	};

	struct ObjTexCoord {
		 float U, V;
	};

	struct ObjTriangle{
		int Vertex[3];
		int Normal[3];
		int Binormal[3];
		int Tangent[3];
		int TexCoord[3];
	};

	struct ObjMaterial {
		float Emission[3];
		float Ambient[3];
		float Diffuse[3];
		float Specular[3];
		float Shininess;
		std::string TextureFileName;
		float BumpMultiplier;
		std::string BumpMapFileName;
	};

	struct ObjPart {
		std::string MaterialName;
		int TriIdxMin;
		int TriIdxMax;
	};

	class ObjModel {
		private:
			ObjModel(const ObjModel& copy);
			ObjModel& operator=(const ObjModel& right);
		public:
			ObjModel();
			~ObjModel();

			std::vector<ObjVertex> Vertices;
			std::vector<ObjNormal> Normals;
			std::vector<ObjTangent> Tangents;
			std::vector<ObjBinormal> Binormals;
			std::vector<ObjTexCoord> TexCoords;
			std::vector<ObjTriangle> Triangles;
			std::map<std::string, ObjMaterial> Materials;
			std::vector<ObjPart> Parts;
	};
/*                                                                 //
//-----------------------------------------------------------------*/



/*-----------------------------------------------------------------//
//     The meat of the sandwitch, the class to load .obj files     //
//                                                                 */
	class ObjLoader  {
		public:
			ObjLoader();
			~ObjLoader();

			ObjLoader(std::string file);
			void LoadObj(std::string file);
			void FreeObj(void);
			ObjModel* ReturnObj(void);

		protected:
			std::string *fileName;
			ObjModel *theObj;
			
			void ReadData(void);
			void ReadMtl(std::string filename);
	};
/*                                                                 //
//-----------------------------------------------------------------*/
}
