#pragma once
#include <chrono>
#undef min

// from https://gist.github.com/bolry/b3458d8c58c7f2db8fbef337fdb36ea1
class MyTimer
{
	using clock = std::chrono::high_resolution_clock;
public:
	MyTimer() {};
	~MyTimer() {};
public:
	void start() { begin_time_ = clock::now(); stop_time_ = clock::time_point::min();	}
	void stop() { if (is_running()) stop_time_ = clock::now(); }
	clock::duration elapsed() const { return end_time() - begin_time_; }

private:
	bool is_running() const { return stop_time_ == clock::time_point::min(); }
	clock::time_point end_time() const { return is_running() ? clock::now() : stop_time_; }
	clock::time_point begin_time_{ clock::now() };
	clock::time_point stop_time_{ clock::time_point::min() };

};

