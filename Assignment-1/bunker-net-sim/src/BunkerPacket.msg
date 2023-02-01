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
    int type;
    string survivorName; // It carries the sender name while sending a text message.
    L3Address ip;
    int bunkerId;
    string textMessage;
}