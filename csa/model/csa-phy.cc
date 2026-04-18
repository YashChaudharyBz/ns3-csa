#include "csa-phy.h"
#include "csa-net-device.h"
#include <ns3/node.h>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(CsaPhy);
NS_LOG_COMPONENT_DEFINE("CsaPhy");

/*
 * -------------------------------------------
 *      Basic Methods
 * -------------------------------------------
 *
*/
TypeId CsaPhy::GetTypeId() {
    static TypeId tid = TypeId("ns3::CsaPhy")
                            .SetParent<Object>()
                            .SetGroupName("Csa")
                            .AddConstructor<CsaPhy>()
                            .AddAttribute("TxPower",
                                          "The transmit power in dBm.",
                                          DoubleValue(20),
                                          MakeDoubleAccessor(&CsaPhy::m_txPower),
                                          MakeDoubleChecker<double>())
                            .AddAttribute("DataRate",
                                          "The data rate index.",
                                          EnumValue(RATE_3Mbps),
                                          MakeEnumAccessor(&CsaPhy::m_dataRate),
                                          MakeRateIndexChecker());
    return tid;
}

CsaPhy::CsaPhy() {
    m_txPower = 20;
    m_dataRate = RateIndex::RATE_12Mbps;
    NS_LOG_FUNCTION(this);
}

CsaPhy::~CsaPhy() {
    NS_LOG_FUNCTION(this);
}


/*
 * -------------------------------------------
 *      Util Methods
 * -------------------------------------------
 *
*/
double CsaPhy::GetSnr(SlotStatePtr slotState, PacketStatePtr packetState) {
    double rxPower = slotState->packets[packetState];
    double noise = slotState->noise;
    return rxPower - noise;
}

bool CsaPhy::IsClear(SlotStatePtr slotState, PacketStatePtr packetState) {
    double snr = GetSnr(slotState, packetState);
    return snr > csaRates[m_dataRate].thresholdDb;
}

double CsaPhy::GetSinr(SlotStatePtr slotState, PacketStatePtr packetState) {
    double rxPower = slotState->packets[packetState];
    double noiseMw = std::pow(10.0, slotState->noise / 10.0);
    double remPowerMw = 0;
    for(auto it = slotState->packets.begin(); it != slotState->packets.end(); it++) {
        if(it->first != packetState) {
            remPowerMw += std::pow(10.0, it->second / 10.0);
        }
    }
    double sinr = rxPower - 10.0 * log10(noiseMw + remPowerMw);
    return sinr;
}

bool CsaPhy::IsDecodable(SlotStatePtr slotState, PacketStatePtr packetState) {
    double sinr = GetSinr(slotState, packetState);
    return sinr > csaRates[m_dataRate].thresholdDb;
}


/*
 * -------------------------------------------
 *      Sending Methods
 * -------------------------------------------
 *
*/
void CsaPhy::StartTx(Ptr<Packet> packet) {
    m_channel->Send(this, packet, m_txPower);
}


/*
 * -------------------------------------------
 *      Receiving Methods
 * -------------------------------------------
 *
*/
void CsaPhy::Receive(Ptr<Packet> packet, double rxPower) {
    NS_LOG_FUNCTION(this << packet << rxPower);
    NS_ASSERT_MSG(m_dataRate < RateIndex::RATE_UNKNOWN, "Invalid data rate");
    Time transmissionDelay = Seconds(static_cast<double>(packet->GetSize()) * 8.0 / csaRates[m_dataRate].GetDataRateBps());
    // CsaHeader header;
    // packet->PeekHeader(header);
    // NS_LOG_DEBUG(header);
    if(!m_rxCallback.IsNull()) {
        Simulator::Schedule(transmissionDelay, &CsaPhy::m_rxCallback, this, packet, rxPower);
    }
}


/*
 * -------------------------------------------
 *      Getters
 * -------------------------------------------
 *
*/

double CsaPhy::GetTxPower() const {
    return m_txPower;
}

Ptr<CsaChannel> CsaPhy::GetChannel() const {
    return m_channel;
}

Ptr<CsaNetDevice> CsaPhy::GetNetDevice() const {
    return m_netDevice;
}


RateIndex CsaPhy::GetDataRate() const {
    return m_dataRate;
}

Ptr<MobilityModel> CsaPhy::GetMobility() const {
    return m_netDevice->GetNode()->GetObject<MobilityModel>();
}


/*
 * -------------------------------------------
 *      Setters
 * -------------------------------------------
 *
*/
void CsaPhy::SetTxPower(double txPower) {
    m_txPower = txPower;
}

void CsaPhy::SetChannel(Ptr<CsaChannel> channel) {
    m_channel = channel;
}

void CsaPhy::SetNetDevice(Ptr<CsaNetDevice> netDevice) {
    m_netDevice = netDevice;
}

void CsaPhy::SetDataRate(RateIndex dataRate) {
    m_dataRate = dataRate;
}

void CsaPhy::SetRxCallback(RxCallback callback) {
    m_rxCallback = callback;
}


}   // namespace ns3