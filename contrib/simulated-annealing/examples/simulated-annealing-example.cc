/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/simulated-annealing.h"

using namespace ns3;

//function to calculate energy - to be used later
double CalcTotalCost()
{
    double result = system("/usr/bin/python average.py");
    cout << "average from python script = " << std::to_string(result) << endl;
    return result;
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


int 
main (int argc, char *argv[])
{
  bool verbose = true;

  CommandLine cmd (__FILE__);
  cmd.AddValue ("verbose", "Tell application to log if true", verbose);
  cmd.Parse (argc,argv);

  uint32_t seed = seedseq_random_using_clock();
  double initTemp = 100;
  int numLinks = 9;
  int numChannels = 13;
  std::map<int, int> startSolution = mapLinkChannel(numLinks, numChannels);

  SimulatedAnnealing SA(initTemp, numLinks, numChannels, startSolution, seed, "SNRaverage.csv");
  cout << "starting solution link => channel \n";
  std::map<int, int>::iterator it;
  for(it=startSolution.begin(); it!=startSolution.end(); ++it){
    cout << it->first << " => " << it->second << '\n';
  }
  SA.setCurrentTemp();
  cout << "Current temp = " << std::to_string(SA.getTemp()) << endl;
  SA.calcSolutionEnergy();
  SA.generateNewSolution();
  std::map<int, int> newSolution = *SA.getCurrentSolution();
  for(it=newSolution.begin(); it!=newSolution.end(); ++it){
    cout << it->first << " => " << it->second << '\n';
  }
  SA.calcSolutionEnergy();
  //SA.Acceptance();


  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}


