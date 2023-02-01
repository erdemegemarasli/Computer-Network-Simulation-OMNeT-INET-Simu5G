#ifndef WarnerAdminApp_H_
#define WarnerAdminApp_H_

#include <vector>

#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet {

extern template class ClockUserModuleMixin<ApplicationBase>;

class INET_API WarnerAdminApp : public ClockUserModuleMixin<ApplicationBase>, public UdpSocket::ICallback
{
  protected:
    enum SelfMsgKinds { START = 1, SEND, STOP };
    clocktime_t startTime;
    clocktime_t stopTime;

    std::string ownName;

    const char *packetName = nullptr;

    bool sendingEnabled;

    bool listeningEnabled;
    int localPort;

    int deviceAppPort_;
    char* sourceSimbolicAddress;            //Ue[x]
    char* deviceSimbolicAppAddress_;        //meHost.virtualisationInfrastructure
    std::string mecAppName;
    void sendStartMEWarningAlertApp();
    L3Address mecAppAddress_;
    int mecAppPort_;
    L3Address deviceAppAddress_;

    UdpSocket socket;
    ClockEvent *selfMsg = nullptr;

    int bunkerId;
    bool isCentralized;
    L3Address serverAddress;
    int serverPort;


    // statistics
    int numSent = 0;
    int numReceived = 0;
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    virtual void sendPacket();
    virtual void sendWarningMessage();

    virtual void processStart();
    virtual void processSend();
    virtual void processStop();

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;

  public:
    WarnerAdminApp() {}
    ~WarnerAdminApp();
};

} // namespace inet

#endif /* WarnerAdminApp_H_ */
