#include "tests.hh"

using namespace tests;

void tests::runTests() {
    std::cout << "Running all tests..." << std::endl;
    tests::testAssembler();

    std::cout << "All tests done. All passed." << std::endl;
}

void tests::testAssembler() {
    std::cout << "Running Assembler tests" << std::endl;

    std::vector<char*> cmdAssemble{"pyton3", "assembler/c8assembler.py", "res/tests/test_1.asm", "-o", "res/tests/test_1.c8c", "--no-print", NULL};
    
    pid_t child;
            
    if((child = fork()) == -1){
        perror("fork error");
        exit(EXIT_FAILURE);
    } else{ 
        if(!child) {
        
            execvp("python3", cmdAssemble.data());
            perror("excvp");
            exit(EXIT_FAILURE);
        }
    }

    int status;
    if(waitpid(child, &status, 0) == -1){
        perror("waitpid");
        exit(EXIT_FAILURE);
    }

    if(WIFEXITED(status)) {
        int exitStatus = WEXITSTATUS(status);
        if(exitStatus != 0) {
            std::cout << "Child error : " << exitStatus << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    TestAssemblerFilesWrapper_t* filesContent = tests::loadTestAssemblerFiles(cmdAssemble[4], "res/tests/test_1_correct.c8c");    
    uint8_t *byte1 = &filesContent->correctFileBinContent[0];
    uint8_t *byte2 = &filesContent->assembledFileBinContent[0];
    for( byte1, byte2; byte1 != NULL && byte2 != NULL; byte1++, byte2++) {
        if(*byte1 != *byte2){
            std::cout << "hi" << std::endl;
        }
    }
    std::cout << "Assembler passed test" << std::endl;
}


TestAssemblerFilesWrapper_t* tests::loadTestAssemblerFiles(const char* toAssembleFile, const std::string&& correctFile){
    TestAssemblerFilesWrapper_t* wrapper = new TestAssemblerFilesWrapper_t();

    std::ifstream binAssembled(toAssembleFile, std::ios::binary | std::ios::ate);
    
    // The correct file that should generated
    
    IS_TRUE(binAssembled.is_open());
    size_t fSize = binAssembled.tellg();
    wrapper->assembledFileBinContent = new uint8_t[fSize];
    binAssembled.read((char*)wrapper->assembledFileBinContent, fSize);
    binAssembled.close();

    std::ifstream correctBin(correctFile, std::ios::binary | std::ios::ate);
    IS_TRUE(correctBin.is_open());
    
    fSize = correctBin.tellg();
    wrapper->correctFileBinContent = new uint8_t[fSize];
    correctBin.read((char*)wrapper->correctFileBinContent, fSize);

    return wrapper;
    

}