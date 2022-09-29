#pragma once

#include <string>
#include <iostream>


/*----------------------------------------------------------------------------------------
 * A StringTokenizer class that mimics the Java StringTokenizer (and can return delimiters)
 * Hans Meeussen October 2019
*---------------------------------------------------------------------------------------- 
*/
class StringTokenizer {
    
private:
    StringTokenizer();
    
    std::string delimiters;
    std::string inputstring;
    bool includeDelimiters;
    long pos; // position points to start position of next token
    
public:
    
    StringTokenizer(const std::string& , const char*, bool);

    bool hasMoreTokens();
    
    void skipDelimiters();
    
    bool isDelimiter(const char& s);
    
    std::string nextToken();
};
