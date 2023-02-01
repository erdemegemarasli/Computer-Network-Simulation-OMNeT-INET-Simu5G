import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.common.L3AddressTag;

namespace inet;

class BunkerPacket extends FieldsChunk
{
    // Type-0 = Heartbeat
    // Type-1 = Lookup request
    // Type-2 = Lookup response
    // Type-3 = P2P Text Message
    // Type-4 = Warning Trigger Message
    // Type-5 = Warning Lookup request
    // Type-6 = Warning Lookup response
    // Type-7 = Warning Message
    int type;
    string survivorName;
    L3Address ip;
    int bunkerId;
    string textMessage;
    string warningIPs;
}