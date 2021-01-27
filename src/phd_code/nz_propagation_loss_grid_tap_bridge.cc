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
#include "ns3/tap-bridge-module.h"

#include <ns3/spectrum-helper.h>
#include <ns3/spectrum-model-ism2400MHz-res1MHz.h>
#include <ns3/spectrum-model-300kHz-300GHz-log.h>
#include <ns3/wifi-spectrum-value-helper.h>
#include <ns3/single-model-spectrum-channel.h>
#include <ns3/propagation-delay-model.h>
#include <ns3/adhoc-aloha-noack-ideal-phy-helper.h>

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

double CalcWatts(double dBmPower)
{
  return pow( 10.0, (dBmPower - 30.0) / 10.0);
}

int main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  GnuplotCollection gnuplots ("nz_propagation_loss.pdf");

  double txPowerDbm = +20; // dBm
  double txPowerW = CalcWatts(txPowerDbm);
  //double distance = 2000.0;
  //int m_nIfaces = 1;
  int m_xSize (2);
  int m_ySize (2);
  double m_step = 20.0;
  Time timeStep = Seconds (0.001);
  Time timeTotal = Seconds (1.0);
  std::string m_stack; ///< stack
  m_stack = "ns3::Dot11sStack";
  const double k = 1.381e-23; //Boltzmann's constant
  const double T = 290; // temperature in Kelvin
  double noisePsdValue = k * T; // watts per hhertz
  uint32_t channelNumber = 1;
  std::string channelType ("ns3::SingleModelSpectrumChannel");

  WifiSpectrumValue5MhzFactory spectrumFactory;
  Ptr<SpectrumValue> noisePsd = spectrumFactory.CreateConstant (noisePsdValue);
  Ptr<SpectrumValue> txPsd =  spectrumFactory.CreateTxPowerSpectralDensity (txPowerW, channelNumber);

  NodeContainer nodes;
  nodes.Create (m_ySize*m_xSize);
  NetDeviceContainer devices;
  Ipv4InterfaceContainer interfaces;
  MeshHelper mesh;

  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper wifiPhy;
  wifiPhy.SetErrorRateModel ("ns3::NistErrorRateModel");
  wifiPhy.SetChannel (wifiChannel.Create ());
  //meshDevices = wifi.Install (wifiPhy, nodes);
  //tr<MatrixPropagationLossModel> propLoss = CreateObject<MatrixPropagationLossModel> ();
  //propLoss->SetLoss (c.Get (0)->GetObject<MobilityModel> (), c.Get (1)->GetObject<MobilityModel> (), lossDb, true);

  Ptr<FriisPropagationLossModel> friis = CreateObject<FriisPropagationLossModel> ();
  // Ptr<LogDistancePropagationLossModel> log = CreateObject<LogDistancePropagationLossModel> ();
  // log->SetAttribute ("Exponent", DoubleValue (2.5));

  Ptr<ConstantPositionMobilityModel> a = CreateObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> b = CreateObject<ConstantPositionMobilityModel> ();

  //channelHelper.AddPropagationLoss (friis);
  //Ptr<SpectrumChannel> channel = channelHelper.Create ();

  // AdhocAlohaNoackIdealPhyHelper phyHelper;
  // phyHelper.SetChannel (channel);
  // phyHelper.SetNoisePowerSpectralDensity(noisePsd);
  // phyHelper.SetTxPowerSpectralDensity(txPsd);
  // NetDeviceContainer devices = phyHelper.Install (nodes);

  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211a);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate54Mbps"));

  //
  // No reason for pesky access points, so we'll use an ad-hoc network.
  //
  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");

  devices = wifi.Install (wifiPhy, wifiMac, nodes);

  // mesh = MeshHelper::Default ();
  // mesh.SetStackInstaller (m_stack);
  // devices = mesh.Install (wifiPhy, nodes);
  // mesh.SetNumberOfInterfaces (m_nIfaces);
  // mesh.SetSpreadInterfaceChannels (MeshHelper::ZERO_CHANNEL);
  //Install protocols and return container if MeshPointDevices

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

  Gnuplot2dDataset dataset;
  dataset.SetTitle (dataTitle);
  dataset.SetStyle (Gnuplot2dDataset::LINES);

 std::ofstream writefile;
 writefile.open("propagation_loss_grid_rxpower.txt");
 writefile << "Position" << std::setw(20) << "RxPower (dBm)" << std::endl;

 TapBridgeHelper tapBridge;
 tapBridge.SetAttribute ("Mode", StringValue ("UseLocal"));
 tapBridge.SetAttribute ("DeviceName", StringValue ("tap-left"));
 tapBridge.Install (nodes.Get (0), devices.Get (0));

 //
 // Connect the right side tap to the right side wifi device on the right-side
 // ghost node.
 //
 tapBridge.SetAttribute ("DeviceName", StringValue ("tap-right"));
 tapBridge.Install (nodes.Get (1), devices.Get (1));


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

          //if (position_a != position_b)
          {
            double rxPowerDbm = friis->CalcRxPower (txPowerDbm, position_a, position_b);
            //Ptr<SpectrumValue> rxPowerPsd = friis->CalcRxPowerSpectralDensity(txPsd, position_a, position_b);
            double snr = rxPowerDbm - noisePsdValue;
            std::cout << "(" << std::setprecision(4) << pos_a.x << ", " << pos_a.y << ") - (" << pos_b.x << "," << pos_b.y << ")," << " " << std::setw(8) << snr << std::endl;
            writefile << "(" << std::setprecision(4) << pos_a.x << ", " << pos_a.y << ") - (" << pos_b.x << "," << pos_b.y << ")," << std::setw(8) << snr << std::endl;
          }
      }

    Simulator::Stop (timeStep);
    Simulator::Run ();
  }

  //Simulator::Schedule (Seconds (m_totalTime), &MeshTest::Report, this);
  //Simulator::Stop (Seconds (m_totalTime));
  //Simulator::Run ();
  // produce clean valgrind
  Simulator::Destroy ();
  return 0;
}
