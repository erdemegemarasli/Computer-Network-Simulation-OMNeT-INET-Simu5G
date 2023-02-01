# -*- coding: utf-8 -*-

import subprocess
import json
import matplotlib.pyplot as plt
import numpy as np
import os
import shutil
import re
import time
import sys
import argparse

parser = argparse.ArgumentParser(description = 'CLI pipeline tool for Bunker-Net-Sim simulator.')
parser.add_argument('--inet', type = str, required = False, metavar = '<INET_Path>', help = 'The path of the root directory of INET Framework')
parser.add_argument('--simu5g', type = str, required = False, metavar = '<Simu5G_Path>', help = 'The path of the root directory of Simu5G Framework')
parser.add_argument('--config', type = str, required = False, metavar = '<Config_Name>', help = 'The name of the config of the desired simulation')
parser.add_argument('--build', action='store_true', help = 'Builds the bunker-net-sim simulator')
parser.add_argument('--clean', action='store_true', help = 'Cleans the working directory and removes the results')
parser.add_argument('--plotonly', action='store_true', help = 'Use if you just want plots of results without running simulations')
args = parser.parse_args()

try:
    scavetool_path = os.path.join(list(filter(lambda x: 'omnetpp-6.0.1/bin' in x, os.getenv('PATH').split(':')))[0], 'opp_scavetool')
except:
    print('*** You need to use "source setenv" in the root directory of OMNeT++ to load required environment variables before running the pipeline.')
    sys.exit()

if args.inet == None:
    INET_path = os.path.join(scavetool_path.replace('/bin/opp_scavetool', ''), 'samples', 'inet4.4')
else:
    INET_path = args.inet

if args.simu5g == None:
    Simu5G_path = os.path.join(scavetool_path.replace('/bin/opp_scavetool', ''), 'samples', 'Simu5G-1.2.1')
else:
    Simu5G_path = args.simu5g

if INET_path == None:
    print('Use "source setenv" in OMNeT++ root directory if INET Framework is placed in "omnetpp-6.0.1/samples/inet4.4"')
    print('Otherwise, use "--inet" argument to specify the directory of INET Framework.')
    sys.exit()

if Simu5G_path == None:
    print('Use "source setenv" in OMNeT++ root directory if INET Framework is placed in "omnetpp-6.0.1/samples/Simu5G-1.2.1"')
    print('Otherwise, use "--simu5g" argument to specify the directory of Simu5G Framework.')
    sys.exit()

print('INET Path:\t' + INET_path)
print('Simu5G Path:\t' + Simu5G_path)

if not os.path.exists('./simulations/results'):
    os.makedirs('./simulations/results')

with open('./simulations/omnetpp.ini') as f:
    omnetINI = f.read()

config_list = []
matches = re.finditer(r"\[Config (.*?)\]", omnetINI, re.MULTILINE)
for matchNum, match in enumerate(matches, start = 1):
    for groupNum in range(0, len(match.groups())):
        groupNum = groupNum + 1
        config_list.append(match.group(groupNum))

def clean_screen():
    for i in range(100):
        print()

def clean_project():
    print('Cleaning bunker-net-sim...')
    subprocess.run(['make', 'clean'], stderr = sys.stderr, stdout = sys.stdout)
    
    results_folder = os.path.join('.', 'simulations', 'results')
    if os.path.exists(results_folder):
        shutil.rmtree(results_folder)

def build_project():
    clean_project()

    print('Building bunker-net-sim...')
    subprocess.run([
        'opp_makemake',
        '--deep',
        '-f',
        '-pINET',
        '-KINET4_4_PROJ=' + INET_path,
        '-KSIMU5G_1_2_1_PROJ=' + Simu5G_path,
        '-DINET_IMPORT',
        '-I' + os.path.join(INET_path, 'src'),
        '-L' + os.path.join(INET_path, 'src'),
        '-L' + os.path.join(Simu5G_path, 'src'),
        '-lINET',
        '-lsimu5g'],
        stderr = sys.stderr, stdout = sys.stdout)

    result = subprocess.run(['make'], stderr = sys.stderr, stdout = sys.stdout)

def run_config(config):
    if not args.plotonly:
        if not os.path.exists(os.path.join('.', 'bunker-net-sim')):
            clean_screen()
            print('*** You need to build the simulator before running the simulations.')
            print()
            return

        print('Running simulation of ' + config + '...')
        result = subprocess.run([
            os.path.join('.', 'bunker-net-sim'),
            '-r',
            '0',
            '-m',
            '-u',
            'Cmdenv',
            '-n',
            '{}:{}:{}:{}'.format(os.path.join('.', 'simulations'),  os.path.join('.', 'src'), os.path.join(INET_path, 'src'), os.path.join(Simu5G_path, 'src')),
            '-l',
            os.path.join(INET_path, 'src', 'INET'),
            '-l',
            os.path.join(Simu5G_path, 'src', 'simu5g'),
            '-c',
            config,
            os.path.join('.', 'simulations', 'omnetpp.ini')],
            stderr = sys.stderr, stdout = sys.stdout)
        
        print('Parsing results of ' + config + '...')
        res = subprocess.run([
            scavetool_path, 'x', 
            os.path.join('.', 'simulations', 'results', config, config + '.vec'), 
            '-F', 'JSON', '-o', 
            os.path.join('.', 'simulations', 'results', config, config + '.vec.json')],
            stderr = sys.stderr, stdout = sys.stdout)

        res = subprocess.run([
            scavetool_path, 'x', 
            os.path.join('.', 'simulations', 'results', config, config + '.sca'), 
            '-F', 'JSON', '-o', 
            os.path.join('.', 'simulations', 'results', config, config + '.sca.json')],
            stderr = sys.stderr, stdout = sys.stdout)

    def convert_dict_to_nested_objects(d):
        result = {}
        for key, value in d.items():
            parts = key.split(".")
            curr = result
            for i, part in enumerate(parts):
                if part not in curr:
                    curr[part] = {}
                if i == len(parts) - 1:
                    curr[part] = value
                else:
                    curr = curr[part]
        return result

    def convert_strarray_to_nested_array(strarray):
        counts = {}
        for node in strarray.keys():
            k = node.split('[')[0]
            if k not in counts:
                counts[k] = 1
            else:
                counts[k] += 1

        elements = {}
        for name in counts.keys():
            elements[name] = [None for i in range(counts[name])]

        for node in strarray.keys():
            k = node.split('[')[0]
            idx = 0
            if '[' in node:
                idx = int(node.split('[')[1].split(']')[0])

            elements[k][idx] = strarray[node]

        return elements

    def clean_vectors(vectors):
        result = {}

        for key in vectors.keys():
            data = vectors[key]['vectors']

            for d in data:
                if d['module'] not in result:
                    result[d['module']] = {}

                if d['name'] not in result[d['module']]:
                    result[d['module']][d['name']] = {}

                result[d['module']][d['name']]['time'] = d['time']
                result[d['module']][d['name']]['value'] = d['value']

        result = convert_dict_to_nested_objects(result)

        for key in result.keys():
            result[key] = convert_strarray_to_nested_array(result[key])

        for key in result.keys():
            for nodeKey in result[key].keys():
                for i in range(len(result[key][nodeKey])):
                    result[key][nodeKey][i] = convert_strarray_to_nested_array(result[key][nodeKey][i])

        return result

    def clean_scalars(scalars):
        result = {}

        for key in scalars.keys():
            data = scalars[key]['scalars']

            for d in data:
                if d['module'] not in result:
                    result[d['module']] = {}

                if d['name'] not in result[d['module']]:
                    result[d['module']][d['name']] = d['value']

        addLater = {}
        for key in result.keys():
            if '.' not in key:
                addLater[key] = result[key]

        for key in addLater.keys():
            result.pop(key)

        result = convert_dict_to_nested_objects(result)

        for key in result.keys():
            result[key] = convert_strarray_to_nested_array(result[key])

        for key in result.keys():
            for nodeKey in result[key].keys():
                for i in range(len(result[key][nodeKey])):
                    result[key][nodeKey][i] = convert_strarray_to_nested_array(result[key][nodeKey][i])

        for key in addLater.keys():
            if key in result:
                for prop in addLater[key].keys():
                    result[key][prop] = addLater[key][prop]

        return result

    scalars = None
    with open(os.path.join('.', 'simulations', 'results', config, config + '.sca.json')) as f:
        scalars = json.loads(f.read())

    scalars = clean_scalars(scalars)

    with open(os.path.join('.', 'simulations', 'results', config, config + '.clean.sca.json'), 'w') as f:
        f.write(json.dumps(scalars, indent = 4))
    
    vectors = None
    with open(os.path.join('.', 'simulations', 'results', config, config + '.vec.json')) as f:
        vectors = json.loads(f.read())

    vectors = clean_vectors(vectors)

    with open(os.path.join('.', 'simulations', 'results', config, config + '.clean.vec.json'), 'w') as f:
        f.write(json.dumps(vectors, indent = 4))

    config_folder = os.path.join('.', 'simulations', 'results', config, 'plots')
    if os.path.exists(config_folder):
        shutil.rmtree(config_folder)

    os.makedirs(config_folder)

    bandwidth = scalars['Network']['cableBandwidth']
############################################################################################################################################
    print('Plotting LinkLayerThroughput of ' + config + '...')
    linkLayerThroughputFolder = os.path.join(config_folder, 'LinkLayerThroughput')
    if not os.path.exists(linkLayerThroughputFolder):
        os.makedirs(linkLayerThroughputFolder)

    for key in vectors.keys():
        for module in vectors[key].keys():
            for i in range(len(vectors[key][module])):
                if 'cellularNic' not in vectors[key][module][i]:
                    time = np.array(vectors[key][module][i]['ppp'][0]['queue']['outgoingDataRate:vector']['time'])
                    value = np.zeros(len(time))

                    for j in range(len(vectors[key][module][i]['ppp'])):
                        value += np.array(vectors[key][module][i]['ppp'][j]['queue']['outgoingDataRate:vector']['value'])

                    value /= len(vectors[key][module][i]['ppp'])

                    plt.title('Link Layer Throughput of ' + module + " - " + str(i))
                    plt.xlabel('Time (s)')
                    plt.ylabel('Throughput (Mbps)')
                    xpoints = time
                    ypoints = value / 1000
                    plt.plot(xpoints, ypoints, linestyle = 'solid')
                    plt.grid()
                    plt.savefig(os.path.join(linkLayerThroughputFolder, module + "-" + str(i) + "-LinkLayerThroughput.png"), bbox_inches = 'tight')
                    plt.clf()
############################################################################################################################################
    print('Plotting LinkUtilization of ' + config + '...')
    linkUtilizationFolder = os.path.join(config_folder, 'LinkUtilization')
    if not os.path.exists(linkUtilizationFolder):
        os.makedirs(linkUtilizationFolder)

    for key in vectors.keys():
        for module in vectors[key].keys():
            for i in range(len(vectors[key][module])):
                if 'app' in vectors[key][module][i]:
                    time = np.array(vectors[key][module][i]['app'][0]['throughput:vector']['time'])
                    value = np.zeros(len(time))

                    for j in range(len(vectors[key][module][i]['app'])):
                        value += np.array(vectors[key][module][i]['app'][j]['throughput:vector']['value'])

                    value /= len(vectors[key][module][i]['app'])

                    plt.title('Link Utilization of ' + module + " - " + str(i))
                    plt.xlabel('Time (s)')
                    plt.ylabel('Link Utilization (%)')
                    xpoints = time
                    ypoints = value * 100 / bandwidth
                    plt.plot(xpoints, ypoints, linestyle = 'solid')
                    plt.grid()
                    plt.savefig(os.path.join(linkUtilizationFolder, module + "-" + str(i) + "-LinkUtilization.png"), bbox_inches = 'tight')
                    plt.clf()
############################################################################################################################################
    print('Plotting ApplicationLayerThroughput of ' + config + '...')
    applicationLayerThroughput = os.path.join(config_folder, 'ApplicationLayerThroughput')
    if not os.path.exists(applicationLayerThroughput):
        os.makedirs(applicationLayerThroughput)

    for key in vectors.keys():
        for module in vectors[key].keys():
            for i in range(len(vectors[key][module])):
                if 'app' in vectors[key][module][i]:
                    time = np.array(vectors[key][module][i]['app'][0]['throughput:vector']['time'])
                    value = np.zeros(len(time))

                    for j in range(len(vectors[key][module][i]['app'])):
                        value += np.array(vectors[key][module][i]['app'][j]['throughput:vector']['value'])

                    value /= len(vectors[key][module][i]['app'])

                    plt.title('Application Layer Throughput of ' + module + " - " + str(i))
                    plt.xlabel('Time (s)')
                    plt.ylabel('Throughput (Mbps)')
                    xpoints = time
                    ypoints = value / 1000
                    plt.plot(xpoints, ypoints, linestyle = 'solid')
                    plt.grid()
                    plt.savefig(os.path.join(applicationLayerThroughput, module + "-" + str(i) + "-ApplicationLayerThroughput.png"), bbox_inches = 'tight')
                    plt.clf()
############################################################################################################################################
    print('Plotting Latency of ' + config + '...')
    latencyFolder = os.path.join(config_folder, 'Latency')
    if not os.path.exists(latencyFolder):
        os.makedirs(latencyFolder)

    for key in vectors.keys():
        for module in vectors[key].keys():
            for i in range(len(vectors[key][module])):
                if 'app' in vectors[key][module][i]:
                    length = 0
                    value = 0
                    time = 0
                    counter = 0

                    for j in range(len(vectors[key][module][i]['app'])):
                        if 'endtoenddelay:vector' in vectors[key][module][i]['app'][j]:
                            if length == 0:
                                length = len(vectors[key][module][i]['app'][j]['endtoenddelay:vector']['time'])
                                value = np.zeros(length)
                                time = np.array(vectors[key][module][i]['app'][j]['endtoenddelay:vector']['time'])

                            value += np.array(vectors[key][module][i]['app'][j]['endtoenddelay:vector']['value'])
                            counter += 1

                    if counter > 0:
                        value /= counter

                    if type(value) != int:
                        plt.title('Latency of ' + module + " - " + str(i))
                        plt.xlabel('Time (s)')
                        plt.ylabel('Latency (ms)')
                        xpoints = time
                        ypoints = value * 1000
                        plt.plot(xpoints, ypoints, linestyle = 'solid')
                        plt.grid()
                        plt.savefig(os.path.join(latencyFolder, module + "-" + str(i) + "-Latency.png"), bbox_inches = 'tight')
                        plt.clf()
############################################################################################################################################
    print('Plotting B2BLatency of ' + config + '...')
    B2BLatencyFolder = os.path.join(config_folder, 'B2BLatency')
    if not os.path.exists(B2BLatencyFolder):
        os.makedirs(B2BLatencyFolder)

    time = []

    B1toB1 = []
    B1toB2 = []
    B1toB3 = []

    B2toB1 = []
    B2toB2 = []
    B2toB3 = []

    B3toB1 = []
    B3toB2 = []
    B3toB3 = []

    B1toB1counter = 0
    B1toB2counter = 0
    B1toB3counter = 0

    B2toB1counter = 0
    B2toB2counter = 0
    B2toB3counter = 0

    B3toB1counter = 0
    B3toB2counter = 0
    B3toB3counter = 0

    for key in vectors.keys():
        for module in vectors[key].keys():
            for i in range(len(vectors[key][module])):
                if 'app' in vectors[key][module][i]:
                    for j in range(len(vectors[key][module][i]['app'])):
                        if 'endtoenddelay:vector' in vectors[key][module][i]['app'][j]:
                            if len(time) == 0:
                                length = len(vectors[key][module][i]['app'][j]['endtoenddelay:vector']['time'])
                                time = np.array(vectors[key][module][i]['app'][j]['endtoenddelay:vector']['time'])
                            
                            if len(B1toB1) == 0:
                                B1toB1 = np.zeros(length)
                            if len(B1toB2) == 0:
                                B1toB2 = np.zeros(length)
                            if len(B1toB3) == 0:
                                B1toB3 = np.zeros(length)
                            if len(B2toB1) == 0:
                                B2toB1 = np.zeros(length)
                            if len(B2toB2) == 0:
                                B2toB2 = np.zeros(length)
                            if len(B2toB3) == 0:
                                B2toB3 = np.zeros(length)
                            if len(B3toB1) == 0:
                                B3toB1 = np.zeros(length)
                            if len(B3toB2) == 0:
                                B3toB2 = np.zeros(length)
                            if len(B3toB3) == 0:
                                B3toB3 = np.zeros(length)

                            for z in range(len(vectors[key][module][i]['app'][j]['senderBunkerId:vector']['value'])):
                                if vectors[key][module][i]['app'][j]['senderBunkerId:vector']['value'][z] == 1 and vectors[key][module][i]['app'][j]['receiverBunkerId:vector']['value'][z] == 1:
                                    B1toB1[z] += vectors[key][module][i]['app'][j]['endtoenddelay:vector']['value'][z]
                                    B1toB1counter += 1
                                elif vectors[key][module][i]['app'][j]['senderBunkerId:vector']['value'][z] == 1 and vectors[key][module][i]['app'][j]['receiverBunkerId:vector']['value'][z] == 2:
                                    B1toB2[z] += vectors[key][module][i]['app'][j]['endtoenddelay:vector']['value'][z]
                                    B1toB2counter += 1
                                elif vectors[key][module][i]['app'][j]['senderBunkerId:vector']['value'][z] == 1 and vectors[key][module][i]['app'][j]['receiverBunkerId:vector']['value'][z] == 3:
                                    B1toB3[z] += vectors[key][module][i]['app'][j]['endtoenddelay:vector']['value'][z]
                                    B1toB3counter += 1
                                elif vectors[key][module][i]['app'][j]['senderBunkerId:vector']['value'][z] == 2 and vectors[key][module][i]['app'][j]['receiverBunkerId:vector']['value'][z] == 1:
                                    B2toB1[z] += vectors[key][module][i]['app'][j]['endtoenddelay:vector']['value'][z]
                                    B2toB1counter += 1
                                elif vectors[key][module][i]['app'][j]['senderBunkerId:vector']['value'][z] == 2 and vectors[key][module][i]['app'][j]['receiverBunkerId:vector']['value'][z] == 2:
                                    B2toB2[z] += vectors[key][module][i]['app'][j]['endtoenddelay:vector']['value'][z]
                                    B2toB2counter += 1
                                elif vectors[key][module][i]['app'][j]['senderBunkerId:vector']['value'][z] == 2 and vectors[key][module][i]['app'][j]['receiverBunkerId:vector']['value'][z] == 3:
                                    B2toB3[z] += vectors[key][module][i]['app'][j]['endtoenddelay:vector']['value'][z]
                                    B2toB3counter += 1
                                elif vectors[key][module][i]['app'][j]['senderBunkerId:vector']['value'][z] == 3 and vectors[key][module][i]['app'][j]['receiverBunkerId:vector']['value'][z] == 1:
                                    B3toB1[z] += vectors[key][module][i]['app'][j]['endtoenddelay:vector']['value'][z]
                                    B3toB1counter += 1
                                elif vectors[key][module][i]['app'][j]['senderBunkerId:vector']['value'][z] == 3 and vectors[key][module][i]['app'][j]['receiverBunkerId:vector']['value'][z] == 2:
                                    B3toB2[z] += vectors[key][module][i]['app'][j]['endtoenddelay:vector']['value'][z]
                                    B3toB2counter += 1
                                elif vectors[key][module][i]['app'][j]['senderBunkerId:vector']['value'][z] == 3 and vectors[key][module][i]['app'][j]['receiverBunkerId:vector']['value'][z] == 3:
                                    B3toB3[z] += vectors[key][module][i]['app'][j]['endtoenddelay:vector']['value'][z]
                                    B3toB3counter += 1

    if B1toB1counter > 0:
        B1toB1 /= B1toB1counter

    if B1toB2counter > 0:
        B1toB2 /= B1toB2counter

    if B1toB3counter > 0:
        B1toB3 /= B1toB3counter

    if B2toB1counter > 0:
        B2toB1 /= B2toB1counter

    if B2toB2counter > 0:
        B2toB2 /= B2toB2counter

    if B2toB3counter > 0:
        B2toB3 /= B2toB3counter

    if B3toB1counter > 0:
        B3toB1 /= B3toB1counter

    if B3toB2counter > 0:
        B3toB2 /= B3toB2counter

    if B3toB3counter > 0:
        B3toB3 /= B3toB3counter

    plt.title('Latency of Bunker 1 to Bunker 1')
    plt.xlabel('Time (s)')
    plt.ylabel('Latency (ms)')
    xpoints = time
    ypoints = B1toB1 * 1000
    plt.plot(xpoints, ypoints, linestyle = 'solid')
    plt.grid()
    plt.savefig(os.path.join(B2BLatencyFolder, "Bunker1-Bunker1.png"), bbox_inches = 'tight')
    plt.clf()

    plt.title('Latency of Bunker 1 to Bunker 2')
    plt.xlabel('Time (s)')
    plt.ylabel('Latency (ms)')
    xpoints = time
    ypoints = B1toB2 * 1000
    plt.plot(xpoints, ypoints, linestyle = 'solid')
    plt.grid()
    plt.savefig(os.path.join(B2BLatencyFolder, "Bunker1-Bunker2.png"), bbox_inches = 'tight')
    plt.clf()

    plt.title('Latency of Bunker 1 to Bunker 3')
    plt.xlabel('Time (s)')
    plt.ylabel('Latency (ms)')
    xpoints = time
    ypoints = B1toB3 * 1000
    plt.plot(xpoints, ypoints, linestyle = 'solid')
    plt.grid()
    plt.savefig(os.path.join(B2BLatencyFolder, "Bunker1-Bunker3.png"), bbox_inches = 'tight')
    plt.clf()

    plt.title('Latency of Bunker 2 to Bunker 1')
    plt.xlabel('Time (s)')
    plt.ylabel('Latency (ms)')
    xpoints = time
    ypoints = B2toB1 * 1000
    plt.plot(xpoints, ypoints, linestyle = 'solid')
    plt.grid()
    plt.savefig(os.path.join(B2BLatencyFolder, "Bunker2-Bunker1.png"), bbox_inches = 'tight')
    plt.clf()

    plt.title('Latency of Bunker 2 to Bunker 2')
    plt.xlabel('Time (s)')
    plt.ylabel('Latency (ms)')
    xpoints = time
    ypoints = B2toB2 * 1000
    plt.plot(xpoints, ypoints, linestyle = 'solid')
    plt.grid()
    plt.savefig(os.path.join(B2BLatencyFolder, "Bunker2-Bunker2.png"), bbox_inches = 'tight')
    plt.clf()

    plt.title('Latency of Bunker 2 to Bunker 3')
    plt.xlabel('Time (s)')
    plt.ylabel('Latency (ms)')
    xpoints = time
    ypoints = B2toB3 * 1000
    plt.plot(xpoints, ypoints, linestyle = 'solid')
    plt.grid()
    plt.savefig(os.path.join(B2BLatencyFolder, "Bunker2-Bunker3.png"), bbox_inches = 'tight')
    plt.clf()

    plt.title('Latency of Bunker 3 to Bunker 1')
    plt.xlabel('Time (s)')
    plt.ylabel('Latency (ms)')
    xpoints = time
    ypoints = B3toB1 * 1000
    plt.plot(xpoints, ypoints, linestyle = 'solid')
    plt.grid()
    plt.savefig(os.path.join(B2BLatencyFolder, "Bunker3-Bunker1.png"), bbox_inches = 'tight')
    plt.clf()

    plt.title('Latency of Bunker 3 to Bunker 2')
    plt.xlabel('Time (s)')
    plt.ylabel('Latency (ms)')
    xpoints = time
    ypoints = B3toB2 * 1000
    plt.plot(xpoints, ypoints, linestyle = 'solid')
    plt.grid()
    plt.savefig(os.path.join(B2BLatencyFolder, "Bunker3-Bunker2.png"), bbox_inches = 'tight')
    plt.clf()

    plt.title('Latency of Bunker 3 to Bunker 3')
    plt.xlabel('Time (s)')
    plt.ylabel('Latency (ms)')
    xpoints = time
    ypoints = B3toB3 * 1000
    plt.plot(xpoints, ypoints, linestyle = 'solid')
    plt.grid()
    plt.savefig(os.path.join(B2BLatencyFolder, "Bunker3-Bunker3.png"), bbox_inches = 'tight')
    plt.clf()
############################################################################################################################################
    print('Plotting Lookup of ' + config + '...')
    lookupFolder = os.path.join(config_folder, 'Lookup')
    if not os.path.exists(lookupFolder):
        os.makedirs(lookupFolder)

    for key in vectors.keys():
        for module in vectors[key].keys():
            for i in range(len(vectors[key][module])):
                if 'app' in vectors[key][module][i]:
                    length1 = 0
                    value1 = 0
                    time1 = 0
                    counter1 = 0

                    length2 = 0
                    value2 = 0
                    time2 = 0
                    counter2 = 0

                    for j in range(len(vectors[key][module][i]['app'])):
                        if 'successfulLookup:vector' in vectors[key][module][i]['app'][j]:
                            if length1 == 0:
                                length1 = len(vectors[key][module][i]['app'][j]['successfulLookup:vector']['time'])
                                value1 = np.zeros(length1)
                                time1 = np.array(vectors[key][module][i]['app'][j]['successfulLookup:vector']['time'])

                            value1 += np.array(vectors[key][module][i]['app'][j]['successfulLookup:vector']['value'])
                            counter1 += 1

                        if 'unsuccessfulLookup:vector' in vectors[key][module][i]['app'][j]:
                            if length2 == 0:
                                length2 = len(vectors[key][module][i]['app'][j]['unsuccessfulLookup:vector']['time'])
                                value2 = np.zeros(length2)
                                time2 = np.array(vectors[key][module][i]['app'][j]['unsuccessfulLookup:vector']['time'])

                            value2 += np.array(vectors[key][module][i]['app'][j]['unsuccessfulLookup:vector']['value'])
                            counter2 += 1

                    if counter1 > 0:
                        value1 /= counter1

                    if counter2 > 0:
                        value2 /= counter2

                    if type(value1) != int and type(value2) != int:
                        plt.title('Lookups of ' + module + " - " + str(i))
                        plt.xlabel('Time (s)')
                        plt.ylabel('# of Lookups')

                        xpoints = time1
                        ypoints = value1 * 1000
                        plt.plot(xpoints, ypoints, linestyle = 'solid', label = 'Successful Lookups')

                        xpoints = time2
                        ypoints = value2 * 1000
                        plt.plot(xpoints, ypoints, linestyle = 'solid', label = 'Unsuccessful Lookups')
                        plt.legend()
                        plt.grid()

                        plt.savefig(os.path.join(lookupFolder, module + "-" + str(i) + "-Lookups.png"), bbox_inches = 'tight')
                        plt.clf()
############################################################################################################################################
def run_everything():
    if not os.path.exists(os.path.join('.', 'bunker-net-sim')):
        clean_screen()
        print('*** You need to build the simulator before running the simulations.')
        print()
        return
    print('Running all simulations...')
    for config in config_list:
        run_config(config)
    print('All simulations finished...')

if args.clean:
    clean_project()
    sys.exit()

if args.build:
    build_project()
    sys.exit()

if args.config != None:
    if args.config == 'all':
        run_everything()
    else:
        if args.config in config_list:
            run_config(args.config)
        else:
            print()
            print('Unknown configuration!')
    sys.exit()

selection = ""
menuOpen = True
while (menuOpen):
    print('==================================================================')
    print('Welcome to bunker-net-sim pipeline tool.')
    print('You can run different configurations with this tool.')
    print('The configurations are getting from omnet.ini file directly.')
    print('To add new configurations, update the omnet.ini file accordingly.')
    print('==================================================================')
    print('To build the project, you can use the following:')
    print('  USAGE: python ' + sys.argv[0] + ' --build')
    print('==================================================================')
    print('To clean the working directory, you can use the following:')
    print('  USAGE: python ' + sys.argv[0] + ' --clean')
    print('==================================================================')
    print('To run a configuration unattended, you can use the following:')
    print('  USAGE: python ' + sys.argv[0] + ' --config <CONFIG_NAME>')
    print('==================================================================')
    print('To run a all simulations unattended, you can use the following:')
    print('  USAGE: python ' + sys.argv[0] + ' --config all')
    print('==================================================================')
    print('To plot the results without running the simulations again: ')
    print('  USAGE: python ' + sys.argv[0] + ' --plotonly')
    print('==================================================================')
    print('Or... You can use the following interactive menu for simulations.')
    print('==================================================================')
    print('Select the configuration that you would like to put into pipeline:')
    print('==================================================================')
    for i in range(len(config_list)):
        print(' ' + str(i + 1) + '.\t' + config_list[i])
    print('==================================================================')
    print(' A.\tRun everything!')
    print('==================================================================')
    print(' B.\tBuild the project.')
    print(' C.\tClean working directory.')
    print(' Q.\tQuit.')
    print('==================================================================')

    selection = input("Your selection:").upper()
    
    if selection.isnumeric() and int(selection) >= 1 and int(selection) <= len(config_list):
        run_config(config_list[int(selection) - 1])
    elif selection == 'A':
        run_everything()
    elif selection == 'B':
        build_project()
    elif selection == 'C':
        clean_project()
    elif selection == 'Q':
        menuOpen = False
    else:
        clean_screen()
        print('Unknown input!')
        print()