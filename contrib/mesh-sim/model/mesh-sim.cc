/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "mesh-sim.h"

namespace ns3 {

MeshSim::MeshSim ()
{
  m_xSize = 3;
  m_ySize = 3;
  m_step = 100.0;
  m_randomStart = 0.1;
  m_totalTime = 50.0;
  m_packetInterval = 0.01;
  m_packetSize = 1024;
  m_nIfaces = 2;
  m_chan = true;
  m_pcap = false;
  m_ascii = true;
  rss = -50;
  waveformPower = 0.1;
  throughput = 0;
  totalPacketsThrough = 0;
  m_stack = "ns3::Dot11sStack";
  m_root = "ff:ff:ff:ff:ff:ff";
}

void
MeshSim::CreateNodes ()
{ 
  /*
   * Create m_ySize*m_xSize stations to form a grid topology
   */
  nodes.Create (m_ySize*m_xSize);
  interfNode.Create(1);
  // Configure YansWifiChannel
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
  spectrumPhy.Set ("ChannelWidth", UintegerValue (20));
  spectrumPhy.Set ("TxPowerStart", DoubleValue (10));
  spectrumPhy.Set ("TxPowerEnd", DoubleValue (10));
  /*
   * Create mesh helper and set stack installer to it
   * Stack installer creates all needed protocols and install them to
   * mesh point device
   */
  mesh = MeshHelper::Default ();
  mesh.SetStackInstaller (m_stack);
  mesh.SetSpreadInterfaceChannels (MeshHelper::SPREAD_CHANNELS);
  mesh.SetMacType ("RandomStart", TimeValue (Seconds (m_randomStart)));
  mesh.SetStandard (WIFI_STANDARD_80211g); //because channels 1-14 are defined
  // Set number of interfaces - default is single-interface mesh point
  mesh.SetNumberOfInterfaces (m_nIfaces);
  // Install protocols and return container if MeshPointDevices
  // meshDevices = mesh.Install (wifiPhy, nodes);
  // AsciiTraceHelper ascii;
  // mesh.EnableAsciiAll (ascii.CreateFileStream("mesh_interfaces_olsr.tr"));
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
  spectrumPhy.EnableAsciiAll (ascii.CreateFileStream ("mesh_olsr.tr"));
}
void
MeshSim::InstallInternetStack ()
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
MeshSim::InstallServerApplication ()
{
  UdpServerHelper echoServer (9);
  serverApps = echoServer.Install (nodes);

  NS_LOG_UNCOND("number of server apps in container = " << int(serverApps.GetN()));

  serverApps.Start (Seconds (0.0));
  serverApps.Stop (Seconds (m_totalTime));
}
void
MeshSim::InstallClientApplication (int serverNode, int clientNode)
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
void MeshSim::GetSetChannelNumber (uint16_t newChannelNumber, uint8_t serverNode, uint8_t clientNode)
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
MeshSim::CalculateThroughput (int channelNum, int node, std::unordered_map<int, double> &throughputMap)
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

  //Config::ConnectWithoutContext ("/NodeList/" + std::to_string(node) + "/DeviceList/0/Phy/MonitorSnifferRx", MakeCallback (&MonitorSniffRx));

  return throughput;
}
int
MeshSim::Run (std::map<int, int> linkChannelMap, std::vector<std::pair<int, int>> links)
{
  g_signalDbmAvg = 0;
  g_noiseDbmAvg = 0;
  g_samples = 0;
  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream("SNRtrace_3.tr");
  Ptr<OutputStreamWrapper> stream2 = asciiTraceHelper.CreateFileStream("Channel-throughput_without_interference.txt");

  PacketMetadata::Enable ();
  CreateNodes ();
  InstallInternetStack ();
  InstallServerApplication ();

  std::srand(std::time(nullptr));
  std::vector<int> values (m_xSize*m_ySize);
  std::iota(values.begin(), values.end(), 0);

  std::vector<int> channels (13);
  std::iota(channels.begin(), channels.end(), 1);
  //std::random_shuffle(std::begin(channels), std::end(channels));
  for (uint8_t i=0; i<channels.size(); i++)
  {
    std::cout << "channel number: " << channels[i] << std::endl;
  }

  int serverNode;
  int clientNode;
  uint8_t channel=0;

  std::vector<std::pair<int, int>>::iterator linkIter;
  std::cout << "links: \n";
  int linkIndex = 0;
  for(linkIter=links.begin(); linkIter!=links.end(); ++linkIter) {
    std::cout << linkIter->first <<  "=> " << linkIter->second << '\n';
    serverNode = linkIter->first;
    clientNode = linkIter->second;
    channel = linkChannelMap[linkIndex];
    linkIndex++;
    InstallClientApplication (serverNode, clientNode);
    Simulator::Schedule(Seconds (0), &MeshSim::GetSetChannelNumber, this, channel, serverNode, clientNode);
    Simulator::Schedule(Seconds (m_totalTime), &MeshSim::CalculateThroughput, this, channel, serverNode, channelThroughputMap);
  }

  Simulator::Stop (Seconds (m_totalTime));

  for (uint8_t node_num=0; node_num<m_xSize*m_ySize; node_num++) {
    for (uint8_t interf=0; interf<m_nIfaces; interf++){
      Config::ConnectWithoutContext (
        "/NodeList/" + std::to_string(node_num) + "/DeviceList/" + std::to_string(interf) + "/Phy/MonitorSnifferRx",
        MakeBoundCallback (&MonitorSniffRx, stream, std::to_string(node_num)+std::to_string(interf))
      );
    }
  }
  Simulator::Run ();

  *stream2->GetStream() << "channel , throughput \n";
  double current_max = 0.0;
  unsigned int max_channel = 0;
  for (int channel=1; channel<=13; channel++) {
      NS_LOG_UNCOND("channel: " << channel << " , throughput: " << channelThroughputMap[channel]);
      *stream2->GetStream() << channel << " , " << throughput << "\n";
      if (channelThroughputMap[channel] > current_max) {
          current_max = channelThroughputMap[channel];
          max_channel = channel;
      }
  }
  NS_LOG_UNCOND ("max throughput: " << current_max << " on channel " << max_channel);
  Simulator::Destroy ();
  return 0;
}

// ---------------------------------------------------------------------------------------------------------------------------
} //namespace ns3

