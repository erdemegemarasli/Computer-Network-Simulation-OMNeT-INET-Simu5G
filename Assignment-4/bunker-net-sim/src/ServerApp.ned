package bunker_net_sim;

import inet.applications.contract.IApp;

simple ServerApp like IApp
{
    parameters:
        int heartbeatThreshold;
        
        string clockModule = default(""); // relative path of a module that implements IClock; optional

        bool sendingEnabled = default(false);
        
        bool listeningEnabled = default(true);
        int localPort = default(4444);
        
        double startTime @unit(s) = default(this.sendInterval); // application start time (start of the first packet)
        double stopTime @unit(s) = default(-1s);  // time of finishing sending, -1s means forever
        volatile double sendInterval @unit(s) = default(1s); // should usually be a random value, e.g. exponential(1)

        @display("i=block/app");
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // timeout value for lifecycle stop operation
        
        @signal[successfulLookup](type="long");
        @statistic[successfulLookup](title="Successful Lookup Count"; source="successfulLookup"; record=vector; interpolationmode=linear);
        
        @signal[unsuccessfulLookup](type="long");
        @statistic[unsuccessfulLookup](title="Unsuccessful Lookup Count"; source="unsuccessfulLookup"; record=vector; interpolationmode=linear);
        
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