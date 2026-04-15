#include "csa-channel.h"
#include "csa-phy.h"
#include "csa-net-device.h"
#include <ns3/mobility-model.h>
#include <ns3/node.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("CsaChannel");
NS_OBJECT_ENSURE_REGISTERED(CsaChannel);

TypeId CsaChannel::GetTypeId() {
    static TypeId tid = 
        TypeId("ns3::CsaChannel")
            .SetParent<Channel>()
            .SetGroupName("Csa")
            .AddConstructor<CsaChannel>()
            .AddAttribute("PropagationLossModel",
                          "A pointer to the propagation loss model attached to this channel.",
                          PointerValue(),
                          MakePointerAccessor(&CsaChannel::m_loss),
                          MakePointerChecker<PropagationLossModel>())
            .AddAttribute("PropagationDelayModel",
                          "A pointer to the propagation delay model attached to this channel.",
                          PointerValue(),
                          MakePointerAccessor(&CsaChannel::m_delay),
                          MakePointerChecker<PropagationDelayModel>());
    return tid;
}

CsaChannel::CsaChannel() {
    NS_LOG_FUNCTION(this);
}

CsaChannel::~CsaChannel() {
    NS_LOG_FUNCTION(this);
    m_phyList.clear();
}

std::size_t CsaChannel::GetNDevices() const {
    return m_phyList.size();
}

Ptr<NetDevice> CsaChannel::GetDevice(std::size_t i) const {
    return m_phyList[i]->GetNetDevice();
}

void CsaChannel::Add(Ptr<CsaPhy> phy) {
    NS_LOG_FUNCTION(this << phy);
    m_phyList.push_back(phy);
}

void CsaChannel::SetPropagationLossModel(const Ptr<PropagationLossModel> loss) {
    NS_LOG_FUNCTION(this << loss);
    m_loss = loss;
}

void CsaChannel::SetPropagationDelayModel(const Ptr<PropagationDelayModel> delay) {
    NS_LOG_FUNCTION(this << delay);
    m_delay = delay;
}

void CsaChannel::Send(Ptr<CsaPhy> sender, Ptr<Packet> pkt, double txPowerDbm) const {
    NS_LOG_FUNCTION(this << sender << pkt << txPowerDbm);
    Ptr<MobilityModel> senderMobility = sender->GetMobility();
    NS_ASSERT(senderMobility);
    for (PhyList::const_iterator i = m_phyList.begin(); i != m_phyList.end(); i++) {
        NS_ASSERT((*i)->GetNetDevice());
        if (sender != (*i)) {
            Ptr<MobilityModel> receiverMobility = (*i)->GetMobility();
            NS_ASSERT(receiverMobility);
            NS_ASSERT(m_delay != nullptr);
            Time delay = m_delay->GetDelay(senderMobility, receiverMobility);
            double rxPowerDbm = m_loss->CalcRxPower(txPowerDbm, senderMobility, receiverMobility);
            NS_LOG_DEBUG("propagation: txPower="
                         << txPowerDbm << "dbm, rxPower=" << rxPowerDbm << "dbm, "
                         << "distance=" << senderMobility->GetDistanceFrom(receiverMobility)
                         << "m, delay=" << delay);
            Ptr<Packet> packet = pkt->Copy();
            Simulator::ScheduleWithContext((*i)->GetNetDevice()->GetNode()->GetId(), 
                                delay, 
                                &CsaPhy::Receive, (*i), 
                                packet, rxPowerDbm);
        }
    }
}


} // namespace ns3