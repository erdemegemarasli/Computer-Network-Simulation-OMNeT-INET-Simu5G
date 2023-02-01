package bunker_net_sim;

import inet.applications.contract.IApp;


simple WarnerApp like IApp
{
    parameters:
        @display("i=block/app");
        @lifecycleSupport;
        
        int localPort = default(7777);
        int destPort = default(5555);
        volatile double sendInterval @unit(s) = default(1s);
        
        int mecAppId;
        double requiredRam @unit("B");
        double requiredDisk @unit("B");
        double requiredCpu;
        int localUePort;
        string mp1Address = default("");
        int mp1Port = default(10021);
        
        string serverAddress = default("server");
        int serverPort = default(4444);

		string clockModule = default("");
        double stopOperationExtraTime @unit(s) = default(-1s);
        double stopOperationTimeout @unit(s) = default(2s);
    gates:
        input socketIn @labels(UdpControlInfo/up);
        output socketOut @labels(UdpControlInfo/down);
}

