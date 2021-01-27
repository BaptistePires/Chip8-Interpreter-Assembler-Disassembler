#include "tests.hh"

void tests::runTests() {
    testAssembler();
}

void tests::testAssembler() {
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
    std::cout << "Child exit :" << status << std::endl;
}