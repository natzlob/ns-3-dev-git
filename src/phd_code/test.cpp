#include "ns3/simulator.h"

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mesh-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/jakes-propagation-loss-model.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-phy-helper.h"
#include "ns3/yans-wifi-helper.h"

using namespace ns3;


int main (int argc, char *argv[])
{
    YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
    YansWifiChannelHelper wifiChannel;
    wifiChannel.AddPropagationLoss("ns3::LogDistancePropagationLossModel",
                                  "Exponent", DoubleValue (2.5));
    wifiPhy.SetChannel (wifiChannel.Create ());

    return 0;
}