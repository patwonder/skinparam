#include "stdafx.h"

#include "RLUnknown.h"

using namespace RLSkin;

ULONG RLUnknown::Release() {
	ULONG ret;
	if ((ret = --refCount) == 0)
		delete this;
	return ret;
}

RLUnknown::~RLUnknown() {
}
