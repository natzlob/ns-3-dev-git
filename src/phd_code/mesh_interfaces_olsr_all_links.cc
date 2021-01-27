#include <iostream>
#include <sstream>
#include <fstream>
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

NS_LOG_COMPONENT_DEFINE ("TestMeshSpectrumChannelsAllLinksScript");

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
   /// Get current channel number and set to new channel
  void GetSetChannelNumber (uint16_t newChannelNumber, uint8_t serverNode, uint8_t clientNode);
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
private:
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
MeshTest::MeshTest () :
  m_xSize (3),
  m_ySize (3),
  m_step (100.0),
  m_randomStart (0.1),
  m_totalTime (50.0),
  m_packetInterval (0.01),
  m_packetSize (1024),
  m_nIfaces (2),
  m_chan (true),
  m_pcap (false),
  m_ascii (true),
  rss (-50),
  waveformPower (0.1),
  throughput (0),
  totalPacketsThrough (0),
  m_stack ("ns3::Dot11sStack"),
  m_root ("ff:ff:ff:ff:ff:ff")
{
}
void
MeshTest::CreateNodes ()
{ 
  /*
   * Create m_ySize*m_xSize stations to form a grid topology
   */
  nodes.Create (m_ySize*m_xSize);
  interfNode.Create(1);
  // Configure YansWifiChannel
  // YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  spectrumPhy.SetErrorRateModel ("ns3::NistErrorRateModel");
  // wifiChannel = YansWifiChannelHelper::Default ();
  //spectrumChannel = SpectrumChannelHelper::Default ();
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
  mesh.SetStackInstaller (m_stack);
  mesh.SetSpreadInterfaceChannels (MeshHelper::SPREAD_CHANNELS);
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

  AsciiTraceHelper ascii;
  spectrumPhy.EnableAsciiAll (ascii.CreateFileStream ("mesh_minimal.tr"));
}
void
MeshTest::InstallInternetStack ()
{
  OlsrHelper olsr;
  InternetStackHelper internetStack;
  internetStack.SetRoutingHelper (olsr);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  internetStack.Install (nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  interfaces = address.Assign (meshDevices);
}
void
MeshTest::InstallServerApplication ()
{
  UdpServerHelper echoServer (9);
  serverApps = echoServer.Install (nodes);

  NS_LOG_UNCOND("number of server apps in container = " << int(serverApps.GetN()));

  serverApps.Start (Seconds (0.0));
  serverApps.Stop (Seconds (m_totalTime));
}
void
MeshTest::InstallClientApplication (int serverNode, int clientNode)
{
  UdpClientHelper echoClient (interfaces.GetAddress (serverNode), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue ((uint32_t)(m_totalTime*(1/m_packetInterval))));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (m_packetInterval)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (m_packetSize));
  clientApps.Add(echoClient.Install(nodes.Get(clientNode)));

  NS_LOG_UNCOND("number of client apps in container = " << int(clientApps.GetN()));
  
  clientApps.Start (Seconds (0.0));
  clientApps.Stop (Seconds (m_totalTime));
}
void MeshTest::GetSetChannelNumber (uint16_t newChannelNumber, uint8_t serverNode, uint8_t clientNode)
{
  // Get specific server node
  Ptr<NetDevice> dev = meshDevices.Get(serverNode);
  Ptr<MeshPointDevice> mp = DynamicCast<MeshPointDevice>(dev);
  NS_ASSERT (mp != 0);
  // loop over all interfaces
  std::vector<Ptr<NetDevice> > meshInterfaces = mp->GetInterfaces ();
  NS_LOG_UNCOND("number of interfaces per mesh point device = " << meshInterfaces.size());

  Ptr<NetDevice> interface = meshInterfaces[0];
  Ptr<WifiNetDevice> ifdevice = DynamicCast<WifiNetDevice>(interface);
  Ptr<MeshWifiInterfaceMac> ifmac = DynamicCast<MeshWifiInterfaceMac>(ifdevice->GetMac());
  NS_ASSERT (ifmac != 0);
  ifmac->SwitchFrequencyChannel (newChannelNumber);
  NS_LOG_UNCOND ("New channel: " << ifmac->GetFrequencyChannel ());

  dev = meshDevices.Get(clientNode);
  mp = DynamicCast<MeshPointDevice>(dev);
  NS_ASSERT (mp != 0);
  // loop over all interfaces
  meshInterfaces = mp->GetInterfaces ();

  interface = meshInterfaces[1];
  ifdevice = DynamicCast<WifiNetDevice>(interface);
  ifmac = DynamicCast<MeshWifiInterfaceMac>(ifdevice->GetMac());
  NS_ASSERT (ifmac != 0); 
  ifmac->SwitchFrequencyChannel (newChannelNumber);
  NS_LOG_UNCOND ("New channel: " << ifmac->GetFrequencyChannel ());
}
double
MeshTest::CalculateThroughput (int channelNum, int node, std::unordered_map<int, double> &throughputMap)
{
  NS_LOG_UNCOND("total packets through before: " << totalPacketsThrough);
  
  currentTotalPackets = 0;
  int totalPacketsPerNode = DynamicCast<UdpServer> (serverApps.Get (node))->GetReceived ();
  currentTotalPackets += totalPacketsPerNode;
  NS_LOG_UNCOND("currentTotalPackets for node " << int(node) << " = " << totalPacketsPerNode);

  packetsInInterval = currentTotalPackets;
  NS_LOG_UNCOND("packets in the interval " << packetsInInterval);
  throughput = packetsInInterval * m_packetSize * 8 / (m_totalTime * 1000000.0); //Mbit/s
  NS_LOG_UNCOND("\n throughput: " << throughput << "\n");
  channelThroughputMap[channelNum] = throughput;
  return throughput;
}
int
MeshTest::Run ()
{
  g_signalDbmAvg = 0;
  g_noiseDbmAvg = 0;
  g_samples = 0;
  PacketMetadata::Enable ();
  CreateNodes ();
  InstallInternetStack ();
  InstallServerApplication ();
  
  int serverNode = 0;
  int clientNode = 4;
  int channel = 1;
  InstallClientApplication (serverNode, clientNode);
  Simulator::Schedule(Seconds (0), &MeshTest::GetSetChannelNumber, this, channel, serverNode, clientNode);
  Simulator::Schedule(Seconds (m_totalTime), &MeshTest::CalculateThroughput, this, channel, serverNode, channelThroughputMap);

  serverNode = 1;
  clientNode = 8;
  channel = 3;
  InstallClientApplication (serverNode, clientNode);
  Simulator::Schedule(Seconds (0), &MeshTest::GetSetChannelNumber, this, channel, serverNode, clientNode);
  Simulator::Schedule(Seconds (m_totalTime), &MeshTest::CalculateThroughput, this, channel, serverNode, channelThroughputMap);

  serverNode = 2;
  clientNode = 5;
  channel = 5;
  InstallClientApplication (serverNode, clientNode);
  Simulator::Schedule(Seconds (0), &MeshTest::GetSetChannelNumber, this, channel, serverNode, clientNode);
  Simulator::Schedule(Seconds (m_totalTime), &MeshTest::CalculateThroughput, this, channel, serverNode, channelThroughputMap);

  serverNode = 3;
  clientNode = 6;
  channel = 7;
  InstallClientApplication (serverNode, clientNode);
  Simulator::Schedule(Seconds (0), &MeshTest::GetSetChannelNumber, this, channel, serverNode, clientNode);
  Simulator::Schedule(Seconds (m_totalTime), &MeshTest::CalculateThroughput, this, channel, serverNode, channelThroughputMap);

  serverNode = 4;
  clientNode = 3;
  channel = 9;
  InstallClientApplication (serverNode, clientNode);
  Simulator::Schedule(Seconds (0), &MeshTest::GetSetChannelNumber, this, channel, serverNode, clientNode);
  Simulator::Schedule(Seconds (m_totalTime), &MeshTest::CalculateThroughput, this, channel, serverNode, channelThroughputMap);

  serverNode = 5;
  clientNode = 7;
  channel = 11;
  InstallClientApplication (serverNode, clientNode);
  Simulator::Schedule(Seconds (0), &MeshTest::GetSetChannelNumber, this, channel, serverNode, clientNode);
  Simulator::Schedule(Seconds (m_totalTime), &MeshTest::CalculateThroughput, this, channel, serverNode, channelThroughputMap);

  serverNode = 6;
  clientNode = 5;
  channel = 13;
  InstallClientApplication (serverNode, clientNode);
  Simulator::Schedule(Seconds (0), &MeshTest::GetSetChannelNumber, this, channel, serverNode, clientNode);
  Simulator::Schedule(Seconds (m_totalTime), &MeshTest::CalculateThroughput, this, channel, serverNode, channelThroughputMap);

  serverNode = 7;
  clientNode = 2;
  channel = 2;
  InstallClientApplication (serverNode, clientNode);
  Simulator::Schedule(Seconds (0), &MeshTest::GetSetChannelNumber, this, channel, serverNode, clientNode);
  Simulator::Schedule(Seconds (m_totalTime), &MeshTest::CalculateThroughput, this, channel, serverNode, channelThroughputMap);

  serverNode = 8;
  clientNode = 0;
  channel = 4;
  InstallClientApplication (serverNode, clientNode);
  Simulator::Schedule(Seconds (0), &MeshTest::GetSetChannelNumber, this, channel, serverNode, clientNode);
  Simulator::Schedule(Seconds (m_totalTime), &MeshTest::CalculateThroughput, this, channel, serverNode, channelThroughputMap);

  Config::ConnectWithoutContext ("/NodeList/0/DeviceList/*/Phy/MonitorSnifferRx", MakeCallback (&MonitorSniffRx));
  Config::ConnectWithoutContext ("/NodeList/1/DeviceList/*/Phy/MonitorSnifferRx", MakeCallback (&MonitorSniffRx));
  Config::ConnectWithoutContext ("/NodeList/2/DeviceList/*/Phy/MonitorSnifferRx", MakeCallback (&MonitorSniffRx));
  Config::ConnectWithoutContext ("/NodeList/3/DeviceList/*/Phy/MonitorSnifferRx", MakeCallback (&MonitorSniffRx));
  Config::ConnectWithoutContext ("/NodeList/4/DeviceList/*/Phy/MonitorSnifferRx", MakeCallback (&MonitorSniffRx));
  Simulator::Stop (Seconds (m_totalTime));
  Simulator::Run ();

  double current_max = 0.0;
  unsigned int max_channel = 0;
  for (int channel=1; channel<=13; channel++) {
      NS_LOG_UNCOND("channel: " << channel << " , throughput: " << channelThroughputMap[channel]);
      if (channelThroughputMap[channel] > current_max) {
          current_max = channelThroughputMap[channel];
          max_channel = channel;
      }
  }
  NS_LOG_UNCOND ("max throughput: " << current_max << " on channel " << max_channel);
  //NS_LOG_UNCOND ("average signal (dBm) " << g_signalDbmAvg << " average noise (dBm) " << g_noiseDbmAvg);
  Simulator::Destroy ();
  return 0;
}
int
main (int argc, char *argv[])
{
  MeshTest t; 
  return t.Run ();
}