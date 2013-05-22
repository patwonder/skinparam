#pragma once

#pragma warning( disable : 4005 ) // disable duplicate macro definition warnings for vs2012

#include "Constants.h"
#include "version.h"

// MFC headers
#define _AFX_NO_MFC_CONTROLS_IN_DIALOGS
#include <afxwin.h>

// Direct3D 11 headers
#include <D3D11.h>
#include <D3DX11.h>
#include <DxErr.h>
#include <D3DX10math.h>
#include <D3Dcompiler.h>
#include <xnamath.h>

#include "TString.h"
#include "UMath.h"
