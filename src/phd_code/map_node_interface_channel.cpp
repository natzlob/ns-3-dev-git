
#include <iostream>
#include <map>
#include <string>
#include <ctime>
#include <random>
#include <cstdlib>
#include <algorithm>
#include <functional>

using namespace std;


// uint32_t seedseq_random_using_clock()
// {
//     uint64_t seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
//     std::seed_seq seeder{uint32_t(seed),uint32_t(seed >> 32)};
//     ++seed;
//     int out;
//     seeder.generate(&out,&out+1);
//     return out;
// }
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

std::map<int, int> generateNewSolution(int numLinks, int numChannels, std::map<int, int> solutionMap) {
    // choose one link, assign a new channel to it
    int link = rand() % numLinks;
    int newChannel = rand() % numChannels;
    solutionMap[link] = newChannel;
    return solutionMap;
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

int main (){

    srand(time(NULL));
    int numLinks = 9;
    int maxChannel = 13;
    uint16_t m_xSize = 3;
    uint16_t m_ySize = 3;
    // std::default_random_engine SAgen(seedseq_random_using_clock());

    std::vector<int> nodeNums (m_xSize*m_ySize);
    std::iota(nodeNums.begin(), nodeNums.end(), 0);
    std::vector< std::pair<int, int> > links;
    links = make_unique_pairs(nodeNums);
    std::map<int, int> solution = mapLinkChannel(numLinks, maxChannel);

    std::vector<std::pair<int, int>>::iterator linkIter;
    for(linkIter=links.begin(); linkIter!=links.end(); ++linkIter) {
        cout << linkIter->first <<  "=> " << linkIter->second << '\n';
    }
  
    std::map<int, int>::iterator it;
    for(it=solution.begin(); it!=solution.end(); ++it){
        cout << it->first << " => " << it->second << '\n';
    }

    solution = generateNewSolution(numLinks, maxChannel, solution);

    cout << "\nnew Solution" << endl;
    for(it=solution.begin(); it!=solution.end(); ++it){
        cout << it->first << " => " << it->second << '\n';
    }
   
    return 0;
}