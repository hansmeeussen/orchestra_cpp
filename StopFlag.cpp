#include "StopFlag.h"

namespace orchestracpp
{

	StopFlag::StopFlag()
	{
		cancelled = false;
	}

	bool StopFlag::isCancelled()
	{
		return cancelled;
	}

	void StopFlag::addChild(StopFlag *child)
	{
		children.push_back(child);
	}

	void StopFlag::removeChild(StopFlag *child)
	{
		std::vector<StopFlag*>::iterator position = std::find(children.begin(), children.end(), child);
		if (position != children.end()) {
			children.erase(position);
		}

		// java children.remove(child);
	}

	void StopFlag::reset()
	{
		cancelled = false;
		for (auto s : children) {
			s->reset();
		}
	}

	void StopFlag::pleaseStop(const std::string &calledFrom)
	{
		// stop flag and all children
		cancelled = true;
		for (auto s : children) {
			s->pleaseStop("Stopping a child flag!");
		}
	}
}
