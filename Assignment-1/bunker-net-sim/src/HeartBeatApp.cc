#include "HeartBeatApp.h"

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

Define_Module(HeartBeatApp);

void HeartBeatApp::sendPacket()
{
    int k = intrand(100);

    if (k < 50) {

        EV_INFO << "HEARTBEAT REQUEST CREATED BY: " << ownName;
        Packet *packet = new Packet("HeartBeatData");
        packet->addTag<FragmentationReq>()->setDontFragment(true);

        const auto& payload = makeShared<BunkerPacket>();
        payload->setChunkLength(B(20));
        payload->setType(0);
        payload->setSurvivorName(ownName.c_str());

        payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
        packet->insertAtBack(payload);

        emit(packetSentSignal, packet);
        socket.sendTo(packet, destAddress, destPort);
    }


}

void HeartBeatApp::finish() {
    super::finish();
}

} // namespace inet
