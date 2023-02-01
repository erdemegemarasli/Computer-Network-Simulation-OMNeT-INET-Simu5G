package bunker_net_sim.simulations;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;
import ned.DatarateChannel;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;

@license(LGPL);
network Network {
    parameters:
        int bunker_number;
        int host_number;
        double cableLength @unit(m);
        double cableBandwidth @unit(bps);
        @display("bgb=1400,700");
        
    types:
        channel cable extends DatarateChannel {
            delay = replaceUnit(parent.cableLength / 2e8, "s");
            datarate = parent.cableBandwidth;
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=61,31");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=168.49875,30.87");
        }
        // =====[ Main ]======
        mainrouter: Router {
            @display("p=575,231");
        }
        server: StandardHost {
            @display("p=575,130");
        }
        
        bunkerrouter[bunker_number]: Router {
            parameters:
                @display("p=475,350,row,100");
        }
        
        bunkerhost[bunker_number * host_number]: StandardHost {
            parameters:
                @display("p=100,550,row,100");
        }
    connections:
        server.ethg++ <--> cable <--> mainrouter.ethg++;
        
        for i=0..bunker_number-1 {
            bunkerrouter[i].ethg++ <--> cable <--> mainrouter.ethg++;
        }
        
        for i=0..bunker_number-1, for j=0..host_number-1 {
            bunkerrouter[i].ethg++ <--> cable <--> bunkerhost[host_number * i + j].ethg++;
        }
}