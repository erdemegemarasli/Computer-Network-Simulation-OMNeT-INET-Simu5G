package bunker_net_sim.simulations;


import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.wireless.AccessPoint;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.node.ethernet.Eth10G;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;

@license(LGPL);
//
// TODO documentation
//
network Network
{
    parameters:
        int bunker1_host_number;
        int bunker2_host_number;
        int bunker3_host_number;
        @display("bgb=1025,631");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=61,31");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=168.49875,30.87");
        }
        // =====[ Main ]======
        mainrouter: Router {
            @display("p=474,231");
        }
        server: StandardHost {
            @display("p=473.34,127.33875");
        }
        // =====[ Bunker 1 ]======
        bunker1router: Router {
            @display("p=280,231");
        }
        bunker1host[bunker1_host_number]: StandardHost {
            parameters:
                @display("p=87.465,127.33875,col,100");
        }
        // =====[ Bunker 2 ]======
        bunker2router: Router {
            @display("p=474,378");
        }
        bunker2host[bunker2_host_number]: StandardHost {
            parameters:
                @display("p=279.11624,491.3475,row,100");
        }
        // =====[ Bunker 3 ]======
        bunker3router: Router {
            @display("p=683,231");
        }
        bunker3host[bunker3_host_number]: StandardHost {
            parameters:
                @display("p=875.9362,127.33875,col,100");
        }
    connections:
        bunker1router.ethg++ <--> Eth10G <--> mainrouter.ethg++;
        bunker2router.ethg++ <--> Eth10G <--> mainrouter.ethg++;
        bunker3router.ethg++ <--> Eth10G <--> mainrouter.ethg++;
        server.ethg++ <--> Eth10G <--> mainrouter.ethg++;

        for i=0..bunker1_host_number-1 {
            bunker1host[i].ethg++ <--> Eth10G <--> bunker1router.ethg++;
        }

        for i=0..bunker2_host_number-1 {
            bunker2host[i].ethg++ <--> Eth10G <--> bunker2router.ethg++;
        }

        for i=0..bunker3_host_number-1 {
            bunker3host[i].ethg++ <--> Eth10G <--> bunker3router.ethg++;
        }
}

