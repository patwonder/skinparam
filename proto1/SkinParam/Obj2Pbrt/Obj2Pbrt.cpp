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

// 唯一的应用程序对象

CWinApp theApp;

using namespace std;
using namespace Utils;

void doConvert(const TCHAR* objFileName, const TCHAR* pbrtFileName)
{
	ObjLoader loader;
	loader.LoadObj(ANSIStringFromTString(objFileName));

	ObjModel* pModel = loader.ReturnObj();
	// Calculate smoothed normals
	computeNormals(pModel);
	computeTangentSpace(pModel);

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

	auto forEachTriangle = [&] (function<void (const ObjTriangle&)> callback) {
		for (const ObjPart& part : pModel->Parts) {
			for (int idxTri = part.TriIdxMin; idxTri < part.TriIdxMax; idxTri++) {
				const ObjTriangle& tri = pModel->Triangles[idxTri];
				callback(tri);
			}
		}
	};
	out << "AttributeBegin" << endl;
	out << "  Shape \"trianglemesh\"" << endl;
    out << "    \"point P\" [" << endl;
	forEachTriangle([&] (const ObjTriangle& tri) {
		for (int j = 0; j < 3; j++) {
			const ObjVertex& v = pModel->Vertices[tri.Vertex[j]];
			out << "      " << v.x << " " << v.y << " " << v.z << endl;
		}
	});
	out << "    ]" << endl;
	if (pModel->Normals.size() > 1) {
		out << "    \"normal N\" [" << endl;
		forEachTriangle([&] (const ObjTriangle& tri) {
			for (int j = 0; j < 3; j++) {
				const ObjNormal& n = pModel->Normals[tri.Normal[j]];
				out << "      " << n.x << " " << n.y << " " << n.z << endl;
			}
		});
		out << "    ]" << endl;
	}
	if (pModel->Tangents.size() > 1) {
		out << "    \"vector S\" [" << endl;
		forEachTriangle([&] (const ObjTriangle& tri) {
			for (int j = 0; j < 3; j++) {
				const ObjTangent& t = pModel->Tangents[tri.Tangent[j]];
				out << "      " << t.x << " " << t.y << " " << t.z << endl;
			}
		});
		out << "    ]" << endl;
	}
	if (pModel->TexCoords.size() > 1) {
		out << "    \"float uv\" [" << endl;
		forEachTriangle([&] (const ObjTriangle& tri) {
			for (int j = 0; j < 3; j++) {
				const ObjTexCoord& tc = pModel->TexCoords[tri.TexCoord[j]];
				out << "      " << tc.U << " " << tc.V << endl;
			}
		});
		out << "    ]" << endl;
	}
	out << "    \"integer indices\" [" << endl;
	int index = 0;
	forEachTriangle([&] (const ObjTriangle& tri) {
		out << "      " << index << " " << index + 1 << " " << index + 2 << endl;
		index += 3;
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
			for (int i = 1; i < argc; i++)
			{
				const TCHAR* arg = argv[i];
				if (!_tcsicmp(arg, _T("-o")))
				{
					needPbrtFileName = true;
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

			doConvert(objFileName, pbrtFileName);

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
