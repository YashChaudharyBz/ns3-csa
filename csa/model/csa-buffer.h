#ifndef CSA_BUFFER_H
#define CSA_BUFFER_H

#include "csa-header.h"
#include "csa-tag.h"
#include <ns3/mac48-address.h>
#include <ns3/core-module.h>
#include <memory>

namespace ns3 {


/*
 * ------------------------------
 * PacketState
 * ------------------------------
*/
struct PacketState {
    Mac48Address source;
    Mac48Address dest;
    uint16_t protocol;
    uint32_t seqNo;
    uint32_t length;
    uint8_t s;
    uint8_t n;
    std::vector<uint32_t> allSlots;
    uint8_t count;
    uint8_t tid;
    double txTime;

    PacketState(CsaHeader &header) {
        source = header.GetSource();
        dest = header.GetDestination();
        protocol = header.GetProtocolNumber();
        seqNo = header.GetSequenceNumber();
        length = header.GetLength();
        s = header.GetS();
        n = header.GetN();
        allSlots = header.GetAllSlots();
        count = 0;
        tid = 0;
        txTime = 0;
    }

    bool IsDecoded() const {
        return count >= s;
    }
};

typedef std::shared_ptr<PacketState> PacketStatePtr;

class PacketStateBuffer {
public:
    PacketStatePtr GetPacketState(Mac48Address source, uint32_t seqNo);
    PacketStatePtr AddPacketState(CsaHeader &header, SimulationTag simTag);
    void DeletePacketState(const PacketStatePtr &packetState);


    void SetTimeout(Time timeout);
    Time GetTimeout() const;

private:
    std::map<std::pair<Mac48Address, uint32_t>, PacketStatePtr> m_packetBuffer;
    Time m_timeout;
};


/*
 * ------------------------------
 * SlotState
 * ------------------------------
*/

struct SlotState {
    std::map<PacketStatePtr, double> packets;
    double noise;

    SlotState(double defaultNoise) {
        noise = defaultNoise;
    }
};

typedef std::shared_ptr<SlotState> SlotStatePtr;

class SlotStateBuffer {
public:
    SlotStateBuffer();
    SlotStatePtr GetSlotState(uint32_t slotNo);
    SlotStatePtr AddSlotState(uint32_t slotNo);
    void DeleteSlotState(uint32_t slotNo);
    void RemovePacketState(uint32_t slotNo, PacketStatePtr packetState, bool isNoise);

    void SetDefaultNoise(double noise);
    double GetDefaultNoise() const;
    void SetTimeout(Time timeout);
    Time GetTimeout() const;
    void SetEfficiency(double efficiency);
    double GetEfficiency() const;

    PacketStateBuffer m_packetStateBuffer;
private:
    std::map<uint32_t, SlotStatePtr> m_slotBuffer;
    double m_defaultNoise;
    Time m_timeout;
    double m_efficiency;
};


}   // namespace ns3

#endif  // CSA_BUFFER_H