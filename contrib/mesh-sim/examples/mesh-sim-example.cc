/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/mesh-sim.h"

using namespace ns3;


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

std::map<int, int> generateNewSolution(int numLinks, int numChannels, std::map<int, int> solutionMap) {
    // choose one link, assign a new channel to it
    int link = rand() % numLinks;
    int newChannel = rand() % numChannels;
    solutionMap[link] = newChannel;
    return solutionMap;
}


int 
main (int argc, char *argv[])
{
  bool verbose = true;

  CommandLine cmd (__FILE__);
  cmd.AddValue ("verbose", "Tell application to log if true", verbose);

  cmd.Parse (argc,argv);

  std::map<int, int> linkChannelMap;
  std::vector<std::pair<int, int>> links;
  srand(time(NULL));
  int maxChannel = 13;
  uint16_t m_xSize = 3;
  uint16_t m_ySize = 3;
  
  std::vector<int> channels (maxChannel);
  std::iota(channels.begin(), channels.end(), 1);

  std::random_shuffle(std::begin(channels), std::end(channels));

  std::vector<int> nodeNums (m_xSize*m_ySize);
  std::iota(nodeNums.begin(), nodeNums.end(), 0);
  links = make_unique_pairs(nodeNums);
  linkChannelMap = mapLinkChannel(links.size(), channels);
  
  MeshSim t(channels);
  return t.Run (linkChannelMap, links, channels);

}


