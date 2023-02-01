package bunker_net_sim.simulations;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;
import ned.DatarateChannel;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;

import simu5g.common.binder.Binder;
import simu5g.common.carrierAggregation.CarrierAggregation;
import simu5g.nodes.mec.MECHost;
import simu5g.nodes.mec.MECOrchestrator.MecOrchestrator;
import simu5g.nodes.mec.UALCMP.UALCMP;
import simu5g.world.radio.LteChannelControl;
import simu5g.nodes.NR.NRUe;
import simu5g.nodes.NR.gNodeB;
import simu5g.nodes.Upf;

@license(LGPL);
network Network
{
    parameters:
        int bunker1_host_number;
        int bunker2_host_number;
        int bunker3_host_number;
        double cableLength @unit(m);
        double cableBandwidth @unit(bps);
        double closeCableLength @unit(m);
        double closeCableBandwidth @unit(bps);
        @display("bgb=4763.689,3631.245");

    types:
        channel cable extends DatarateChannel
        {
            delay = replaceUnit(parent.cableLength / 2e8, "s");
            datarate = parent.cableBandwidth;
        }

        channel closeCable extends DatarateChannel
        {
            delay = replaceUnit(parent.closeCableLength / 2e8, "s");
            datarate = parent.closeCableBandwidth;
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=3898.5762,3014.899");
        }
        carrierAggregation: CarrierAggregation {
            @display("p=3898.5762,2424.543");
        }
        channelControl: LteChannelControl {
            @display("p=3898.5762,1867.6036");
        }
        binder: Binder {
            @display("p=3898.5762,2725.2903");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=3898.5762,2134.9346");
        }
        // =====[ Main ]======
        mainrouter: Router {
            @display("p=2030.9725,159.65598");
        }
        server: StandardHost {
            @display("p=1477.746,159.65598");
        }
        upf: Upf {
            @display("p=2030.9725,471.54205");
        }
        iUpf1: Upf {
            @display("p=779.7152,471.54205");
        }
        iUpf2: Upf {
            @display("p=1113.8789,2569.3474");
        }
        iUpf3: Upf {
            @display("p=3620.1064,471.54205");
        }
        ualcmp: UALCMP {
            @display("p=2428.2559,159.65598");
        }
        mecOrchestrator: MecOrchestrator {
            @display("p=2743.855,159.65598");
        }
        mecHost1: MECHost {
            @display("p=779.7152,950.51");
        }
        mecHost2: MECHost {
            @display("p=1511.1624,2569.3474");
        }
        mecHost3: MECHost {
            @display("p=3620.1064,876.2514");
        }
        // =====[ Bunker 1 ]======
        bunker1gnb1: gNodeB {
            @display("p=450,475");
        }
        bunker1host[bunker1_host_number]: NRUe {
            parameters:
                @display("p=240,475");
        }
        // =====[ Bunker 2 ]======
        bunker2gnb1: gNodeB {
            @display("p=1110,3050");
        }
        bunker2host[bunker2_host_number]: NRUe {
            parameters:
                @display("p=1110,3250");
        }
        // =====[ Bunker 3 ]======
        bunker3gnb1: gNodeB {
            @display("p=4100,475");
        }
        bunker3host[bunker3_host_number]: NRUe {
            parameters:
                @display("p=4350,475");
        }
    connections:
        server.pppg++ <--> closeCable <--> mainrouter.pppg++;
        upf.filterGate <--> closeCable <--> mainrouter.pppg++;
        ualcmp.ppp++ <--> closeCable <--> mainrouter.pppg++;
        ualcmp.toMecOrchestrator --> mecOrchestrator.fromUALCMP;
        ualcmp.fromMecOrchestrator <-- mecOrchestrator.toUALCMP;

        upf.pppg++ <--> cable <--> iUpf1.pppg++;
        upf.pppg++ <--> cable <--> iUpf2.pppg++;
        upf.pppg++ <--> cable <--> iUpf3.pppg++;

        mecHost1.ppp++ <--> closeCable <--> iUpf1.pppg++;
        mecHost2.ppp++ <--> closeCable <--> iUpf2.pppg++;
        mecHost3.ppp++ <--> closeCable <--> iUpf3.pppg++;
        bunker1gnb1.ppp <--> closeCable <--> iUpf1.pppg++;
        bunker2gnb1.ppp <--> closeCable <--> iUpf2.pppg++;
        bunker3gnb1.ppp <--> closeCable <--> iUpf3.pppg++;
}
