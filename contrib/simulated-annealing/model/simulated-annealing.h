/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef SIMULATED_ANNEALING_H
#define SIMULATED_ANNEALING_H

#include <iostream>     // needed for basic IO
#include <cmath>        // needed for chance calculation
#include <assert.h> // will use assert to check certain values
#include <vector>
#include <random>
#include <chrono>
#include <map>
#include <string>
#include <fstream>
#include <algorithm>

#include "ns3/output-stream-wrapper.h"
#include "ns3/trace-helper.h"

namespace ns3 {

class SimulatedAnnealing
{
public:
    SimulatedAnnealing(double Ti, std::vector<std::pair<int, int>> links, int numberOfChannels, std::map<int, int> startSolution, uint32_t Seed, std::string filename);
    void generateNewSolution();
    void setCurrentTemp();
    void Acceptance();
    void calcSolutionEnergy();
    double getTemp();
    void Initialize();
    void Run();
    double _initTemp;
    unsigned int solnIter;
    //std::vector<std::vector<int> > _solnVec;
    std::default_random_engine generator;
    std::map<int, int>*  getCurrentSolution();
    std::vector<double> getEnergyVec();
    std::vector<double> getSolnEnergyVec();
    void initializeEnergyVec();
    unsigned int _algIter;
    int currentBest=0;
    double avgdE;
    // std::default_random_engine gen;
    // std::uniform_int_distribution<int> dis{};

private:
    int _numLinks;
    int _numChannels;
    std::vector<std::pair<int, int>> _links;
    std::map<int, int> _currentSolutionMap;
    std::string _sinrAvgFilename;
    std::ofstream _solutionFile;
    std::vector< double > _solnEnergyVec;
    std::vector<std::map<int, int>> _solnVec;
    std::vector<double> _energyVec;
    std::vector<double>::iterator _energyVecPtr;
    double _currentTemp;
    double _currentEnergy;
    double _newEnergy;
    int32_t _seed;
    std::default_random_engine gen;
    std::uniform_int_distribution<int> dis;
};

}

#endif /* SIMULATED_ANNEALING_H */

