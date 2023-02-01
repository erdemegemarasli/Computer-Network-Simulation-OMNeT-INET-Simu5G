//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "TcpHeartBeatApp.h"

#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"

#include "inet/applications/tcpapp/GenericAppMsg_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "BunkerPacket_m.h"
#include "inet/networklayer/common/FragmentationTag_m.h"



namespace inet {

#define MSGKIND_CONNECT    0
#define MSGKIND_SEND       1

Define_Module(TcpHeartBeatApp);


TcpHeartBeatApp::~TcpHeartBeatApp()
{
    cancelAndDelete(timeoutMsg);
}

void TcpHeartBeatApp::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {

        ownName = std::string(getParentModule()->getFullName());
        startTime = par("startTime");
        stopTime = par("stopTime");
        if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
            throw cRuntimeError("Invalid startTime/stopTime parameters");
        timeoutMsg = new cMessage("timer");
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        const char *localAddress = par("localAddress");
        int localPort = par("localPort");
        socket.bind(*localAddress ? L3AddressResolver().resolve(localAddress) : L3Address(), localPort);

        socket.setCallback(this);
        socket.setOutputGate(gate("socketOut"));
    }
}

void TcpHeartBeatApp::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage())
        handleTimer(msg);
    else
        socket.processMessage(msg);
}

void TcpHeartBeatApp::connect()
{
    if (socket.getState() == TcpSocket::CONNECTED || socket.getState() == TcpSocket::CONNECTING){
        cancelEvent(timeoutMsg);
        // start another session after a delay
        if (timeoutMsg) {

            simtime_t d = par("sendInterval");
            rescheduleAfterOrDeleteTimer(d, MSGKIND_CONNECT);
        }
    }
    else{
        // we need a new connId if this is not the first connection
        socket.renewSocket();

        const char *localAddress = par("localAddress");
        int localPort = par("localPort");
        socket.bind(*localAddress ? L3AddressResolver().resolve(localAddress) : L3Address(), localPort);

        // connect
        const char *connectAddress = par("connectAddress");
        int connectPort = par("connectPort");

        L3Address destination;
        L3AddressResolver().tryResolve(connectAddress, destination);
        if (destination.isUnspecified()) {
            EV_ERROR << "Connecting to " << connectAddress << " port=" << connectPort << ": cannot resolve destination address\n";
        }
        else {
            EV_INFO << "Connecting to " << connectAddress << "(" << destination << ") port=" << connectPort << endl;

            socket.connect(destination, connectPort);
        }
    }

}

void TcpHeartBeatApp::close()
{
    EV_INFO << "issuing CLOSE command\n";
    socket.close();
    //cancelEvent(timeoutMsg);
}

void TcpHeartBeatApp::sendPacket(Packet *msg)
{
    socket.send(msg);
}

void TcpHeartBeatApp::refreshDisplay() const
{
    ApplicationBase::refreshDisplay();
    getDisplayString().setTagArg("t", 0, TcpSocket::stateName(socket.getState()));
}

void TcpHeartBeatApp::socketEstablished(TcpSocket *)
{
    EV_INFO << "connected\n";

    sendRequest();

}

void TcpHeartBeatApp::socketDataArrived(TcpSocket *socket, Packet *msg, bool urgent)
{
    delete msg;
}

void TcpHeartBeatApp::socketPeerClosed(TcpSocket *socket_)
{
    ASSERT(socket_ == &socket);
    // close the connection (if not already closed)
    if (socket.getState() == TcpSocket::PEER_CLOSED) {
        EV_INFO << "remote TCP closed, closing here as well\n";
        close();
    }
}

void TcpHeartBeatApp::socketClosed(TcpSocket *)
{
    // *redefine* to start another session etc.
    EV_INFO << "connection closed\n";


}

void TcpHeartBeatApp::socketFailure(TcpSocket *, int code)
{
    // subclasses may override this function, and add code try to reconnect after a delay.
    EV_WARN << "connection broken"<<endl;


    // reconnect after a delay
    if (timeoutMsg) {
        simtime_t d = par("reconnectInterval");
        rescheduleAfterOrDeleteTimer(d, MSGKIND_CONNECT);
    }
}

void TcpHeartBeatApp::finish()
{
}

void TcpHeartBeatApp::handleStartOperation(LifecycleOperation *operation)
{
    simtime_t now = simTime();
    simtime_t start = std::max(startTime, now);
    simtime_t d = par("sendInterval");
    if (timeoutMsg && ((stopTime < SIMTIME_ZERO) || (start < stopTime) || (start == stopTime && startTime == stopTime))) {
        timeoutMsg->setKind(MSGKIND_CONNECT);
        scheduleAt(d, timeoutMsg);
    }
}

void TcpHeartBeatApp::handleStopOperation(LifecycleOperation *operation)
{
    cancelEvent(timeoutMsg);
    if (socket.getState() == TcpSocket::CONNECTED || socket.getState() == TcpSocket::CONNECTING || socket.getState() == TcpSocket::PEER_CLOSED)
        close();
}

void TcpHeartBeatApp::handleCrashOperation(LifecycleOperation *operation)
{
    cancelEvent(timeoutMsg);
    if (operation->getRootModule() != getContainingNode(this))
        socket.destroy();
}

void TcpHeartBeatApp::sendRequest()
{

//    const auto& payload = makeShared<GenericAppMsg>();
//    Packet *packet = new Packet("data");
//    payload->setChunkLength(B(10));
//    payload->setExpectedReplyLength(B(10));
//    payload->setServerClose(false);
//    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
//    packet->insertAtBack(payload);
//
//
//    sendPacket(packet);
//
//
//    close();


//    int k = intrand(100);
//
//    if (k < 50) {
//
//        EV_INFO << "HEARTBEAT REQUEST CREATED BY: " << ownName;
//        Packet *packet = new Packet("HeartBeatData");
//        packet->addTag<FragmentationReq>()->setDontFragment(true);
//
//        const auto& payload = makeShared<BunkerPacket>();
//        payload->setChunkLength(B(20));
//        payload->setType(0);
//        payload->setSurvivorName(ownName.c_str());
//
//        payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
//        packet->insertAtBack(payload);
//
//        sendPacket(packet);
//
//        close();
//    }

    EV_INFO << "HEARTBEAT REQUEST CREATED BY: " << ownName;
    Packet *packet = new Packet("HeartBeatData");
    packet->addTag<FragmentationReq>()->setDontFragment(true);

    const auto& payload = makeShared<BunkerPacket>();
    payload->setChunkLength(B(20));
    payload->setType(0);
    payload->setSurvivorName(ownName.c_str());

    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    packet->insertAtBack(payload);

    sendPacket(packet);

    // start another session after a delay
    cancelEvent(timeoutMsg);
    if (timeoutMsg) {
        simtime_t d = par("sendInterval");
        rescheduleAfterOrDeleteTimer(d, MSGKIND_CONNECT);
    }

}

void TcpHeartBeatApp::handleTimer(cMessage *msg)
{
    switch (msg->getKind()) {
        case MSGKIND_CONNECT:
            connect(); // active OPEN
            break;

        case MSGKIND_SEND:
            sendRequest();
            break;

        default:
            throw cRuntimeError("Invalid timer msg: kind=%d", msg->getKind());
    }
}

void TcpHeartBeatApp::rescheduleAfterOrDeleteTimer(simtime_t d, short int msgKind)
{
    if (stopTime < SIMTIME_ZERO || d < stopTime) {
        timeoutMsg->setKind(msgKind);
        rescheduleAfter(d, timeoutMsg);
    }
    else {
        cancelAndDelete(timeoutMsg);
        timeoutMsg = nullptr;
    }
}

} // namespace inet

