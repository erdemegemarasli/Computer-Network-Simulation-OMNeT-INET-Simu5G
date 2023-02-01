#ifndef TCPCLIENTAPPX_H_
#define TCPCLIENTAPPX_H_

#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/socket/SocketMap.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"
#include "inet/common/INETMath.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/NodeStatus.h"

#include <unordered_map>
#include "inet/networklayer/common/L3AddressTag_m.h"


#define TCPCLIENTTHREAD_CLASS     "bunker_net_sim.TcpClientThread"

#define MSGKIND_CONNECT     0
#define MSGKIND_SEND        1

namespace inet {

class TcpClientThread;

class TcpClientAppX : public ApplicationBase, public TcpSocket::ICallback {
    protected:
        typedef std::set<TcpClientThread *> ThreadSet;
        TcpSocket serverSocket;
        SocketMap socketMap;
        ThreadSet threadSet;
        std::string ownName;

        struct addressEntry {
            L3Address ip;
            simtime_t ts;
        };

        std::unordered_map<std::string, addressEntry> addressBook;
        std::vector<std::string> possibleSurvivors;

        int learntFromMessages;
        std::unordered_set<std::string> textingSelectionAdded;
        std::vector<std::string> textingSelection;

    protected:
        void possibleSurvivorsInit();
        std::string randomSelectForLookup();
        std::string randomSelectForTexting();

        void sendLookupRequest();
        void sendLookupRequest(std::string survivor);

        void sendTextMessage();
        void sendTextMessage(std::string receiver);

    protected:
        virtual void initialize(int stage) override;
        virtual int numInitStages() const override;
        virtual void handleMessageWhenUp(cMessage *msg) override;
        virtual void finish() override;
        virtual void refreshDisplay() const override;

        virtual void socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent) override;
        virtual void socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) override;
        virtual void socketEstablished(TcpSocket *socket) override;
        virtual void socketPeerClosed(TcpSocket *socket_) override;
        virtual void socketClosed(TcpSocket *socket) override;
        virtual void socketFailure(TcpSocket *socket, int code) override;
        virtual void socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) override;
        virtual void socketDeleted(TcpSocket *socket) override;

        virtual void handleStartOperation(LifecycleOperation *operation) override;
        virtual void handleStopOperation(LifecycleOperation *operation) override;
        virtual void handleCrashOperation(LifecycleOperation *operation) override;

    public:
        TcpClientAppX();
        virtual ~TcpClientAppX();

    public:
        virtual void removeThread(TcpClientThread *thread);
        virtual void threadClosed(TcpClientThread *thread);

        friend class TcpClientThread;

    protected:
        virtual void sendDown(Packet *packet);

    // TcpAppBase
    protected:
        TcpSocket socket;

    protected:
        virtual void connect();
        virtual void close();
        virtual void sendPacket(Packet *pkt);
        virtual void handleTimer(cMessage *msg);

    // TcpBasicClientApp
    protected:
        cMessage *timeoutMsg = nullptr;
        simtime_t startTime;
        simtime_t stopTime;

    protected:
        virtual void sendRequest();
        virtual void rescheduleAfterOrDeleteTimer(simtime_t d, short int msgKind);

    protected:
        simsignal_t chunkLength;
        simsignal_t endtoenddelay;

        // statistics
        int numSent = 0;
        int numReceived = 0;

};

class INET_API TcpClientThread : public cSimpleModule, public TcpSocket::ICallback
{
  protected:
    TcpClientAppX *hostmod = nullptr;
    TcpSocket *sock; // TcpApp socket

    virtual void socketDataArrived(TcpSocket *socket, Packet *msg, bool urgent) override;
    virtual void socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) override;
    virtual void socketEstablished(TcpSocket *socket) override;
    virtual void socketPeerClosed(TcpSocket *socket) override;
    virtual void socketClosed(TcpSocket *socket) override;
    virtual void socketFailure(TcpSocket *socket, int code) override;
    virtual void socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) override;
    virtual void socketDeleted(TcpSocket *socket) override;

    virtual void refreshDisplay() const override;

  public:
    TcpClientThread();
    virtual ~TcpClientThread();

    virtual void init(TcpClientAppX *hostmodule, TcpSocket *socket);
    virtual TcpSocket *getSocket();
    virtual TcpClientAppX *getHostModule();
    virtual void established();
    virtual void dataArrived(Packet *msg, bool urgent);
    virtual void timerExpired(cMessage *timer);
    virtual void peerClosed();
    virtual void failure(int code);
    virtual void statusArrived(TcpStatusInfo *status);
};

}

#endif
