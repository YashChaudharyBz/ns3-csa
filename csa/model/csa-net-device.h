#ifndef CSA_NET_DEVICE_H
#define CSA_NET_DEVICE_H

#include "csa-mac.h"
#include <ns3/core-module.h>
#include <ns3/net-device.h>

namespace ns3 {

class CsaNetDevice: public NetDevice {
public:
    static TypeId GetTypeId();
    void SetIfIndex(const uint32_t index) override;
    uint32_t GetIfIndex() const override;
    Ptr<Channel> GetChannel() const override;
    void SetAddress(Address address) override;
    Address GetAddress() const override;
    bool SetMtu(const uint16_t mtu) override;
    uint16_t GetMtu() const override;
    bool IsLinkUp() const override;
    void AddLinkChangeCallback(Callback<void> callback) override;
    bool IsBroadcast() const override;
    Address GetBroadcast() const override;
    bool IsMulticast() const override;
    Address GetMulticast(Ipv4Address multicastGroup) const override;
    Address GetMulticast(Ipv6Address addr) const override;
    bool IsBridge() const override;
    bool IsPointToPoint() const override;
    bool Send(Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber) override;
    bool SendFrom(Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber) override;
    Ptr<Node> GetNode() const override;
    void SetNode(Ptr<Node> node) override;
    bool NeedsArp() const override;
    void SetReceiveCallback(ReceiveCallback cb) override;
    void SetPromiscReceiveCallback(PromiscReceiveCallback cb) override;
    bool SupportsSendFrom() const override;

    Ptr<CsaMac> GetMac() const;
    void SetMac(Ptr<CsaMac> mac);
    void ForwardUp(Ptr<Packet> packet, uint16_t protocolNumber, Address sender);

    void DoInitialize() override;

private:
    uint32_t m_ifIndex;
    Ptr<Node> m_node;
    ReceiveCallback m_rxCallback;
    PromiscReceiveCallback m_promiscRxCallback;
    Address m_address;
    uint16_t m_mtu;
    Ptr<CsaMac> m_mac;
};

}   // namespace ns3

#endif  // CSA_NET_DEVICE_H