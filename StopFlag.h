#pragma once

#include <algorithm>
#include <string>
#include <vector>
#include <atomic>

namespace orchestracpp
{

	class StopFlag final
	{

		// in Java its appears significantly faster to use a volatile variable for 
		// checking the flag than a synchronized method
		// we only set the flag via synchronized methods, we read it unsynchronized

		// The C++ version needs to be checked/prepared for multithreaded use!

	public:
    //JAVA  'volatile' has a different meaning in C++:
    //ORIGINAL LINE: public volatile boolean cancelled;
		std::atomic<bool> cancelled;


	private:
		std::vector<StopFlag*> children;

	public:
		StopFlag();

		bool isCancelled();

		void addChild(StopFlag *child);

		void removeChild(StopFlag *child);

		void reset();

		/**
		 * stops all children, but not the parent!
		 */
		void pleaseStop(const std::string &calledFrom);

	};

}
