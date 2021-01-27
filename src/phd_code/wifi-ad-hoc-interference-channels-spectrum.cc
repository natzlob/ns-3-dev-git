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

// Fix MCS, change channels
//Network topology:
//
//  Wi-Fi 192.168.1.0
//
//   ad-hoc              ad-hoc
//    * <--- distance ---> *<--distance2--> *
//    |      channel 1     |   channel 1    |
//    n1                   n2               n3 (interferer)
//
//
//  Wi-Fi 192.168.1.0
//
//   STA                  AP
//    * <-- distance -->  *
//    |     channel 2     |
//    n1                  n2

using namespace ns3;

// Global variables for use in callbacks.
double g_signalDbmAvg;
double g_noiseDbmAvg;
uint32_t g_samples;
int i = 0;

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

// index i to channel mapping:
// i=1: channel=36 (5180)
// i=2: channel=40 (5200)
// i=3: channel=44 (5220)
// i=4: channel=48 (5240)

NS_LOG_COMPONENT_DEFINE ("WifiAdHocInterferenceChannels");


int main (int argc, char *argv[])
{
    //double distance1 = 50;
    //double distance2 = 100;
    double simulationTime = 10;
    std::string errorModelType = "ns3::NistErrorRateModel";
    uint32_t payloadSize = 972;
    std::string phyMode ("HtMcs0");
    double rss = -80;
    //uint32_t numPackets = 10;
    //uint32_t packetSize = 1000;
    double interval = 1.0;
    Time interPacketInterval = Seconds (interval);
    //uint8_t channelNum = 44;

    std::cout << std::setw (5) << "Channel" <<
    std::setw (12) << "freq (MHz)" <<
    std::setw (13) << "Rate (Mb/s)" <<
    std::setw (12) << "Tput (Mb/s) " <<
    std::setw (10) << "Received " <<
    std::setw (12) << "Signal (dBm)" <<
    std::setw (12) << "Noi+Inf(dBm)" <<
    std::setw (9) << "SNR (dB)" <<
    std::endl;

    for (uint16_t i = 1; i <= 4; i++)
    {
        NodeContainer adhocNodes;
        adhocNodes.Create(2);

        NodeContainer interferingNode;
        interferingNode.Create(1);

        YansWifiPhyHelper nodePhy;
        nodePhy.SetErrorRateModel ("ns3::NistErrorRateModel");
        SpectrumWifiPhyHelper spectrumPhy;
        spectrumPhy.SetErrorRateModel ("ns3::NistErrorRateModel");
        Ptr<MultiModelSpectrumChannel> spectrumChannel;

        YansWifiPhyHelper interfPhy;
        interfPhy.SetErrorRateModel ("ns3::NistErrorRateModel");
        NetDeviceContainer adhocDevs;
        YansWifiChannelHelper channel;
        if (i == 1)
        {
            spectrumChannel = CreateObject<MultiModelSpectrumChannel> ();
            Ptr<FriisPropagationLossModel> lossModel;
            lossModel = CreateObject<FriisPropagationLossModel> ();
            lossModel->SetFrequency (5.180e9);
            spectrumChannel->AddPropagationLossModel (lossModel);
            // Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
            // spectrumChannel->SetPropagationDelayModel (delayModel);

            spectrumPhy.SetChannel (spectrumChannel);
            spectrumPhy.SetErrorRateModel (errorModelType);
            spectrumPhy.Set ("Frequency", UintegerValue (5180));

            // interfPhy.SetChannel (channel.Create());
            // interfPhy.Set ("Frequency", UintegerValue (5180));

            WifiHelper wifi;
            wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
            wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                        "DataMode",StringValue (phyMode),
                                        "ControlMode",StringValue (phyMode));
            WifiMacHelper mac;
            mac.SetType ("ns3::AdhocWifiMac");

            adhocDevs = wifi.Install (spectrumPhy, mac, adhocNodes);

            // Ptr<NetDevice> netdev = adhocDevs.Get (0);
            // Ptr<WifiNetDevice> adhocnetdev = DynamicCast<WifiNetDevice> (netdev);
            // Ptr<WifiPhy> phyPtr = adhocnetdev->GetPhy();
            // uint16_t channelWidth = phyPtr->GetChannelWidth();
            // std::cout << "Channel width: " << int(channelWidth) << std::endl;
        }

        else if (i == 2)
        {
            rss = -65;
            channel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss", DoubleValue (rss));
            channel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
            nodePhy.SetChannel (channel.Create ());
            nodePhy.Set ("Frequency", UintegerValue (5200));

            interfPhy.SetChannel (channel.Create());
            interfPhy.Set ("Frequency", UintegerValue (5200));

            WifiHelper wifi;
            wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
            wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                        "DataMode",StringValue (phyMode),
                                        "ControlMode",StringValue (phyMode));
            WifiMacHelper mac;
            mac.SetType ("ns3::AdhocWifiMac");

            adhocDevs = wifi.Install (nodePhy, mac, adhocNodes);

            Ptr<NetDevice> netdev = adhocDevs.Get (0);
            Ptr<WifiNetDevice> adhocnetdev = DynamicCast<WifiNetDevice> (netdev);
            Ptr<WifiPhy> phyPtr = adhocnetdev->GetPhy();
            uint16_t channelWidth = phyPtr->GetChannelWidth();
            std::cout << "Channel width: " << int(channelWidth) << std::endl;
        }

        else if (i == 3)
        {
            rss = -75;
            channel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss", DoubleValue (rss));
            channel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
            nodePhy.SetChannel (channel.Create ());
            nodePhy.Set ("Frequency", UintegerValue (5220));

            interfPhy.SetChannel (channel.Create());
            interfPhy.Set ("Frequency", UintegerValue (5220));

            WifiHelper wifi;
            wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
            wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                        "DataMode",StringValue (phyMode),
                                        "ControlMode",StringValue (phyMode));
            WifiMacHelper mac;
            mac.SetType ("ns3::AdhocWifiMac");

            adhocDevs = wifi.Install (nodePhy, mac, adhocNodes);
            Ptr<NetDevice> netdev = adhocDevs.Get (0);
            Ptr<WifiNetDevice> adhocnetdev = DynamicCast<WifiNetDevice> (netdev);
            Ptr<WifiPhy> phyPtr = adhocnetdev->GetPhy();
            uint16_t channelWidth = phyPtr->GetChannelWidth();
            std::cout << "Channel width: " << int(channelWidth) << std::endl;
        }

        else if (i == 4)
        {
            rss = -75;
            channel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss", DoubleValue (rss));
            channel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
            nodePhy.SetChannel (channel.Create ());
            nodePhy.Set ("Frequency", UintegerValue (5240));
            //nodePhy.Set ("ChannelWidth", UintegerValue (40));

            // interfPhy.SetChannel (channel.Create());
            // interfPhy.Set ("Frequency", UintegerValue (5240));
            // interfPhy.Set ("ChannelWidth",UintegerValue (40));
            
            WifiHelper wifi;
            wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
            wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                        "DataMode",StringValue (phyMode),
                                        "ControlMode",StringValue (phyMode));
            WifiMacHelper mac;
            mac.SetType ("ns3::AdhocWifiMac");

            adhocDevs = wifi.Install (nodePhy, mac, adhocNodes);
            Ptr<NetDevice> netdev = adhocDevs.Get (0);
            Ptr<WifiNetDevice> adhocnetdev = DynamicCast<WifiNetDevice> (netdev);
            Ptr<WifiPhy> phyPtr = adhocnetdev->GetPhy();
            uint16_t channelWidth = phyPtr->GetChannelWidth();
            std::cout << "Channel width: " << int(channelWidth) << std::endl;
            // netdev = adhocDevs.Get(1);
            // adhocnetdev = DynamicCast<WifiNetDevice> (netdev);
            // phyPtr = adhocnetdev->GetPhy();
            // phyPtr->SetChannelWidth(40);
        }
        
        else
        {
            NS_FATAL_ERROR("Index out of range");
        }

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

        // Ptr<NetDevice> devPtr = adhocDevs.Get (0);
        // Ptr<WifiNetDevice> wifiDevPtr = devPtr->GetObject <WifiNetDevice> ();
        // uint16_t freq = wifiDevPtr->GetPhy ()->GetFrequency ();
        // channelNum = wifiDevPtr->GetPhy ()->GetChannelNumber ();

        Config::ConnectWithoutContext ("/NodeList/0/DeviceList/*/Phy/MonitorSnifferRx", MakeCallback (&MonitorSniffRx));

        g_signalDbmAvg = 0;
        g_noiseDbmAvg = 0;
        g_samples = 0;

        // Tracing
        spectrumPhy.EnablePcap ("wifi-adhoc-channels-spectrum", adhocDevs);

        Ptr<NetDevice> devicePtr = adhocDevs.Get (0);
        Ptr<WifiNetDevice> wifiDevicePtr = devicePtr->GetObject <WifiNetDevice> ();
        UintegerValue channel_number;
        wifiDevicePtr->GetPhy ()->GetAttribute ("ChannelNumber", channel_number);
        UintegerValue freq;
        wifiDevicePtr->GetPhy ()->GetAttribute ("Frequency", freq);
        
        Simulator::Stop (Seconds (simulationTime + 1));
        Simulator::Run ();

        uint64_t totalPacketsThrough = 0;
        double throughput = 0;
        totalPacketsThrough = DynamicCast<UdpServer> (serverApp.Get (0))->GetReceived ();
        throughput = totalPacketsThrough * payloadSize * 8 / (simulationTime * 1000000.0); //Mbit/s
        
        std::cout << std::setw (5) << int(channel_number.Get()) <<
            std::setw (12) << int(freq.Get()) <<
            std::setprecision (2) << std::fixed <<
            std::setw (10) << phyMode <<
            std::setw (12) << throughput <<
            std::setw (12) << totalPacketsThrough <<
            std::setw (8) << g_signalDbmAvg <<
            std::setw (12) << g_noiseDbmAvg <<
            std::setw (12) << (g_signalDbmAvg - g_noiseDbmAvg) <<
            std::endl;

        //NS_LOG_UNCOND ("Testing " << numPackets  << " packets sent with receiver rss " << rss );

        Simulator::Destroy ();
    }
    return 0;
}