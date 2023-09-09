#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/stats-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ssid.h"
#include "ns3/yans-wifi-helper.h"
#include <fstream>
#define TX_RANGE 5

NS_LOG_COMPONENT_DEFINE("NS3-Offline-1");

using namespace ns3;

//Ptr<PacketSink> sink;     //!< Pointer to the packet sink application

int packetSent = 0;
int packetReceived = 0;
double_t totalBytes = 0;

bool mobile = false;

void TxTrigger(Ptr<const Packet> p){
    packetSent++;
}

void RxTrigger(Ptr<const Packet> p, const Address &address) {
    packetReceived++;
    totalBytes += p->GetSize()*8.0;
}


//Node Creation
//Helper Configuration
//Helper Device Install
//Internet Stack Helper Install
//Address Allocation
//Send Data
//Populate routing table 
//Simulation Actions

int main(int argc, char* argv[])
{
    uint32_t payloadSize = 1024;           /* Transport layer payload size in bytes. */ //--- 
    std::string dataRate = "1Mbps";      /* Application layer datarate. */
    double simulationTime = 3.0;            /* Simulation time in seconds. */
    //bool verbose = true;


    

    int nWifi = 10; // 20, 40, 60, 80, 100
    int flowNo = 10; // 10, 20, 30, 40, 50  -----> x 2
    double packetPerSecond = 128; // 100, 200, 300, 400, 500
    double speed = 10; // 5, 10, 15, 20 -----> m/s
    double coverageFactor = 2; // 1, 2, 3, 4, 5 -----> *Tx Range
    int x, y;

    CommandLine cmd(__FILE__);
    cmd.AddValue ("nWifi","No. of nodes one each side", nWifi);
    cmd.AddValue ("flowNo","Total flow count in network", flowNo);
    cmd.AddValue ("packetPerSecond", "Packet from each source per second", packetPerSecond);
    cmd.AddValue ("coverageFactor", "Coverage factor in static network", coverageFactor);
    cmd.AddValue ("x", "parameter", x);
    cmd.AddValue ("y", "plotted value", y);
    cmd.Parse(argc, argv);


    
    
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(payloadSize)); //--- socket segment size?

    if(argc>=2) nWifi = atoi(argv[1]);
    if(argc>=3) flowNo = atoi(argv[2]);
    if(argc>=4) packetPerSecond = atof(argv[3]);
    if(argc>=5) (mobile)? speed = atof(argv[4]) : coverageFactor = atof(argv[4]);
    if(argc>=6) x = atoi(argv[5]);
    if(argc>=7) y = atoi(argv[6]);

    payloadSize = 1024*128*1/packetPerSecond; // /* Transport layer payload size in bytes. */ //--- 

    // std::cout<< "no. of node/2: "<< nWifi
    //          << "\nno. of flows: "<< flowNo
    //          << "\npckt per sec: "<< packetPerSecond
    //          << "\nspeed of nodes: "<< speed
    //          << "\ncoverage factor: "<< coverageFactor
    //          << "\nmobile: "<< mobile<< std::endl;

    

    std::string bottleNeckLinkBw = "10Mbps";
    std::string bottleNeckLinkDelay = "5ms";
    int nSender = nWifi;
    int nReceiver = nWifi;


//Node Creation
    NodeContainer p2pBottleneck;
    p2pBottleneck.Create(2);

    NodeContainer wifiSenders; //--- one sender network
    wifiSenders.Create(nSender);
    NodeContainer wifiApSender = p2pBottleneck.Get(0);

    NodeContainer wifiReceivers; //--- one receiver network
    wifiReceivers.Create(nReceiver);
    NodeContainer wifiApReceiver = p2pBottleneck.Get(1);

//Helper Configuration

    PointToPointHelper bottleneck;
    bottleneck.SetDeviceAttribute ("DataRate", StringValue(bottleNeckLinkBw));
    bottleneck.SetChannelAttribute ("Delay", StringValue(bottleNeckLinkDelay));

    YansWifiChannelHelper channelSenders = YansWifiChannelHelper::Default();
    YansWifiChannelHelper channelReceivers = YansWifiChannelHelper::Default();
    if(!mobile) channelSenders.AddPropagationLoss("ns3::RangePropagationLossModel", "MaxRange", DoubleValue(coverageFactor*TX_RANGE));
    YansWifiPhyHelper phySenders, phyReceivers;
    phySenders.SetChannel(channelSenders.Create());
    phyReceivers.SetChannel(channelReceivers.Create());
//++
    WifiMacHelper macSenders, macReceivers;
    Ssid ssidSenders = Ssid("ns-3-ssid");
    Ssid ssidReceivers = Ssid("ns-3-ssid");
//++
    WifiHelper wifi;
//++
    MobilityHelper mobility;

    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(0.0),
                                  "MinY", DoubleValue(0.0),
                                  "DeltaX", DoubleValue(1.0),
                                  "DeltaY", DoubleValue(2.0),
                                  "GridWidth", UintegerValue(3),
                                  "LayoutType", StringValue("RowFirst"));

//Helper Device Install
    NetDeviceContainer bottleneckDevices;
    bottleneckDevices = bottleneck.Install(p2pBottleneck);

    NetDeviceContainer staSenders, staReceivers;
    macSenders.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssidSenders), "ActiveProbing", BooleanValue(false));
    macReceivers.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssidReceivers), "ActiveProbing", BooleanValue(false));
    staSenders = wifi.Install(phySenders, macSenders, wifiSenders);
    staReceivers = wifi.Install(phyReceivers, macReceivers, wifiReceivers);

    NetDeviceContainer apSenders, apReceivers;
    macSenders.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssidSenders));
    macReceivers.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssidReceivers));
    apSenders = wifi.Install(phySenders, macSenders, wifiApSender);
    apReceivers = wifi.Install(phyReceivers, macReceivers, wifiApReceiver);

    if(!mobile){
        mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel"
                                  /*"Bounds", RectangleValue (Rectangle (-50, 50, -50, 50))*/);
    }
    else{
        double bound = coverageFactor*30;
        mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", 
                                   "Bounds", RectangleValue (Rectangle (-bound, bound, -bound, bound)),
                                   "Speed", StringValue ("ns3::ConstantRandomVariable[Constant="+std::to_string(speed)+"]"));
    }
    mobility.Install(wifiSenders);
    mobility.Install(wifiReceivers);
//++
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(p2pBottleneck);

//Internet Stack Helper Install
    InternetStackHelper stack;
    stack.Install(wifiSenders);
    stack.Install(wifiReceivers);
    stack.Install(p2pBottleneck);

//Address Allocation
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0"); 
    Ipv4InterfaceContainer senderInterfaces = address.Assign(staSenders);
    address.Assign(apSenders);

    address.SetBase("10.1.2.0", "255.255.255.0"); 
    address.Assign(bottleneckDevices);

    address.SetBase("10.1.3.0", "255.255.255.0"); 
    Ipv4InterfaceContainer receiverInterfaces = address.Assign(staReceivers);
    address.Assign(apReceivers);

//Send Data
    Address receiverLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), 9));
    PacketSinkHelper receiverHelper("ns3::TcpSocketFactory", receiverLocalAddress);
    ApplicationContainer receiverApp;
    for(int i=0; i<nReceiver; i++){
        receiverApp.Add(receiverHelper.Install(wifiReceivers.Get(i)));
    }
    for(int i=0; i<nReceiver; i++){
        receiverApp.Get(i)->TraceConnectWithoutContext("Rx", MakeCallback(&RxTrigger));
    }
    receiverApp.Start(Seconds(0.0));

    OnOffHelper senderHelper("ns3::TcpSocketFactory", Address());
    senderHelper.SetAttribute("PacketSize", UintegerValue(payloadSize));
    senderHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    senderHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    senderHelper.SetAttribute("DataRate", DataRateValue(DataRate(dataRate)));
    ApplicationContainer senderApp;
    int currFlows = 0;
    for (int i = 0; i < nReceiver; i++){
        // Create an on/off app on right side node which sends packets to the left side
        AddressValue remoteAddress (InetSocketAddress (receiverInterfaces.GetAddress(i), 9));
        for(int j = 0; j < nSender; j++){
            senderHelper.SetAttribute ("Remote", remoteAddress);
            senderApp.Add(senderHelper.Install(wifiSenders.Get(j)));
            currFlows++;
            if(currFlows >= flowNo) break;
        }
    }
    for(int i=0; i<nSender; i++){
        senderApp.Get(i)->TraceConnectWithoutContext("Tx", MakeCallback(&TxTrigger));
    }
    senderApp.Start(Seconds(1.0));
    
//Populate routing table 
    Ipv4GlobalRoutingHelper::PopulateRoutingTables(); //--- important

//Simulation Actions
    Simulator::Stop(Seconds(1.0+simulationTime));
    Simulator::Run();
    double averageThroughput = totalBytes/(1e6 * simulationTime);
    Simulator::Destroy();
    double packetDeliveryRatio = (double)packetReceived*100.00/(double)packetSent;

    switch (x)
    {
    case 1:
        std::cout<< nWifi<< " "; break;
    case 2:
        std::cout<< flowNo<< " "; break;
    case 3:
        std::cout<< packetPerSecond<< " "; break;
    case 4:
        std::cout<< speed<< " "; break;
    case 5:
        std::cout<< coverageFactor<< " "; break;
    default:
        break;
    }

    switch (y)
    {
    case 1:
        std::cout<< packetDeliveryRatio<< "\n"; break;
    case 2:
        std::cout<< averageThroughput<< "\n"; break;
    default:
        break;
    }
    return 0;
}
