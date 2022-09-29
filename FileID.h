#pragma once

#include <string>

namespace orchestracpp { class FileBasket; }

namespace orchestracpp
{

	/**
	 * A FileID class stores a filename + connected filebasket
	 */
	class FileID
	{

	public:
		std::string name;
		FileBasket *basket = nullptr;

		FileID(FileBasket *b, const std::string &n) :name(n), basket(b) {
		};

	};

}
