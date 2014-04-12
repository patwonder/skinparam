#pragma once

#include <future>


namespace Parallel {

template <class T, class AbortHandle, class ProgressHandle>
class AbortableFuture : public std::future<T> {
public:
	typedef std::future<T> Base;
	AbortableFuture(Base&& base, AbortHandle handle, ProgressHandle progHandle)
		: Base(std::move(base))
	{
		abortHandle = handle;
		progressHandle = progHandle;
	}

	void abort() {
		abortHandle();
	}
	void abortWait() {
		abort();
		wait();
	}
	double progress() {
		return progressHandle();
	}
	AbortableFuture(AbortableFuture&& other)
		: Base(std::move(other))
	{
		abortHandle = std::move(other.abortHandle);
		progressHandle = std::move(other.progressHandle);
	}
private:
	AbortHandle abortHandle;
	ProgressHandle progressHandle;
};

}
