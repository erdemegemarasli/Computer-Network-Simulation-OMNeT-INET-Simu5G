package bunker_net_sim.simulations;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;
import ned.DatarateChannel;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;

import simu5g.common.binder.Binder;
import simu5g.common.carrierAggregation.CarrierAggregation;
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
        @display("bgb=3000,3000");

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
            @display("p=43,29");
        }
        carrierAggregation: CarrierAggregation {
            @display("p=276,29");
        }
        channelControl: LteChannelControl {
            @display("p=387,32");
        }
        binder: Binder {
            @display("p=186,29");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=119,29");
        }
        // =====[ Main ]======
        mainrouter: Router {
            @display("p=1115.4438,227.84377");
        }
        server: StandardHost {
            @display("p=1115.4438,47.550003");
        }

        upf: Upf {
            @display("p=1115.4438,384.36252");
        }
        // =====[ Bunker 1 ]======
        bunker1host[bunker1_host_number]: NRUe {
            parameters:
                @display("p=81,227");
        }

        bunker1iUpf: Upf {
            @display("p=602,384");
        }

        bunker1gnb1: gNodeB {
            @display("p=318,211");
        }

        bunker1gnb2: gNodeB {
            @display("p=318,671");
        }
        // =====[ Bunker 2 ]======
        bunker2host[bunker2_host_number]: NRUe {
            parameters:
                @display("p=735,1509");
        }
        bunker2iUpf: Upf {
            @display("p=1115.4438,1022.3251");
        }

        bunker2gnb1: gNodeB {
            @display("p=735,1228");
        }

        bunker2gnb2: gNodeB {
            @display("p=1557,1228");
        }
        // =====[ Bunker 3 ]======
        bunker3host[bunker3_host_number]: NRUe {
            parameters:
                @display("p=2280,100");
        }
        bunker3iUpf: Upf {
            @display("p=1559.2438,384.36252");
        }

        bunker3gnb1: gNodeB {
            @display("p=1890,124");
        }

        bunker3gnb2: gNodeB {
            @display("p=1890,643");
        }
    connections:
        server.pppg++ <--> closeCable <--> mainrouter.pppg++;


        upf.filterGate <--> closeCable <--> mainrouter.pppg++;
        upf.pppg++ <--> cable <--> bunker1iUpf.pppg++;
        bunker1gnb1.ppp <--> closeCable <--> bunker1iUpf.pppg++;
        bunker1gnb2.ppp <--> closeCable <--> bunker1iUpf.pppg++;
        bunker1gnb1.x2++ <--> closeCable <--> bunker1gnb2.x2++;


        upf.pppg++ <--> cable <--> bunker2iUpf.pppg++;
        bunker2gnb1.ppp <--> closeCable <--> bunker2iUpf.pppg++;
        bunker2gnb2.ppp <--> closeCable <--> bunker2iUpf.pppg++;
        bunker2gnb1.x2++ <--> closeCable <--> bunker2gnb2.x2++;


        upf.pppg++ <--> cable <--> bunker3iUpf.pppg++;
        bunker3gnb1.ppp <--> closeCable <--> bunker3iUpf.pppg++;
        bunker3gnb2.ppp <--> closeCable <--> bunker3iUpf.pppg++;
        bunker3gnb1.x2++ <--> closeCable <--> bunker3gnb2.x2++;

}
