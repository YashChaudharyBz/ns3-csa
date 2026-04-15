#ifndef CSA_HELPER_H
#define CSA_HELPER_H

#include "ns3/csa-channel.h"
#include "ns3/csa-phy.h"
#include <ns3/network-module.h>

namespace ns3 {

class CsaHelper {
public:
    CsaHelper();

    void SetPhyAttribute(std::string name, const AttributeValue &value);
    void SetMacAttribute(std::string name, const AttributeValue &value);
    void SetChannelAttribute(std::string name, const AttributeValue &value);
    void SetStrategies(std::vector<std::pair<uint8_t, uint8_t>> strategies);
    void SetProbabilities(std::vector<double>& probabilities);

    NetDeviceContainer Install(NodeContainer nodes);
private:
    ObjectFactory m_phy;
    ObjectFactory m_mac;
    ObjectFactory m_channel;

    std::vector<std::pair<uint8_t, uint8_t>> m_strategies;
    std::vector<double> m_probabilities;
};

}   // namespace ns3

#endif  // CSA_HELPER_H
