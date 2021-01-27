//this example is just to test received signal strength for specific propagation loss models

#include <iomanip>
#include "ns3/propagation-loss-model.h"
#include "ns3/jakes-propagation-loss-model.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-helper.h"

#include "ns3/config.h"
#include "ns3/command-line.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/pointer.h"
#include "ns3/gnuplot.h"
#include "ns3/simulator.h"

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mesh-module.h"
#include "ns3/mobility-module.h"
#include "ns3/mesh-helper.h"

#include <map>
#include <iostream>
#include <sstream>
#include <fstream>

using namespace ns3;

//Gnuplot CreateFileStream
std::string fileNameWithNoExtension = "nz_propagation";
std::string graphicsFileName        = fileNameWithNoExtension + ".png";
std::string plotFileName            = fileNameWithNoExtension + ".plt";
std::string plotTitle               = "Rx power";
std::string dataTitle               = "Rx power vs transmit power";

int main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  GnuplotCollection gnuplots ("nz_propagation_loss.pdf");

  Ptr<FriisPropagationLossModel> friis = CreateObject<FriisPropagationLossModel> ();

  Ptr<LogDistancePropagationLossModel> logModel = CreateObject<LogDistancePropagationLossModel> ();
  logModel->SetAttribute ("Exponent", DoubleValue (0.5));

  Ptr<ConstantPositionMobilityModel> a = CreateObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> b = CreateObject<ConstantPositionMobilityModel> ();

  double txPowerDbm = +20; // dBm
  //double distance = 2000.0;
  int m_nIfaces = 1;
  int m_xSize (3);
  int m_ySize (3);
  double m_step = 100.0;
  Time timeStep = Seconds (0.001);
  Time timeTotal = Seconds (5.0);
  double m_timeTotal = 5;
  std::string m_stack; ///< stack
  m_stack = "ns3::Dot11sStack";
  double    m_packetInterval = 0.1; ///< packet interval
  uint16_t  m_packetSize = 1024; ///< packet size

  std::cout << std::setw(13) << "a(x,y)-coords " <<
    std::setw(16) << "b(x,y)-coords " <<
    std::setw(16) << "rx power (dBm) " <<
    std::setw(11) << "throughput" << std::endl;

  NodeContainer nodes;
  nodes.Create (m_ySize*m_xSize);
  NetDeviceContainer meshDevices;
  Ipv4InterfaceContainer interfaces;
  MeshHelper mesh;

  YansWifiPhyHelper wifiPhy.SetErrorRateModel ("ns3::NistErrorRateModel");
  YansWifiChannelHelper wifiChannel;
  wifiChannel.AddPropagationLoss("ns3::LogDistancePropagationLossModel",
                                  "Exponent", DoubleValue (2.5));
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiPhy.SetChannel (wifiChannel.Create ());

  mesh = MeshHelper::Default ();
  mesh.SetStackInstaller (m_stack);
  meshDevices = mesh.Install (wifiPhy, nodes);
  mesh.SetNumberOfInterfaces (m_nIfaces);
  mesh.SetSpreadInterfaceChannels (MeshHelper::ZERO_CHANNEL);
  // Install protocols and return container if MeshPointDevices
  meshDevices = mesh.Install (wifiPhy, nodes);

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

  InternetStackHelper internetStack;
  internetStack.Install (nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  interfaces = address.Assign (meshDevices);
  Gnuplot2dDataset dataset;
  dataset.SetTitle (dataTitle);
  dataset.SetStyle (Gnuplot2dDataset::LINES);

  UdpServerHelper echoServer (9);
  
  UdpClientHelper echoClient (interfaces.GetAddress (0), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue ((uint32_t)(m_timeTotal*(1/m_packetInterval))));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (m_packetInterval)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (m_packetSize));

 // {
 //  for (double txPowerDbm = 13.0; txPowerDbm <30; txPowerDbm += 2.0)
 //  //for minx=
 //    {
 //      // CalcRxPower() returns dBm.
 //      double rxPowerDbm = model->CalcRxPower (txPowerDbm, a, b);
 //
 //      dataset.Add (txPowerDbm, rxPowerDbm);
 //
 //      Simulator::Stop (Seconds (1.0));
 //      Simulator::Run ();
 //    }
//}
 a->SetPosition (Vector (0.0, 0.0, 0.0));

 Time start = Simulator::Now ();
 while( Simulator::Now () < start + timeTotal )
  {
  for (NodeContainer::Iterator j = nodes.Begin ();
       j != nodes.End (); ++j)
      for (NodeContainer::Iterator i = nodes.Begin ();
          i != nodes.End (); ++i)
        {
          Ptr<Node> object_a = *j;
          Ptr<MobilityModel> position_a = object_a->GetObject<MobilityModel> ();
          Vector pos_a = position_a->GetPosition ();

          Ptr<Node> object_b = *i;
          Ptr<MobilityModel> position_b = object_b->GetObject<MobilityModel> ();
          Vector pos_b = position_b->GetPosition ();

          if (pos_a != pos_b)
          {
            ApplicationContainer serverApps = echoServer.Install (object_a);
            serverApps.Start (Seconds (0.0));
            serverApps.Stop (timeTotal);
            ApplicationContainer clientApps = echoClient.Install (object_b);
            clientApps.Start (Seconds (0.0));
            clientApps.Stop (timeTotal);
            double throughput = 0;
            uint64_t totalPacketsThrough = 0;
            totalPacketsThrough = DynamicCast<UdpServer> (serverApps.Get (0))->GetReceived ();
            throughput = totalPacketsThrough * m_packetSize * 8 / (m_timeTotal * 1000000.0); //Mbit/
            double rxPowerDbm = logModel->CalcRxPower (txPowerDbm, position_a, position_b);
            std::cout << std::setw (13) << pos_a.x << "," << pos_a.y <<
              std::setw(16) << pos_b.x << "," << pos_b.y <<
              std:: setw(16) << rxPowerDbm  <<
              std::setw(14) << throughput << std::endl;
          }
      }
  }
  Simulator::Stop (Seconds(m_timeTotal+1));
  Simulator::Run ();

  //Simulator::Schedule (Seconds (m_totalTime), &MeshTest::Report, this);
  //Simulator::Stop (Seconds (m_totalTime));
  //Simulator::Run ();
  // produce clean valgrind
  Simulator::Destroy ();
  return 0;
}
