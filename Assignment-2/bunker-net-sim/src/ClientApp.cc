#include "ClientApp.h"

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

#include <algorithm>
#include <random>

namespace inet {

Define_Module(ClientApp);

void ClientApp::possibleSurvivorsInit() {
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

void ClientApp::initialize(int stage)
{
    super::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        chunkLength = registerSignal("chunkLength");
        possibleSurvivorsInit();
        learntFromMessages = 0;

        endtoenddelay = registerSignal("endtoenddelay");
    }
}

std::string ClientApp::randomSelectForLookup() {
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

std::string ClientApp::randomSelectForTexting() {
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

void ClientApp::sendLookupRequest() {
    std::string survivor = randomSelectForLookup();

    if (survivor.size() > 0) {
        sendLookupRequest(survivor);
    }
}

void ClientApp::sendLookupRequest(std::string survivor) {
    EV_INFO << "LOOKUP REQUEST: " << ownName << "---->" << survivor << endl;
    Packet *packet = new Packet("Lookup Request");
    packet->addTag<FragmentationReq>()->setDontFragment(true);

    const auto& payload = makeShared<BunkerPacket>();
    payload->setChunkLength(B(20));
    payload->setType(1);

    payload->setSurvivorName(survivor.c_str());

    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    packet->insertAtBack(payload);

    emit(packetSentSignal, packet);
    socket.sendTo(packet, destAddress, destPort);
    numSent++;
}

void ClientApp::sendTextMessage() {
    std::string survivor = randomSelectForTexting();
    if (survivor.size() > 0) {
        if (addressBook.find(survivor) != addressBook.end()) { // Survivor exists in AdressBook
            sendTextMessage(survivor);
        }
        else {  // Survivor does not exist in AdressBook
            sendLookupRequest(survivor);
        }
    }
}

void ClientApp::sendTextMessage(std::string receiver)
{
    EV_INFO << "TEXT MESSAGE SENDING: " << ownName << "---->" << receiver << endl;
    Packet *packet = new Packet("Text Message");
    packet->addTag<FragmentationReq>()->setDontFragment(true);

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
    socket.sendTo(packet, addressBook[receiver].ip, 5555);
    numSent++;
}

void ClientApp::sendPacket()
{
    int k = intrand(100);

    if (k < 50) {
        sendLookupRequest();
    }
    else {
        sendTextMessage();
    }
}

void ClientApp::socketDataArrived(UdpSocket *socket, Packet *pk)
{
    emit(endtoenddelay, simTime() - pk->getCreationTime());

    emit(packetReceivedSignal, pk);
    numReceived++;

    auto data = pk->peekData<BunkerPacket>();

    auto packetType = data->getType();
    std::string survivorName = std::string(data->getSurvivorName());
    auto bunkerId = data->getBunkerId();
    auto ip = data->getIp();

    if (packetType == 2) {
        if (bunkerId == -1) { // Not Found
            EV_INFO << "LOOKUP RESPONSE RECEIVED BY: " << ownName << ". SURVIVOR: " << survivorName << " IS NOT AT THE BUNKER "<< endl;
        }
        else { // Survivor Found
            addressBook[survivorName].ip = ip;
            addressBook[survivorName].ts = simTime();

            if (textingSelectionAdded.find(survivorName) == textingSelectionAdded.end()) {
                textingSelection.push_back(survivorName);
                textingSelectionAdded.insert(survivorName);
            }

            EV_INFO << "LOOKUP RESPONSE RECEIVED BY: " << ownName << ". SURVIVOR: " << survivorName << " IS AT THE BUNKER "<< endl;

            sendTextMessage(survivorName);
        }
    }
    else if (packetType == 3) {
        L3Address remoteAddress = pk->getTag<L3AddressInd>()->getSrcAddress();
        auto textMessage = data->getTextMessage();

        if (addressBook.find(survivorName) == addressBook.end()){

            if (textingSelectionAdded.find(survivorName) == textingSelectionAdded.end()) {
                learntFromMessages++;
            }

            // Add the sender to the address book.
            addressBook[survivorName].ip = remoteAddress;
            addressBook[survivorName].ts = simTime();

            if (textingSelectionAdded.find(survivorName) == textingSelectionAdded.end()) {
                textingSelection.push_back(survivorName);
                textingSelectionAdded.insert(survivorName);
            }
        }

        EV_INFO << "TEXT MESSAGE FROM " << survivorName << " RECEIVED BY: " << ownName << endl;
    }

    delete pk;
}

void ClientApp::finish() {
    recordScalar("chunkLength", chunkLength);
    super::finish();
}

} // namespace inet
