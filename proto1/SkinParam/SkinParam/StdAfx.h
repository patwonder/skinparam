#pragma once

#pragma warning( disable : 4005 ) // disable duplicate macro definition warnings for vs2012

#define NOMINMAX

#include "Constants.h"
#include "version.h"

// MFC headers
#define _AFX_NO_MFC_CONTROLS_IN_DIALOGS
#include <afxwin.h>
// Add support for common dialogs,
// such as the CFontDialog, CColorDialog and CFileDialog
#include <afxdlgs.h>

// Direct3D 11 headers
#include <D3D11.h>
#include <D3DX11.h>
#include <DxErr.h>
#include <D3DX10math.h>
#include <D3Dcompiler.h>
#include <xnamath.h>

#include <algorithm>
using std::min;
using std::max;

#include "TString.h"
#include "UMath.h"
