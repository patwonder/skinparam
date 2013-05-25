/**
 * Interface for renderable objects
 * This file is for default implementaions
 */

#include "stdafx.h"

#include "Renderable.h"

using namespace Skin;
using namespace Utils;

const XMFLOAT4 IRenderer::COPY_DEFAULT_SCALE_FACTOR = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
const XMFLOAT4 IRenderer::COPY_DEFAULT_VALUE = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
const XMFLOAT4 IRenderer::COPY_DEFAULT_LERPS = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
