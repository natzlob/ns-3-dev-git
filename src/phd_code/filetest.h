#ifndef FILETEST_H
#define FILETEST

#include <string>
#include <fstream>
#include <iostream>

class FileTest
{
public:
    FileTest();
    void writeToFile();
    void Run();

private:
    std::ofstream file;

};

#endif /* FILETEST_H */