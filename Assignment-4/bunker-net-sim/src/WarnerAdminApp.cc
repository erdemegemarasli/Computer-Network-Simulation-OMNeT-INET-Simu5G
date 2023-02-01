#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "BunkerPacket_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "apps/mec/DeviceApp/DeviceAppMessages/DeviceAppPacket_m.h"
#include "apps/mec/DeviceApp/DeviceAppMessages/DeviceAppPacket_Types.h"
#include "WarnerAdminApp.h"

namespace inet {

Define_Module(WarnerAdminApp);

WarnerAdminApp::~WarnerAdminApp()
{
    cancelAndDelete(selfMsg);
}

void WarnerAdminApp::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        ownName = std::string(getParentModule()->getFullName());
        startTime = par("startTime");
        stopTime = par("stopTime");

        sendingEnabled = par("sendingEnabled");
        listeningEnabled = par("listeningEnabled");

        if (listeningEnabled) {
            localPort = par("localPort");
        }

        if (stopTime >= CLOCKTIME_ZERO && stopTime < startTime) {
            throw cRuntimeError("Invalid startTime/stopTime parameters");
        }

        selfMsg = new ClockEvent("sendTimer");

        // statistics
        numSent = 0;
        numReceived = 0;
        WATCH(numSent);
        WATCH(numReceived);

        bunkerId = par("bunkerId");
        isCentralized = par("isCentralized");
    }

    if (stage == INITSTAGE_APPLICATION_LAYER) {
       deviceAppPort_ = par("deviceAppPort");
       sourceSimbolicAddress = (char*)getParentModule()->getFullName();
       deviceSimbolicAppAddress_ = (char*)par("deviceAppAddress").stringValue();
       deviceAppAddress_ = L3AddressResolver().resolve(deviceSimbolicAppAddress_);
       mecAppName = par("mecAppName").stringValue();
       mecAppPort_ = -1;

       serverAddress = L3AddressResolver().resolve(par("serverAddress"));
       serverPort = par("serverPort");
   }
}

void WarnerAdminApp::finish()
{
    // statistics
    recordScalar("packets sent", numSent);
    recordScalar("packets received", numReceived);
    ApplicationBase::finish();
}

void WarnerAdminApp::sendWarningMessage() {
    EV_INFO << "WARNING REQUEST CREATED BY: " << ownName;
    Packet *packet = new Packet("Warning Request");
    packet->addTag<FragmentationReq>()->setDontFragment(true);

    const auto& payload = makeShared<BunkerPacket>();
    payload->setChunkLength(B(20));
    payload->setType(4);
    payload->setBunkerId(bunkerId);
    payload->setTextMessage("INCOMING MISSLE! THIS IS NOT A DRILL !!!");

    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    packet->insertAtBack(payload);

    emit(packetSentSignal, packet);
    socket.sendTo(packet, mecAppAddress_, mecAppPort_);
    numSent++;
}

void WarnerAdminApp::sendPacket() {
    if (!isCentralized) {
        if (mecAppPort_ < 0) {
            sendStartMEWarningAlertApp();
        }
        else {
            sendWarningMessage();
        }
    }
    else {
        EV_INFO << "WARNING REQUEST CREATED BY: " << ownName;
        Packet *packet = new Packet("Warning Request");
        packet->addTag<FragmentationReq>()->setDontFragment(true);

        const auto& payload = makeShared<BunkerPacket>();
        payload->setChunkLength(B(20));
        payload->setType(4);
        payload->setBunkerId(bunkerId);
        payload->setTextMessage("INCOMING MISSLE! THIS IS NOT A DRILL !!!");

        payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
        packet->insertAtBack(payload);

        emit(packetSentSignal, packet);
        socket.sendTo(packet, serverAddress, serverPort);
        numSent++;
    }
}

void WarnerAdminApp::sendStartMEWarningAlertApp()
{
    Packet* packet = new Packet("DeviceApp Request Packet");
    auto start = makeShared<DeviceAppStartPacket>();

    start->setType(START_MECAPP);
    start->setMecAppName(mecAppName.c_str());

    start->setChunkLength(B(2 + mecAppName.size() + 1));
    start->addTagIfAbsent<CreationTimeTag>()->setCreationTime(simTime());

    packet->insertAtBack(start);

    socket.sendTo(packet, deviceAppAddress_, deviceAppPort_);
}

void WarnerAdminApp::processStart()
{
    socket.setOutputGate(gate("socketOut"));
    if (listeningEnabled) {
        socket.bind(localPort);
    }

    socket.setCallback(this);

    if (sendingEnabled) {
        selfMsg->setKind(SEND);
        processSend();
    }
    else {
        if (stopTime >= CLOCKTIME_ZERO) {
            selfMsg->setKind(STOP);
            scheduleClockEventAt(stopTime, selfMsg);
        }
    }
}

void WarnerAdminApp::processSend()
{
    sendPacket();
    clocktime_t d = par("sendInterval");
    if (stopTime < CLOCKTIME_ZERO || getClockTime() + d < stopTime) {
        selfMsg->setKind(SEND);
        scheduleClockEventAfter(d, selfMsg);
    }
    else {
        selfMsg->setKind(STOP);
        scheduleClockEventAt(stopTime, selfMsg);
    }
}

void WarnerAdminApp::processStop()
{
    socket.close();
}

void WarnerAdminApp::handleMessageWhenUp(cMessage *msg)
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

            case STOP:
                processStop();
                break;

            default:
                throw cRuntimeError("Invalid kind %d in self message", (int)selfMsg->getKind());
        }
    }
    else
        socket.processMessage(msg);
}

void WarnerAdminApp::socketDataArrived(UdpSocket *socket, Packet *packet) {
    L3Address remoteAddress = packet->getTag<L3AddressInd>()->getSrcAddress();

    if (remoteAddress == deviceAppAddress_ || remoteAddress == L3Address("127.0.0.1")) {
        auto mePkt = packet->peekAtFront<DeviceAppPacket>();

        if (mePkt == 0) {
            throw cRuntimeError("UEWarningAlertApp::handleMessage - \tFATAL! Error when casting to DeviceAppPacket");
        }

        if (!strcmp(mePkt->getType(), ACK_START_MECAPP)) {
            auto pkt = packet->peekAtFront<DeviceAppStartAckPacket>();

            if(pkt->getResult() == true) {
                mecAppAddress_ = L3AddressResolver().resolve(pkt->getIpAddress());
                mecAppPort_ = pkt->getPort();
                EV << "MEC HOST INFO - Received " << pkt->getType() << " type. mecApp instance is at: "<< mecAppAddress_<< ":" << mecAppPort_ << endl;

                sendWarningMessage();
            }
        }
        else {
            throw cRuntimeError("UEWarningAlertApp::handleMessage - \tFATAL! Error, DeviceAppPacket type %s not recognized", mePkt->getType());
        }
    }

    delete packet;
}

void WarnerAdminApp::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "Ignoring UDP error report " << indication->getName() << endl;
    delete indication;
}

void WarnerAdminApp::socketClosed(UdpSocket *socket)
{
    if (operationalState == State::STOPPING_OPERATION)
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void WarnerAdminApp::refreshDisplay() const
{
    ApplicationBase::refreshDisplay();

    char buf[100];
    sprintf(buf, "---");
    getDisplayString().setTagArg("t", 0, buf);
}

void WarnerAdminApp::handleStartOperation(LifecycleOperation *operation)
{
    clocktime_t start = std::max(startTime, getClockTime());
    if ((stopTime < CLOCKTIME_ZERO) || (start < stopTime) || (start == stopTime && startTime == stopTime)) {
        selfMsg->setKind(START);
        scheduleClockEventAt(start, selfMsg);
    }
}

void WarnerAdminApp::handleStopOperation(LifecycleOperation *operation)
{
    cancelEvent(selfMsg);
    socket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void WarnerAdminApp::handleCrashOperation(LifecycleOperation *operation)
{
    cancelClockEvent(selfMsg);
    socket.destroy(); // TODO  in real operating systems, program crash detected by OS and OS closes sockets of crashed programs.
    socket.setCallback(nullptr);
}

} // namespace inet
