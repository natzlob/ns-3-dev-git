/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/simulated-annealing.h"

using namespace ns3;


//function to calculate energy - to be used later
double CalcTotalCost()
{
    double result = system("/usr/bin/python average.py");
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

std::map<int, int> mapLinkChannel(int numLinks, std::vector<int> channels) {
    std::map<int, int> solution;
    int channelIndex = 0;
    int maxChannel = channels.size()-1;

    for (int n=0; n<numLinks; n++) {
        solution.insert({ n, channels[channelIndex] });
        if (channelIndex < maxChannel) {
            channelIndex ++;
        }
        else {
            channelIndex=0;
        }
    }
    return solution;
}

template < typename T > 
std::vector<std::pair<T,T> > make_unique_pairs(const std::vector<T>& set)
{
  std::vector< std::pair<T,T> > result;
  std::vector< std::reference_wrapper< const T > > seq(set.begin(), set.end());

  std::random_shuffle(std::begin(seq), std::end(seq));

  for (size_t i=0; i<seq.size() -1; i++) {
    result.emplace_back(set[i], seq[i]);
  }

  return result;
}

int 
main (int argc, char *argv[])
{
  bool verbose = true;

  CommandLine cmd (__FILE__);
  cmd.AddValue ("verbose", "Tell application to log if true", verbose);
  cmd.Parse (argc,argv);

  uint32_t seed = seedseq_random_using_clock();
  double initTemp = 100.00;
  int numLinks = 9;
  int numChannels = 13;
  uint16_t maxIterations = 10000;

  std::vector<int> channels (numChannels);
  std::iota(channels.begin(), channels.end(), 1);
  std::random_shuffle(std::begin(channels), std::end(channels));

  std::map<int, int> startSolution = mapLinkChannel(numLinks, channels);

  std::vector<int> nodeNums (numLinks);
  std::iota(nodeNums.begin(), nodeNums.end(), 0);
  std::vector<std::pair <int, int>> links = make_unique_pairs(nodeNums);

  // std::cout << "starting solution link => channel \n";
  // std::map<int, int>::iterator it;
  // for(it=startSolution.begin(); it!=startSolution.end(); ++it){
  //   std::cout << it->first << " => " << it->second << '\n';
  // }
  SimulatedAnnealing SA(initTemp, links, numChannels, startSolution, seed, "SNRaverage.csv");

  SA.setCurrentTemp();
  // std::cout << "Current temp = " << std::to_string(SA.getTemp()) << std::endl;
  SA.calcSolutionEnergy();
  SA.generateNewSolution();
  std::map<int, int> newSolution = *SA.getCurrentSolution();
  // for(it=newSolution.begin(); it!=newSolution.end(); ++it){
  //   std::cout << it->first << " => " << it->second << '\n';
  // }
  SA.calcSolutionEnergy();
  SA.Acceptance();

  while ( (SA._algIter < maxIterations) && SA.getTemp()>2 ) {
    SA.setCurrentTemp();
    SA.generateNewSolution();
    SA.calcSolutionEnergy();
    SA.Acceptance();
  }


  // Simulator::Run ();
  // Simulator::Destroy ();
  return 0;
}


