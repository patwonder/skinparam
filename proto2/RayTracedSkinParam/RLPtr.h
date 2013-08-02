/*
 * Smart pointer for RLUnknown s
 */

#pragma once

namespace RLSkin {
	template <class T>
	class RLPtr {
	private:
		template <class Q>
		class _NoAddRefReleaseOnCComPtr : 
			public Q
		{
			private:
				virtual ULONG AddRef() = 0;
				virtual ULONG Release() = 0;
		};
	public:
		T* p;

		RLPtr() {
			p = nullptr;
		}
		RLPtr(T* lp) {
			p = lp;
			if (p != nullptr)
				p->AddRef();
		}
		RLPtr(const RLPtr<T>& lp) {
			p = lp.p;
			if (p != nullptr)
				p->AddRef();
		}
		void Swap(RLPtr<T>& other)
		{
			T* pTemp = p;
			p = other.p;
			other.p = pTemp;
		}
		~RLPtr() {
			if (p)
				p->Release();
		}
		operator T*() const {
			return p;
		}
		T& operator*() const {
			return *p;
		}
		//The assert on operator& usually indicates a bug.  If this is really
		//what is needed, however, take the address of the p member explicitly.
		T** operator&() {
			ATLASSERT(p == nullptr);
			return &p;
		}
		_NoAddRefReleaseOnCComPtr<T>* operator->() const {
			ATLASSERT(p != nullptr);
			return (_NoAddRefReleaseOnCComPtr<T>*)p;
		}
		bool operator!() const {
			return (p == nullptr);
		}
		bool operator<(T* pT) const {
			return p < pT;
		}
		bool operator!=(T* pT) const {
			return !operator==(pT);
		}
		bool operator==(T* pT) const {
			return p == pT;
		}

		// Release the interface and set to nullptr
		void Release() {
			T* pTemp = p;
			if (pTemp)
			{
				p = nullptr;
				pTemp->Release();
			}
		}
		// Attach to an existing interface (does not AddRef)
		void Attach(T* p2) {
			if (p) {
				ULONG ref = p->Release();
				(ref);
				// Attaching to the same object only works if duplicate references are being coalesced.  Otherwise
				// re-attaching will cause the pointer to be released and may cause a crash on a subsequent dereference.
				ATLASSERT(ref != 0 || p2 != p);
			}
			p = p2;
		}
		// Detach the interface (does not Release)
		T* Detach() {
			T* pt = p;
			p = nullptr;
			return pt;
		}
		T* operator=(const RLPtr<T>& lp) {
			if (*this != lp) {
				RLPtr(lp).Swap(*this);
			}
			return *this;
		}	
		RLPtr(RLPtr<T>&& lp) {
			p = nullptr;
			lp.Swap(*this);
		}	
		T* operator=(RLPtr<T>&& lp) {			
			if (*this != lp) {
				RLPtr(static_cast<RLPtr&&>(lp)).Swap(*this);
			}
			return *this;		
		}
	};
} // namespace RLSkin
