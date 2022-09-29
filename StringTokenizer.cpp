//
//  A C++ StringTokenizer to mimic the Java version...
//
//  Hans Meeussen on 26/10/2019.

#include <stdio.h>
#include <string>
#include "StringTokenizer.h"


   StringTokenizer::StringTokenizer(const std::string& s, const char* delim = NULL, bool includeDel = true) :
       inputstring(s), includeDelimiters(includeDel),pos(0) {

       if (!delim)
          delimiters = " \f\n\r\t\v";  //default to whitespace
       else{
          delimiters = delim;
       }
    }
    
    bool StringTokenizer::hasMoreTokens() {
        return(pos<inputstring.length());
        
    }
    
    void StringTokenizer::skipDelimiters(){
        while(isDelimiter(inputstring[pos])){
            pos++;
            if (pos>=inputstring.length()){break;}
        }
    }
    
   bool StringTokenizer::isDelimiter(const char& s){
        if (delimiters.find(s) != std::string::npos) {
            return true;
        }
        return false;
    }

    
    std::string StringTokenizer::nextToken() {

        if(hasMoreTokens()) {
            if (isDelimiter(inputstring[pos])){
                if(includeDelimiters){
                   return std::string (1, inputstring[pos++]); // return the delimiter
                }else{
                    skipDelimiters();
                }
            }
        }
        
        if(hasMoreTokens()) {
                std::string tmptoken = "";
                do{
                    tmptoken = tmptoken + std::string (1, inputstring[pos]);
                    pos++;
                    if (pos>=inputstring.length()){break;}
                }while(!isDelimiter(inputstring[pos]));
            return tmptoken;
        }
            
        return "";
        
    }

