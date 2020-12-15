#ifndef SA_ALGORITHM_H
#define SA_ALGORITHM_H

#include <iostream>     // needed for basic IO
#include <cmath>        // needed for chance calculation
#include <assert.h> // will use assert to check certain values
#include <vector>
#include <random>
#include <chrono>
#include <map>
using namespace std;

class SA_algorithm
{
public:
    SA_algorithm(double Ti, int numberOfLinks, int numberOfChannels, std::map<int, int> startSolution, uint32_t Seed);
    std::pair<int, int> generateNewSolution(int numLinks, int numChannels);
    void setCurrentTemp();
    void Acceptance();
    void calcSolutionEnergy();
    double getTemp();
    double _initTemp;
    unsigned int solnIter;
    //std::vector<std::vector<int> > _solnVec;
    std::default_random_engine generator;
    std::vector<vector<int> > getScheduleVec();
    std::vector<vector<int> > getScheduleVecPtr();
    std::vector<double> getEnergyVec();
    std::vector<double> getSolnEnergyVec();
    unsigned int _algIter;
    int currentBest=0;
    double avgdE;
    std::default_random_engine gen;
    std::uniform_int_distribution<int> dis{};

private:
    int _numLinks;
    int _numChannels;
    std::map<int, int> _currentSolutionMap;
    std::vector< double > _solnEnergyVec;
    static std::vector<double> _energyVec;
    std::vector<double>::iterator _energyVecPtr;
    double _currentTemp;
    double _currentEnergy;
    double _newEnergy;
    int32_t _seed;
};

#endif // SA_ALGORITHM_H_INCLUDED
