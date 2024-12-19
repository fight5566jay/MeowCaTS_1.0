#pragma once
#include <future>
#include <memory>

using std::future;
using std::shared_ptr;

class MyFutureHandler {
public:
	MyFutureHandler(const uint32_t uiFutureCount);

	MyFutureHandler() {};
	~MyFutureHandler() {};
private:
	shared_ptr<future<void>> vFuture;
};
