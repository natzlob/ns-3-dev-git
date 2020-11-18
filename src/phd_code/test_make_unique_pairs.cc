#include <set>
#include <vector>
#include <functional>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <numeric>
#include "ns3/random-variable-stream.h"

using namespace ns3;

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
  std::srand(std::time(nullptr));

  std::vector<int> numbers(9);
  std::iota(numbers.begin(), numbers.end(), 0);
  //std::vector<int> numbers1(numbers);
  //numbers.insert( numbers.end(), numbers1.begin(), numbers1.end() );
  //const std::vector<int> values { 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8} ;

  for (auto pair : make_unique_pairs(numbers)) {
    std::cout<< pair.first << ", " << pair.second << std::endl;
  }

  return 0;
}