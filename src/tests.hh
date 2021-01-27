#ifndef TESTS_HEADERS
#define TESTS_HEADERS
/**
 *  This file will contains all of the tests. 
 **/
#define XSTR(s) STR(s)
#define STR(s) #s
#define IS_TRUE(x)  if (!(x)) throw TestException(__PRETTY_FUNCTION__ , __LINE__, STR(x)); 

#include <exception>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <sstream>

#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
namespace tests {
    class TestException: public std::exception {

        const char* fct;
        const int lineNo;
        const char* testSent;



        public:
            TestException(const char* fct, const int lineNo, const char* testSent): fct(fct), lineNo(lineNo), testSent(testSent){}

            char const* what() const noexcept{
                std::stringstream ss;
                ss << "Test didn't pass at line : " << lineNo << std::endl << "\tin function : " << fct << std::endl;
                ss << "Test that failed : " << testSent << std::endl;

                return ss.str().c_str();
            }   

        
    };

    struct TestAssemblerFilesWrapper{
        uint8_t *assembledFileBinContent;
        uint8_t *correctFileBinContent;

        ~TestAssemblerFilesWrapper();
    };

    typedef TestAssemblerFilesWrapper TestAssemblerFilesWrapper_t;

    void runTests();
    void testAssembler();
    void testOpcodes();

    TestAssemblerFilesWrapper_t* loadTestAssemblerFiles(const char* toAssembleFile, const std::string&& correctFile);
};

#endif

