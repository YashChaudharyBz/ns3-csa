#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/internet-module.h>
#include <ns3/qos-utils.h>

#include <ns3/csa-helper.h>
#include <ns3/csa-mac.h>
#include <ns3/csa-net-device.h>
#include <sys/types.h>

using namespace ns3;

/*
* Simulation script for CSA algorithm
*
* Setable parameters:
* 1. nVehicels: number of vehicles
* 2. lambda0: rate for the exponential traffic
* 3. lambda1: rate for the deterministic traffic
* 4. packetSizeBits: packet size in bits
* 5. dataRate: data rate in Mbps as a string
*/

// Strategies and probabilities
std::vector<std::pair<uint8_t, uint8_t>> strategies = {{3, 2}, {4, 2}, {7, 2}};
std::vector<double> probabilities = {0.5943, 0.2077, 0.1980};

// slot duration calculator
Time slotDuration(RateIndex dataRate, uint32_t packetSizeBits) {
    double dataRateMbps = csaRates[dataRate].dataRateMbps;
    Time txDelay = MicroSeconds((packetSizeBits * 1.0 / strategies[0].second) / dataRateMbps);
    Time headerDelay = MicroSeconds((85*8.0) / dataRateMbps); // rough estimate of CSA header
    Time propDelay = MicroSeconds(3);   // 300m at speed of light + 2us for boundary
    Time slotDuration = txDelay + propDelay + headerDelay;
    // std::cout << "DataRate: " << dataRateMbps << ", packetSizeBits: " << packetSizeBits << ", txDelay: " << txDelay << ", propDelay: " << propDelay << ", headerDelay: " << headerDelay << ", slotDuration: " << slotDuration << std::endl;
    return slotDuration;
}

// storage for traces
std::map<uint32_t, uint32_t> g_voTxCount;
std::map<std::pair<uint32_t, uint32_t>, uint32_t> g_voRxCount;
std::map<std::pair<uint32_t, uint32_t>, double> g_voDelay;
std::map<uint32_t, uint32_t> g_viTxCount;
std::map<std::pair<uint32_t, uint32_t>, uint32_t> g_viRxCount;
std::map<std::pair<uint32_t, uint32_t>, double> g_viDelay;

void RecreateExp(uint32_t nVehicles, double lambda0, double lambda1, uint32_t packetSizeBits, RateIndex dataRate, 
                 double sendProb, double defaultNoise, Time timeout, double efficiency, uint32_t queueSize, bool powerSolvable);
void WriteOutput(uint32_t nVehicles, std::ofstream *outFile);

// void PrintTime() {
//     std::cout << "Time: " << Simulator::Now().GetSeconds() << std::endl;
//     Simulator::Schedule(Seconds(0.1), &PrintTime);
// }
bool verbose = false;
std::ofstream* verboseFilePtr = nullptr;

int main(int argc, char* argv[]) {
    LogComponentEnable("CsaMac", ns3::LOG_LEVEL_ALL);
    // Setable parameters
    uint32_t nVehicles = 50;
    double lambda0 = 35;
    double lambda1 = 35;
    uint32_t packetSizeBits = 2400;
    std::string dataRateString = "12Mbps";
    std::string fileName = "results.csv";
    std::string folderName = "scratch/simulate-csa/";
    
    uint32_t queueSize = 100;
    double sendProb = 0.5;
    double defaultNoise = -101.0;
    Time timeout = MilliSeconds(500);
    double efficiency = 1.0;
    bool powerSolvable = false;
    uint64_t run = 0;
    std::string verboseFile = "verbose.csv";
    std::string verboseFolder = "scratch/simulate-csa/";

    
    CommandLine cmd;
    cmd.AddValue("nVehicles", "Number of vehicles", nVehicles);
    cmd.AddValue("lambda0", "Rate for the exponential traffic", lambda0);
    cmd.AddValue("lambda1", "Rate for the deterministic traffic", lambda1);
    cmd.AddValue("packetSizeBits", "Packet size in bits", packetSizeBits);
    cmd.AddValue("dataRate", "Data rate in Mbps as a string", dataRateString);
    cmd.AddValue ("fileName", "Name of file to generate output", fileName);
    cmd.AddValue ("folderName", "Name of folder to generate output", folderName);
    cmd.AddValue("queueSize", "Queue size", queueSize);
    cmd.AddValue("sendProb", "Send probability", sendProb);
    cmd.AddValue("defaultNoise", "Default noise", defaultNoise);
    cmd.AddValue("timeout", "Timeout", timeout);
    cmd.AddValue("efficiency", "Efficiency", efficiency);
    cmd.AddValue("powerSolvable", "Power Solvable (NOMA-CSA)", powerSolvable);
    cmd.AddValue("run", "Run number", run);
    cmd.AddValue("verbose", "Verbose (Packet Level)", verbose);
    cmd.AddValue("verboseFile", "Verbose File", verboseFile);
    cmd.AddValue("verboseFolder", "Verbose Folder", verboseFolder);
    cmd.Parse(argc, argv);

    // check if datarate is valid
    auto it = rateIndexMap.find(dataRateString);
    if (it == rateIndexMap.end()) {
        std::cerr << "Invalid data rate: " << dataRateString << std::endl;
        return 1;
    }
    RateIndex dataRate = it->second;
    
    // setup output file
    fileName = folderName + fileName;
    std::ofstream *outFile = new std::ofstream(fileName.c_str(), std::ios::out);
    *outFile << "nVehicles,lambda0,lambda1,packetSizeBits,dataRate,voPSR,voDelay,viPSR,viDelay\n";
    *outFile << nVehicles << "," << lambda0 << "," << lambda1 << "," << packetSizeBits << "," << dataRateString << ",";

    // setup verbose file
    if(verbose) {
        verboseFile = verboseFolder + verboseFile;
        verboseFilePtr = new std::ofstream(verboseFile.c_str(), std::ios::out);
        *verboseFilePtr << "sender,receiver,ac,delay\n";
    }

    // run the simulation
    RngSeedManager::SetSeed(12345);
    RngSeedManager::SetRun(run);
    RecreateExp(nVehicles, lambda0, lambda1, packetSizeBits, dataRate, sendProb, defaultNoise, timeout, efficiency, queueSize, powerSolvable);

    WriteOutput(nVehicles, outFile);
    return 0;
}


void EnqueueCallback(uint32_t sender, uint8_t tid) {
    // std::cout << "EnqueueCallback: " << sender << ", tid: " << int(tid) << std::endl;
    AcIndex ac = QosUtilsMapTidToAc(tid);
    if(ac == AC_VO) {
        g_voTxCount[sender] ++;
    }
    else if(ac == AC_VI) {
        g_viTxCount[sender] ++;
    }
    else {
        std::cerr << "Invalid AC(Sender): " << int(ac) << ", tid: " << int(tid) << std::endl;
    }
}

void ReceiveCallback(uint32_t sender, uint32_t receiver, double txTime, double rxTime, uint8_t tid) {
    AcIndex ac = QosUtilsMapTidToAc(tid);
    if(ac == AC_VO) {
        if(verbose) {
            *verboseFilePtr << sender << "," << receiver << "," << "VO" << "," << (rxTime - txTime) << std::endl;
        }
        g_voRxCount[{sender, receiver}] ++;
        g_voDelay[{sender, receiver}] += (rxTime - txTime);
    }
    else if(ac == AC_VI) {
        if(verbose) {
            *verboseFilePtr << sender << "," << receiver << "," << "VI" << "," << (rxTime - txTime) << std::endl;
        }
        g_viRxCount[{sender, receiver}] ++;
        g_viDelay[{sender, receiver}] += (rxTime - txTime);
    }
    else  {
        std::cerr << "Invalid AC(Receiver): " << int(ac) << ", tid: " << int(tid) << std::endl;
    }
}

// Global function to schedule randomized Poisson traffic for AC_VO
void ScheduleNextPoisson (Ptr<Socket> socket, uint32_t pktSize, double lambda) {
    Ptr<Packet> pkt = Create<Packet> (pktSize / 8); // Convert bits to bytes
    // socket->SetIpTos (0xe0); // AC_VO
    SimulationTag simTag;
    simTag.SetTid(6);           // AC_VO
    pkt->AddPacketTag(simTag);
    socket->Send (pkt);

    // Exponential inter-arrival time for Poisson process: 1/lambda
    Ptr<ExponentialRandomVariable> x = CreateObject<ExponentialRandomVariable> ();
    x->SetAttribute ("Mean", DoubleValue (1.0 / lambda));
    Time nextInterval = Seconds (x->GetValue ());

    Simulator::Schedule (nextInterval, &ScheduleNextPoisson, socket, pktSize, lambda);
}

// Global function to schedule Deterministic traffic for AC_VI
void ScheduleNextDeterministic (Ptr<Socket> socket, uint32_t pktSize, double lambda) {
    Ptr<Packet> pkt = Create<Packet> (pktSize / 8); 
    // socket->SetIpTos (0xa0); // AC_VI
    SimulationTag simTag;
    simTag.SetTid(4);           // AC_VI
    pkt->AddPacketTag(simTag);
    socket->Send (pkt);

    // Deterministic interval: 1/lambda
    Time nextInterval = Seconds (1.0 / lambda);
    Simulator::Schedule (nextInterval, &ScheduleNextDeterministic, socket, pktSize, lambda);
}

void RecreateExp(uint32_t nVehicles, double lambda0, double lambda1, uint32_t packetSizeBits, RateIndex dataRate, 
                 double sendProb, double defaultNoise, Time timeout, double efficiency, uint32_t queueSize, bool powerSolvable) {
    // create the nodes
    NodeContainer vehicles;
    vehicles.Create (nVehicles);

    // add mobility (300 x 6 zone, position assigned randomly)
    MobilityHelper mobility;
    mobility.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
                                  "X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=300.0]"),
                                  "Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=6.0]"));
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (vehicles);

    // setup CSA
    CsaHelper helper;
    helper.SetStrategies(strategies);
    helper.SetProbabilities(probabilities);

    // setup channel
    Ptr<TwoRayGroundPropagationLossModel> loss = CreateObject<TwoRayGroundPropagationLossModel>();
    loss->SetFrequency(5.86e9);
    loss->SetMinDistance(1.0);
    loss->SetHeightAboveZ(1.5);
    loss->SetSystemLoss(3.0);
    Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel>(); 
    helper.SetChannelAttribute("PropagationLossModel", PointerValue(loss));
    helper.SetChannelAttribute("PropagationDelayModel", PointerValue(delay));

    // setup MAC 
    helper.SetMacAttribute("SlotDuration", TimeValue(slotDuration(dataRate, packetSizeBits)));
    helper.SetMacAttribute("QueueSize", UintegerValue(queueSize));
    helper.SetMacAttribute("SendProb", DoubleValue(sendProb));
    helper.SetMacAttribute("DefaultNoise", DoubleValue(defaultNoise));
    helper.SetMacAttribute("Timeout", TimeValue(timeout));
    helper.SetMacAttribute("Efficiency", DoubleValue(efficiency));
    helper.SetMacAttribute("IsPowerSolvable", BooleanValue(powerSolvable));

    // setup PHY
    helper.SetPhyAttribute("DataRate", EnumValue(dataRate));

    // install
    NetDeviceContainer devices = helper.Install(vehicles);

    // add traces for send/receive
    for(auto i = devices.Begin(); i != devices.End(); ++i) {
        Ptr<CsaNetDevice> device = DynamicCast<CsaNetDevice>(*i);
        Ptr<CsaMac> mac = device->GetMac();
        mac->TraceConnectWithoutContext("EnqueueCb", MakeCallback(&EnqueueCallback));
        mac->TraceConnectWithoutContext("ReceiveCb", MakeCallback(&ReceiveCallback));
    }

    // Stack & IP
    InternetStackHelper stack;
    stack.Install (vehicles);
    Ipv4AddressHelper address;
    // Should allow 1022 addresses
    address.SetBase ("10.1.0.0", "255.255.252.0");
    address.Assign (devices);

    // Setup Sockets for every vehicle
    uint16_t port = 9000;
    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
        for (uint32_t i = 0; i < nVehicles; ++i) {
        // Broadcaster Socket
        Ptr<Socket> sourceSocket = Socket::CreateSocket (vehicles.Get (i), tid);
        sourceSocket->Connect (InetSocketAddress (Ipv4Address::GetBroadcast (), port));
        sourceSocket->SetAllowBroadcast (true);

        // Schedule first packets with a small random jitter to avoid massive t=0 collisions
        Ptr<UniformRandomVariable> jitter = CreateObject<UniformRandomVariable> ();
        Simulator::Schedule (Seconds (jitter->GetValue (0.1, 1.0)), 
                            &ScheduleNextPoisson, sourceSocket, packetSizeBits, lambda0);
        Simulator::Schedule (Seconds (jitter->GetValue (0.1, 1.0)), 
                            &ScheduleNextDeterministic, sourceSocket, packetSizeBits, lambda1);
    }

    // start simulation
    // PrintTime();
    Simulator::Stop (Seconds (10));
    Simulator::Run ();
    Simulator::Destroy ();
}


void WriteOutput(uint32_t nVehicles, std::ofstream *outFile) {
    uint32_t totalVoTransmissions = 0;
    uint32_t totalViTransmissions = 0;
    uint32_t totalVoReceptions = 0;
    uint32_t totalViReceptions = 0;
    double totalVoDelaySum = 0;
    double totalViDelaySum = 0;
    for (auto const& [nodeId, count] : g_voTxCount) {
        totalVoTransmissions += count;
    }
    for (auto const& [pair, count] : g_voRxCount) {
        totalVoReceptions += count;
    }
    for (auto const& [pair, delay] : g_voDelay) {
        totalVoDelaySum += delay;
    }
    for (auto const& [nodeId, count] : g_viTxCount) {
        totalViTransmissions += count;
    }
    for (auto const& [pair, count] : g_viRxCount) {
        totalViReceptions += count;
    }
    for (auto const& [pair, delay] : g_viDelay) {
        totalViDelaySum += delay;
    }
    double voPSR = (totalVoTransmissions > 0) ? 
        (double)totalVoReceptions / (totalVoTransmissions * (nVehicles - 1)) : 0;
    double voAvgDelay = (totalVoReceptions > 0) ? 
        (totalVoDelaySum / totalVoReceptions) : 0;
    double viPSR = (totalViTransmissions > 0) ? 
        (double)totalViReceptions / (totalViTransmissions * (nVehicles - 1)) : 0;
    double viAvgDelay = (totalViReceptions > 0) ? 
        (totalViDelaySum / totalViReceptions) : 0;
    voAvgDelay *= 1000;
    viAvgDelay *= 1000;

    *outFile << voPSR << "," << voAvgDelay << "," << viPSR << "," << viAvgDelay << "\n";
    outFile->close();
    delete outFile;
}