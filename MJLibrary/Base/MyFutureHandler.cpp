#include "MyFutureHandler.h"

MyFutureHandler::MyFutureHandler(const uint32_t uiFutureCount)
{
	vFuture = shared_ptr<future<void>>(new future<void>[uiFutureCount], std::default_delete<future<void>>());
}
