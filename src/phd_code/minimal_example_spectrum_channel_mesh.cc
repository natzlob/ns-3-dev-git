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
#include "ns3/channel.h"
#include "ns3/spectrum-channel.h"

using namespace ns3;

int main(int argc, char *argv[])
{
    NodeContainer nodes;
    MeshHelper mesh;
    NetDeviceContainer meshDevices;
    SpectrumWifiPhyHelper spectrumPhy;
    nodes.Create (9);
    
    spectrumPhy = SpectrumWifiPhyHelper::Default ();
    Ptr<MultiModelSpectrumChannel> spectrumChannel
        = CreateObject<MultiModelSpectrumChannel> ();
    Ptr<FixedRssLossModel> lossModel
        = CreateObject<FixedRssLossModel> ();
    lossModel->SetRss(-50);

    spectrumChannel->AddPropagationLossModel (lossModel);

    Ptr<ConstantSpeedPropagationDelayModel> delayModel
        = CreateObject<ConstantSpeedPropagationDelayModel> ();
    spectrumChannel->SetPropagationDelayModel (delayModel);

    spectrumPhy.SetChannel (spectrumChannel);
    mesh = MeshHelper::Default ();
    mesh.SetStackInstaller ("ns3::Dot11sStack");
    mesh.SetMacType ("RandomStart", TimeValue (Seconds (0.1)));
    mesh.SetNumberOfInterfaces (2);
    meshDevices = mesh.Install (spectrumPhy, nodes);

    Ptr<MeshPointDevice> mp = DynamicCast<MeshPointDevice>(meshDevices.Get(0));
    Ptr<Channel> mp_channel = mp->GetChannel();
    Ptr<SpectrumChannel> sp_channel = DynamicCast<SpectrumChannel>(mp_channel);
    if (sp_channel != 0) {
        NS_LOG_UNCOND("sp_channel pointer not empty");
    }
    else {
        NS_LOG_UNCOND("sp_channel is null");
    }

    return 0;
}