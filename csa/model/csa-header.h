#ifndef CSA_HEADER_H
#define CSA_HEADER_H

#include <ns3/header.h>
#include <ns3/mac48-address.h>
#include <ns3/timer.h>

namespace ns3 {

class CsaHeader: public Header {

public:
    CsaHeader();

    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;
    uint32_t GetSerializedSize() const override;
    void Serialize (Buffer::Iterator start) const override;
    uint32_t Deserialize (Buffer::Iterator start) override;
    void Print (std::ostream &os) const override;

    void SetSource(Mac48Address source);
    void SetDestination(Mac48Address dest);
    void SetProtocolNumber(uint16_t protocolNumber);
    void SetSequenceNumber(uint32_t sequenceNumber);
    void SetLength(uint32_t length);
    void SetS(uint8_t s);
    void SetN(uint8_t n);
    void SetAllSlots(std::vector<uint32_t> allSlots);
    void SetCurrentSlot(uint32_t currentSlot);

    Mac48Address GetSource() const;
    Mac48Address GetDestination() const;
    uint16_t GetProtocolNumber() const;
    uint32_t GetSequenceNumber() const;
    uint32_t GetLength() const;
    uint8_t GetS() const;
    uint8_t GetN() const;
    std::vector<uint32_t> GetAllSlots() const;
    uint32_t GetCurrentSlot() const;

private:
    Mac48Address m_source;
    Mac48Address m_dest;
    uint16_t m_protocolNumber;
    uint32_t m_sequenceNumber;
    uint32_t m_length;
    uint8_t m_s;
    uint8_t m_n;
    std::vector<uint32_t> m_allSlots;
    uint32_t m_currentSlot;
};

}   // namespace ns3

#endif  // CSA_HEADER_H