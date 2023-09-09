/*
 *
 * Network topology:
 *
 * S0-------------R1-------------R2--------------Rec
 *      10Mbps          1 Mbps          10Mbps
 *        2ms             5ms             2ms
 * Calculate throughput for this network at 5ms interval and plot a throughput vs time graph
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/applications-module.h"
#include "ns3/stats-module.h"
#include "ns3/callback.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/csma-module.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("Offline2");


class TutorialApp : public Application
{
  public:
    TutorialApp();
    ~TutorialApp() override;

    /**
     * Register this type.
     * \return The TypeId.
     */
    static TypeId GetTypeId();

    /**
     * Setup the socket.
     * \param socket The socket.
     * \param address The destination address.
     * \param packetSize The packet size to transmit.
     * \param nPackets The number of packets to transmit.
     * \param dataRate the data rate to use.
     */
    void Setup(Ptr<Socket> socket,
               Address address,
               uint32_t packetSize,
               DataRate dataRate,
               uint32_t simultime);

  private:
    void StartApplication() override;
    void StopApplication() override;

    /// Schedule a new transmission.
    void ScheduleTx();
    /// Send a packet.
    void SendPacket();

    Ptr<Socket> m_socket;   //!< The transmission socket.
    Address     m_peer;         //!< The destination address.
    uint32_t    m_packetSize;  //!< The packet size.
    uint32_t    m_nPackets;    //!< The number of packets to send.
    DataRate    m_dataRate;    //!< The data rate to use.
    EventId     m_sendEvent;    //!< Send event.
    bool        m_running;         //!< True if the application is running.
    uint32_t    m_packetsSent; //!< The number of packets sent.
    uint32_t    m_simultime; // might need
};

TutorialApp::TutorialApp()
    : m_socket(nullptr),
      m_peer(),
      m_packetSize(0),
      m_nPackets(0), //extra
      m_dataRate(0),
      m_sendEvent(),
      m_running(false),
      m_packetsSent(0),
      m_simultime(0)
{
}

TutorialApp::~TutorialApp()
{
    m_socket = nullptr;
}

/* static */
TypeId
TutorialApp::GetTypeId()
{
    static TypeId tid = TypeId("TutorialApp")
                            .SetParent<Application>()
                            .SetGroupName("Tutorial")
                            .AddConstructor<TutorialApp>();
    return tid;
}

void
TutorialApp::Setup(Ptr<Socket> socket,
                   Address address,
                   uint32_t packetSize,
                   DataRate dataRate,
                   uint32_t simultime)
{
    m_socket = socket;
    m_peer = address;
    m_packetSize = packetSize;
    m_dataRate = dataRate;
    m_simultime = simultime;
}

void
TutorialApp::StartApplication()
{
    m_running = true;
    m_packetsSent = 0;
    (InetSocketAddress::IsMatchingType(m_peer))? m_socket->Bind() : m_socket->Bind6();
    m_socket->Connect(m_peer);
    SendPacket();
}

void
TutorialApp::StopApplication()
{
    m_running = false;

    if(m_sendEvent.IsRunning())
    {
        Simulator::Cancel(m_sendEvent);
    }

    if(m_socket)
    {
        m_socket->Close();
    }
}

void
TutorialApp::SendPacket()
{
    Ptr<Packet> packet = Create<Packet>(m_packetSize);
    m_socket->Send(packet);

    if(Simulator::Now().GetSeconds() < m_simultime)
    {
        ScheduleTx();
    }
}

void
TutorialApp::ScheduleTx()
{
    if(m_running)
    {
        Time tNext(Seconds(m_packetSize * 8 / static_cast<double>(m_dataRate.GetBitRate())));
        m_sendEvent = Simulator::Schedule(tNext, &TutorialApp::SendPacket, this);
    }
}

static void
CwndChange(Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
    // NS_LOG_UNCOND(Simulator::Now().GetSeconds() << " " << newCwnd);
    *stream->GetStream() << Simulator::Now().GetSeconds() << " " << newCwnd << std::endl;
}


int main(int argc, char* argv[])
{

//Values 
    uint32_t payloadSize = 1400;  
    int nSender = 2;
    int nReceiver = 2;  
    int nFlows=2; 
    string bottleneckLinkCapacity="50Mbps"; 
    int bottleneckLinkCapacityInt=50;   
    string bottleneckDelay="100ms"; 
    int bottleneckDelayInt=100; 
    string leafDataRate = "1Gbps";    
    string leafDelay="1ms";   
    double packetLossRate = 0.000001;
    string TcpAlgo[2] = {"ns3::TcpNewReno","ns3::TcpAdaptiveReno"}; 
    double lossRateExp=6;
    string fileName="scratch/transport/throughput.dat";
    double simulationTime = 10;  
    double packetSize=1000;
    int cleanupTime=2;

    if(argc>1){
        bottleneckLinkCapacityInt=std::stoi(argv[1]);
        bottleneckLinkCapacity=argv[1];//+"Mbps";
        bottleneckLinkCapacity.append("Mbps");
        lossRateExp=std::stod(argv[2]);
        packetLossRate=std::pow(10,-lossRateExp);
        fileName = argv[3];
        TcpAlgo[0]=argv[4];
        TcpAlgo[1]=argv[5];
    }
    cout<< bottleneckLinkCapacityInt<< " "<< lossRateExp<< " "<< fileName<< " "<< TcpAlgo[0]<< " "<< TcpAlgo[1]<< endl;

//Helper Configuration
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(payloadSize));
    PointToPointHelper bottleneck; 
    bottleneck.SetDeviceAttribute("DataRate", StringValue(bottleneckLinkCapacity)); 
    bottleneck.SetChannelAttribute("Delay", StringValue(bottleneckDelay));  
//++
    PointToPointHelper leaf;  
    leaf.SetDeviceAttribute("DataRate", StringValue(leafDataRate)); 
    leaf.SetChannelAttribute("Delay", StringValue(leafDelay));  
    leaf.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue(std::to_string((bottleneckDelayInt*bottleneckLinkCapacityInt)/packetSize) + "p")); 

//Node Creation + Helper Device Install
    PointToPointDumbbellHelper dumbbell(nSender,leaf,nReceiver,leaf,bottleneck);         

//Error Model  
    Ptr<RateErrorModel> errorModel= CreateObject<RateErrorModel>(); 
    errorModel->SetAttribute("ErrorRate", DoubleValue(packetLossRate)); 
    dumbbell.m_routerDevices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(errorModel));   

//Internet Stack Helper Install
    InternetStackHelper stack[2]; 
    for(int algo = 0; algo<2; algo++){
        Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue(TcpAlgo[algo])); 
        for(uint32_t i=algo; i<dumbbell.LeftCount(); i+=2) stack[algo].Install(dumbbell.GetLeft(i)); // left nodes
        for(uint32_t i=algo; i<dumbbell.RightCount(); i+=2) stack[algo].Install(dumbbell.GetRight(i)); // right nodes
        if(algo==0){
            stack[0].Install(dumbbell.GetLeft());
            stack[0].Install(dumbbell.GetRight());
        }
    }

//Address Allocation
    dumbbell.AssignIpv4Addresses(Ipv4AddressHelper("10.1.1.0", "255.255.255.0"),
                                 Ipv4AddressHelper("10.2.1.0", "255.255.255.0"),  
                                 Ipv4AddressHelper("10.3.1.0", "255.255.255.0"));

//Install Flow Monitor
    FlowMonitorHelper flowMonitor; 
    flowMonitor.SetMonitorAttribute("MaxPerHopDelay", TimeValue(Seconds(cleanupTime)));
    Ptr<FlowMonitor> monitor = flowMonitor.InstallAll();

//Send Data
    ApplicationContainer receiverApp;
    Ptr<TutorialApp> senderApp;
    for(int i=0;i<nFlows;i++){
        Address receiverAddress(InetSocketAddress(dumbbell.GetRightIpv4Address(i), 9));
        PacketSinkHelper receiverHelper("ns3::TcpSocketFactory",InetSocketAddress(dumbbell.GetRightIpv4Address(i), 9));
        receiverApp=receiverHelper.Install(dumbbell.GetRight(i));
        receiverApp.Start(Seconds(0.0));
        receiverApp.Stop(Seconds(simulationTime+cleanupTime));

        senderApp = CreateObject<TutorialApp>();
        Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(dumbbell.GetLeft(i), TcpSocketFactory::GetTypeId());
        senderApp->Setup(ns3TcpSocket, receiverAddress, payloadSize, DataRate(leafDataRate), simulationTime);
        dumbbell.GetLeft(i)->AddApplication(senderApp);
        senderApp->SetStartTime(Seconds(1));
        senderApp->SetStopTime(Seconds(simulationTime));

        std::ostringstream os;
        os << "scratch/cwnd/cwnd_vs_t-"<< bottleneckLinkCapacityInt<< "-"<< lossRateExp<< "-"<< TcpAlgo[i]<< ".dat";
        AsciiTraceHelper asciiTraceHelper;
        Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream(os.str());
        ns3TcpSocket->TraceConnectWithoutContext("CongestionWindow", MakeBoundCallback(&CwndChange, stream));
    }

//Populate routing table
    Ipv4GlobalRoutingHelper::PopulateRoutingTables(); // populate routing table

//Simulation Actions
    Simulator::Stop(Seconds(simulationTime+cleanupTime));
    Simulator::Run();

    double currentThroughput = 0;
    double algoThroughput[2] = {0};

    double fairnessIndex0 = 0;
    double fairnessIndex1 = 0;

    uint32_t sentPackets = 0;
    uint32_t receivedPackets = 0;

    
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowMonitor.GetClassifier());
    monitor->CheckForLostPackets();
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats();

    int n = 0;
    for(FlowMonitor::FlowStatsContainer::const_iterator it = stats.begin(); it!=stats.end(); ++it){
        currentThroughput = it->second.rxBytes * 8.0/((simulationTime+cleanupTime)*1000);
        algoThroughput[n%2]+=it->second.rxBytes; 

        sentPackets = sentPackets +(it->second.txPackets);
        receivedPackets = receivedPackets +(it->second.rxPackets);

        fairnessIndex0 += currentThroughput;
        fairnessIndex1 += (currentThroughput*currentThroughput);
        n++;
    }

    double fairness = (fairnessIndex0*fairnessIndex0)/(n*fairnessIndex1);
    algoThroughput[0] /= ((simulationTime + cleanupTime)*1000);
    algoThroughput[1] /= ((simulationTime + cleanupTime)*1000);
    std::ofstream throughput(fileName, std::ios_base::app);
    throughput << bottleneckLinkCapacityInt << " " << -1*lossRateExp<< " " << algoThroughput[0] << " " << algoThroughput[1] << " "<< fairness<< endl;

    Simulator::Destroy ();
    return 0;
}


