#include "SA_algorithm.h"
#include <cmath>
using std::log;

std::vector<vector<int>> SA_algorithm::_scheduleVec = {};

vector<double> SA_algorithm::_energyVec = {};

SA_algorithm::SA_algorithm(double Ti, int numberOfLinks, int numberOfChannels, std::map<int, int> startSolution, uint32_t Seed)
{
    _initTemp=Ti;
    _currentTemp=_initTemp;
    _numLinks = numberOfLinks;
    _numChannels = numberOfChannels;
    _currentSolutionMap = startSolution;
    // _solnEnergyVec.push_back(*_energyVec.begin());
    solnIter=1;
    _algIter=0;
    currentBest=0;
    avgdE=0;
    _seed=Seed;
    std::default_random_engine gen(_seed);
    std::uniform_int_distribution<int> dis{1,RAND_MAX};
//    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
//    std::default_random_engine gen;
//    std::uniform_int_distribution<int> dis{1,RAND_MAX};
}

void SA_algorithm::setCurrentTemp()
{
    double newTemp = _initTemp/(log(_algIter+2));
    _currentTemp=newTemp;
}

double SA_algorithm::getTemp()
{
    return _currentTemp;
}

std::pair<int, int> generateNewSolution(int numLinks, int numChannels) {
    // choose one link, assign a new channel to it
    int link = rand() % numLinks;
    int newChannel = rand() % numChannels;
    return std::make_pair(link, newChannel);
}

void SA_algorithm::Acceptance()
{
    bool acceptpoint;
    double dE=_solnEnergyVec.at(_solnEnergyVec.size()-1) - _solnEnergyVec.at(_solnEnergyVec.size()-2);
    avgdE=(avgdE+abs(dE))/2;
    double n=0;
    double h=0;

    if (dE < 0)
    {
        acceptpoint = true;
    }

    else
    {
        n=dis(gen)/((double)RAND_MAX+1);
        h = 1/(1+exp(dE/_currentTemp));
        if (h > n)
            acceptpoint = true;
        else
            acceptpoint = false;
    }
    if ((acceptpoint==false)&&(_solnEnergyVec.size()>2))
    {
            _solnEnergyVec.pop_back();
            //_solnVec.pop_back();
    }
    _algIter++;
}

void SA_algorithm::calcSolutionEnergy() 
{
    //run simulation, getting SNR value sample, get average SNR, write to file, read it here
}

vector< vector<int> > SA_algorithm::getScheduleVec()
{
    return _scheduleVec;
}

vector<double> SA_algorithm::getEnergyVec()
{
    return _energyVec;
}

vector<double> SA_algorithm::getSolnEnergyVec()
{
    return _solnEnergyVec;
}


