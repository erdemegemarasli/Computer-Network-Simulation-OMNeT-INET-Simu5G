#include "TcpClientAppX.h"

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/stlutils.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/networklayer/common/L3AddressResolver.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/Packet_m.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/applications/tcpapp/GenericAppMsg_m.h"

#include "BunkerPacket_m.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

#include <algorithm>
#include <random>

namespace inet {

Define_Module(TcpClientAppX);
Define_Module(TcpClientThread);

TcpClientAppX::TcpClientAppX() {
}

TcpClientAppX::~TcpClientAppX() {
    socketMap.deleteSockets();
    cancelAndDelete(timeoutMsg);
}

void TcpClientAppX::possibleSurvivorsInit() {
    std::string host_prefix = std::string(par("host_prefix"));
    int bunker_number = par("bunker_number");
    int host_number = par("host_number");
    int nonExist_host_number = par("nonExist_host_number");

    int total_host_number = bunker_number * host_number;

    std::vector<std::string> hosts;

    for (int i = 0; i < total_host_number; i++) {
        std::stringstream tmp;
        tmp << host_prefix << "[" << i << "]";
        std::string result;
        tmp >> result;

        if (strcmp(result.c_str(), ownName.c_str()) != 0) {
            hosts.push_back(result);
        }
    }

    for (int i = 0; i < nonExist_host_number; i++) {
        std::stringstream tmp;
        tmp << "nonExist" << "[" << i << "]";
        std::string result;
        tmp >> result;

        hosts.push_back(result);
    }

    int size = intrand(hosts.size());
    EV_INFO << "-------" << ownName << "'s Survivor Search List-------" << endl;

    std::shuffle(std::begin(hosts), std::end(hosts), std::random_device());

    for (int i = 0; i < size; i++) {
        possibleSurvivors.push_back(hosts[i]);

        if (textingSelectionAdded.find(hosts[i]) == textingSelectionAdded.end()) {
            textingSelection.push_back(hosts[i]);
            textingSelectionAdded.insert(hosts[i]);
        }

        EV_INFO << hosts[i] << endl;
    }
}

void TcpClientAppX::initialize(int stage) {
    ApplicationBase::initialize(stage);


    if (stage == INITSTAGE_LOCAL) {
        ownName = std::string(getParentModule()->getFullName());
        chunkLength = registerSignal("chunkLength");
        possibleSurvivorsInit();
        learntFromMessages = 0;

        endtoenddelay = registerSignal("endtoenddelay");


        startTime = par("startTime");
        stopTime = par("stopTime");
        if (stopTime >= SIMTIME_ZERO && stopTime < startTime) {
            throw cRuntimeError("Invalid startTime/stopTime parameters");
        }
        timeoutMsg = new cMessage("timer");
        WATCH(numSent);
        WATCH(numReceived);
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        const char *localAddress = par("localAddress");
        int localPort = par("localPort");
        socket.bind(*localAddress ? L3AddressResolver().resolve(localAddress) : L3Address(), localPort);

        socket.setCallback(this);
        socket.setOutputGate(gate("socketOut"));
    }
}

std::string TcpClientAppX::randomSelectForLookup() {
    std::string survivor = "";
    do {
        if (possibleSurvivors.size() + learntFromMessages == addressBook.size()) {
            survivor = "";
            break;
        }

        int k = intrand(possibleSurvivors.size());
        survivor = possibleSurvivors[k];
    } while (addressBook.find(survivor) != addressBook.end());

    return survivor;
}

std::string TcpClientAppX::randomSelectForTexting() {
    std::string survivor = "";
    int size = textingSelection.size();

    if (size > 0) {
        int k = intrand(size);
        survivor = textingSelection[k];
    }

    if (survivor.size() > 0) {
        EV_INFO << "----" << "--- CLIENT: TEXT RECEIVER SELECTED: " << survivor << endl;
    }
    else {
        EV_INFO << "--- CLIENT: TEXT RECEIVER COULD NOT BE SELECTED" << endl;
    }

    return survivor;
}

void TcpClientAppX::sendLookupRequest() {
    std::string survivor = randomSelectForLookup();

    if (survivor.size() > 0) {
        sendLookupRequest(survivor);
    }
}

void TcpClientAppX::sendLookupRequest(std::string survivor) {
    EV_INFO << "LOOKUP REQUEST: " << ownName << "---->" << survivor << endl;
    Packet *packet = new Packet("Lookup Request");

    const auto& payload = makeShared<BunkerPacket>();
    payload->setChunkLength(B(40));
    payload->setType(1);

    payload->setSurvivorName(survivor.c_str());

    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    packet->insertAtBack(payload);

    emit(packetSentSignal, packet);
    sendPacket(packet);
    numSent++;
}

void TcpClientAppX::sendTextMessage() {
    EV_INFO << "--- TEXT YOLLAMAYA CALISIYORUM----------" << endl;

    std::string survivor = randomSelectForTexting();
    if (survivor.size() > 0) {
        if (addressBook.find(survivor) != addressBook.end()) { // Survivor exists in AdressBook
            EV_INFO << "--- TEXT YOLLUYORUM CUNKU BULDUM----------" << endl;

            sendTextMessage(survivor);
        }
        else {  // Survivor does not exist in AdressBook
            sendLookupRequest(survivor);
        }
    }
}

void TcpClientAppX::sendTextMessage(std::string receiver)
{
    EV_INFO << "TEXT MESSAGE SENDING: " << ownName << "---->" << receiver << endl;
    Packet *packet = new Packet("Text Message");

    const auto& payload = makeShared<BunkerPacket>();

    int packetSize = 20 + intrand(980);
    payload->setChunkLength(B(packetSize));
    emit(chunkLength, packetSize);
    payload->setType(3);
    payload->setSurvivorName(ownName.c_str());
    payload->setTextMessage("Test Message!");

    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    packet->insertAtBack(payload);

    emit(packetSentSignal, packet);
    sendPacket(packet);
    numSent++;
}

int TcpClientAppX::numInitStages() const {
    return NUM_INIT_STAGES;
}

void TcpClientAppX::handleMessageWhenUp(cMessage *msg) {
    if (msg->isSelfMessage()) {
        TcpClientThread *thread = (TcpClientThread *)msg->getContextPointer();

        if (thread != nullptr) { // TcpAppBase Logic
            if (!contains(threadSet, thread)) {
                throw cRuntimeError("Invalid thread pointer in the timer (msg->contextPointer is invalid)");
            }
            thread->timerExpired(msg);
        }
        else {
            handleTimer(msg);
        }
    }
    else {
        TcpSocket *tcpSocket = check_and_cast_nullable<TcpSocket *>(socketMap.findSocketFor(msg));
        if (tcpSocket){
            tcpSocket->processMessage(msg);
        }

        else if (serverSocket.belongsToSocket(msg))
        {
            serverSocket.processMessage(msg);
        }

        else if (socket.belongsToSocket(msg)){
            socket.processMessage(msg);
        } // TcpAppBase Logic

        else {
            EV_ERROR << "message " << msg->getFullName() << "(" << msg->getClassName() << ") arrived for unknown socket \n";
            delete msg;
        }
    }
}

void TcpClientAppX::finish() {
    // remove and delete threads
    while (!threadSet.empty())
        removeThread(*threadSet.begin());
    recordScalar("chunkLength", chunkLength);
}

void TcpClientAppX::refreshDisplay() const {
    ApplicationBase::refreshDisplay();

    char buf[32];
    sprintf(buf, "%d threads", socketMap.size());
    getDisplayString().setTagArg("t", 0, buf);
}

void TcpClientAppX::socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent) {
    delete packet;
}

void TcpClientAppX::socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) {
    // new TCP connection -- create new socket object and server process
    TcpSocket *newSocket = new TcpSocket(availableInfo);
    newSocket->setOutputGate(gate("socketOut"));

    cModuleType *moduleType = cModuleType::get(TCPCLIENTTHREAD_CLASS);
    char name[80];
    sprintf(name, "thread_%i", newSocket->getSocketId());
    TcpClientThread *proc = check_and_cast<TcpClientThread *>(moduleType->create(name, this));
    proc->finalizeParameters();
    proc->callInitialize();

    newSocket->setCallback(proc);
    proc->init(this, newSocket);

    socketMap.addSocket(newSocket);
    threadSet.insert(proc);

    socket->accept(availableInfo->getNewSocketId());
}

void TcpClientAppX::socketEstablished(TcpSocket *socket) {
    EV_INFO << "connected\n";

    sendRequest();
}

void TcpClientAppX::socketPeerClosed(TcpSocket *socket_) {
    ASSERT(socket_ == &socket);
    // close the connection (if not already closed)
    if (socket.getState() == TcpSocket::PEER_CLOSED) {
        EV_INFO << "remote TCP closed, closing here as well\n";
        close();
    }
}

void TcpClientAppX::socketClosed(TcpSocket *socket) {
    if (operationalState == State::STOPPING_OPERATION && threadSet.empty() && !serverSocket.isOpen())
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));

//    EV_INFO << "connection closed\n";
//
//    // start another session after a delay
//    if (timeoutMsg) {
//        simtime_t d = par("sendInterval");
//        rescheduleAfterOrDeleteTimer(d, MSGKIND_CONNECT);
//    }
}

void TcpClientAppX::socketFailure(TcpSocket *socket, int code) {
    EV_WARN << "connection broken\n";

    // reconnect after a delay
    if (timeoutMsg) {
        simtime_t d = par("reconnectInterval");
        rescheduleAfterOrDeleteTimer(d, MSGKIND_CONNECT);
    }
}

void TcpClientAppX::socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) {

}

void TcpClientAppX::socketDeleted(TcpSocket *socket) {
    socketMap.removeSocket(socket);
}

void TcpClientAppX::handleStartOperation(LifecycleOperation *operation) {
    int listenPort = par("listenPort");

    serverSocket.setOutputGate(gate("socketOut"));
    serverSocket.setCallback(this);
    serverSocket.bind(listenPort);
    serverSocket.listen();

    simtime_t now = simTime();
    simtime_t start = std::max(startTime, now);
    simtime_t d = par("sendInterval");
    if (timeoutMsg && ((stopTime < SIMTIME_ZERO) || (start < stopTime) || (start == stopTime && startTime == stopTime))) {
        timeoutMsg->setKind(MSGKIND_CONNECT);
        scheduleAt(d, timeoutMsg);
    }
}

void TcpClientAppX::handleStopOperation(LifecycleOperation *operation) {
    for (auto thread : threadSet)
        thread->getSocket()->close();
    serverSocket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));

    cancelEvent(timeoutMsg);
    if (socket.getState() == TcpSocket::CONNECTED || socket.getState() == TcpSocket::CONNECTING || socket.getState() == TcpSocket::PEER_CLOSED)
        close();
}

void TcpClientAppX::handleCrashOperation(LifecycleOperation *operation) {
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

    cancelEvent(timeoutMsg);
    if (operation->getRootModule() != getContainingNode(this))
        socket.destroy();
}

void TcpClientAppX::removeThread(TcpClientThread *thread) {
    // remove socket
    socketMap.removeSocket(thread->getSocket());
    threadSet.erase(thread);

    // remove thread object
    thread->deleteModule();
}

void TcpClientAppX::threadClosed(TcpClientThread *thread) {
    // remove socket
    socketMap.removeSocket(thread->getSocket());
    threadSet.erase(thread);

    socketClosed(thread->getSocket());

    // remove thread object
    thread->deleteModule();
}

// Comes from EchoApp.
void TcpClientAppX::sendDown(Packet *packet) {
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::tcp);
    packet->getTag<SocketReq>();
    send(packet, "socketOut");
}

// ---- TcpAppBase Thread ----
void TcpClientAppX::connect() {
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

void TcpClientAppX::close() {
    EV_INFO << "issuing CLOSE command\n";
    socket.close();
    //cancelEvent(timeoutMsg);
}

void TcpClientAppX::sendPacket(Packet *pkt) {
    socket.send(pkt);
}

void TcpClientAppX::handleTimer(cMessage *msg) {
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

void TcpClientAppX::sendRequest() {

    int k = intrand(100);

    if (k < 50) {
        sendLookupRequest();
    }
    else {
        sendTextMessage();
    }

    // start another session after a delay
    cancelEvent(timeoutMsg);
    if (timeoutMsg) {
        simtime_t d = par("sendInterval");
        rescheduleAfterOrDeleteTimer(d, MSGKIND_CONNECT);
    }

}

void TcpClientAppX::rescheduleAfterOrDeleteTimer(simtime_t d, short int msgKind)
{
    EV_INFO << "------TESTTESTEST" << endl;
    if (stopTime < SIMTIME_ZERO || d < stopTime) {
        timeoutMsg->setKind(msgKind);
        rescheduleAfter(d, timeoutMsg);
    }
    else {
        cancelAndDelete(timeoutMsg);
        timeoutMsg = nullptr;
    }
}

// ---- Tcp Thread ----
TcpClientThread::TcpClientThread() {
    sock = nullptr;
    hostmod = nullptr;
}

TcpClientThread::~TcpClientThread() {
    delete sock;
}

void TcpClientThread::socketDataArrived(TcpSocket *socket, Packet *msg, bool urgent) {
    dataArrived(msg, urgent);
}

void TcpClientThread::socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) {
    socket->accept(availableInfo->getNewSocketId());
}

void TcpClientThread::socketEstablished(TcpSocket *socket) {
    established();
}

void TcpClientThread::socketPeerClosed(TcpSocket *socket) {
    peerClosed();
}

void TcpClientThread::socketClosed(TcpSocket *socket) {
    hostmod->threadClosed(this);
}

void TcpClientThread::socketFailure(TcpSocket *socket, int code) {
    failure(code);
}

void TcpClientThread::socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) {
    statusArrived(status);
}

void TcpClientThread::socketDeleted(TcpSocket *socket) {
    if (socket == sock) {
        sock = nullptr;
        hostmod->socketDeleted(socket);
    }
}

void TcpClientThread::refreshDisplay() const {
    getDisplayString().setTagArg("t", 0, TcpSocket::stateName(sock->getState()));
}

void TcpClientThread::init(TcpClientAppX *hostmodule, TcpSocket *socket) {
    hostmod = hostmodule;
    sock = socket;
}

TcpSocket* TcpClientThread::getSocket() {
    return sock;
}

TcpClientAppX* TcpClientThread::getHostModule() {
    return hostmod;
}

void TcpClientThread::established() {

}

void TcpClientThread::dataArrived(Packet *msg, bool urgent) {

    EV_INFO << "DATA ARRIVED CLIENT" << endl;

    if (sock->getState() == TcpSocket::CONNECTED) {
        EV_INFO << "DATA ARRIVED CONNECTED" << endl;
        emit(hostmod->endtoenddelay, simTime() - msg->getCreationTime());

        emit(packetReceivedSignal, msg);
        hostmod->numReceived++;

        auto data = msg->peekData<BunkerPacket>();

        auto packetType = data->getType();
        std::string survivorName = std::string(data->getSurvivorName());
        auto bunkerId = data->getBunkerId();
        auto ip = data->getIp();

        if (packetType == 2) {
            if (bunkerId == -1) { // Not Found
                EV_INFO << "LOOKUP RESPONSE RECEIVED BY: " << hostmod->ownName << ". SURVIVOR: " << survivorName << " IS NOT AT THE BUNKER "<< endl;
            }
            else { // Survivor Found
                hostmod->addressBook[survivorName].ip = ip;
                hostmod->addressBook[survivorName].ts = simTime();

                if (hostmod->textingSelectionAdded.find(survivorName) == hostmod->textingSelectionAdded.end()) {
                    hostmod->textingSelection.push_back(survivorName);
                    hostmod->textingSelectionAdded.insert(survivorName);
                }

                //Burayı survivorName e yönlendirmek lazım
                EV_INFO << "LOOKUP RESPONSE RECEIVED BY: " << hostmod->ownName << ". SURVIVOR: " << survivorName << " IS AT THE BUNKER "<< endl;


                EV_INFO << "TEXT MESSAGE SENDING: " << hostmod->ownName << "---->" << survivorName << endl;
                Packet *packet = new Packet("Text Message", TCP_C_SEND);

                const auto& payload = makeShared<BunkerPacket>();

                int packetSize = 20 + intrand(980);
                payload->setChunkLength(B(packetSize));
                emit(hostmod->chunkLength, packetSize);
                payload->setType(3);
                payload->setSurvivorName(hostmod->ownName.c_str());
                payload->setTextMessage("Test Message!");

                payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
                packet->insertAtBack(payload);

                emit(packetSentSignal, packet);
                hostmod->numSent++;
                hostmod->sendDown(packet);


            }
        }
        else if (packetType == 3) {
            L3Address remoteAddress = sock->getRemoteAddress();
            auto textMessage = data->getTextMessage();

            if (hostmod->addressBook.find(survivorName) == hostmod->addressBook.end()){

                if (hostmod->textingSelectionAdded.find(survivorName) == hostmod->textingSelectionAdded.end()) {
                    hostmod->learntFromMessages++;
                }

                // Add the sender to the address book.
                hostmod->addressBook[survivorName].ip = remoteAddress;
                hostmod->addressBook[survivorName].ts = simTime();

                if (hostmod->textingSelectionAdded.find(survivorName) == hostmod->textingSelectionAdded.end()) {
                    hostmod->textingSelection.push_back(survivorName);
                    hostmod->textingSelectionAdded.insert(survivorName);
                }
            }

            EV_INFO << "TEXT MESSAGE FROM " << survivorName << " RECEIVED BY: " << hostmod->ownName << endl;
        }
    }

    delete msg;

    getSocket()->close();




}

void TcpClientThread::timerExpired(cMessage *timer) {
    Packet *pkt = check_and_cast<Packet *>(timer);
    pkt->setContextPointer(nullptr);
    hostmod->sendDown(pkt);
}

void TcpClientThread::peerClosed() {
    getSocket()->close();
}

void TcpClientThread::failure(int code) {
    hostmod->removeThread(this);
}

void TcpClientThread::statusArrived(TcpStatusInfo *status) {
}

}
