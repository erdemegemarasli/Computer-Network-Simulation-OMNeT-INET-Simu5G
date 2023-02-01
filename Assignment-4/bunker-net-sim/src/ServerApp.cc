#include "ServerApp.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/common/TimeTag_m.h"
#include "BunkerPacket_m.h"

namespace inet {

Define_Module(ServerApp);

void ServerApp::initialize(int stage)
{
    super::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        successfulLookupCount = 0;
        unsuccessfulLookupCount = 0;
        successfulLookup = registerSignal("successfulLookup");
        unsuccessfulLookup = registerSignal("unsuccessfulLookup");

        endtoenddelay = registerSignal("endtoenddelay");

        senderBunkerId = registerSignal("senderBunkerId");
        senderHostId = registerSignal("senderHostId");
        receiverBunkerId = registerSignal("receiverBunkerId");
        receiverHostId = registerSignal("receiverHostId");
    }
}

void ServerApp::socketDataArrived(UdpSocket *socket, Packet *pk)
{
    int senderBunkerIdInt = 0;
    int senderHostIdInt = 0;
    int receivedBunkerIdInt = 0;
    int receivedHostIdInt = 0;

    emit(senderBunkerId, senderBunkerIdInt);
    emit(receiverBunkerId, receivedBunkerIdInt);
    emit(senderHostId, senderHostIdInt);
    emit(receiverHostId, receivedHostIdInt);

    emit(endtoenddelay, simTime() - pk->getCreationTime());

    emit(packetReceivedSignal, pk);
    numReceived++;

    L3Address remoteAddress = pk->getTag<L3AddressInd>()->getSrcAddress();
    int srcPort = pk->getTag<L4PortInd>()->getSrcPort();
    pk->clearTags();
    pk->trim();

    auto currentTime = simTime();

    auto data = pk->peekData<BunkerPacket>();
    auto packetType = data->getType();
    auto survivorName = data->getSurvivorName();

    if (packetType == 0) { // Heartbeat Packet
        EV_INFO << "SERVER PACKET IS Heartbeat" << endl;

        survivorDatabase[survivorName].bunkerId = data->getBunkerId();
        survivorDatabase[survivorName].ip = remoteAddress;
        survivorDatabase[survivorName].ts = simTime();
        EV_INFO << "--- SERVER: HEARTBEAT RECEIVED FROM: " << survivorName << endl;
    }
    else if (packetType == 1) { // Lookup Request
        EV_INFO << "SERVER PACKET IS Lookup Request" << endl;

        Packet *packet = new Packet("Lookup Response");
        packet->addTag<FragmentationReq>()->setDontFragment(true);
        const auto& payload = makeShared<BunkerPacket>();
        payload->setChunkLength(B(300));
        payload->setType(2);  // Lookup Response

        if (survivorDatabase.find(survivorName) == survivorDatabase.end()) { // Not Exists
            payload->setSurvivorName(survivorName);
            payload->setBunkerId(-1);
            EV_INFO << "--- SERVER: SURVIVOR NOT FOUND: " << survivorName << endl;
            unsuccessfulLookupCount++;
            emit(unsuccessfulLookup, unsuccessfulLookupCount);
        }
        else { // Exists
            if (currentTime - survivorDatabase[survivorName].ts > par("heartbeatThreshold")) { // Last heartbeat is too old
                survivorDatabase.erase(survivorName);
                payload->setSurvivorName(survivorName);
                payload->setBunkerId(-1);
                EV_INFO << "--- SERVER: SURVIVOR NOT FOUND (LAST HEARTBEAT TIMEOUT): " << survivorName << endl;
                unsuccessfulLookupCount++;
                emit(unsuccessfulLookup, unsuccessfulLookupCount);
            }
            else {  // Survivor Found
                payload->setSurvivorName(survivorName);
                payload->setBunkerId(survivorDatabase[survivorName].bunkerId);
                payload->setIp(survivorDatabase[survivorName].ip);
                EV_INFO << "--- SERVER: SURVIVOR FOUND: " << survivorName << endl;
                successfulLookupCount++;
                emit(successfulLookup, successfulLookupCount);
            }
        }

        packet->insertAtBack(payload);
        socket->sendTo(packet, remoteAddress, srcPort);

        emit(packetSentSignal, packet);
        numSent++;
    }
    else if (packetType == 5) { // Warning Lookup Request
        EV_INFO << "SERVER PACKET IS Warning Lookup Request" << endl;

        Packet *packet = new Packet("Warning Lookup Response");
        packet->addTag<FragmentationReq>()->setDontFragment(true);
        const auto& payload = makeShared<BunkerPacket>();
        payload->setChunkLength(B(20));
        payload->setType(6);  // Warning Lookup Response

        int bunkerId = data->getBunkerId();
        std::string ip_list = "";
        for (auto& survivor: survivorDatabase) {
            if (survivor.second.bunkerId == bunkerId)  {
                ip_list.append(survivor.second.ip.str());
                ip_list.append(",");
            }
        }

        if (ip_list.size() > 0) {
            ip_list.pop_back();
        }

        payload->setWarningIPs(ip_list.c_str());

        packet->insertAtBack(payload);
        socket->sendTo(packet, remoteAddress, srcPort);

        emit(packetSentSignal, packet);
        numSent++;
    }
    else if(packetType == 4) { // Warning Trigger Message
        EV_INFO << "SERVER PACKET IS Warning Trigger Message" << endl;
        auto bunkerId = data->getBunkerId();
        auto warningMessage = data->getTextMessage();

        for (auto& survivor: survivorDatabase) {
            if (survivor.second.bunkerId == bunkerId)  {
                L3Address ip = survivor.second.ip;

                EV_INFO << "WARNING MESSAGE SENDING: SERVER APP " << "---->" << ip.str() << endl;
                Packet *packet = new Packet("Warning Message");
                packet->addTag<FragmentationReq>()->setDontFragment(true);

                const auto& payload = makeShared<BunkerPacket>();

                payload->setChunkLength(B(20));
                payload->setType(7);
                payload->setTextMessage(warningMessage);

                payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
                packet->insertAtBack(payload);

                socket->sendTo(packet, ip, 5555);
                emit(packetSentSignal, packet);
                numSent++;
            }
        }
    }

    delete pk;
}

void ServerApp::finish() {
    recordScalar("successfulLookup", successfulLookup);
    recordScalar("unsuccessfulLookup", unsuccessfulLookup);
    super::finish();
}

} /* namespace inet */
