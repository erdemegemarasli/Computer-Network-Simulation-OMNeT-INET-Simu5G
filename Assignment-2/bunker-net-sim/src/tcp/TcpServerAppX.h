#ifndef TCPSERVERAPPX_H_
#define TCPSERVERAPPX_H_

#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/socket/SocketMap.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"
#include "inet/common/INETMath.h"

#include <unordered_map>
#include "inet/networklayer/common/L3AddressTag_m.h"

#define TCPTHREAD_CLASS "bunker_net_sim.TcpThread"

namespace inet {

class TcpThread;

class TcpServerAppX : public ApplicationBase, public TcpSocket::ICallback {
    protected:
        typedef std::set<TcpThread *> ThreadSet;
        TcpSocket serverSocket;
        SocketMap socketMap;
        ThreadSet threadSet;
        int heartbeatThreshold;

        struct SurvivorData {
            L3Address ip;
            int bunkerId;
            simtime_t ts;
        };
        std::unordered_map<std::string, SurvivorData> survivorDatabase;

    protected:
        virtual void initialize(int stage) override;
        virtual int numInitStages() const override;
        virtual void handleMessageWhenUp(cMessage *msg) override;
        virtual void finish() override;
        virtual void refreshDisplay() const override;

        virtual void socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent) override;
        virtual void socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) override;
        virtual void socketEstablished(TcpSocket *socket) override;
        virtual void socketPeerClosed(TcpSocket *socket) override;
        virtual void socketClosed(TcpSocket *socket) override;
        virtual void socketFailure(TcpSocket *socket, int code) override;
        virtual void socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) override;
        virtual void socketDeleted(TcpSocket *socket) override;

        virtual void handleStartOperation(LifecycleOperation *operation) override;
        virtual void handleStopOperation(LifecycleOperation *operation) override;
        virtual void handleCrashOperation(LifecycleOperation *operation) override;

    public:
        TcpServerAppX();
        virtual ~TcpServerAppX();

    public:
        virtual void removeThread(TcpThread *thread);
        virtual void threadClosed(TcpThread *thread);

        friend class TcpThread;

    protected:
        virtual void sendDown(Packet *packet);
};

class INET_API TcpThread : public cSimpleModule, public TcpSocket::ICallback
{
  protected:
    TcpServerAppX *hostmod = nullptr;
    TcpSocket *sock; // TcpServerAppX socket

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
    TcpThread();
    virtual ~TcpThread();

    virtual void init(TcpServerAppX *hostmodule, TcpSocket *socket);
    virtual TcpSocket *getSocket();
    virtual TcpServerAppX *getHostModule();
    virtual void established();
    virtual void dataArrived(Packet *msg, bool urgent);
    virtual void timerExpired(cMessage *timer);
    virtual void peerClosed();
    virtual void failure(int code);
    virtual void statusArrived(TcpStatusInfo *status);
};

}

#endif
