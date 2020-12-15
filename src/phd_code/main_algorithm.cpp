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
double CalcTotalCost(vector<int> schedule, double lengths[10], double starts[10], double ends[10], double ePen[10], double tPen[10])
{
    double weightedEarliness[10]={0,0,0,0,0,0,0,0,0,0};
    double weightedTardiness[10]={0,0,0,0,0,0,0,0,0,0};
    double completionTime[10];

    for (int j = 1; j<10; j++)
    {
        completionTime[j] = completionTime[j-1]+lengths[schedule[j]];
        weightedEarliness[j]=max((starts[j]-completionTime[j]), 0.00)*ePen[j];
        weightedTardiness[j]=max((completionTime[j]-ends[j]), 0.00)*tPen[j];
    }
    double cost=0;
    for (int i=0; i<10; i++)
    {
    cost=cost+weightedEarliness[i]+weightedTardiness[i];
    }
    return cost;
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

int main()
{
/**********************************************************************************************************************************/
/*declare all variables for future use*/
    std::ifstream scheduleFile("C:\\Users\\ZlobinN\\Documents\\Personal\\Masters\\Main Research Project\\SA_algorithm proj\\tenJobSchedule.csv", ios::in); //contains one original schedule
    string scheduleString;
    std::vector<int> scheduleVec;
    std::vector< vector<int> > SchedulePerms;
    double lengthArray[10]={94.2693, 33.0928, 38.3982, 48.7581, 61.5517, 11.3459, 30.7500, 79.5173, 47.3413, 25.5062};
    double startDueArray[10]={409.2334, 712.3790, 105.8300, 420.4227, 504.3522, 505.1008, 632.1335, 8.6627, 43.6808, 385.5200};
    double endDueArray[10]={426.9969, 738.0007, 139.1257, 437.0022, 550.3267, 536.0192, 676.6914, 39.1235, 47.3486, 406.1114};
    double ePenaltiesArray[10]={2.9044, 5.3071, 3.3947, 0.7354, 4.5492, 2.2288, 2.0975, 3.1438, 3.4458, 3.7409};
    double tPenaltiesArray[10]={5.2552, 9.6028, 6.1425, 1.3307, 8.2314, 4.0329, 3.7953, 5.6886, 6.2349, 6.7689};

/**********************************************************************************************************************************/
    int p=0;
    vector<double> EnergyVec;
    std::map<int, int> linkChannelMap;
    std::map<int, int>::iterator j=SchedulePerms.begin();
    //push all schedule permutations onto SchedulePerms, calculate energies for each permutation and push onto EnergyVec
    do
       {
          SchedulePerms.push_back(scheduleVec);
          p++;
          ++j;
          EnergyVec.push_back(CalcTotalCost(scheduleVec, lengthArray, startDueArray, endDueArray, ePenaltiesArray, tPenaltiesArray));
       }
    while (next_permutation(scheduleVec.begin(), scheduleVec.end()) );

/********************************************************************************************************************************************************/

vector<double> roughEnergies; //container to keep all sample energies
vector<int> roughEnergyLoc; //container to keep all sampled locations
vector<double> minEnergies; //container to keep the min energy in each sampling interval
vector<int> minEnergyLocations; //location of the min energies in each interval
vector<double> aveEnergies;
vector<int> aveEnergyLocations;

//use another file to record all the rough energy samples
//std::ofstream roughEnergSAseed = seedseq_random_using_clock();
//std::default_random_engine SAgen(SAseed);
//roughEnergyOF("C:\\Users\\ZlobinN\\Documents\\Personal\\Masters\\Main Research Project\\SA_algorithm proj\\SampleEnergies3.csv", ios::out);
//if (!roughEnergyOF)
// {
//     cerr << "File not opened" << endl;
//     exit(1);
// }

std::ofstream ModSAResultsOF("C:\\Users\\ZlobinN\\Documents\\Personal\\Masters\\Main Research Project\\SA_algorithm proj\\SA Mod2 Ti=10000 numIterations=1000000.csv", ios::out);
if (!ModSAResultsOF)
 {
     cerr << "File not opened" << endl;
     exit(1);
 }

const int interval = 36000; const unsigned int numSamples = 100; double Ti = 10000.00; const unsigned int numIterations = 1000000; int eqmax=1000; const int numIntervals=100;
ModSAResultsOF << "Interval size , " <<interval<<", Number of inner samples , "<<numSamples<<", Initial Temp , "<<Ti<<", Number of SA Iterations , "<<numIterations<<", max eq results , "<<eqmax<<endl<<endl;
ModSAResultsOF << "Index, Result" <<endl;

/*******************************************************************************************************************************************/
//This section takes samples from uniform distr. within each subinterval of 0-3628800 and adds these samples to a vector for later use
int32_t seed=0;
int32_t SAseed=0;
seed = seedseq_random_using_clock();
std::default_random_engine Roughgen(seed);

for (unsigned int SArun=0; SArun<100; SArun++)
{
SAseed = seedseq_random_using_clock();
std::default_random_engine SAgen(SAseed);

int rmin=0;
while (rmin<numIntervals)
{
    std::uniform_int_distribution<int> Roughdis(rmin*interval,rmin*interval+interval-1); //bound the distribution in the current interval
    for (int innerit=0;innerit<numSamples;innerit++)
    {
        unsigned int roughSample=Roughdis(Roughgen);
        roughEnergyLoc.push_back(roughSample);
        roughEnergies.push_back(EnergyVec.at(roughEnergyLoc.back()));
        //RoughEnergyLocOF<<*roughEnergyLoc.rbegin()<<endl;
    }
    rmin++;
    //RoughEnergyLocOF<<endl;
}

///**************************************************************************************************************************************/
///*MODIFICATION 1: use min. sample as starting point for SA*/

//cout<<"location " << distance(roughEnergies.begin(), std::min_element(roughEnergies.begin(),roughEnergies.end()))<<endl;

//unsigned int minEnergyloc = roughEnergyLoc.at(distance(roughEnergies.begin(), std::min_element(roughEnergies.begin(),roughEnergies.end())));
//unsigned int start=minEnergyloc;
////cout<<"start: " << start<<endl;
//ModSAResultsOF<<"start: " << start;
///**************************************************************************************************************************************/
///*MODIFICATION2: use sample mins for probability*/
//Find the min energy in each subinterval. Use inverse of energy to quantify probability and construct piecewise_constant_distribution
std::vector<double>::iterator EnergyIt = roughEnergies.begin();
int counter=0;
while (counter<numIntervals)
{
    vector<double>::iterator minEnergyloc = std::min_element(EnergyIt , (EnergyIt+numSamples));
    minEnergyLocations.push_back(roughEnergyLoc.at(distance(roughEnergies.begin(),minEnergyloc)));
    EnergyIt=EnergyIt+numSamples;
    //std::cout << "minEnergyLocation: " << roughEnergyLoc.at(distance(roughEnergies.begin(),minEnergyloc))<<endl;
    counter++;
}
vector<double> Probabilities;
vector<double> intervals;

double sum=0; double total=0;
for (vector<int>::iterator startPoints = minEnergyLocations.begin(); startPoints != minEnergyLocations.end(); startPoints++ )
{
    sum=sum+1/EnergyVec.at(*(startPoints));
    Probabilities.push_back( 1/(EnergyVec.at(*startPoints)) );
}
cout<<"sum = " <<sum<<endl<<"probabilities: ";
for (vector<double>::iterator prob=Probabilities.begin(); prob!=Probabilities.end(); prob++)
{
    *prob = (*prob)/sum;
    total+= *prob;
}

for (unsigned int range=0;range<100;range++)
    intervals.push_back(range*36000);

std::piecewise_constant_distribution<double> roughEnergyPDF(intervals.begin(), intervals.end(), Probabilities.begin());
//randomly choose starting point according to the roughEnergyPDF

double start = roughEnergyPDF(SAgen);
//cout <<"start point: " << static_cast <int> (std::floor(start))<<endl;
//
//
//////    std::uniform_int_distribution<int> SAstart(0,3628800);
//////    int start = SAstart(SAgen);
    vector<double> finalEnergies;
    if (start>=0 && start<3628800)
    {
//    //SA_algorithm alg1(Ti, SchedulePerms, EnergyVec, start, SAseed);
    SA_algorithm alg1(Ti, SchedulePerms, EnergyVec, static_cast <int> (std::floor(start)),SAseed);
    unsigned int eq=0;
    while( (alg1._algIter<numIterations) && (eq<eqmax) && (alg1.getTemp()>2)) //eqmax is the max number of results within 1 of each other in successive iterations before termintating SA
    {
        alg1.setCurrentTemp();
        alg1.New_soln();
        alg1.Acceptance();
        if (alg1.avgdE<1)
            eq++;
    }
        finalEnergies.push_back(alg1.getEnergyVec().at(alg1.currentBest));
        cout<<"best solution: " << alg1.getEnergyVec().at(alg1.currentBest) <<endl;
        double Result=*(std::min_element(finalEnergies.begin(),finalEnergies.end()));
        ModSAResultsOF<<"," <<alg1.currentBest<<", " <<Result<<endl;
    }
}

//cout<<"Final Result: "<<Result<<endl;
//    std::vector<double>::iterator EnergyIt = roughEnergies.begin()+(rmin*innersamples);
//    //vector<double>::iterator maxEnergyloc = std::max_element(EnergyIt , (EnergyIt+(innersamples-1)));
//    //double maxEnergy = *(std::max_element( EnergyIt , (EnergyIt+(innersamples-1)) ));
//    vector<double>::iterator minEnergyloc = std::min_element(EnergyIt , (EnergyIt+(innersamples-1)));
//    vector<double>::iterator minAveEnergyloc = std::min_element(EnergyIt , (EnergyIt+(innersamples-1)));
//    double minEnergy = *(std::min_element( EnergyIt , (EnergyIt+(innersamples-1)) ));
//    double sum=0;
//    for (EnergyIt=roughEnergies.begin()+rmin*innersamples; EnergyIt!=roughEnergies.begin()+rmin*innersamples+innersamples; EnergyIt++)
//    {
//        sum=sum + *EnergyIt;
//    }
//    double aveEnergy = sum/innersamples;
//    aveEnergies.push_back(aveEnergy);
//    aveEnergyLocations.push_back(distance(roughEnergies.begin(),minAveEnergyloc));
//    //cout << "max = " <<maxEnergy << endl << "min = " << minEnergy << endl << "average = " << aveEnergy << endl;
//    //roughEnergyOF << distance(roughEnergies.begin(),maxEnergyloc) << ","<< maxEnergy <<","<< distance(roughEnergies.begin(),minEnergyloc)<<"," << minEnergy<< "," << distance(roughEnergies.begin(),EnergyIt)<< ","<< aveEnergy <<endl;
//    //maxEnergies.at(0).at(rmin)=distance(roughEnergies.begin(),maxEnergyloc);
//    //maxEnergies.at(1).at(rmin)=(maxEnergy);
//    minEnergyLocations.push_back(distance(roughEnergies.begin(),minEnergyloc));
//    minEnergies.push_back(minEnergy);

//    cout << minEnergyLocations.at(rmin) <<"  :  " << minEnergies.at(rmin)<<endl;
//    double absmin = *(std::min_element(minEnergies.begin() , minEnergies.end()));
//    minit=distance(minEnergies.begin() ,std::min_element(minEnergies.begin() , minEnergies.end()));
//    cout << absmin << endl;
//    cout << minEnergyLocations.at(minit) << endl;
//}
//while (it!=aveEnergies.end())
//{
//    roughEnergyOF << *it << endl;
//    //roughEnergyOF<< aveEnergyLocations.at(rmin) <<"  :  " << aveEnergies.at(rmin)<<endl;
//    it++;
//}

//vector<double>::iterator it=finalEnergies.begin();
//while (it!=finalEnergies.end())
//{
//    scheduleOutFile << *it << endl;
//    it++;
//}





return(0);
}


