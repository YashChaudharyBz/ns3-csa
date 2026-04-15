#include "csa-header.h"
#include <ns3/address-utils.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("CsaHeader");
NS_OBJECT_ENSURE_REGISTERED(CsaHeader);

/*
* -------------------------------------
*   Main methods
* -------------------------------------
*/
CsaHeader::CsaHeader() {}

TypeId CsaHeader::GetTypeId() {
    static TypeId tid = TypeId("ns3::CsaHeader")
                        .SetParent<Header>()
                        .SetGroupName("Csa")
                        .AddConstructor<CsaHeader>();
    return tid;
}

TypeId CsaHeader::GetInstanceTypeId() const {
    return GetTypeId();
}

uint32_t CsaHeader::GetSerializedSize() const {
    return  6    // sender
            + 6  // destination
            + 2  // protocol number
            + 4  // sequence number
            + 4  // length
            + 1  // s
            + 1  // n
            + 4*m_n  // all slots
            + 4;  // current slot
}

void CsaHeader::Serialize (Buffer::Iterator start) const {
    WriteTo(start, m_source);
    WriteTo(start, m_dest);
    start.WriteHtolsbU16(m_protocolNumber);
    start.WriteHtolsbU32(m_sequenceNumber);
    start.WriteHtolsbU32(m_length);
    start.WriteU8(m_s);
    start.WriteU8(m_n);
    NS_ASSERT_MSG(m_allSlots.size() == m_n, "Invalid number of slots");
    for (uint8_t i = 0; i < m_n; i++) {
        start.WriteHtolsbU32(m_allSlots[i]);
    }
    start.WriteHtolsbU32(m_currentSlot);
}

uint32_t CsaHeader::Deserialize (Buffer::Iterator start) {
    ReadFrom(start, m_source);
    ReadFrom(start, m_dest);
    m_protocolNumber = start.ReadLsbtohU16();
    m_sequenceNumber = start.ReadLsbtohU32();
    m_length = start.ReadLsbtohU32();
    m_s = start.ReadU8();
    m_n = start.ReadU8();
    m_allSlots.resize(m_n);
    for (uint8_t i = 0; i < m_n; i++) {
        m_allSlots[i] = start.ReadLsbtohU32();
    }
    m_currentSlot = start.ReadLsbtohU32();
    return GetSerializedSize();
}

void CsaHeader::Print (std::ostream &os) const {
    os << "CSA Header" << std::endl
        << "\tSource: " << m_source
        << "\tDestination: " << m_dest
        << "\tProtocol Number: " << m_protocolNumber
        << "\tSequence Number: " << m_sequenceNumber
        << "\tLength: " << m_length
        << "\tS: " << int(m_s)
        << "\tN: " << int(m_n)
        << "\tCurrent Slot: " << m_currentSlot
        << "\tAll Slots: ";
    for (uint8_t i = 0; i < m_n; i++) {
        os << m_allSlots[i] << " ";
    }
    os << std::endl;
}

/*
* -------------------------------------
*   Setters
* -------------------------------------
*/
void CsaHeader::SetSource(Mac48Address source) {
    m_source = source;
}

void CsaHeader::SetDestination(Mac48Address dest) {
    m_dest = dest;
}

void CsaHeader::SetProtocolNumber(uint16_t protocolNumber) {
    m_protocolNumber = protocolNumber;
}

void CsaHeader::SetSequenceNumber(uint32_t sequenceNumber) {
    m_sequenceNumber = sequenceNumber;
}

void CsaHeader::SetLength(uint32_t length) {
    m_length = length;
}

void CsaHeader::SetS(uint8_t s) {
    m_s = s;
}

void CsaHeader::SetN(uint8_t n) {
    m_n = n;
}

void CsaHeader::SetAllSlots(std::vector<uint32_t> allSlots) {
    m_allSlots = allSlots;
}

void CsaHeader::SetCurrentSlot(uint32_t currentSlot) {
    m_currentSlot = currentSlot;
}


/*
* -------------------------------------
*   Getters
* -------------------------------------
*/
Mac48Address CsaHeader::GetSource() const {
    return m_source;
}

Mac48Address CsaHeader::GetDestination() const {
    return m_dest;
}

uint16_t CsaHeader::GetProtocolNumber() const {
    return m_protocolNumber;
}

uint32_t CsaHeader::GetSequenceNumber() const {
    return m_sequenceNumber;
}

uint32_t CsaHeader::GetLength() const {
    return m_length;
}

uint8_t CsaHeader::GetS() const {
    return m_s;
}

uint8_t CsaHeader::GetN() const {
    return m_n;
}

std::vector<uint32_t> CsaHeader::GetAllSlots() const {
    return m_allSlots;
}

uint32_t CsaHeader::GetCurrentSlot() const {
    return m_currentSlot;
}


}   // namespace ns3