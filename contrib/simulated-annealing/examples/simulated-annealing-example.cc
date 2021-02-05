/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/simulated-annealing.h"

using namespace ns3;


//function to calculate energy - to be used later
double CalcTotalCost()
{
    double result = system("/usr/bin/python average.py");
    std::cout << "average from python script = " << std::to_string(result) << std::endl;
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

  system("python src/phd_code/average.py");

  uint32_t seed = seedseq_random_using_clock();
  double initTemp = 100;
  int numLinks = 9;
  int numChannels = 13;
  std::map<int, int> startSolution = mapLinkChannel(numLinks, numChannels);
  std::vector<int> nodeNums (numLinks);
  std::iota(nodeNums.begin(), nodeNums.end(), 0);
  std::vector<std::pair <int, int>> links = make_unique_pairs(nodeNums);

  SimulatedAnnealing SA(initTemp, &links, numChannels, startSolution, seed, "SNRaverage.csv");
  std::cout << "starting solution link => channel \n";
  std::map<int, int>::iterator it;
  for(it=startSolution.begin(); it!=startSolution.end(); ++it){
    std::cout << it->first << " => " << it->second << '\n';
  }
  SA.setCurrentTemp();
  std::cout << "Current temp = " << std::to_string(SA.getTemp()) << std::endl;
  SA.calcSolutionEnergy();
  SA.generateNewSolution();
  std::map<int, int> newSolution = *SA.getCurrentSolution();
  for(it=newSolution.begin(); it!=newSolution.end(); ++it){
    std::cout << it->first << " => " << it->second << '\n';
  }
  SA.calcSolutionEnergy();
  //SA.Acceptance();


  // Simulator::Run ();
  // Simulator::Destroy ();
  return 0;
}


