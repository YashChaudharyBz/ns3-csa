#ifndef CSA_CHANNEL_H
#define CSA_CHANNEL_H

#include <ns3/core-module.h>
#include <ns3/channel.h>
#include <ns3/propagation-loss-model.h>
#include <ns3/propagation-delay-model.h>
#include <ns3/packet.h>

namespace ns3 {

class CsaPhy;

class CsaChannel : public Channel {
public:
    static TypeId GetTypeId();
    CsaChannel();
    ~CsaChannel() override;

    std::size_t GetNDevices() const override;
    Ptr<NetDevice> GetDevice(std::size_t i) const override;
    void Add(Ptr<CsaPhy> phy);
    void SetPropagationLossModel(const Ptr<PropagationLossModel> loss);
    void SetPropagationDelayModel(const Ptr<PropagationDelayModel> delay);
    void Send(Ptr<CsaPhy> sender, Ptr<Packet> pkt, double txPowerDbm) const;

private:
    typedef std::vector<Ptr<CsaPhy>> PhyList;

    PhyList m_phyList;
    Ptr<PropagationLossModel> m_loss;
    Ptr<PropagationDelayModel> m_delay;
};

}   // namespace ns3

#endif  // CSA_CHANNEL_H