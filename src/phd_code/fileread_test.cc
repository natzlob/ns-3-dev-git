#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

int main()
{
    string _energyFile = "SNRaverage.csv";
    std::vector<double> _energyVec;
    ifstream file;
    file.open(_energyFile.c_str());
    double snrAvg;
    if (file.is_open())
    {
        file.seekg(-1,ios_base::end);
        bool keepLooping = true;
        while(keepLooping) {
            char ch;
            file.get(ch);                            // Get current byte's data

            if((int)file.tellg() <= 1) {             // If the data was at or before the 0th byte
                file.seekg(0);                       // The first line is the last line
                keepLooping = false;                // So stop there
            }
            else if(ch == '\n') {                   // If the data was a newline
                keepLooping = false;                // Stop at the current position.
            }
            else {                                  // If the data was neither a newline nor at the 0 byte
                file.seekg(-2,ios_base::cur);        // Move to the front of that data, then to the front of the data before it
            }
        }

        string lastLine;            
        getline(file,lastLine);                      // Read the current line
        cout << "Result: " << lastLine << '\n'; 
        snrAvg = std::stod(lastLine);

        cout << "Result: " << snrAvg << '\n';
        _energyVec.push_back(snrAvg);
        file.close();
    }

    return 0;
}