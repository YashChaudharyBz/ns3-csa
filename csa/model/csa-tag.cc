#include "csa-tag.h"
#include <ns3/address-utils.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("CsaTag");
NS_OBJECT_ENSURE_REGISTERED(CsaTag);
/*
* --------------------------------
*   Main Methods
* --------------------------------
*/
TypeId CsaTag::GetTypeId() {
    static TypeId tid = TypeId("ns3::CsaTag")
                        .SetParent<Tag>()
                        .SetGroupName("Csa")
                        .AddConstructor<CsaTag>();
    return tid;
}

TypeId CsaTag::GetInstanceTypeId() const {
    return GetTypeId();
}

uint32_t CsaTag::GetSerializedSize() const {
    return 6    // dest
        + 2;   // protocol
}

void CsaTag::Serialize(ns3::TagBuffer i) const {
    uint8_t buffer[6];
    m_dest.CopyTo(buffer);
    i.Write(buffer, 6);
    i.WriteU16(m_protocol);
}

void CsaTag::Deserialize(ns3::TagBuffer i) {
    uint8_t buffer[6];
    i.Read(buffer, 6);
    m_dest.CopyFrom(buffer);
    m_protocol = i.ReadU16();
}

void CsaTag::Print(std::ostream &os) const {
    os << "Dest: " << m_dest << " Protocol: " << m_protocol;
}


/*
* --------------------------------
*   Getters and Setters
* --------------------------------
*/
void CsaTag::SetDestAddress(Mac48Address dest) {
    m_dest = dest;
}

void CsaTag::SetProtocol(uint16_t protocol) {
    m_protocol = protocol;
}

Mac48Address CsaTag::GetDestAddress() const {
    return m_dest;
}

uint16_t CsaTag::GetProtocol() const {
    return m_protocol;
}




TypeId SimulationTag::GetTypeId() {
    static TypeId tid = TypeId("ns3::SimulationTag")
                        .SetParent<Tag>()
                        .SetGroupName("Simulation")
                        .AddConstructor<SimulationTag>();
    return tid;
}

TypeId SimulationTag::GetInstanceTypeId() const {
    return GetTypeId();
}

uint32_t SimulationTag::GetSerializedSize() const {
    return 1
        + sizeof(double);
}

void SimulationTag::Serialize(ns3::TagBuffer i) const {
    i.WriteU8(m_tid);
    i.WriteDouble(m_txTime);
}

void SimulationTag::Deserialize(ns3::TagBuffer i) {
    m_tid = i.ReadU8();
    m_txTime = i.ReadDouble();
}

void SimulationTag::Print(std::ostream &os) const {
    os << "TID: " << m_tid << " Time: " << m_txTime;
}


void SimulationTag::SetTid(uint8_t tid) {
    m_tid = tid;
}

uint8_t SimulationTag::GetTid() const {
    return m_tid;
}

void SimulationTag::SetTxTime(double time) {
    m_txTime = time;
}

double SimulationTag::GetTxTime() const {
    return m_txTime;
}


}   // namespace ns3