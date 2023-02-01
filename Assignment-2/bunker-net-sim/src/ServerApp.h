#ifndef SERVERAPP_H_
#define SERVERAPP_H_

#include <unordered_map>
#include "UdpBasicAppX.h"
#include "inet/networklayer/common/L3AddressTag_m.h"

namespace inet {

class INET_API ServerApp : public UdpBasicAppX {
    protected:
        typedef UdpBasicAppX super;

        struct SurvivorData {
            L3Address ip;
            int bunkerId;
            simtime_t ts;
        };
        std::unordered_map<std::string, SurvivorData> survivorDatabase;

    protected:
        void initialize(int stage) override;
        void socketDataArrived(UdpSocket *socket, Packet *packet) override;
        void finish() override;

    protected:
        int successfulLookupCount = 0;
        int unsuccessfulLookupCount = 0;
        simsignal_t successfulLookup;
        simsignal_t unsuccessfulLookup;

        simsignal_t endtoenddelay;
};


} /* namespace inet */

#endif /* SERVERAPP_H_ */
