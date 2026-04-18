#include "csa-mac.h"
#include "csa-buffer.h"
#include "csa-header.h"
#include "csa-solver.h"
#include "csa-tag.h"
#include "csa-net-device.h"
#include <ns3/drop-tail-queue.h>
#include <ns3/socket.h>
#include <ns3/qos-utils.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("CsaMac");
NS_OBJECT_ENSURE_REGISTERED(CsaMac);


/*
 * -------------------------------------------
 *      Basic Methods
 * -------------------------------------------
 *
*/

TypeId CsaMac::GetTypeId() {
    static TypeId tid = TypeId("ns3::CsaMac")
                        .SetParent<Object>()
                        .SetGroupName("Csa")
                        .AddConstructor<CsaMac>()
                        .AddAttribute("SlotDuration",
                                      "The duration of a slot in seconds",
                                      TimeValue(MicroSeconds(100)),
                                      MakeTimeAccessor(&CsaMac::m_slotDuration),
                                      MakeTimeChecker())
                        .AddAttribute("QueueSize",
                                      "The maximum number of packets in the queue",
                                      UintegerValue(100),
                                      MakeUintegerAccessor(&CsaMac::m_queueSize),
                                      MakeUintegerChecker<uint32_t>())
                        .AddAttribute("SendProb", 
                                      "Probability of sending a packet",
                                      DoubleValue(0.02),
                                      MakeDoubleAccessor(&CsaMac::m_sendProb),
                                      MakeDoubleChecker<double>())
                        .AddAttribute("DefaultNoise", 
                                      "Default noise level",
                                      DoubleValue(-100.0),
                                      MakeDoubleAccessor(&CsaMac::m_defaultNoise),
                                      MakeDoubleChecker<double>())
                        .AddAttribute("Timeout", 
                                      "Timeout for which a slot is valid", 
                                      TimeValue(MilliSeconds(500)), 
                                      MakeTimeAccessor(&CsaMac::m_timeout), 
                                      MakeTimeChecker())
                        .AddAttribute("Efficiency",
                                      "Efficiency of the solver",
                                      DoubleValue(1.0),
                                      MakeDoubleAccessor(&CsaMac::m_efficiency),
                                      MakeDoubleChecker<double>())
                        .AddAttribute("IsPowerSolvable",
                                      "Is the solver power solvable",
                                      BooleanValue(false),
                                      MakeBooleanAccessor(&CsaMac::m_isPowerSolvable),
                                      MakeBooleanChecker())
                        .AddTraceSource ("EnqueueCb", 
                                      "Fired when a packet is Enqueued",
                                      MakeTraceSourceAccessor (&CsaMac::m_enqueueTrace),
                                      "ns3::CsaMac::TxCallback")
                        .AddTraceSource ("ReceiveCb", 
                                      "Fired when a packet is forwarded up",
                                      MakeTraceSourceAccessor (&CsaMac::m_receiveTrace),
                                      "ns3::CsaMac::RxCallback");
    return tid;
}

CsaMac::CsaMac() {
    // NS_LOG_FUNCTION(this);
    m_slotDuration = MicroSeconds(100);
    m_queueSize = 100;
    m_s = 1;
    m_n = 3;
    m_sendProb = 0.02;
    m_stream = 0;
    m_defaultNoise = -100.0;
    m_timeout = MilliSeconds(500);
    m_efficiency = 1.00;
    m_seqNo = 0;
    m_rng = CreateObject<UniformRandomVariable>();
    m_isSleeping = true;
    m_isTx = false;
    m_solvingAsked = false;
    m_epsilonTime = MicroSeconds(1);
    m_buffer = new SlotStateBuffer;
    m_isPowerSolvable = false;
}

CsaMac::~CsaMac() {
    // NS_LOG_FUNCTION(this);
    delete m_buffer;
}

void CsaMac::DoInitialize() {
    // NS_LOG_FUNCTION(GetAddress() << m_n << m_s << m_defaultNoise << m_timeout << m_efficiency);
    m_queue = CreateObject<DropTailQueue<Packet>>();
    m_queue->SetMaxSize(QueueSize(QueueSizeUnit::PACKETS, m_queueSize));
    m_rng->SetStream(m_stream);
    m_buffer->SetDefaultNoise(m_defaultNoise);
    m_buffer->SetEfficiency(m_efficiency);
    m_buffer->SetTimeout(m_timeout);
    // DebugBuffer();
}


/*
 * -------------------------------------------
 *      Util Methods
 * -------------------------------------------
 *
*/
Time CsaMac::NextSlotTime() const {
    Time currentTime = Simulator::Now();
    Time timeInCurrentSlot = currentTime % m_slotDuration;
    if (timeInCurrentSlot.IsZero()) {
        return Seconds(0);
    }
    return m_slotDuration - timeInCurrentSlot;
}

uint32_t CsaMac::GetCurrentSlotNumber() const {
    Time currentTime = Simulator::Now();
    return currentTime.GetNanoSeconds() / m_slotDuration.GetNanoSeconds();
}

uint32_t CsaMac::GetGeometricValue() const {
    uint32_t value = 0;
    while(m_rng->GetValue(0, 1) > m_sendProb) {
        value++;
    }
    return value;
}

std::vector<uint32_t> CsaMac::GetNSlots() const {
    std::vector<uint32_t> slots;
    uint32_t base = GetCurrentSlotNumber();
    for (uint32_t i = 0; i < m_n; i++) {
        base += GetGeometricValue();
        slots.push_back(base);
        base ++;
    }
    return slots;
}

Mac48Address CsaMac::GetAddress() const {
    return Mac48Address::ConvertFrom(m_netDevice->GetAddress());
}

void CsaMac::SetupSolver() {
    // NS_LOG_FUNCTION(this);
    m_solver.SetBuffer(m_buffer);
    m_solver.SetForwardUpCallback(MakeCallback(&CsaMac::ForwardUp, this));
    m_solver.SetIsClearCallback(MakeCallback(&CsaPhy::IsClear, m_phy));
    m_solver.SetIsDecodableCallback(MakeCallback(&CsaPhy::IsDecodable, m_phy));
    m_solver.SetIsPowerSolvable(m_isPowerSolvable);
}


/*
 * -------------------------------------------
 *      Sending Methods
 * -------------------------------------------
 *
*/
void CsaMac::WakeUp() {
    if(m_isSleeping) {
        m_isSleeping = false;
        Simulator::Schedule(NextSlotTime(), &CsaMac::Dequeue, this);
    }
}

void CsaMac::Sleep() {
    if(!m_isSleeping) {
        m_isSleeping = true;
    }
}

bool CsaMac::Enqueue(Ptr<Packet> packet, Mac48Address dest, uint16_t protocolNumber) {
    // NS_LOG_FUNCTION(GetAddress() << GetCurrentSlotNumber());
    CsaTag tag;
    tag.SetDestAddress(dest);
    tag.SetProtocol(protocolNumber);
    packet->AddPacketTag(tag);

    SimulationTag simTag;
    packet->RemovePacketTag(simTag);
    simTag.SetTxTime(Simulator::Now().GetSeconds());
    packet->AddPacketTag(simTag);

    Mac48Address myMac = GetAddress(); 
    uint8_t buffer[6];
    myMac.CopyTo (buffer);
    uint32_t senderId = (uint32_t(buffer[4]) << 8) | uint32_t(buffer[5]);
    // trace callback
    m_enqueueTrace(senderId, simTag.GetTid());

    bool success = m_queue->Enqueue(packet);
    if(success) {
        WakeUp();
    }
    return success;
}

void CsaMac::Dequeue() {
    // NS_LOG_FUNCTION(GetAddress() << GetCurrentSlotNumber());
    Ptr<Packet> packet = m_queue->Dequeue();
    if(packet) {
        SendPacket(packet);
    } 
    else {
        Sleep();
    }
}

CsaHeader CsaMac::PrepareHeader(Ptr<Packet> packet, std::vector<uint32_t> &allSlots) {
    CsaTag tag;
    packet->PeekPacketTag(tag);
    CsaHeader header;
    header.SetSource(GetAddress());
    header.SetDestination(tag.GetDestAddress());
    header.SetProtocolNumber(tag.GetProtocol());
    header.SetSequenceNumber(m_seqNo++);
    header.SetLength(packet->GetSize());
    header.SetS(m_s);
    header.SetN(m_n);
    header.SetAllSlots(allSlots);
    return header;
}

void CsaMac::SendPacket(Ptr<Packet> packet) {
    SimulationTag simTag;
    packet->PeekPacketTag(simTag);

    std::vector<uint32_t> allSendSlots = GetNSlots();
    CsaHeader header = PrepareHeader(packet, allSendSlots);
    Ptr<Packet> fragBase = Create<Packet>(packet->GetSize()/m_s + (packet->GetSize() % m_s != 0));
    fragBase->AddPacketTag(simTag);

    uint32_t currSlot = GetCurrentSlotNumber();
    for(int i = 0; i < m_n; i ++) {
        Ptr<Packet> frag = fragBase->Copy();
        header.SetCurrentSlot(allSendSlots[i]);
        frag->AddHeader(header);
        // NS_LOG_DEBUG(header);
        SendAtSlot(frag, allSendSlots[i] - currSlot);
    }
    Simulator::Schedule((allSendSlots[m_n-1]+1-currSlot)*m_slotDuration, &CsaMac::Dequeue, this);
}

void CsaMac::SendAtSlot(Ptr<Packet> packet, uint32_t slot) {
    // NS_LOG_FUNCTION(GetAddress() << packet << slot + GetCurrentSlotNumber());
    Simulator::Schedule(slot*m_slotDuration, &CsaMac::StartSend, this, packet);
}

void CsaMac::StartSend(Ptr<Packet> packet) {
    m_isTx = true;
    m_phy->StartTx(packet);
    Simulator::Schedule(m_slotDuration, &CsaMac::EndSend, this);
}

void CsaMac::EndSend() {
    m_isTx = false;
}


/*
 * -------------------------------------------
 *      Receiving Methods
 * -------------------------------------------
 *
*/
void CsaMac::Receive(Ptr<Packet> packet, double rxPower) {
    // NS_LOG_FUNCTION(this << Simulator::Now().GetNanoSeconds());
    if(!m_isTx) {
        uint32_t currSlot = GetCurrentSlotNumber();
        // remove the header
        CsaHeader header;
        packet->RemoveHeader(header);
        // NS_LOG_DEBUG("Header: " << header);
        NS_ASSERT_MSG(header.GetCurrentSlot() == currSlot, "Packet is crossing slots: " << currSlot << " " << packet->GetSize() << " " << m_slotDuration << " " << header);
        // NS_LOG_FUNCTION(GetAddress() << GetCurrentSlotNumber() << header);
        Mac48Address source = header.GetSource();
        uint32_t seqNo = header.GetSequenceNumber();

        SimulationTag simTag;
        packet->PeekPacketTag(simTag);

        
        // get the PacketState
        PacketStatePtr packetState = m_buffer->m_packetStateBuffer.GetPacketState(source, seqNo);
        if(packetState == nullptr) {
            // add if not found
            packetState = m_buffer->m_packetStateBuffer.AddPacketState(header, simTag);
        }

        // get the SlotState
        SlotStatePtr slotState = m_buffer->GetSlotState(currSlot);
        if(slotState == nullptr) {
            // add if not found
            slotState = m_buffer->AddSlotState(currSlot);
        }
        
        // add the packet in the slot buffer
        slotState->packets[packetState] = rxPower;
        
        if(!m_solvingAsked) {
            m_solvingAsked = true;
            Solve();
        }
    }
}

void CsaMac::Solve() {
    // NS_LOG_FUNCTION(this << Simulator::Now().GetNanoSeconds());
    Time remTime = NextSlotTime();
    NS_ASSERT_MSG(remTime > 2*m_epsilonTime, "Late arrival! " << Simulator::Now().GetNanoSeconds() << " " << remTime.GetNanoSeconds() << " " << 2*m_epsilonTime.GetNanoSeconds());
    // call solver at the end of the slot
    Simulator::Schedule(remTime-2*m_epsilonTime, &CsaSolver::StartSolveAtSlot, &m_solver, GetCurrentSlotNumber());
    // after it's solved, we can end it
    Simulator::Schedule(remTime-m_epsilonTime, &CsaMac::EndSolve, this);
}

void CsaMac::EndSolve() {
    m_solvingAsked = false;
}

void CsaMac::ForwardUp(PacketStatePtr packetState) {
    // uint16_t protocolNumber = packetState->protocol;
    Mac48Address source = packetState->source;
    // NS_LOG_FUNCTION(GetAddress() << protocolNumber << source << GetCurrentSlotNumber());
    uint8_t buffer[6];
    source.CopyTo (buffer);
    uint32_t senderId = (uint32_t(buffer[4]) << 8) | uint32_t(buffer[5]);
    Mac48Address myMac = GetAddress();
    myMac.CopyTo (buffer);
    uint32_t receiverId = (uint32_t(buffer[4]) << 8) | uint32_t(buffer[5]);
    m_receiveTrace(senderId, receiverId, packetState->txTime, Simulator::Now().GetSeconds(), packetState->tid);
    // m_netDevice->ForwardUp(packet, protocolNumber, Address(source));
}




/*
 * -------------------------------------------
 *      Getters
 * -------------------------------------------
 *
*/

Ptr<CsaPhy> CsaMac::GetPhy() const {
    return m_phy;
}


Ptr<CsaNetDevice> CsaMac::GetNetDevice() const {
    return m_netDevice;
}

SlotStateBuffer* CsaMac::GetBuffer() const {
    return m_buffer;
}

/*
 * -------------------------------------------
 *      Setters
 * -------------------------------------------
 *
*/

void CsaMac::SetPhy(Ptr<CsaPhy> phy) {
    m_phy = phy;
}


void CsaMac::SetNetDevice(Ptr<CsaNetDevice> netDevice) {
    m_netDevice = netDevice;
}

void CsaMac::SetBuffer(SlotStateBuffer* buffer) {
    m_buffer = buffer;
}

void CsaMac::SetStream(int64_t stream) {
    m_stream = stream;
}

void CsaMac::SetStrategy(const std::vector<std::pair<uint8_t, uint8_t>>& strategies, 
                         const std::vector<double>& probabilities) {
    NS_ASSERT_MSG(strategies.size() == probabilities.size(), "Strategy and Probability vectors must match in size");
    
    double random = m_rng->GetValue(0, 1);
    double cumulative = 0;
    
    for(size_t i = 0; i < strategies.size(); ++i) {
        cumulative += probabilities[i];
        if(random <= cumulative || i == strategies.size() - 1) {
            m_n = strategies[i].first;
            m_s = strategies[i].second;
            break;
        }
    }
}



// void CsaMac::DebugBuffer() {
//     Time now = Simulator::Now();
//     uint32_t slotBufferSize = m_buffer->m_slotBuffer.size();
//     uint32_t packetBufferSize = m_buffer->m_packetStateBuffer.m_packetBuffer.size();
//     Mac48Address mac = GetAddress();
//     NS_LOG_DEBUG("Buffer Debug Log: [" << now << ", " << mac << "] Slot Buffer Size: " << slotBufferSize << " Packet Buffer Size: " << packetBufferSize);
//     Simulator::Schedule(MilliSeconds(100), &CsaMac::DebugBuffer, this);
// }

}   // namespace ns3