#ifndef CSA_SOLVER_H
#define CSA_SOLVER_H

#include "csa-buffer.h"
#include <ns3/packet.h>

namespace ns3 {

class CsaSolver {
public:
    CsaSolver();
    virtual ~CsaSolver();

    void StartSolveAtSlot(uint32_t slot);
    void Solve();
    void Decode(PacketStatePtr packetState);
    void CleanSlot(uint32_t slot);

    
    typedef Callback<void, PacketStatePtr> ForwardUpCallback;
    void SetForwardUpCallback(ForwardUpCallback callback);

    typedef Callback<bool, SlotStatePtr, PacketStatePtr> IsClearCallback;
    void SetIsClearCallback(IsClearCallback callback);

    void SetBuffer(SlotStateBuffer* buffer);

private:
    SlotStateBuffer* m_buffer;
    ForwardUpCallback m_forwardUpCallback;
    IsClearCallback m_isClearCallback;
    std::queue<uint32_t> m_activeSlotQueue;
    uint32_t m_currentSlot;
};

}

#endif  // CSA_SOLVER_H