// Obj2Pbrt.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Obj2Pbrt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include <iostream>
#include <fstream>
#include <iomanip>
#include <functional>
#include "ObjLoader.h"
#include "TString.h"
#include "NormalCalc.h"
#include <unordered_map>
#include <vector>

// 唯一的应用程序对象

CWinApp theApp;

using namespace std;
using namespace Utils;

struct MapEntry {
	int id;
	int texCoord;
	int normal;
	int tangent;
};

void doConvert(const TCHAR* objFileName, const TCHAR* pbrtFileName, bool mirrorX = false,
			   bool cut = false, float a = 0, float b = 0, float c = 0, float d = -1)
{
	ObjLoader loader;
	loader.LoadObj(ANSIStringFromTString(objFileName));

	ObjModel* pModel = loader.ReturnObj();
	// Cut the model first
	if (cut)
		removeModelPart(pModel, a, b, c, d);

	// Calculate smoothed normals
	computeNormals(pModel);
	computeTangentSpace(pModel);
	duplicateVerticesForDifferentTexCoords(pModel);

	// for each triangle iterator
	auto forEachTriangle = [&] (function<void (const ObjTriangle&)> callback) {
		for (const ObjPart& part : pModel->Parts) {
			for (int idxTri = part.TriIdxMin; idxTri < part.TriIdxMax; idxTri++) {
				const ObjTriangle& tri = pModel->Triangles[idxTri];
				callback(tri);
			}
		}
	};

	// Build vector -> { texCoord, normal, tangent } mapping
	unordered_map<int, MapEntry> mapVectorToEntry;
	forEachTriangle([&] (const ObjTriangle& tri) {
		for (int j = 0; j < 3; j++) {
			auto iter = mapVectorToEntry.find(tri.Vertex[j]);
			if (iter != mapVectorToEntry.end()) continue;

			MapEntry& entry = mapVectorToEntry[tri.Vertex[j]];
			entry.id = -1;
			entry.normal = tri.Normal[j];
			entry.tangent = tri.Tangent[j];
			entry.texCoord  = tri.TexCoord[j];
		}
	});

	ofstream pbrtOut;
	if (pbrtFileName)
	{
		pbrtOut.open(pbrtFileName, ios::trunc);
		if (!pbrtOut) {
			cout << "Failed to open output file!" << endl;
			return;
		}
	}
	ostream& out = pbrtFileName ? pbrtOut : cout;

	out << "AttributeBegin" << endl;
	out << "  Shape \"trianglemesh\"" << endl;
    out << "    \"point P\" [" << endl;
	// record order of vertices
	vector<int> vertexOrder;
	forEachTriangle([&] (const ObjTriangle& tri) {
		for (int j = 0; j < 3; j++) {
			int vertexId = tri.Vertex[j];
			auto iter = mapVectorToEntry.find(vertexId);
			if (iter != mapVectorToEntry.end()) {
				MapEntry& entry = iter->second;
				if (entry.id < 0) {
					entry.id = vertexOrder.size();
					vertexOrder.push_back(vertexId);
					const ObjVertex& v = pModel->Vertices[vertexId];
					out << "      " << (mirrorX ? -v.x : v.x) << " " << v.y << " " << v.z << endl;
				}
			}
		}
	});
	out << "    ]" << endl;
	if (pModel->Normals.size() > 1) {
		out << "    \"normal N\" [" << endl;
		for (int vertexId : vertexOrder) {
			const MapEntry& entry = mapVectorToEntry[vertexId];
			const ObjNormal& n = pModel->Normals[entry.normal];
			out << "      " << (mirrorX ? -n.x : n.x) << " " << n.y << " " << n.z << endl;
		}
		out << "    ]" << endl;
	}
	if (pModel->Tangents.size() > 1) {
		out << "    \"vector S\" [" << endl;
		for (int vertexId : vertexOrder) {
			const MapEntry& entry = mapVectorToEntry[vertexId];
			const ObjTangent& t = pModel->Tangents[entry.tangent];
			out << "      " << (mirrorX ? -t.x : t.x) << " " << t.y << " " << t.z << endl;
		}
		out << "    ]" << endl;
	}
	if (pModel->TexCoords.size() > 1) {
		out << "    \"float uv\" [" << endl;
		for (int vertexId : vertexOrder) {
			const MapEntry& entry = mapVectorToEntry[vertexId];
			const ObjTexCoord& tc = pModel->TexCoords[entry.texCoord];
			out << "      " << tc.U << " " << tc.V << endl;
		}
		out << "    ]" << endl;
	}
	out << "    \"integer indices\" [" << endl;
	forEachTriangle([&] (const ObjTriangle& tri) {
		out << "      " << mapVectorToEntry[tri.Vertex[0]].id
			     << " " << mapVectorToEntry[tri.Vertex[1]].id
				 << " " << mapVectorToEntry[tri.Vertex[2]].id << endl;
	});
	out << "    ]" << endl;
	out << "AttributeEnd" << endl;

	if (pbrtFileName)
		pbrtOut.close();
	delete pModel;
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// 初始化 MFC 并在失败时显示错误
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: 更改错误代码以符合您的需要
			_tprintf(_T("错误: MFC 初始化失败\n"));
			nRetCode = 1;
		}
		else
		{
			// TODO: 在此处为应用程序的行为编写代码。
			const TCHAR* objFileName = NULL;
			const TCHAR* pbrtFileName = NULL;
			bool needPbrtFileName = false;
			bool mirrorX = false;
			bool cut = false;
			float a, b, c, d;
			for (int i = 1; i < argc; i++)
			{
				const TCHAR* arg = argv[i];
				if (!_tcsicmp(arg, _T("-o")))
				{
					needPbrtFileName = true;
				}
				else if (!_tcsicmp(arg, _T("-mx")))
				{
					mirrorX = true;
				}
				else if (!_tcsicmp(arg, _T("-cut")) && i + 4 < argc)
				{
					cut = true;
					a = (float)_tcstod(argv[++i], NULL);
					b = (float)_tcstod(argv[++i], NULL);
					c = (float)_tcstod(argv[++i], NULL);
					d = (float)_tcstod(argv[++i], NULL);
				}
				else
				{
					if (needPbrtFileName)
					{
						pbrtFileName = arg;
						needPbrtFileName = false;
					}
					else
					{
						objFileName = arg;
					}
				}
			}
			if (!objFileName)
			{
				cout << "Usage: " << ANSIStringFromTString(argv[0]) << " objfilename [-o pbrtfilename]" << endl;
				cout << "  When omitted, writes to standard output." << endl;
				return 0;
			}

			doConvert(objFileName, pbrtFileName, mirrorX, cut, a, b, c, d);

			return 0;
		}
	}
	else
	{
		// TODO: 更改错误代码以符合您的需要
		_tprintf(_T("错误: GetModuleHandle 失败\n"));
		nRetCode = 1;
	}

	return nRetCode;
}
