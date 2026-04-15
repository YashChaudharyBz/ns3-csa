#ifndef CSA_MAC_H
#define CSA_MAC_H

#include "csa-buffer.h"
#include "csa-header.h"
#include "csa-phy.h"
#include "csa-solver.h"
#include <ns3/object.h>
#include <ns3/queue.h>
#include <cstdint>

namespace ns3 {

class CsaNetDevice;

class CsaMac: public Object {
public:
    /*
    * Basic Methods
    */
    static TypeId GetTypeId();
    CsaMac();
    virtual ~CsaMac();
    void DoInitialize() override;

    /*
    * Util Methods
    */
    Time NextSlotTime() const;
    uint32_t GetCurrentSlotNumber() const;
    uint32_t GetGeometricValue() const;
    std::vector<uint32_t> GetNSlots() const;
    Mac48Address GetAddress() const;
    void SetupSolver();


    /*
    * Sending Methods
    */
    void WakeUp();
    void Sleep();
    bool Enqueue(Ptr<Packet> packet, Mac48Address dest, uint16_t protocolNumber);
    void Dequeue();
    CsaHeader PrepareHeader(Ptr<Packet> packet, std::vector<uint32_t> &allSlots);
    void SendPacket(Ptr<Packet> packet);
    void SendAtSlot(Ptr<Packet> packet, uint32_t slot);
    void StartSend(Ptr<Packet> packet);
    void EndSend();


    /*
    * Receiving Methods
    */
    void Receive(Ptr<Packet> packet, double rxPower);
    void Solve();
    void EndSolve();
    void ForwardUp(PacketStatePtr packetState);


    /*
    * Getters
    */
    Ptr<CsaPhy> GetPhy() const;
    Ptr<CsaNetDevice> GetNetDevice() const;
    SlotStateBuffer* GetBuffer() const;

    /*
    * Setters
    */
    void SetPhy(Ptr<CsaPhy> phy);
    void SetNetDevice(Ptr<CsaNetDevice> netDevice);
    void SetBuffer(SlotStateBuffer* buffer);
    void SetStream(int64_t stream);
    void SetStrategy(const std::vector<std::pair<uint8_t, uint8_t>> &strategies, const std::vector<double> &probabilities);

    typedef void (* TxCallback)(uint32_t sender, uint8_t tid);
    typedef void (* RxCallback)(uint32_t sender, uint32_t receiver, double txTime, double rxTime, uint8_t tid);

    // void DebugBuffer();

private:
    /*
    * Attributes variables
    */
    Time m_slotDuration;
    uint32_t m_queueSize;
    double m_sendProb;
    double m_defaultNoise;
    Time m_timeout;
    double m_efficiency;
    

    /*
    * Initialized variales
    */
    uint8_t m_s;
    uint8_t m_n;
    CsaSolver m_solver;
    Ptr<CsaPhy> m_phy;
    Ptr<CsaNetDevice> m_netDevice;
    SlotStateBuffer* m_buffer;
    Ptr<Queue<Packet>> m_queue;
    uint32_t m_seqNo;
    Ptr<UniformRandomVariable> m_rng;
    bool m_isSleeping;
    bool m_isTx;
    bool m_solvingAsked;
    Time m_epsilonTime;
    int64_t m_stream;

    TracedCallback<uint32_t, uint8_t> m_enqueueTrace;
    TracedCallback<uint32_t, uint32_t, double, double, uint8_t> m_receiveTrace;
};

}   // namespace ns3

#endif  // CSA_MAC_H