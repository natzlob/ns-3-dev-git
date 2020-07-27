/* Example adapted from wifi-spectrum-per-interference.cc to show 
 * two communicating wifi ad-hoc nodes with a third interferer.
 * The nodes switch channels 
 */

#include <iomanip>
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/string.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/mobility-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/waveform-generator.h"
#include "ns3/waveform-generator-helper.h"
#include "ns3/non-communicating-net-device.h"
#include "ns3/wifi-net-device.h"

// Fix MCS, change received signal strength
//  Wi-Fi 192.168.1.0
//
//   Ad-hoc             Ad-hoc
//    * <-- distance -->  *
//    |     channel 2     |
//    n1                  n2

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


NS_LOG_COMPONENT_DEFINE ("WifiAdHocInterferenceChannels");


int main (int argc, char *argv[])
{
    double simulationTime = 10;
    uint32_t payloadSize = 972;
    std::string phyMode ("HtMcs4");
    double rss = -60;
    double interval = 1.0;
    Time interPacketInterval = Seconds (interval);
    uint8_t channelNum = 44;

    std::cout << std::setw (5) << "Channel" <<
    std::setw (12) << "freq (MHz)" <<
    std::setw (13) << "Rate (Mb/s)" <<
    std::setw (12) << "Tput (Mb/s) " <<
    std::setw (10) << "Received " <<
    std::setw (12) << "Signal (dBm)" <<
    std::setw (12) << "Noi+Inf(dBm)" <<
    std::setw (9) << "SNR (dB)" <<
    std::endl;

    for (uint16_t i = 1; i <= 25; i++)
    {
        rss -= 1;
        NodeContainer adhocNodes;
        adhocNodes.Create(2);

        YansWifiPhyHelper nodePhy = YansWifiPhyHelper::Default ();
        SpectrumWifiPhyHelper spectrumPhy = SpectrumWifiPhyHelper::Default ();
        Ptr<MultiModelSpectrumChannel> spectrumChannel;
        NetDeviceContainer adhocDevs;
        YansWifiChannelHelper channel;
        
        channel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss", DoubleValue (rss));
        channel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
        nodePhy.SetChannel (channel.Create ());
        
        WifiHelper wifi;
        wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
        wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                    "DataMode",StringValue (phyMode),
                                    "ControlMode",StringValue (phyMode));
        WifiMacHelper mac;
        mac.SetType ("ns3::AdhocWifiMac");
        adhocDevs = wifi.Install (nodePhy, mac, adhocNodes);

        MobilityHelper mobility;
        Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
        positionAlloc->Add (Vector (0.0, 0.0, 0.0));
        positionAlloc->Add (Vector (5.0, 0.0, 0.0));
        positionAlloc->Add (Vector (0.0, 5.0, 0.0));
        mobility.SetPositionAllocator (positionAlloc);
        mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
        mobility.Install (adhocNodes);

        InternetStackHelper internet;
        internet.Install(adhocNodes);

        Ipv4AddressHelper ipv4;
        NS_LOG_INFO ("Assign IP Addresses.");
        ipv4.SetBase ("10.1.1.0", "255.255.255.0");
        Ipv4InterfaceContainer interf = ipv4.Assign (adhocDevs);

        uint16_t port = 9;
        UdpServerHelper server (port);
        ApplicationContainer serverApp;
        serverApp = server.Install (adhocNodes.Get (0));
        serverApp.Start (Seconds (0.0));
        serverApp.Stop (Seconds (simulationTime + 1));

        UdpClientHelper client (interf.GetAddress (0), port);
        client.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
        client.SetAttribute ("Interval", TimeValue (Time ("0.0001"))); //packets/s
        client.SetAttribute ("PacketSize", UintegerValue (payloadSize));
        ApplicationContainer clientApp = client.Install (adhocNodes.Get (1));
        clientApp.Start (Seconds (1.0));
        clientApp.Stop (Seconds (simulationTime + 1));

        Ptr<NetDevice> devPtr = adhocDevs.Get (0);
        Ptr<WifiNetDevice> wifiDevPtr = devPtr->GetObject <WifiNetDevice> ();
        uint16_t freq = wifiDevPtr->GetPhy ()->GetFrequency ();
        channelNum = wifiDevPtr->GetPhy ()->GetChannelNumber ();

        Config::ConnectWithoutContext ("/NodeList/0/DeviceList/*/Phy/MonitorSnifferRx", MakeCallback (&MonitorSniffRx));

        g_signalDbmAvg = 0;
        g_noiseDbmAvg = 0;
        g_samples = 0;
        
        Simulator::Stop (Seconds (simulationTime + 1));
        Simulator::Run ();

        uint64_t totalPacketsThrough = 0;
        double throughput = 0;
        totalPacketsThrough = DynamicCast<UdpServer> (serverApp.Get (0))->GetReceived ();
        throughput = totalPacketsThrough * payloadSize * 8 / (simulationTime * 1000000.0); //Mbit/s
        
        std::cout << std::setw (5) << int(channelNum) <<
            std::setw (12) << int(freq) <<
            std::setprecision (2) << std::fixed <<
            std::setw (10) << phyMode <<
            std::setw (12) << throughput <<
            std::setw (12) << totalPacketsThrough <<
            std::setw (8) << g_signalDbmAvg <<
            std::setw (12) << g_noiseDbmAvg <<
            std::setw (12) << (g_signalDbmAvg - g_noiseDbmAvg) <<
            std::endl;

        Simulator::Destroy ();
    }
    return 0;
}