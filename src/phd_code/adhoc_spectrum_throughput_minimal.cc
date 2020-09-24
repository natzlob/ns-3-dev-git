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

NS_LOG_COMPONENT_DEFINE ("TestMeshSpectrumChannelsScript");

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
  NodeContainer nodes;

  NetDeviceContainer meshDevices;
  /// Addresses of interfaces:
  Ipv4InterfaceContainer interfaces;
  /// WifiHelper. Report is not static methods
  WifiHelper wifi;
  /// Channel number
  uint16_t channelNumber;
  ///The Yans wifi channel
  YansWifiChannelHelper wifiChannel;
  SpectrumWifiPhyHelper spectrumPhy;
  ///Spectrum channel helper
  Ptr<MultiModelSpectrumChannel> spectrumChannel;
  ///ApplicationContainer for throughput measurement
  ApplicationContainer serverApps;
private:
  /// Create nodes and setup their mobility
  void CreateNodes ();
  /// Install internet m_stack on nodes
  void InstallInternetStack ();
  /// Install applications
  void InstallApplication ();
  /// Calculate throughput
  double CalculateThroughput ();
  /// Print mesh devices diagnostics
  void Report ();
};
MeshTest::MeshTest () :
  m_xSize (2),
  m_ySize (2),
  m_step (100.0),
  m_randomStart (0.1),
  m_totalTime (50.0),
  m_packetInterval (0.01),
  m_packetSize (1024),
  m_nIfaces (1),
  m_chan (true),
  m_pcap (false),
  m_ascii (true),
  rss (-50),
  waveformPower (0.2),
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
  spectrumPhy = SpectrumWifiPhyHelper::Default ();
  spectrumChannel = CreateObject<MultiModelSpectrumChannel> ();
  Ptr<FriisPropagationLossModel> lossModel
    = CreateObject<FriisPropagationLossModel> ();
  lossModel->SetFrequency (5.180e9);
  spectrumChannel->AddPropagationLossModel (lossModel);

  Ptr<ConstantSpeedPropagationDelayModel> delayModel
    = CreateObject<ConstantSpeedPropagationDelayModel> ();
  spectrumChannel->SetPropagationDelayModel (delayModel);

  spectrumPhy.SetChannel (spectrumChannel);
  spectrumPhy.SetErrorRateModel ("ns3::NistErrorRateModel");
  spectrumPhy.Set ("Frequency", UintegerValue(5180));
  
  wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  wifi.SetRemoteStationManager ("ns3::IdealWifiManager");
  WifiMacHelper mac;
  mac.SetType ("ns3::AdhocWifiMac");
  meshDevices = wifi.Install (spectrumPhy, mac, nodes);

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

  AsciiTraceHelper ascii;
  spectrumPhy.EnableAsciiAll (ascii.CreateFileStream ("ad_hoc_minimal.tr"));
}
void
MeshTest::InstallInternetStack ()
{
  InternetStackHelper internetStack;
  internetStack.Install (nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  interfaces = address.Assign (meshDevices);
}
void
MeshTest::InstallApplication ()
{
  UdpServerHelper echoServer (9);
  serverApps = echoServer.Install (nodes.Get (3));
  //NS_LOG_UNCOND("number of server apps in container = " << int(serverApps.GetN()));
  serverApps.Start (Seconds (0.0));
  serverApps.Stop (Seconds (m_totalTime+1));

  UdpClientHelper echoClient (interfaces.GetAddress (3), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue ((uint32_t)(m_totalTime*(1/m_packetInterval))));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (m_packetInterval)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (m_packetSize));
  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (0.0));
  clientApps.Stop (Seconds (m_totalTime));
}
double
MeshTest::CalculateThroughput ()
{
  NS_LOG_UNCOND("total packets through before: " << totalPacketsThrough);
  double packetsInInterval;
  double currentTotalPackets;
  currentTotalPackets = DynamicCast<UdpServer> (serverApps.Get (0))->GetReceived ();
  NS_LOG_UNCOND("current total packets " << currentTotalPackets);
  packetsInInterval = currentTotalPackets - totalPacketsThrough;
  NS_LOG_UNCOND("packets in the interval " << packetsInInterval);
  totalPacketsThrough = currentTotalPackets;
  //NS_LOG_UNCOND("total packets through after: " << totalPacketsThrough);
  throughput = packetsInInterval * m_packetSize * 8 / (10 * 1000000.0); //Mbit/s
  NS_LOG_UNCOND("\n throughput: " << throughput << "\n");
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
  InstallApplication ();

  Config::ConnectWithoutContext ("/NodeList/0/DeviceList/*/Phy/MonitorSnifferRx", MakeCallback (&MonitorSniffRx));
  Simulator::Stop (Seconds (m_totalTime));
  Simulator::Run ();
  CalculateThroughput();
  NS_LOG_UNCOND ("average signal (dBm) " << g_signalDbmAvg << " average noise (dBm) " << g_noiseDbmAvg);
  Simulator::Destroy ();
  return 0;
}
int
main (int argc, char *argv[])
{
  MeshTest t; 
  return t.Run ();
}