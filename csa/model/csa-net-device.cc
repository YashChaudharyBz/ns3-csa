#include "csa-net-device.h"
#include "csa-phy.h"
#include <ns3/mac48-address.h>
#include <ns3/node.h>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(CsaNetDevice);
NS_LOG_COMPONENT_DEFINE("CsaNetDevice");

TypeId CsaNetDevice::GetTypeId() {
    static TypeId tid = TypeId("ns3::CsaNetDevice")
                    .SetParent<NetDevice>()
                    .SetGroupName("Csa")
                    .AddConstructor<CsaNetDevice>();
    return tid;
}

void CsaNetDevice::SetIfIndex(const uint32_t index) {
    m_ifIndex = index;
}

uint32_t CsaNetDevice::GetIfIndex() const {
    return m_ifIndex;
}

Ptr<Channel> CsaNetDevice::GetChannel() const {
    return m_mac->GetPhy()->GetChannel();
}

void CsaNetDevice::SetAddress(Address address) {
    NS_LOG_FUNCTION (this);
    m_address = address;
}

Address CsaNetDevice::GetAddress() const {
    NS_LOG_FUNCTION (this);
    return m_address;
}

bool CsaNetDevice::SetMtu(const uint16_t mtu) {
    m_mtu = mtu;
    return true;
}

uint16_t CsaNetDevice::GetMtu() const {
    return m_mtu;
}

bool CsaNetDevice::IsLinkUp() const {
    return true;
}

void CsaNetDevice::AddLinkChangeCallback(Callback<void> callback) {}

bool CsaNetDevice::IsBroadcast() const {
    NS_LOG_FUNCTION (this);
    return true;
}

Address CsaNetDevice::GetBroadcast() const {
    NS_LOG_FUNCTION (this);
    return Address(Mac48Address::GetBroadcast());
}

bool CsaNetDevice::IsMulticast() const {
    return false;
}

Address CsaNetDevice::GetMulticast(Ipv4Address multicastGroup) const {
    return Address();
}

Address CsaNetDevice::GetMulticast(Ipv6Address addr) const {
    return Address();
}

bool CsaNetDevice::IsBridge() const {
    return false;
}

bool CsaNetDevice::IsPointToPoint() const {
    return false;
}

bool CsaNetDevice::Send(Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber) {
    NS_LOG_FUNCTION (this << packet << dest << protocolNumber);
    return m_mac->Enqueue(packet, Mac48Address::ConvertFrom(dest), protocolNumber);
}

bool CsaNetDevice::SendFrom(Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber) {
    NS_LOG_FUNCTION (this);
    return false;
}

Ptr<Node> CsaNetDevice::GetNode() const {
    NS_LOG_FUNCTION (this);
    return m_node;
}

void CsaNetDevice::SetNode(Ptr<Node> node) {
    NS_LOG_FUNCTION (this);
    m_node = node;
}

bool CsaNetDevice::NeedsArp() const {
    return false;
}

void CsaNetDevice::SetReceiveCallback(ReceiveCallback cb) {
    NS_LOG_FUNCTION (this);
    m_rxCallback = cb;
}

void CsaNetDevice::SetPromiscReceiveCallback(PromiscReceiveCallback cb) {
    NS_LOG_FUNCTION (this);
    m_promiscRxCallback = cb;
}

bool CsaNetDevice::SupportsSendFrom() const {
    return false;
}

Ptr<CsaMac> CsaNetDevice::GetMac() const {
    NS_LOG_FUNCTION (this);
    return m_mac;
}

void CsaNetDevice::SetMac(Ptr<CsaMac> mac) {
    NS_LOG_FUNCTION (this);
    m_mac = mac;
}

void CsaNetDevice::ForwardUp(Ptr<Packet> packet, uint16_t protocolNumber, Address sender) {
    NS_LOG_FUNCTION (this);
    m_rxCallback(this, packet, protocolNumber, sender);
}

void CsaNetDevice::DoInitialize() {
    m_mac->DoInitialize();
}

} // namespace ns3