#ifndef  CSA_TAG_H
#define CSA_TAG_H

#include <ns3/tag.h>
#include <ns3/mac48-address.h>
#include <ns3/timer.h>

namespace ns3 {

class CsaTag : public ns3::Tag
{
public:
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(ns3::TagBuffer i) const override;
    void Deserialize(ns3::TagBuffer i) override;
    void Print(std::ostream &os) const override;

    void SetDestAddress(Mac48Address dest);
    void SetProtocol(uint16_t protocol);
    Mac48Address GetDestAddress() const;
    uint16_t GetProtocol() const;

private:
    Mac48Address m_dest;
    uint16_t m_protocol;
};

class SimulationTag: public Tag {
public:
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(ns3::TagBuffer i) const override;
    void Deserialize(ns3::TagBuffer i) override;
    void Print(std::ostream &os) const override;

    void SetTid(uint8_t tid);
    uint8_t GetTid() const;
    void SetTxTime(double time);
    double GetTxTime() const;

private:
    uint8_t m_tid;
    double m_txTime;
};

}   // namespace ns3


#endif