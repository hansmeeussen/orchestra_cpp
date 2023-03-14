#pragma once

#include <algorithm>
#include <string>
#include <vector>
#include <atomic>

namespace orchestracpp
{

	class StopFlag
	{

		// in Java its appears significantly faster to use a volatile variable for 
		// checking the flag than a synchronized method
		// we only set the flag via synchronized methods, we read it unsynchronized

		// The C++ version needs to be checked/prepared for multithreaded use!

	public:
    //JAVA  'volatile' has a different meaning in C++:
    //ORIGINAL LINE: public volatile boolean cancelled;
		std::atomic<bool> cancelled = false;


	private:
		std::vector<StopFlag*> children;

	public:
		StopFlag();

		virtual bool isCancelled();

		virtual void addChild(StopFlag *child);

		virtual void removeChild(StopFlag *child);

		virtual void reset();

		/**
		 * stops all children, but not the parent!
		 */
		virtual void pleaseStop(const std::string &calledFrom);

	};

}
