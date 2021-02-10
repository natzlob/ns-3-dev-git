#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

int main()
{
    string _energyFile = "SNRaverage.txt";
    std::vector<double> _energyVec;
    ifstream file;
    file.open(_energyFile.c_str(), std::ios::in);
    char ch;
    string lastLine;
    double snrAvg;
    if (file.is_open())
    {
        file.seekg(-1,ios_base::end);
        file.get(ch);                         // get next char at loc 66
        if (ch == '\n')
        {
            file.seekg(-2, std::ios::cur);    // move to loc 64 for get() to read loc 65 
            file.seekg(-1, std::ios::cur);    // move to loc 63 to avoid reading loc 65
            file.get(ch);                     // get the char at loc 64 ('5')
            while(ch != '\n')                   // read each char backward till the next '\n'
            {
                file.seekg(-2, std::ios::cur);    
                file.get(ch);
            }
            std::getline(file,lastLine);
            cout << "The last line : " << lastLine << '\n';     
        }
        snrAvg = std::stod(lastLine);

        cout << "Result: " << snrAvg << '\n';
        _energyVec.push_back(snrAvg);
        file.close();
    }

    return 0;
}