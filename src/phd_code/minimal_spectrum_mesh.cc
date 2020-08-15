/* minimal example of mesh with spectrumWifiPhy */

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

using namespace ns3;

int main (int argc, char *argv[])
{
    int xSize = 3;
    int ySize = 3;
    NodeContainer nodes;
    NetDeviceContainer meshDevices;
    MeshHelper mesh;
    SpectrumWifiPhyHelper spectrumPhy;
    SpectrumChannelHelper spectrumChannel;

    nodes.Create (xSize*ySize);
    mesh = MeshHelper::Default ();
    mesh.SetStackInstaller ("ns3::Dot11sStack");
    mesh.SetNumberOfInterfaces (2);
    spectrumPhy = SpectrumWifiPhyHelper::Default ();
    spectrumPhy.Set ("Frequency", UintegerValue (5180));
    spectrumChannel = SpectrumChannelHelper::Default ();
    spectrumPhy.SetChannel (spectrumChannel.Create ());

    meshDevices = mesh.Install (spectrumPhy, nodes);

    return 0;
}