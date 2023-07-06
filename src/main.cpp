#include <iostream>
#include <unistd.h>

#include <test1.h>

int main(int argc, char* argv[]){

    std::cout << A() << std::endl;
    std::cout << "Hello, from modeler!" << std::endl;

    std::cout << argv[0] << std::endl;
    auto path = getcwd(NULL, 0);

    std::cout << path << std::endl;

    return 0;
}
