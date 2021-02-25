/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008,2009 IITP RAS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Kirill Andreev <andreev@iitp.ru>
 *
 *
 * By default this script creates m_xSize * m_ySize square grid topology with
 * IEEE802.11s stack installed at each node with peering management
 * and HWMP protocol.
 * The side of the square cell is defined by m_step parameter.
 * When topology is created, UDP ping is installed to opposite corners
 * by diagonals. packet size of the UDP ping and interval between two
 * successive packets is configurable.
 * 
 *  m_xSize * step
 *  |<--------->|
 *   step
 *  |<--->|
 *  * --- * --- * <---Ping sink  _
 *  | \   |   / |                ^
 *  |   \ | /   |                |
 *  * --- * --- * m_ySize * step |
 *  |   / | \   |                |
 *  | /   |   \ |                |
 *  * --- * --- *                _
 *  ^ Ping source
 *
 *  See also MeshTest::Configure to read more about configurable
 *  parameters.
 */

#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>
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
#include "ns3/random-variable-stream.h"

using namespace ns3;

// Global variables for use in callbacks.
double g_signalDbmAvg;
double g_noiseDbmAvg;
uint32_t g_samples;

void MonitorSniffRx (Ptr<const Packet> packet,
                     uint16_t channelFreqMhz,
                     WifiTxVector txVector,
                     MpduInfo aMpdu,
                     SignalNoiseDbm signalNoise)

{
  g_samples++;
  g_signalDbmAvg += ((signalNoise.signal - g_signalDbmAvg) / g_samples);
  g_noiseDbmAvg += ((signalNoise.noise - g_noiseDbmAvg) / g_samples);
}

std::unordered_map<int, double> channelGainMap = {
  {1, 10}, {2, 8}, {3, 6}, {4, 4}, {5, 2}
};

std::unordered_map<int, double> channelThroughputMap = {
  {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7,0}, {8, 0}, {9, 0}, {10, 0}, {11, 0}, {12, 0}, {13, 0}
};

NS_LOG_COMPONENT_DEFINE ("TestMeshSpectrumChannelsScript");

Ptr<SpectrumModel> SpectrumModel2417MHz;

class static_SpectrumModel2417MHz_initializer
{
public:
    static_SpectrumModel2417MHz_initializer ()
    {
        BandInfo bandInfo;
        bandInfo.fc = 2417e6;
        bandInfo.fl = 2417e6 - 10e6;
        bandInfo.fh = 2417e6 + 10e6;
        Bands bands;
        bands.push_back (bandInfo);

        SpectrumModel2417MHz = Create<SpectrumModel> (bands);
    }
} static_SpectrumModel2417MHz_initializer_inst;

/**
 * \ingroup mesh
 * \brief MeshTest class
 */
class MeshTest
{
public:
  /// Init test
  MeshTest ();
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
  int Run ();
  ///Get Phy pointer
  Ptr<WifiPhy> GetPhy (int i);
  /// Map channel number to PropagationLossModel
  void MapChanneltoLoss (YansWifiChannelHelper wifiChannel, uint16_t channelNumber);
  /// Get current channel numbers and map to loss
  void GetSetChannelNumber (uint16_t newChannelNumber);
  /// Get the current channel number and set to new channel number
private:
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
  /// List of network nodes
  double packetsInInterval=0;
  double currentTotalPackets=0;

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
  /// Helper for waveform generator
  WaveformGeneratorHelper waveformGeneratorHelper;
  /// container for waveform generator devices
  NetDeviceContainer waveformGeneratorDevices;
private:
  /// Setup channels with different propagation and delay models
  void PrepareChannels ();
  /// Create nodes and setup their mobility
  void CreateNodes ();
  /// Install internet m_stack on nodes
  void InstallInternetStack ();
  /// Install applications
  void ConfigureWaveform ();
  /// Configure waveform generator for interfering node
  void InstallApplication ();
  /// Calculate throughput
  void CalculateThroughput (int channelNum, std::unordered_map<int, double> &throughputMap);
  /// Print mesh devices diagnostics
  void Report ();
};
MeshTest::MeshTest () :
  m_xSize (3),
  m_ySize (3),
  m_step (150.0),
  m_randomStart (0.1),
  m_totalTime (20.0),
  m_packetInterval (0.001),
  m_packetSize (1024),
  m_nIfaces (2),
  m_chan (true),
  m_pcap (false),
  m_ascii (true),
  rss (-50),
  waveformPower (0.0),
  throughput (0),
  totalPacketsThrough (0),
  m_stack ("ns3::Dot11sStack"),
  m_root ("ff:ff:ff:ff:ff:ff")
{
}
void
MeshTest::Configure (int argc, char *argv[])
{
  CommandLine cmd (__FILE__);
  cmd.AddValue ("x-size", "Number of nodes in a row grid", m_xSize);
  cmd.AddValue ("y-size", "Number of rows in a grid", m_ySize);
  cmd.AddValue ("step",   "Size of edge in our grid (meters)", m_step);
  // Avoid starting all mesh nodes at the same time (beacons may collide)
  cmd.AddValue ("start",  "Maximum random start delay for beacon jitter (sec)", m_randomStart);
  cmd.AddValue ("time",  "Simulation time (sec)", m_totalTime);
  cmd.AddValue ("packet-interval",  "Interval between packets in UDP ping (sec)", m_packetInterval);
  cmd.AddValue ("packet-size",  "Size of packets in UDP ping (bytes)", m_packetSize);
  cmd.AddValue ("interfaces", "Number of radio interfaces used by each mesh point", m_nIfaces);
  cmd.AddValue ("channels",   "Use different frequency channels for different interfaces", m_chan);
  cmd.AddValue ("pcap",   "Enable PCAP traces on interfaces", m_pcap);
  cmd.AddValue ("ascii",   "Enable Ascii traces on interfaces", m_ascii);
  cmd.AddValue ("stack",  "Type of protocol stack. ns3::Dot11sStack by default", m_stack);
  cmd.AddValue ("root", "Mac address of root mesh point in HWMP", m_root);

  cmd.Parse (argc, argv);
  NS_LOG_DEBUG ("Grid:" << m_xSize << "*" << m_ySize);
  NS_LOG_DEBUG ("Simulation time: " << m_totalTime << " s");
  if (m_ascii)
    {
      PacketMetadata::Enable ();
    }
}
void MeshTest::PrepareChannels ()
{
  spectrumPhy = SpectrumWifiPhyHelper::Default ();
  spectrumChannel = CreateObject<MultiModelSpectrumChannel> ();

  Ptr<FriisPropagationLossModel> friisModel
    = CreateObject<FriisPropagationLossModel> ();
  friisModel->SetFrequency (2.417e9);

  Ptr<Cost231PropagationLossModel> costModel
    = CreateObject<Cost231PropagationLossModel> ();
  costModel->SetAttribute ("Frequency", DoubleValue(2.417e9));

  Ptr<FixedRssLossModel> fixedLoss1
    =  CreateObject<FixedRssLossModel> ();
  fixedLoss1->SetRss (-70);

  Ptr<FixedRssLossModel> fixedLoss2
    =  CreateObject<FixedRssLossModel> ();
  fixedLoss2->SetRss (-60);

  spectrumChannel->AddPropagationLossModel (friisModel);

  Ptr<ConstantSpeedPropagationDelayModel> delayModel
    = CreateObject<ConstantSpeedPropagationDelayModel> ();
  spectrumChannel->SetPropagationDelayModel (delayModel);
}
void
MeshTest::CreateNodes ()
{ 
  /*
   * Create m_ySize*m_xSize stations to form a grid topology
   */
  nodes.Create (m_ySize*m_xSize);
  interfNode.Create(1);
  spectrumPhy = SpectrumWifiPhyHelper::Default ();
  spectrumChannel = CreateObject<MultiModelSpectrumChannel> ();
  Ptr<FriisPropagationLossModel> lossModel
    = CreateObject<FriisPropagationLossModel> ();
  lossModel->SetFrequency (2.417e9);
  spectrumChannel->AddPropagationLossModel (lossModel);

  Ptr<ConstantSpeedPropagationDelayModel> delayModel
    = CreateObject<ConstantSpeedPropagationDelayModel> ();
  spectrumChannel->SetPropagationDelayModel (delayModel);

  spectrumPhy.SetChannel (spectrumChannel);
  spectrumPhy.SetErrorRateModel ("ns3::NistErrorRateModel");
  spectrumPhy.Set ("Frequency", UintegerValue(2417));
  /*
   * Create mesh helper and set stack installer to it
   * Stack installer creates all needed protocols and install them to
   * mesh point device
   */
  mesh = MeshHelper::Default ();
  if (!Mac48Address (m_root.c_str ()).IsBroadcast ())
    {
      mesh.SetStackInstaller (m_stack, "Root", Mac48AddressValue (Mac48Address (m_root.c_str ())));
    }
  else
    {
      //If root is not set, we do not use "Root" attribute, because it
      //is specified only for 11s
      mesh.SetStackInstaller (m_stack);
    }
  if (m_chan)
    {
      mesh.SetSpreadInterfaceChannels (MeshHelper::SPREAD_CHANNELS);
    }
  else
    {
      mesh.SetSpreadInterfaceChannels (MeshHelper::ZERO_CHANNEL);
    }
  mesh.SetMacType ("RandomStart", TimeValue (Seconds (m_randomStart)));
  // Set number of interfaces - default is single-interface mesh point
  mesh.SetNumberOfInterfaces (m_nIfaces);
  // Install protocols and return container if MeshPointDevices
  // meshDevices = mesh.Install (wifiPhy, nodes);
  meshDevices = mesh.Install (spectrumPhy, nodes);
  // Setup mobility - static grid topology
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (m_step),
                                 "DeltaY", DoubleValue (m_step),
                                 "GridWidth", UintegerValue (m_xSize),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);

  MobilityHelper interfMobility;
  Ptr<ListPositionAllocator> posAlloc = CreateObject<ListPositionAllocator> ();
  posAlloc->Add (Vector (m_step, m_step, 0.0));
  interfMobility.SetPositionAllocator (posAlloc);
  interfMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  interfMobility.Install (interfNode);

  if (m_pcap)
    // wifiPhy.EnablePcapAll (std::string ("mp-"));
    spectrumPhy.EnablePcapAll (std::string ("mp-"));
  if (m_ascii)
    {
      AsciiTraceHelper ascii;
      // wifiPhy.EnableAsciiAll (ascii.CreateFileStream ("mesh.tr"));
      spectrumPhy.EnableAsciiAll (ascii.CreateFileStream ("mesh.tr"));
    }
}

Ptr<WifiPhy> MeshTest::GetPhy (int i)
{
    Ptr<NetDevice> netdev = meshDevices.Get(i);
    if (netdev != 0){
      NS_LOG_UNCOND("netdev is not null");
    }
    else {
      NS_LOG_ERROR("netdev is null pointer");
    }
    Ptr<WifiNetDevice> wifinetdev = DynamicCast<WifiNetDevice> (netdev);
    if (wifinetdev!=0){
      NS_LOG_UNCOND("wifinetdev not a null pointer");
    }
    else {
      NS_LOG_ERROR("wifinetdev is a null pointer");
    }
    Ptr<WifiPhy> wifiPhyPtr = wifinetdev->GetPhy ();
    return wifiPhyPtr;

}

void MeshTest::GetSetChannelNumber (uint16_t newChannelNumber)
{
  // loop over all mesh points
  for (NetDeviceContainer::Iterator i = meshDevices.Begin(); i !=meshDevices.End(); ++i)
    {
        Ptr<MeshPointDevice> mp = DynamicCast<MeshPointDevice>(*i);
        NS_ASSERT (mp != 0);
        // loop over all interfaces
        std::vector<Ptr<NetDevice> > meshInterfaces = mp->GetInterfaces ();

        for (std::vector<Ptr<NetDevice> >::iterator j = meshInterfaces.begin(); j != meshInterfaces.end(); ++j)
        {
            Ptr<WifiNetDevice> ifdevice = DynamicCast<WifiNetDevice>(*j);
            //access the WifiPhy ptr
            Ptr<WifiPhy> wifiPhyPtr = ifdevice->GetPhy ();
            NS_ASSERT(wifiPhyPtr != 0);
            // access MAC
            //NS_LOG_UNCOND("the configured channel number in phy is " << int(wifiPhyPtr->GetChannelNumber()));
            Ptr<MeshWifiInterfaceMac> ifmac = DynamicCast<MeshWifiInterfaceMac>(ifdevice->GetMac());
            NS_ASSERT (ifmac != 0);
            // Access channel number)

            //NS_LOG_UNCOND ("Old channel: " << ifmac->GetFrequencyChannel ());
            // Change channel 
            ifmac->SwitchFrequencyChannel (newChannelNumber);
            NS_LOG_UNCOND ("New channel: " << ifmac->GetFrequencyChannel ());
            wifiPhyPtr = ifdevice->GetPhy ();
            NS_ASSERT(wifiPhyPtr != 0);
            // access MAC
            NS_LOG_UNCOND("the new configured channel number in phy is " << int(wifiPhyPtr->GetChannelNumber()));
        }
    }
  NS_LOG_UNCOND ("average signal (dBm) " << g_signalDbmAvg << " average noise (dBm) " << g_noiseDbmAvg);
}
void
MeshTest::InstallInternetStack ()
{
  InternetStackHelper internetStack;
  internetStack.Install (nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  interfaces = address.Assign (meshDevices);
  NS_LOG_UNCOND("number of IPv4 interfaces in container: " << interfaces.GetN());
}
void
MeshTest::ConfigureWaveform ()
{
  Ptr<SpectrumValue> wgPsd = Create<SpectrumValue> (SpectrumModel2417MHz);
  *wgPsd = waveformPower / 20e6;
  waveformGeneratorHelper.SetChannel (spectrumChannel);
  waveformGeneratorHelper.SetTxPowerSpectralDensity (wgPsd);
  waveformGeneratorHelper.SetPhyAttribute ("Period", TimeValue (Seconds (0.0007)));
  waveformGeneratorHelper.SetPhyAttribute ("DutyCycle", DoubleValue (1));
  waveformGeneratorDevices = waveformGeneratorHelper.Install (interfNode);
  NS_LOG_UNCOND("configuring waveform\n");
}
void
MeshTest::InstallApplication ()
{
  UdpServerHelper echoServer (9);

  //for (int i=0; i<m_xSize*m_ySize; i++) {
  // serverApps.Add(echoServer.Install (nodes));
    //echoClient (interfaces.GetAddress (i), 9);
  //}
  //NS_LOG_UNCOND("number of server apps in container = " << int(serverApps.GetN()));
  //serverApps = echoServer.Install (nodes.Get (server));
  serverApps = echoServer.Install (nodes.Get (0));
  serverApps.Start (Seconds (0.0));                                          
  serverApps.Stop (Seconds (m_totalTime));
  
  UdpClientHelper echoClient (interfaces.GetAddress (0), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue ((uint32_t)(m_totalTime*(1/m_packetInterval))));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (m_packetInterval)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (m_packetSize));
  ApplicationContainer clientApps;
  // clientApps.Add(echoClient.Install (nodes));
  clientApps = echoClient.Install (nodes.Get (1));
  //NS_LOG_UNCOND("number of client apps in container = " << int(clientApps.GetN()));
  //clientApps = echoClient.Install (nodes.Get (client));
  clientApps.Start (Seconds (0.0));
  clientApps.Stop (Seconds (m_totalTime));
}
void
MeshTest::CalculateThroughput (int channelNum, std::unordered_map<int, double> &throughputMap)
{
  //NS_LOG_UNCOND("total packets through before: " << totalPacketsThrough);

  // for (int serverapp=0; serverapp < m_ySize*m_xSize; serverapp++) {
  //   int totalperServer = DynamicCast<UdpServer> (serverApps.Get (serverapp))->GetReceived ();
  //   NS_LOG_UNCOND("total per server app " << serverapp << " = " << totalperServer);
  //   currentTotalPackets += DynamicCast<UdpServer> (serverApps.Get (serverapp))->GetReceived ();
  //   NS_LOG_UNCOND("current total packets " << currentTotalPackets);
  // }
  // packetsInInterval = currentTotalPackets - totalPacketsThrough;
  // totalPacketsThrough = currentTotalPackets;
  // NS_LOG_UNCOND("packets in the interval " << packetsInInterval);
  // throughput = packetsInInterval * m_packetSize * 8 / (10 * 1000000.0); //10s interval, Mbit/s
  // channelThroughputMap[channelNum] = throughput;
  // NS_LOG_UNCOND("\n throughput: " << channelThroughputMap[channelNum] << "\n");
  // NS_LOG_UNCOND ("average signal (dBm) " << g_signalDbmAvg << " average noise (dBm) " << g_noiseDbmAvg);
  currentTotalPackets = DynamicCast<UdpServer> (serverApps.Get (0))->GetReceived ();
  NS_LOG_UNCOND("current total packets " << currentTotalPackets);
  packetsInInterval = currentTotalPackets - totalPacketsThrough;
  NS_LOG_UNCOND("packets in the interval " << packetsInInterval);
  totalPacketsThrough = currentTotalPackets;
  
  //NS_LOG_UNCOND("total packets through after: " << totalPacketsThrough);
  
  throughput = packetsInInterval * m_packetSize * 8 / (10 * 1000000.0); //10s interval, Mbit/s
  channelThroughputMap[channelNum] = throughput;
  NS_LOG_UNCOND("\n throughput: " << channelThroughputMap[channelNum] << "\n");
  // NS_LOG_UNCOND ("average signal (dBm) " << g_signalDbmAvg << " average noise (dBm) " << g_noiseDbmAvg);
}
int
MeshTest::Run ()
{
  g_signalDbmAvg = 0;
  g_noiseDbmAvg = 0;
  g_samples = 0;
  CreateNodes ();
  InstallInternetStack ();
  ConfigureWaveform();
  Simulator::Schedule (Seconds (0), &WaveformGenerator::Start,
    waveformGeneratorDevices.Get (0)->GetObject<NonCommunicatingNetDevice> ()->GetPhy ()->GetObject<WaveformGenerator> ());

  for (int channel=1; channel<=13; channel++) {
      Simulator::Schedule(Seconds (10*channel-10), &MeshTest::GetSetChannelNumber, this, channel);
      Simulator::Schedule(Seconds (10*channel), &MeshTest::CalculateThroughput, this, channel, channelThroughputMap);
  }

  InstallApplication ();
  Simulator::Schedule (Seconds (m_totalTime), &MeshTest::Report, this);
  Config::ConnectWithoutContext ("/NodeList/0/DeviceList/*/Phy/MonitorSnifferRx", MakeCallback (&MonitorSniffRx));
  Simulator::Stop (Seconds (m_totalTime));
  Simulator::Run ();
  double current_max = 0.0;
  unsigned int max_channel = 0;
  for (int channel=1; channel<=13; channel++) {
      NS_LOG_UNCOND("channel: " << channel << " , throughput: " << channelThroughputMap[channel]);
      std::cout << channel << " , " << throughput << "\n";
      if (channelThroughputMap[channel] > current_max) {
          current_max = channelThroughputMap[channel];
          max_channel = channel;
      }
  }
  NS_LOG_UNCOND ("max throughput: " << current_max << " on channel " << max_channel);
  Simulator::Destroy ();
  return 0;
}
void
MeshTest::Report ()
{
  unsigned n (0);
  for (NetDeviceContainer::Iterator i = meshDevices.Begin (); i != meshDevices.End (); ++i, ++n)
    {
      std::ostringstream os;
      os << "mp-report-" << n << ".xml";
      //std::cerr << "Printing mesh point device #" << n << " diagnostics to " << os.str () << "\n";
      std::ofstream of;
      of.open (os.str ().c_str ());
      if (!of.is_open ())
        {
          std::cerr << "Error: Can't open file " << os.str () << "\n";
          return;
        }
      mesh.Report (*i, of);
      of.close ();
    }
}
int
main (int argc, char *argv[])
{
  MeshTest t; 
  t.Configure (argc, argv);
  return t.Run ();
}
