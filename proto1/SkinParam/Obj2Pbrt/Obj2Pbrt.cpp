// Obj2Pbrt.cpp : �������̨Ӧ�ó������ڵ㡣
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

// Ψһ��Ӧ�ó������

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
		// ��ʼ�� MFC ����ʧ��ʱ��ʾ����
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: ���Ĵ�������Է���������Ҫ
			_tprintf(_T("����: MFC ��ʼ��ʧ��\n"));
			nRetCode = 1;
		}
		else
		{
			// TODO: �ڴ˴�ΪӦ�ó������Ϊ��д���롣
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
		// TODO: ���Ĵ�������Է���������Ҫ
		_tprintf(_T("����: GetModuleHandle ʧ��\n"));
		nRetCode = 1;
	}

	return nRetCode;
}
