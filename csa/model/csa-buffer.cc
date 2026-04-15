#include "csa-buffer.h"
#include "csa-tag.h"

namespace ns3 {

/*
 * ------------------------------
 * PacketState
 * ------------------------------
*/
PacketStatePtr PacketStateBuffer::GetPacketState(Mac48Address source, uint32_t seqNo) {
    auto it = m_packetBuffer.find({source, seqNo});
    if (it != m_packetBuffer.end()) {
        return it->second;
    }
    return nullptr;
}

PacketStatePtr PacketStateBuffer::AddPacketState(CsaHeader &header, SimulationTag simTag) {
    PacketStatePtr packetStatePtr = std::make_shared<PacketState>(header);
    packetStatePtr->tid = simTag.GetTid();
    packetStatePtr->txTime = simTag.GetTxTime();
    m_packetBuffer[{header.GetSource(), header.GetSequenceNumber()}] = packetStatePtr;
    Simulator::Schedule(m_timeout, &PacketStateBuffer::DeletePacketState, this, packetStatePtr);
    return packetStatePtr;
}

void PacketStateBuffer::DeletePacketState(const PacketStatePtr &packetState) {
    if(packetState == nullptr) {
        return;
    }
    m_packetBuffer.erase({packetState->source, packetState->seqNo});
}


void PacketStateBuffer::SetTimeout(Time timeout) {
    m_timeout = timeout;
}

Time PacketStateBuffer::GetTimeout() const {
    return m_timeout;
}


/*
 * ------------------------------
 * SlotState
 * ------------------------------
*/

SlotStateBuffer::SlotStateBuffer():
    m_defaultNoise(0.0),
    m_timeout(Seconds(0)) {}

SlotStatePtr SlotStateBuffer::GetSlotState(uint32_t slotNo) {
    auto it = m_slotBuffer.find(slotNo);
    if (it != m_slotBuffer.end()) {
        return it->second;
    }
    return nullptr;
}

SlotStatePtr SlotStateBuffer::AddSlotState(uint32_t slotNo) {
    SlotStatePtr slotStatePtr = std::make_shared<SlotState>(m_defaultNoise);
    m_slotBuffer[slotNo] = slotStatePtr;
    Simulator::Schedule(m_timeout, &SlotStateBuffer::DeleteSlotState, this, slotNo);
    return slotStatePtr;
}

void SlotStateBuffer::DeleteSlotState(uint32_t slotNo) {
    auto it = m_slotBuffer.find(slotNo);
    if (it == m_slotBuffer.end()) {
        return;
    }
    m_slotBuffer.erase(it);
}

void SlotStateBuffer::RemovePacketState(uint32_t slotNo, PacketStatePtr packetState, bool isNoise) {
    SlotStatePtr slot = GetSlotState(slotNo);
    if (!slot || !packetState) {
        return;
    }

    auto it = slot->packets.find(packetState);
    if (it != slot->packets.end()) {
        if (m_efficiency < 1.0) {
            double powerMw = std::pow(10.0, it->second / 10.0);
            double noiseMw = std::pow(10.0, slot->noise / 10.0);
            double residualMw;
            if(isNoise) {
                residualMw = powerMw;
            }
            else {
                residualMw = powerMw * (1.0 - m_efficiency);
            }
            slot->noise = 10.0 * std::log10(noiseMw + residualMw);
        }
        slot->packets.erase(it);
    }
}


/* ------------------------------
 *      Getters and Setters
 * ------------------------------
 */
void SlotStateBuffer::SetDefaultNoise(double noise) {
    m_defaultNoise = noise;
}

double SlotStateBuffer::GetDefaultNoise() const {
    return m_defaultNoise;
}

void SlotStateBuffer::SetTimeout(Time timeout) {
    m_timeout = timeout;
    m_packetStateBuffer.SetTimeout(timeout);
}

Time SlotStateBuffer::GetTimeout() const {
    return m_timeout;
}

void SlotStateBuffer::SetEfficiency(double efficiency) {
    m_efficiency = efficiency;
}

double SlotStateBuffer::GetEfficiency() const {
    return m_efficiency;
}

}   // namespace ns3