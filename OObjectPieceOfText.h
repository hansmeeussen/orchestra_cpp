#pragma once

#include <string>

namespace orchestracpp
{

	class OObjectPieceOfText final
	{
	public:
		std::string text;
        
		OObjectPieceOfText(const std::string &text);
        ~OObjectPieceOfText(){
		};

    };
}
