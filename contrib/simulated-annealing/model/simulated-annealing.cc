/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "simulated-annealing.h"
#include "ns3/mesh-sim.h"
#include <cmath>
using std::log;


namespace ns3 {

SimulatedAnnealing::SimulatedAnnealing(double Ti, std::vector<std::pair<int, int>>* links, int numberOfChannels, std::map<int, int> startSolution, uint32_t Seed, std::string filename)
{
    _initTemp = Ti;
    std::vector<std::pair<int, int>> _links = *links;
    _currentTemp =_initTemp;
    _numLinks = sizeof(_links);
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
    std::default_random_engine gen(_seed);
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

void SimulatedAnnealing::calcSolutionEnergy() 
{
    //run simulation, getting SNR value sample, get average SNR, write to file, read it here
    MeshSim mesh;
    mesh.Run(_currentSolutionMap, _links);
    ifstream file;
    file.open (_energyFile.c_str());
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
        _energyVec.push_back(snrAvg);
        file.close();
    }
}

vector<double> SimulatedAnnealing::getEnergyVec()
{
    return _energyVec;
}

std::map<int, int>* SimulatedAnnealing::getCurrentSolution()
{
    return &_currentSolutionMap;
}


}

