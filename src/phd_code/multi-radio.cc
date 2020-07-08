/* multi-radio wireless node */

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
#include "wifi-net-device.h"
#include "ns3/wifi-helper"

using namespace ns3;

int main (int argc, char** argv)
{
 std::string standard = "11a";
 int bw = 20;
 double pow = 23; //dBm

 Ptr<Node> MRnode = CreateObject<Node> ();
 Ptr<WifiNetDevice> dev0 = CreateObject<WifiNetDevice> ();
 Ptr<WifiNetDevice> dev1 = CreateObject<WifiNetDevice> ();
 MRNode->AddDevice (dev0);
 MRNode->AddDevice (dev1);


 YansWifiChannelHelper channel0 = YansWifiChannelHelper::Default();
 YansWifiPhyHelper phy0 = YansWifiPhyHelper::Default();  
 phy0.SetChannel (channel.Create ());
 phy0.Set("ChannelNumber", UintegerValue(36))

 YansWifiChannelHelper channel1 = YansWifiChannelHelper::Default();
 YansWifiPhyHelper phy1 = YansWifiPhyHelper::Default();  
 phy1.SetChannel (channel.Create ());
 phy1.Set("ChannelNumber", UintegerValue(101))

 WifiMacHelper mac0; //default is ad-hoc
 mac0.SetType("ns3::AdhocWifiMac")

 WifiMacHelper mac1; //default is ad-hoc
 mac1.SetType("ns3::AdhocWifiMac")

 dev0->SetPhy(phy0)

 WifiHelper wifi;
 wifi.SetStandard(WIFI_PHY_STANDARD-80211a);
 NetDeviceContainer devices = wifi.Install(phy, mac, MRnodes);
}


