#include "csa-solver.h"
#include "csa-buffer.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("CsaSolver");

CsaSolver::CsaSolver() {
    NS_LOG_FUNCTION(this);
}

CsaSolver::~CsaSolver() {
    NS_LOG_FUNCTION(this);
}


void CsaSolver::StartSolveAtSlot(uint32_t slot) {
    NS_LOG_FUNCTION(this << slot);
    m_activeSlotQueue.push(slot);
    m_currentSlot = slot;
    Solve();
}

void CsaSolver::Solve() {
    while(!m_activeSlotQueue.empty()) {
        uint32_t slot = m_activeSlotQueue.front();
        m_activeSlotQueue.pop();
        if(slot > m_currentSlot) {
            continue;
        }
        SlotStatePtr slotState = m_buffer->GetSlotState(slot);
        // if the slot does not exist, skip
        if(slotState == nullptr) {
            continue;
        }
        CleanSlot(slot);
        // NS_LOG_DEBUG("Solving slot " << slot << " with " << slotState->packets.size() << " packets");
        if(slotState->packets.size() == 1) {
            NS_LOG_DEBUG("Slot has 1 packet: " << slot);
            // one fragment = fragment decoded
            // so, we increase the count for the packetState
            PacketStatePtr packetState = slotState->packets.begin()->first;
            packetState->count ++;
            // if decoding condition is met now, let's decode
            if(packetState->IsDecoded()) {
                // NS_LOG_DEBUG("Decoding packet: " << packetState->source);
                Decode(packetState);
            }
            // make sure to remove that packetState
            // or it will redo itself
            m_buffer->RemovePacketState(slot, packetState, false);
        }
    }
}

void CsaSolver::Decode(PacketStatePtr packetState) {
    // NS_LOG_FUNCTION(this << packetState);
    m_forwardUpCallback(packetState);
    
    for(auto slot: packetState->allSlots) {
        // SlotStatePtr slotState = m_buffer->GetSlotState(slot);
        // if(slotState) {
        //     NS_LOG_DEBUG("Slot become active: " << slot << " with packet count " << slotState->packets.size());
        // }
        // m_buffer->RemovePacketState(slot, packetState, false);
        m_activeSlotQueue.push(slot);
    }
}

void CsaSolver::CleanSlot(uint32_t slot) {
    NS_LOG_FUNCTION(this << slot << m_buffer->GetSlotState(slot)->packets.size());
    NS_ASSERT(m_buffer != nullptr);
    // auto buffer = m_buffer->m_slotBuffer;
    // NS_LOG_DEBUG("Buffer " << m_buffer->GetDefaultNoise());
    SlotStatePtr slotState = m_buffer->GetSlotState(slot);
    NS_ASSERT(slotState != nullptr);
    // NS_LOG_DEBUG("SlotState " << slotState->packets.size());
    bool isClear = true;
    for(auto const &[packetState, power] : slotState->packets) {
        // NS_LOG_DEBUG("Packet: " << packetState->source << " power: " << power << " noise: " << slotState->noise);
        if(packetState->IsDecoded()) {
            // NS_LOG_DEBUG("Removeing packet " << packetState->source << " because received");
            m_buffer->RemovePacketState(slot, packetState, false);
            isClear = false;
            break;
        }
        else if(!m_isClearCallback(slotState, packetState)) {
            // NS_LOG_DEBUG("Removeing packet " << packetState->source << " because noise: " << power << " < " << slotState->noise);
            m_buffer->RemovePacketState(slot, packetState, true);
            isClear = false;
            break;
        }
    }
    if(!isClear) {
        CleanSlot(slot);
    }
}




void CsaSolver::SetForwardUpCallback(ForwardUpCallback callback) {
    m_forwardUpCallback = callback;
}

void CsaSolver::SetIsClearCallback(IsClearCallback callback) {
    m_isClearCallback = callback;
}

void CsaSolver::SetBuffer(SlotStateBuffer* buffer) {
    m_buffer = buffer;
}

}   // namespace ns3