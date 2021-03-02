#include "filetest.h"

FileTest::FileTest()
{
    file.open("/home/natasha/repos/ns-3-dev-git/acceptedSolutions.txt", std::ios::out | std::ios::app);
    if (!file) {
        std::cout << "can't open output file" << std::endl;
    }
}

void FileTest::writeToFile()
{
    file << "writing to file \n";
}

void FileTest::Run()
{
    writeToFile();
    std::cout << "written to file \n";
}
