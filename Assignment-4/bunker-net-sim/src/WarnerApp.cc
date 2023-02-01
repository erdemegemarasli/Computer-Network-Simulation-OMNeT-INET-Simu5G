#include "WarnerApp.h"

#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "BunkerPacket_m.h"

namespace inet {

Define_Module(WarnerApp);

WarnerApp::~WarnerApp()
{
    cancelAndDelete(selfMsg);
}

void WarnerApp::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        destPort = par("destPort");
        selfMsg = new ClockEvent("sendTimer");
        serverAddress = L3AddressResolver().resolve(par("serverAddress"));
        serverPort = par("serverPort");
    }

    if (stage == INITSTAGE_APPLICATION_LAYER) {
        mecAppId = par("mecAppId");
        requiredRam = par("requiredRam").doubleValue();
        requiredDisk = par("requiredDisk").doubleValue();
        requiredCpu = par("requiredCpu").doubleValue();

        const char *mp1Ip = par("mp1Address");
        mp1Address = L3AddressResolver().resolve(mp1Ip);
        mp1Port = par("mp1Port");
    }
}

void WarnerApp::finish()
{
    ApplicationBase::finish();
}

void WarnerApp::setSocketOptions()
{
    int localPort = par("localUePort");

    socket.setOutputGate(gate("socketOut"));
    socket.bind(localPort);
    socket.setBroadcast(true);
    socket.setCallback(this);
}

void WarnerApp::sendPacket()
{
//    Packet *packet = new Packet("Warning Message");
//    packet->addTag<FragmentationReq>()->setDontFragment(true);
//
//    const auto& payload = makeShared<BunkerPacket>();
//    payload->setChunkLength(B(20));
//    payload->setType(54);
//
//    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
//    packet->insertAtBack(payload);
//
//    L3Address destAddr = L3Address("10.0.2.255"); // Broadcast address
//    socket.sendTo(packet, destAddr, destPort);
//
//    EV_INFO << getFullName() << " Warning Sent!" << endl;
}

void WarnerApp::processStart()
{
    setSocketOptions();

    selfMsg->setKind(SEND);
    processSend();
}

void WarnerApp::processSend()
{
    sendPacket();

    clocktime_t delay = par("sendInterval");
    selfMsg->setKind(SEND);
    scheduleClockEventAfter(delay, selfMsg);
}

void WarnerApp::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        ASSERT(msg == selfMsg);

        switch (selfMsg->getKind()) {
            case START:
                processStart();
                break;

            case SEND:
                processSend();
                break;

            default:
                throw cRuntimeError("Invalid kind %d in self message", (int)selfMsg->getKind());
        }
    }
    else
        socket.processMessage(msg);
}

void WarnerApp::socketDataArrived(UdpSocket *sock, Packet *packet)
{
    EV_INFO << "Received packet from Warner" << endl;

    L3Address remoteAddress = packet->getTag<L3AddressInd>()->getSrcAddress();
    int srcPort = packet->getTag<L4PortInd>()->getSrcPort();
    packet->clearTags();
    packet->trim();

    auto data = packet->peekData<BunkerPacket>();
    auto packetType = data->getType();

    if (packetType == 4) { // Warning Trigger Message
        Packet *pk = new Packet("Warning Lookup Request");
        pk->addTag<FragmentationReq>()->setDontFragment(true);

        warningMessage = std::string(data->getTextMessage());

        const auto& payload = makeShared<BunkerPacket>();
        payload->setChunkLength(B(20));
        payload->setType(5);  // Warning Lookup Request
        payload->setBunkerId(data->getBunkerId());

        pk->addTag<CreationTimeTag>()->setCreationTime(simTime());
        pk->insertAtBack(payload);

        emit(packetSentSignal, pk);
        socket.sendTo(pk, serverAddress, serverPort);
    }
    else if(packetType == 6) { // Warning Lookup request
        auto ip_list = data->getWarningIPs();

        cStringTokenizer tokenizer(ip_list, ",");
        const char *token;

        while ((token = tokenizer.nextToken()) != nullptr) {
            L3Address ip;
            L3AddressResolver().tryResolve(token, ip);

            EV_INFO << "WARNING MESSAGE SENDING: WARNER APP " << "---->" << token << endl;
            Packet *packet = new Packet("Warning Message");
            packet->addTag<FragmentationReq>()->setDontFragment(true);

            const auto& payload = makeShared<BunkerPacket>();

            payload->setChunkLength(B(20));
            payload->setType(7);
            payload->setTextMessage(warningMessage.c_str());

            payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
            packet->insertAtBack(payload);

            socket.sendTo(packet, ip, 5555);
        }
    }
}

void WarnerApp::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "Ignoring UDP error report " << indication->getName() << endl;
    delete indication;
}

void WarnerApp::socketClosed(UdpSocket *socket)
{
    if (operationalState == State::STOPPING_OPERATION)
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void WarnerApp::handleStartOperation(LifecycleOperation *operation)
{
    selfMsg->setKind(START);
    scheduleClockEventAt(getClockTime(), selfMsg);
}

void WarnerApp::handleStopOperation(LifecycleOperation *operation)
{
    cancelEvent(selfMsg);
    socket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void WarnerApp::handleCrashOperation(LifecycleOperation *operation)
{
    cancelClockEvent(selfMsg);
    socket.destroy();
}

}
