#ifndef __INET_WARNERAPP_H
#define __INET_WARNERAPP_H

#include <vector>

#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet {

extern template class ClockUserModuleMixin<ApplicationBase>;

class INET_API WarnerApp : public ClockUserModuleMixin<ApplicationBase>, public UdpSocket::ICallback
{
  protected:
    enum SelfMsgKinds { START = 1, SEND = 2 };

    int localPort;
    int destPort;

    UdpSocket socket;
    ClockEvent *selfMsg = nullptr;

    int mecAppId;
    double requiredRam;
    double requiredDisk;
    double requiredCpu;

    L3Address mp1Address;
    int mp1Port;

    L3Address serverAddress;
    int serverPort;

    std::string warningMessage;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;

    virtual void sendPacket();
    virtual void setSocketOptions();

    virtual void processStart();
    virtual void processSend();

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void socketDataArrived(UdpSocket *sock, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;

  public:
    WarnerApp() {}
    ~WarnerApp();
};

}

#endif
