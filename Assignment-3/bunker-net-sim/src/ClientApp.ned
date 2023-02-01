package bunker_net_sim;

import inet.applications.contract.IApp;

simple ClientApp like IApp
{
    parameters:    
        int bunker1_host_number;
        int bunker2_host_number;
        int bunker3_host_number;
        int nonExist_host_number;
        string bunker1_host_prefix = default("bunker1host");
        string bunker2_host_prefix = default("bunker2host");
        string bunker3_host_prefix = default("bunker3host");
        
        string clockModule = default(""); // relative path of a module that implements IClock; optional
        
        bool sendingEnabled = default(true);
        string destAddress = default("");
        int destPort = default(4444);

        bool listeningEnabled = default(true);
        int localPort = default(5555);

        double startTime @unit(s) = default(this.sendInterval); // application start time (start of the first packet)
        double stopTime @unit(s) = default(-1s);  // time of finishing sending, -1s means forever
        volatile double sendInterval @unit(s); // should usually be a random value, e.g. exponential(1)

        @display("i=block/app");
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // timeout value for lifecycle stop operation
        
        @signal[chunkLength](type="long");
        @statistic[chunkLength](title="Chunk Length"; source="chunkLength"; record=vector; interpolationmode=none);
        
        @signal[endtoenddelay](type="long");
        @statistic[endtoenddelay](title="End-to-end Delay"; source="endtoenddelay"; record=vector; interpolationmode=linear);
        
        @signal[senderBunkerId](type="long");
        @statistic[senderBunkerId](title="Sender Bunker ID"; source="senderBunkerId"; record=vector; interpolationmode=linear);
        
        @signal[senderHostId](type="long");
        @statistic[senderHostId](title="Sender Host ID"; source="senderHostId"; record=vector; interpolationmode=linear);
        
        @signal[receiverBunkerId](type="long");
        @statistic[receiverBunkerId](title="Receiver Bunker ID"; source="receiverBunkerId"; record=vector; interpolationmode=linear);
        
        @signal[receiverHostId](type="long");
        @statistic[receiverHostId](title="Receiver Host ID"; source="receiverHostId"; record=vector; interpolationmode=linear);
        
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[throughput](title="throughput"; unit=bps; source="throughput(packetSent)"; record=vector);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[rcvdPkLifetime](title="received packet lifetime"; source="dataAge(packetReceived)"; unit=s; record=stats,vector; interpolationmode=none);
        @statistic[rcvdPkSeqNo](title="received packet sequence number"; source="appPkSeqNo(packetReceived)"; record=vector; interpolationmode=none);
    gates:
        input socketIn @labels(UdpControlInfo/up);
        output socketOut @labels(UdpControlInfo/down);
}