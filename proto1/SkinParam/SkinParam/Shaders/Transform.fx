/**
 * General buffer for 3D transform
 */
#pragma once

cbuffer Transform : register(b0) {
	matrix g_matWorld;
	matrix g_matViewProj;
}
