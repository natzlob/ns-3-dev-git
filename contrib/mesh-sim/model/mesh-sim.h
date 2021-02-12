/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MESH_SIM_H
#define MESH_SIM_H

#include <sstream>
#include <fstream>
#include <vector>
#include <functional>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <numeric>
#include <string>

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mesh-module.h"
#include "ns3/mobility-module.h"
#include "ns3/mesh-helper.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/wifi-module.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/spectrum-helper.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/cost231-propagation-loss-model.h"
#include "ns3/waveform-generator.h"
#include "ns3/waveform-generator-helper.h"
#include "ns3/non-communicating-net-device.h"
#include "ns3/olsr-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/snr-tag.h"
#include "ns3/trace-helper.h"


// Global variables for use in callbacks.
double g_signalDbmAvg;
double g_noiseDbmAvg;
uint32_t g_samples;

void MonitorSniffRx (ns3::Ptr<ns3::OutputStreamWrapper> stream,
                     std::string node_num,
                     ns3::Ptr<const ns3::Packet> packet,
                     uint16_t channelFreqMhz,
                     ns3::WifiTxVector txVector,
                     ns3::MpduInfo aMpdu,
                     ns3::SignalNoiseDbm signalNoise)

{
  *stream->GetStream () << node_num << ", " << signalNoise.signal - signalNoise.noise << "\n";
}


namespace ns3 {

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

// std::unordered_map<int, double> channelGainMap = {
//   {1, 10}, {2, 8}, {3, 6}, {4, 4}, {5, 2}
// };

Ptr<SpectrumModel> SpectrumModel5180MHz;

class static_SpectrumModel5180MHz_initializer
{
public:
    static_SpectrumModel5180MHz_initializer ()
    {
        BandInfo bandInfo;
        bandInfo.fc = 5180e6;
        bandInfo.fl = 5180e6 - 10e6;
        bandInfo.fh = 5180e6 + 10e6;
        Bands bands;
        bands.push_back (bandInfo);

        SpectrumModel5180MHz = Create<SpectrumModel> (bands);
    }
} static_SpectrumModel5180MHz_initializer_inst;


class MeshSim : public Object
{
public:
  /// Init test
  MeshSim ();
  /**
   * Configure test from command line arguments
   *
   * \param argc command line argument count
   * \param argv command line arguments
   */
  void Configure (int argc, char ** argv);
  /**
   * Run test
   * \returns the test status
   */
  int Run (std::map<int, int>& linkChannelMap, std::vector<std::pair<int, int>>& links);
   /// Get current channel number and set to new channel
  void GetSetChannelNumber (uint16_t newChannelNumber, uint8_t serverNode, uint8_t clientNode);
private:
  std::unordered_map<int, double> channelThroughputMap;
  int       m_xSize; ///< X size
  int       m_ySize; ///< Y size
  double    m_step; ///< step
  double    m_randomStart; ///< random start
  double    m_totalTime; ///< total time
  double    m_packetInterval; ///< packet interval
  uint16_t  m_packetSize; ///< packet size
  uint32_t  m_nIfaces; ///< number interfaces
  bool      m_chan; ///< channel
  bool      m_pcap; ///< PCAP
  bool      m_ascii; ///< ASCII
  double    rss;
  double    waveformPower;
  double throughput;
  uint64_t totalPacketsThrough;
  std::string m_stack; ///< stack
  std::string m_root; ///< root
  double packetsInInterval=0;
  double currentTotalPackets=0;
  /// List of network nodes
  NodeContainer nodes;
  /// List of all mesh point devices
  NodeContainer interfNode;
  /// Interfering node
  NetDeviceContainer meshDevices;
  /// Addresses of interfaces:
  Ipv4InterfaceContainer interfaces;
  /// MeshHelper. Report is not static methods
  MeshHelper mesh;
  /// Channel number
  uint16_t channelNumber;
  ///The Yans wifi channel
  YansWifiChannelHelper wifiChannel;
  SpectrumWifiPhyHelper spectrumPhy;
  ///Spectrum channel helper
  Ptr<MultiModelSpectrumChannel> spectrumChannel;
  ///ApplicationContainer for throughput measurement
  ApplicationContainer serverApps;
  ApplicationContainer clientApps;
  /// Helper for waveform generator
  WaveformGeneratorHelper waveformGeneratorHelper;
  /// container for waveform generator devices
  NetDeviceContainer waveformGeneratorDevices;

  /// Create nodes and setup their mobility
  void CreateNodes ();
  /// Install internet m_stack on nodes
  void InstallInternetStack ();
  /// Install applications
  void InstallServerApplication ();
  void InstallClientApplication (int serverNode, int clientNode);
  /// Configure waveform generator for interfering node
  void ConfigureWaveform ();
  /// Calculate throughput
  double CalculateThroughput (int channelNum, int node, std::unordered_map<int, double> &throughputMap);
  /// Print mesh devices diagnostics
  void Report ();
};

}

#endif /* MESH_SIM_H */

