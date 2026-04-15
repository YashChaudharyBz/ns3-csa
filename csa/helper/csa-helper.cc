#include "csa-helper.h"
#include "ns3/csa-net-device.h"

namespace ns3 {

CsaHelper::CsaHelper() {
    m_channel.SetTypeId("ns3::CsaChannel");
    m_phy.SetTypeId("ns3::CsaPhy");
    m_mac.SetTypeId("ns3::CsaMac");
}

void CsaHelper::SetPhyAttribute(std::string name, const AttributeValue &value) {
    m_phy.Set(name, value);
}

void CsaHelper::SetMacAttribute(std::string name, const AttributeValue &value) {
    m_mac.Set(name, value);
}

void CsaHelper::SetChannelAttribute(std::string name, const AttributeValue &value) {
    m_channel.Set(name, value);
}

void CsaHelper::SetStrategies(std::vector<std::pair<uint8_t, uint8_t>> strategies) {
    m_strategies = strategies;
}

void CsaHelper::SetProbabilities(std::vector<double>& probabilities) {
    m_probabilities = probabilities;
}

NetDeviceContainer CsaHelper::Install(NodeContainer nodes) {
    NS_ASSERT_MSG(m_strategies.size() > 0, "Strategies not set.");
    NS_ASSERT_MSG(m_probabilities.size() > 0, "Probabilities not set.");
    NetDeviceContainer devices;
    Ptr<CsaChannel> channel = m_channel.Create<CsaChannel>();
    int64_t stream = 0;
    for(NodeContainer::Iterator i = nodes.Begin(); i != nodes.End(); ++i) {
        Ptr<Node> node = *i;
        Ptr<CsaNetDevice> netDevice = Create<CsaNetDevice>();
        Ptr<CsaPhy> phy = m_phy.Create<CsaPhy>();
        Ptr<CsaMac> mac = m_mac.Create<CsaMac>();

        node->AddDevice(netDevice);

        netDevice->SetNode(node);
        netDevice->SetMtu(5000);
        netDevice->SetMac(mac);
        netDevice->SetAddress(Mac48Address::Allocate());

        phy->SetChannel(channel);
        phy->SetNetDevice(netDevice);
        phy->SetRxCallback(MakeCallback(&CsaMac::Receive, mac));

        channel->Add(phy);

        mac->SetStrategy(m_strategies, m_probabilities);
        mac->SetPhy(phy);
        mac->SetNetDevice(netDevice);
        mac->SetStream(stream++);
        mac->SetupSolver();

        devices.Add(netDevice);
    }
    return devices;
}

}
