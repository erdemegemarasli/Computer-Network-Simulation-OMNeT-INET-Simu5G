#include "TcpServerAppX.h"

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/stlutils.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/networklayer/common/L3AddressResolver.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/packet/Packet_m.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"

#include "BunkerPacket_m.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

namespace inet {

Define_Module(TcpServerAppX);
Define_Module(TcpThread);

TcpServerAppX::TcpServerAppX() {
}

TcpServerAppX::~TcpServerAppX() {
    socketMap.deleteSockets();
}

void TcpServerAppX::initialize(int stage) {

    ApplicationBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        heartbeatThreshold = par("heartbeatThreshold");
    }

}

int TcpServerAppX::numInitStages() const {
    return NUM_INIT_STAGES;
}

void TcpServerAppX::handleMessageWhenUp(cMessage *msg) {
    if (msg->isSelfMessage()) {
        TcpThread *thread = (TcpThread *)msg->getContextPointer();
        if (!contains(threadSet, thread))
            throw cRuntimeError("Invalid thread pointer in the timer (msg->contextPointer is invalid)");
        thread->timerExpired(msg);
    }
    else {
        TcpSocket *socket = check_and_cast_nullable<TcpSocket *>(socketMap.findSocketFor(msg));
        if (socket)
            socket->processMessage(msg);
        else if (serverSocket.belongsToSocket(msg))
            serverSocket.processMessage(msg);
        else {
            EV_ERROR << "message " << msg->getFullName() << "(" << msg->getClassName() << ") arrived for unknown socket \n";
            delete msg;
        }
    }
}

void TcpServerAppX::finish() {
    // remove and delete threads
    while (!threadSet.empty())
        removeThread(*threadSet.begin());
}

void TcpServerAppX::refreshDisplay() const {
    ApplicationBase::refreshDisplay();

    char buf[32];
    sprintf(buf, "%d threads", socketMap.size());
    getDisplayString().setTagArg("t", 0, buf);
}

void TcpServerAppX::socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent) {

}

void TcpServerAppX::socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) {
    // new TCP connection -- create new socket object and server process
    TcpSocket *newSocket = new TcpSocket(availableInfo);
    newSocket->setOutputGate(gate("socketOut"));

    cModuleType *moduleType = cModuleType::get(TCPTHREAD_CLASS);
    char name[80];
    sprintf(name, "thread_%i", newSocket->getSocketId());
    TcpThread *proc = check_and_cast<TcpThread *>(moduleType->create(name, this));
    proc->finalizeParameters();
    proc->callInitialize();

    newSocket->setCallback(proc);
    proc->init(this, newSocket);

    socketMap.addSocket(newSocket);
    threadSet.insert(proc);

    socket->accept(availableInfo->getNewSocketId());
}

void TcpServerAppX::socketEstablished(TcpSocket *socket) {

}

void TcpServerAppX::socketPeerClosed(TcpSocket *socket) {

}

void TcpServerAppX::socketClosed(TcpSocket *socket) {
    if (operationalState == State::STOPPING_OPERATION && threadSet.empty() && !serverSocket.isOpen())
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void TcpServerAppX::socketFailure(TcpSocket *socket, int code) {

}

void TcpServerAppX::socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) {

}

void TcpServerAppX::socketDeleted(TcpSocket *socket) {
    socketMap.removeSocket(socket);
}

void TcpServerAppX::handleStartOperation(LifecycleOperation *operation) {
    int listenPort = par("listenPort");

    serverSocket.setOutputGate(gate("socketOut"));
    serverSocket.setCallback(this);
    serverSocket.bind(listenPort);
    serverSocket.listen();
}

void TcpServerAppX::handleStopOperation(LifecycleOperation *operation) {
    for (auto thread : threadSet)
        thread->getSocket()->close();
    serverSocket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void TcpServerAppX::handleCrashOperation(LifecycleOperation *operation) {
    // remove and delete threads
    while (!threadSet.empty()) {
        auto thread = *threadSet.begin();
        // TODO destroy!!!
        thread->getSocket()->close();
        removeThread(thread);
    }
    // TODO always?
    if (operation->getRootModule() != getContainingNode(this))
        serverSocket.destroy();
}

void TcpServerAppX::removeThread(TcpThread *thread) {
    // remove socket
    socketMap.removeSocket(thread->getSocket());
    threadSet.erase(thread);

    // remove thread object
    thread->deleteModule();
}

void TcpServerAppX::threadClosed(TcpThread *thread) {
    // remove socket
    socketMap.removeSocket(thread->getSocket());
    threadSet.erase(thread);

    socketClosed(thread->getSocket());

    // remove thread object
    thread->deleteModule();
}

// Comes from EchoApp.
void TcpServerAppX::sendDown(Packet *packet) {
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::tcp);
    packet->getTag<SocketReq>();
    send(packet, "socketOut");
}

// ---- Tcp Thread ----
TcpThread::TcpThread() {
    sock = nullptr;
    hostmod = nullptr;
}

TcpThread::~TcpThread() {
    delete sock;
}

void TcpThread::socketDataArrived(TcpSocket *socket, Packet *msg, bool urgent) {
    dataArrived(msg, urgent);
}

void TcpThread::socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) {
    socket->accept(availableInfo->getNewSocketId());
}

void TcpThread::socketEstablished(TcpSocket *socket) {
    established();
}

void TcpThread::socketPeerClosed(TcpSocket *socket) {
    peerClosed();
}

void TcpThread::socketClosed(TcpSocket *socket) {
    hostmod->threadClosed(this);
}

void TcpThread::socketFailure(TcpSocket *socket, int code) {
    failure(code);
}

void TcpThread::socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) {
    statusArrived(status);
}

void TcpThread::socketDeleted(TcpSocket *socket) {
    if (socket == sock) {
        sock = nullptr;
        hostmod->socketDeleted(socket);
    }
}

void TcpThread::refreshDisplay() const {
    getDisplayString().setTagArg("t", 0, TcpSocket::stateName(sock->getState()));
}

void TcpThread::init(TcpServerAppX *hostmodule, TcpSocket *socket) {
    hostmod = hostmodule;
    sock = socket;
}

TcpSocket* TcpThread::getSocket() {
    return sock;
}

TcpServerAppX* TcpThread::getHostModule() {
    return hostmod;
}

void TcpThread::established() {

}

void TcpThread::dataArrived(Packet *msg, bool urgent) {

    EV_INFO << "DATA ARRIVED SERVER" << endl;
    int64_t rcvdBytes = msg->getByteLength();

    if (sock->getState() == TcpSocket::CONNECTED) {

        L3Address remoteAddress = sock->getRemoteAddress();
        int srcPort = sock->getRemotePort();

        auto currentTime = simTime();

        auto data = msg->peekData<BunkerPacket>();
        auto packetType = data->getType();
        auto survivorName = data->getSurvivorName();

        if (packetType == 0) { // Heartbeat Packet
            hostmod->survivorDatabase[survivorName].bunkerId = 0;
            hostmod->survivorDatabase[survivorName].ip = remoteAddress;
            hostmod->survivorDatabase[survivorName].ts = simTime();
            EV_INFO << "--- SERVER: HEARTBEAT RECEIVED FROM: " << survivorName << endl;
        }
        else if (packetType == 1) { // Lookup Request
            Packet *packet = new Packet("Lookup Response", TCP_C_SEND);

            int socketId = msg->getTag<SocketInd>()->getSocketId();
            packet->addTag<SocketReq>()->setSocketId(socketId);


            const auto& payload = makeShared<BunkerPacket>();
            payload->setChunkLength(B(30));
            payload->setType(2);  // Lookup Response

            if (hostmod->survivorDatabase.find(survivorName) == hostmod->survivorDatabase.end()) { // Not Exists
                payload->setSurvivorName(survivorName);
                payload->setBunkerId(-1);
                EV_INFO << "--- SERVER: SURVIVOR NOT FOUND: " << survivorName << endl;
            }
            else { // Exists
                if (currentTime - hostmod->survivorDatabase[survivorName].ts > hostmod->heartbeatThreshold) { // Last heartbeat is too old
                    hostmod->survivorDatabase.erase(survivorName);
                    payload->setSurvivorName(survivorName);
                    payload->setBunkerId(-1);
                    EV_INFO << "--- SERVER: SURVIVOR NOT FOUND (LAST HEARTBEAT TIMEOUT): " << survivorName << endl;
                }
                else {  // Survivor Found
                    payload->setSurvivorName(survivorName);
                    payload->setBunkerId(hostmod->survivorDatabase[survivorName].bunkerId);
                    payload->setIp(hostmod->survivorDatabase[survivorName].ip);
                    EV_INFO << "--- SERVER: SURVIVOR FOUND: " << survivorName << endl;
                }
            }

            packet->insertAtBack(payload);
            hostmod->sendDown(packet);
            //socket->sendTo(packet, remoteAddress, srcPort);
        }

//        Packet *outPkt = new Packet(msg->getName(), TCP_C_SEND);
//        int socketId = msg->getTag<SocketInd>()->getSocketId();
//        outPkt->addTag<SocketReq>()->setSocketId(socketId);
//
//        long outByteLen = rcvdBytes;
//
//        if (outByteLen < 1)
//            outByteLen = 1;
//
//        int64_t len = 0;
//        for (; len + rcvdBytes <= outByteLen; len += rcvdBytes) {
//            outPkt->insertAtBack(msg->peekDataAt(B(0), B(rcvdBytes)));
//        }
//        if (len < outByteLen)
//            outPkt->insertAtBack(msg->peekDataAt(B(0), B(outByteLen - len)));
//
//        hostmod->sendDown(outPkt);
    }

    delete msg;

    getSocket()->close();
}

void TcpThread::timerExpired(cMessage *timer) {
//    Packet *pkt = check_and_cast<Packet *>(timer);
//    pkt->setContextPointer(nullptr);
//    hostmod->sendDown(pkt);
}

void TcpThread::peerClosed() {
    getSocket()->close();
}

void TcpThread::failure(int code) {
    hostmod->removeThread(this);
}

void TcpThread::statusArrived(TcpStatusInfo *status) {
}

}
