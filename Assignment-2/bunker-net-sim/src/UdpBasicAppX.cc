#include "UdpBasicAppX.h"

#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"

namespace inet {

UdpBasicAppX::~UdpBasicAppX()
{
    cancelAndDelete(selfMsg);
}

void UdpBasicAppX::initialize(int stage)
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
    }

    if (stage == INITSTAGE_LAST) {
        if (sendingEnabled) {
            destAddress = L3AddressResolver().resolve(par("destAddress"));
            destPort = par("destPort");
        }
    }
}

void UdpBasicAppX::finish()
{
    // statistics
    recordScalar("packets sent", numSent);
    recordScalar("packets received", numReceived);
    ApplicationBase::finish();
}

void UdpBasicAppX::sendPacket() {}

void UdpBasicAppX::processStart()
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

void UdpBasicAppX::processSend()
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

void UdpBasicAppX::processStop()
{
    socket.close();
}

void UdpBasicAppX::handleMessageWhenUp(cMessage *msg)
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

void UdpBasicAppX::socketDataArrived(UdpSocket *socket, Packet *packet) { }

void UdpBasicAppX::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "Ignoring UDP error report " << indication->getName() << endl;
    delete indication;
}

void UdpBasicAppX::socketClosed(UdpSocket *socket)
{
    if (operationalState == State::STOPPING_OPERATION)
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void UdpBasicAppX::refreshDisplay() const
{
    ApplicationBase::refreshDisplay();

    char buf[100];
    sprintf(buf, "---");
    getDisplayString().setTagArg("t", 0, buf);
}

void UdpBasicAppX::handleStartOperation(LifecycleOperation *operation)
{
    clocktime_t start = std::max(startTime, getClockTime());
    if ((stopTime < CLOCKTIME_ZERO) || (start < stopTime) || (start == stopTime && startTime == stopTime)) {
        selfMsg->setKind(START);
        scheduleClockEventAt(start, selfMsg);
    }
}

void UdpBasicAppX::handleStopOperation(LifecycleOperation *operation)
{
    cancelEvent(selfMsg);
    socket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void UdpBasicAppX::handleCrashOperation(LifecycleOperation *operation)
{
    cancelClockEvent(selfMsg);
    socket.destroy(); // TODO  in real operating systems, program crash detected by OS and OS closes sockets of crashed programs.
    socket.setCallback(nullptr);
}

} // namespace inet
