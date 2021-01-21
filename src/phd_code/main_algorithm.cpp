#include <iostream>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <algorithm>
using std::next_permutation;
using std::sort;
#include <fstream>
#include "~/repos/ns-3-dev-git/src/phd_code/SA_algorithm.h"
#include <chrono>
#include <map>

//function to calculate energy - to be used later
double CalcTotalCost()
{
    int result = system("/usr/bin/python average.py");
    cout << "average from python script = " << result << endl;;
}

uint32_t seedseq_random_using_clock()
{
    uint64_t seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::seed_seq seeder{uint32_t(seed),uint32_t(seed >> 32)};
    ++seed;
    //cout<<seed<<endl;
    int out;
    seeder.generate(&out,&out+1);
    return out;
}

std::map<int, int> mapLinkChannel(int numLinks, int maxChannel) {
    std::map<int, int> solution;
    int channel = 1;

    for (int n=0; n<numLinks; n++) {
        solution.insert({ n, channel });
        if (channel < maxChannel) {
            channel ++;
        }
        else {
            channel=1;
        }
    }
    return solution;
}

int main()
{
/**********************************************************************************************************************************/
/*declare all variables for future use*/

    int p=0;
    vector<double> EnergyVec;
    std::map<int, int> linkChannelMap;
    std::map<int, int>::iterator j=linkChannelMap.begin();

/********************************************************************************************************************************************************/

std::ofstream SAResultsOF("/home/natasha/repos/ns-3-dev-git/src/phd_code/SA_results.csv", ios::out);
if (!SAResultsOF)
 {
     cerr << "File not opened" << endl;
     exit(1);
 }

SAResultsOF << "Index, Result" <<endl;
double Ti;
int numOfLinks;
int numOfChannels;
std::map<int, int> startSolution = mapLinkChannel(numOfLinks, numOfChannels);
uint32_t Seed;
std::string filename;

SA_algorithm alg1(Ti, numberOfLinks, numOfChannels, startSolution, Seed, filename);
unsigned int eq=0;
while( (alg1._algIter<numIterations) && (eq<eqmax) && (alg1.getTemp()>2)) //eqmax is the max number of results within 1 of each other in successive iterations before termintating SA
{
    alg1.setCurrentTemp();
    alg1.generateNewSolution();
    alg1.Acceptance();
    if (alg1.avgdE<1)
        eq++;
}
    finalEnergies.push_back(alg1.getEnergyVec().at(alg1.currentBest));
    cout<<"best solution: " << alg1.getEnergyVec().at(alg1.currentBest) <<endl;
    double Result=*(std::min_element(finalEnergies.begin(),finalEnergies.end()));
    SAResultsOF<<"," <<alg1.currentBest<<", " <<Result<<endl;
}

cout<<"Final Result: "<<Result<<endl;

return(0);
}


