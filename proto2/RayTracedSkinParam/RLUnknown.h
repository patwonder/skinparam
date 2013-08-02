/*
 * Handle for OpenRL entities
 */

#pragma once

namespace RLSkin {
	class RLUnknown {
	private:
		ULONG refCount;
		RLUnknown(const RLUnknown&);
		RLUnknown& operator=(const RLUnknown&);
	protected:
		RLUnknown() : refCount(1) {}
		virtual ~RLUnknown() = 0;
	public:
		ULONG AddRef() { return ++refCount; }
		ULONG Release();
	};
} // namespace RLSkin
