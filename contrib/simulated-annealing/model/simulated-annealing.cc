/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "simulated-annealing.h"
#include "ns3/mesh-sim.h"
#include <cmath>
#include <regex>
using std::log;
using std::ifstream;

namespace ns3 {

SimulatedAnnealing::SimulatedAnnealing(double Ti, std::vector<std::pair<int, int>> links, int numberOfChannels, std::map<int, int> startSolution, uint32_t Seed, std::string filename)
{
    _initTemp = Ti;
    _links = links;
    _currentTemp =_initTemp;
    _numLinks = _links.size();
    _numChannels = numberOfChannels;
    _currentSolutionMap = startSolution;
    _energyVec = {};
    _energyFile = filename;
    // _solnEnergyVec.push_back(*_energyVec.begin());
    solnIter = 1;
    _algIter = 0;
    currentBest = 0;
    avgdE = 0;
    _seed = Seed;
    gen.seed(_seed);
    std::uniform_int_distribution<int> dis{1,RAND_MAX};
  
//    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
//    std::default_random_engine gen;
}

void SimulatedAnnealing::setCurrentTemp()
{
    double newTemp = _initTemp/(log(_algIter+2));
    _currentTemp = newTemp;
}

double SimulatedAnnealing::getTemp()
{
    return _currentTemp;
}

void SimulatedAnnealing::generateNewSolution() {
    // choose one link, assign a new channel to it
    int link = rand() % _numLinks;
    int newChannel = rand() % _numChannels;
    _currentSolutionMap[link] = newChannel;
}

void SimulatedAnnealing::Acceptance()
{
    bool acceptpoint;
    double dE=_energyVec.at(_energyVec.size()-1) - _energyVec.at(_energyVec.size()-2);
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
    if ((acceptpoint==false)&&(_energyVec.size()>2))
    {
            _energyVec.pop_back();
            //_solnVec.pop_back();
    }
    _algIter++;
}

void SimulatedAnnealing::calcSolutionEnergy() 
{
    //run simulation, getting SNR value sample, get average SNR, write to file, read it here
    MeshSim mesh({});
    mesh.Run(_currentSolutionMap, _links, {});
    ifstream file;
    file.open(_energyFile.c_str(), std::ios::in);
    char ch;
    std::string lastLine;
    double snrAvg;
    if (file.is_open())
    {
        file.seekg(-1, std::ios_base::end);
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
        }
        snrAvg = std::stod(lastLine);

        std::cout << "Result: " << snrAvg << '\n';
        _energyVec.push_back(1/snrAvg);
        file.close();
    }
}

std::vector<double> SimulatedAnnealing::getEnergyVec()
{
    return _energyVec;
}

std::map<int, int>* SimulatedAnnealing::getCurrentSolution()
{
    return &_currentSolutionMap;
}


} //namespace ns3

