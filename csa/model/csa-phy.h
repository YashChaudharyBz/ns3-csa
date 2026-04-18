#ifndef CSA_PHY_H
#define CSA_PHY_H

#include "csa-buffer.h"
#include "csa-channel.h"
#include <ns3/core-module.h>
#include <ns3/packet.h>
#include <ns3/mobility-model.h>

namespace ns3 {

class CsaNetDevice;

/*
* Data Rate Standards and thresholds
*/

enum RateIndex : uint8_t {
    RATE_3Mbps   = 0,
    RATE_4_5Mbps = 1,
    RATE_6Mbps   = 2,
    RATE_9Mbps   = 3,
    RATE_12Mbps  = 4,
    RATE_18Mbps  = 5,
    RATE_24Mbps  = 6,
    RATE_27Mbps  = 7,
    RATE_UNKNOWN = 8
};

inline Ptr<const AttributeChecker> MakeRateIndexChecker () {
  return MakeEnumChecker (
    RATE_3Mbps,   "3Mbps",
    RATE_4_5Mbps, "4.5Mbps",
    RATE_6Mbps,   "6Mbps",
    RATE_9Mbps,   "9Mbps",
    RATE_12Mbps,  "12Mbps",
    RATE_18Mbps,  "18Mbps",
    RATE_24Mbps,  "24Mbps",
    RATE_27Mbps,  "27Mbps",
    RATE_UNKNOWN, "Unknown"
  );
}

inline const std::map<std::string, RateIndex> rateIndexMap = {
    {"3Mbps",   RATE_3Mbps},
    {"4.5Mbps", RATE_4_5Mbps},
    {"6Mbps",   RATE_6Mbps},
    {"9Mbps",   RATE_9Mbps},
    {"12Mbps",  RATE_12Mbps},
    {"18Mbps",  RATE_18Mbps},
    {"24Mbps",  RATE_24Mbps},
    {"27Mbps",  RATE_27Mbps},
};

struct McsRecord {
    std::string modulation;
    double dataRateMbps;
    double thresholdDb;

    double GetDataRateBps() const {
        return dataRateMbps * 1e6;
    }
};

inline const McsRecord csaRates[] = {
    {"BPSK 1/2",   3.0,   4.5},
    {"BPSK 3/4",   4.5,   7.0},  
    {"QPSK 1/2",   6.0,   9.0},
    {"QPSK 3/4",   9.0,   12.5}, 
    {"16-QAM 1/2", 12.0,  16.0}, 
    {"16-QAM 3/4", 18.0,  20.0}, 
    {"64-QAM 2/3", 24.0,  24.0},
    {"64-QAM 3/4", 27.0,  26.0}
};



class CsaPhy: public Object {
public:
    /*
    * Basic Methods
    */
    static TypeId GetTypeId();
    CsaPhy();
    virtual ~CsaPhy();

    /*
    * Util Methods
    */
    double GetSnr(SlotStatePtr slotState, PacketStatePtr packetState);
    bool IsClear(SlotStatePtr slotState, PacketStatePtr packetState);
    double GetSinr(SlotStatePtr slotState, PacketStatePtr packetState);
    bool IsDecodable(SlotStatePtr slotState, PacketStatePtr packetState);

    /*
    * Sending Methods
    */
    void StartTx(Ptr<Packet> packet);

    /*
    * Receiving Methods
    */
    void Receive(Ptr<Packet> pkt, double rxPower);

    /*
    * Getters
    */
    double GetTxPower() const;
    Ptr<CsaChannel> GetChannel() const;
    Ptr<CsaNetDevice> GetNetDevice() const;
    RateIndex GetDataRate() const;
    Ptr<MobilityModel> GetMobility() const;

    /*
    * Setters
    */
    void SetTxPower(double txPower);
    void SetChannel(Ptr<CsaChannel> channel);
    void SetNetDevice(Ptr<CsaNetDevice> netDevice);
    void SetDataRate(RateIndex dataRate);

    typedef Callback<void, Ptr<Packet>, double> RxCallback;
    void SetRxCallback(RxCallback callback);

private:
    double m_txPower;
    Ptr<CsaChannel> m_channel;
    Ptr<CsaNetDevice> m_netDevice;
    RateIndex m_dataRate;

    RxCallback m_rxCallback;
};

}   // namespace ns3

#endif  // CSA_PHY_H