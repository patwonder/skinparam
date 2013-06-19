#pragma once

#pragma warning( disable : 4005 ) // disable duplicate macro definition warnings for vs2012

#include "Constants.h"
#include "version.h"

// MFC headers
#define _AFX_NO_MFC_CONTROLS_IN_DIALOGS
#include <afxwin.h>

// OpenRL headers
#include <OpenRL\OpenRL.h>
#include <OpenRL\rl.h>

#include "TString.h"
#include "UMath.h"
