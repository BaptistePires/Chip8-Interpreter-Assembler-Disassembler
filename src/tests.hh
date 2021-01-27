#ifndef TESTS_HEADERS
#define TESTS_HEADERS
/**
 *  This file will contains all of the tests. 
 **/

#define IS_TRUE(x)  if (!(x)) throw TestException(__PRETTY_FUNCTION__ , __LINE__, str(x)); 

#include <exception>
#include <string>
#include <iostream>
#include <sstream>
#include <istream>
#include <ostream>
#include <vector>

#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

namespace tests {
    class TestException: public std::exception {

        const char* fct;
        const int lineNo;
        const char* testSent;


        TestException(const char* fct, const int lineNo, const char* testSent): fct(fct), lineNo(lineNo), testSent(testSent){}

        public:
            char const* what() const noexcept{
                std::stringstream ss;
                ss << "Test didn't pass at line : " << lineNo << std::endl << "\tin function : " << fct << std::endl;
                ss << "Test that failed : " << testSent << std::endl;
                return ss.str().c_str();
            }   

        
    };



    void runTests();
    void testAssembler();
    void testOpcodes();
};

#endif

