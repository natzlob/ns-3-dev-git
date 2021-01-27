//this example is just to test received signal strength for specific propagation loss models

#include "ns3/propagation-loss-model.h"
#include "ns3/jakes-propagation-loss-model.h"
#include "ns3/constant-position-mobility-model.h"

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
#include <iomanip>

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

  Ptr<LogDistancePropagationLossModel> log = CreateObject<LogDistancePropagationLossModel> ();
  log->SetAttribute ("Exponent", DoubleValue (2.5));

  Ptr<ConstantPositionMobilityModel> a = CreateObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> b = CreateObject<ConstantPositionMobilityModel> ();

  double txPowerDbm = +20; // dBm
  //double distance = 2000.0;
  int m_nIfaces = 1;
  int m_xSize (3);
  int m_ySize (3);
  //double m_step = 50.0;
  Time timeStep = Seconds (0.001);
  Time timeTotal = Seconds (1.0);
  std::string m_stack; ///< stack
  m_stack = "ns3::Dot11sStack";

  NodeContainer nodes;
  nodes.Create (m_ySize*m_xSize);
  NetDeviceContainer meshDevices;
  Ipv4InterfaceContainer interfaces;
  MeshHelper mesh;

  YansWifiPhyHelper wifiPhy;
  wifiPhy.SetErrorRateModel ("ns3::NistErrorRateModel");
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());


  mesh = MeshHelper::Default ();
  mesh.SetStackInstaller (m_stack);
  meshDevices = mesh.Install (wifiPhy, nodes);
  mesh.SetNumberOfInterfaces (m_nIfaces);
  mesh.SetSpreadInterfaceChannels (MeshHelper::ZERO_CHANNEL);
  // Install protocols and return container if MeshPointDevices
  meshDevices = mesh.Install (wifiPhy, nodes);

  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
                                 "rho", DoubleValue (9.0),
                                 "X", DoubleValue (0.0),
                                 "Y", DoubleValue (0.0));

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);

  Gnuplot2dDataset dataset;
  dataset.SetTitle (dataTitle);
  dataset.SetStyle (Gnuplot2dDataset::LINES);

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

 std::ofstream writefile;
 writefile.open("propagation_loss_uniform_disc_rxpower.txt");
 writefile << "Position" << std::setw(20)<< "RxPower (dBm)" << std::endl;
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

          if ((pos_a.x != pos_b.x) && (pos_a.y != pos_b.y))
          {
            double rxPowerDbm = friis->CalcRxPower (txPowerDbm, position_a, position_b);
            writefile << "(" << std::setprecision(2) << pos_a.x << ", " << pos_a.y << ") - (" << pos_b.x << "," << pos_b.y << ")," << " " << std::setw(10) << rxPowerDbm << std::endl;
          }
      }
    Simulator::Stop (timeStep);
    Simulator::Run ();
  }

  writefile.close();
  //Simulator::Schedule (Seconds (m_totalTime), &MeshTest::Report, this);
  //Simulator::Stop (Seconds (m_totalTime));
  //Simulator::Run ();
  // produce clean valgrind
  Simulator::Destroy ();
  return 0;
}
