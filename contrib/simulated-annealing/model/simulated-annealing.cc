/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "simulated-annealing.h"
#include "ns3/mesh-sim.h"
#include "ns3/global.h"
#include <cmath>
#include <regex>
using std::log;
using std::ifstream;

namespace ns3 {

    const double kBoltzmann=1.3806504e-23;

SimulatedAnnealing::SimulatedAnnealing(double Ti, std::vector<std::pair<int, int>> links, int numberOfChannels, std::map<int, int> startSolution, uint32_t Seed, std::string filename)
{
    _initTemp = Ti;
    _links = links;
    _currentTemp =_initTemp;
    _numLinks = _links.size();
    _numChannels = numberOfChannels;
    _currentSolutionMap = startSolution;
    _energyVec = {};
    _solnVec.push_back(_currentSolutionMap);
    _sinrAvgFilename = filename;
    _solutionFile.open("/home/natasha/repos/ns-3-dev-git/acceptedSolutions_exp_cooling_0.8_Ti=800.txt", std::ios::out | std::ios::app);
    if (!_solutionFile) {
        std::cerr << "can't open output file" << std::endl;
    }

    _algIter = 0;
    currentBest = 0;
    _seed = Seed;
    gen.seed(_seed);
    std::uniform_int_distribution<int> dis{1,RAND_MAX};
  
//    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
//    std::default_random_engine gen;
}

void SimulatedAnnealing::setCurrentTemp()
{
    // double newTemp = _initTemp/(1+log(_algIter+1));
    double newTemp = _initTemp*(pow(0.8, _algIter));
    _currentTemp = newTemp;
    std::cout << "\ncurrent temp = " << _currentTemp << "\n";
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
    _solnVec.push_back(_currentSolutionMap);
    std::map<int, int>::iterator mapit;
    // for (mapit=_currentSolutionMap.begin(); mapit!=_currentSolutionMap.end(); ++mapit) {
    //     std::cout << mapit->first << " => " << mapit->second << std::endl;
    // }
}

void SimulatedAnnealing::Acceptance()
{
    bool acceptpoint;
    double dE =_energyVec.at(_energyVec.size()-1) - _energyVec.at(_energyVec.size()-2);
    std::cout << "dE = " << dE << "\n";
    double n=0.0;
    double h=0.0;

    if (dE < 0)
    {
        acceptpoint = true;
    }
    else
    {
        n=dis(gen)/((double)RAND_MAX+1);
        // h = 1/(1+exp(dE/(kBoltzmann*_currentTemp)));
        h = exp(-1*dE/_currentTemp);
        std::cout << "random value = " << n << " , h = " << h << "\n";
        if (h > n)
            acceptpoint = true;
        else
            acceptpoint = false;
    }
    NS_LOG_UNCOND("point accepted is " << acceptpoint);
    if ((acceptpoint==false)&&(_energyVec.size()>2))
    {
            _energyVec.pop_back();
            //_solnVec.pop_back();_solutionFile << _energyVec.back() << "\n";
    }
    std::cout << "solutionFile << " << _energyVec.back() << "\n";
    _solutionFile << _energyVec.back() << "\n";
    _algIter++;
}

void SimulatedAnnealing::calcSolutionEnergy() 
{
    //run simulation, getting SNR value sample, get average SNR, write to file, read it here
    MeshSim mesh({});
    mesh.Run(_currentSolutionMap, _links);
    std::cout << "Run mesh simulation to completion" << "\n";
    ifstream file;
    file.open(_sinrAvgFilename.c_str(), std::ios::in);
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
        _energyVec.push_back(1000/snrAvg);
        std::cout << "Pushed " << _energyVec.back() << " to _energyVec\n";
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

void SimulatedAnnealing::Initialize()
{
    NS_LOG_UNCOND("Initializing");
    calcSolutionEnergy();
    _solutionFile << _energyVec.back() << "\n";
    std::cout << "solutionFile << " << _energyVec.back() << "\n";
    _algIter++;
}

void SimulatedAnnealing::Run()
{
    setCurrentTemp();
    generateNewSolution();
    calcSolutionEnergy();
    Acceptance();
}


} //namespace ns3

