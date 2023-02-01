# -*- coding: utf-8 -*-

import subprocess
import json
import matplotlib.pyplot as plt
import numpy as np
import os
import shutil
import re
import time

scavetool_path = os.path.join(list(filter(lambda x: 'omnetpp-6.0.1/bin' in x, os.getenv('PATH').split(':')))[0], 'opp_scavetool')
INET_path = os.path.join(scavetool_path.replace('/bin/opp_scavetool', ''), 'samples', 'inet4.4')

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

for config in config_list:
    print('Running config ' + config + '...')
    result = subprocess.run([
        './src/bunker-net-sim',
        '-r',
        '0',
        '-m',
        '-u',
        'Cmdenv',
        '-n',
        './simulations:./src:' + INET_path + '/src',
        '--image-path=' + INET_path + '/images',
        '-l',
        INET_path + '/src/INET',
        '-c',
        config,
        './simulations/omnetpp.ini'],
        stdout = subprocess.PIPE)

    runLog = result.stdout.decode('utf-8').splitlines()
    #     print(*runLog, sep='\n')

    res = subprocess.run([scavetool_path, 'x', './simulations/results/'+ config + '.vec', '-F', 'JSON', '-o', './simulations/results/'+ config + '.vec.json'],
        stdout = subprocess.PIPE)

    runLog = res.stdout.decode('utf-8').splitlines()
    #     print(*runLog, sep = '\n')

    res = subprocess.run([scavetool_path, 'x', './simulations/results/'+ config + '.sca', '-F', 'JSON', '-o', './simulations/results/'+ config + '.sca.json'],
        stdout = subprocess.PIPE)

    runLog = res.stdout.decode('utf-8').splitlines()
    #     print(*runLog, sep = '\n')

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
    with open('./simulations/results/'+ config + '.sca.json') as f:
        scalars = json.loads(f.read())

    scalars = clean_scalars(scalars)

    with open('./simulations/results/'+ config + '.clean.sca.json', 'w') as f:
        f.write(json.dumps(scalars, indent = 4))

    vectors = None
    with open('./simulations/results/'+ config + '.vec.json') as f:
        vectors = json.loads(f.read())

    vectors = clean_vectors(vectors)

    with open('./simulations/results/'+ config + '.clean.vec.json', 'w') as f:
        f.write(json.dumps(vectors, indent = 4))

    if not os.path.exists('./simulations/results/plots'):
        os.makedirs('./simulations/results/plots')

    config_folder = os.path.join('./simulations/results/plots', config)
    if os.path.exists(config_folder):
        shutil.rmtree(config_folder)

    os.makedirs(config_folder)

    bandwidth = scalars['Network']['cableBandwidth']

    linkLayerThroughputFolder = os.path.join(config_folder, 'LinkLayerThroughput')
    if not os.path.exists(linkLayerThroughputFolder):
        os.makedirs(linkLayerThroughputFolder)

    for key in vectors.keys():
        for module in vectors[key].keys():
            for i in range(len(vectors[key][module])):
                time = np.array(vectors[key][module][i]['eth'][0]['queue']['outgoingDataRate:vector']['time'])
                value = np.zeros(len(time))

                for j in range(len(vectors[key][module][i]['eth'])):
                    value += np.array(vectors[key][module][i]['eth'][j]['queue']['outgoingDataRate:vector']['value'])

                value /= len(vectors[key][module][i]['eth'])

                plt.title('Link Layer Throughput of ' + module + " - " + str(i))
                plt.xlabel('Time (s)')
                plt.ylabel('Throughput (Mbps)')
                xpoints = time
                ypoints = value / 1000
                plt.plot(xpoints, ypoints, linestyle = 'solid')
                plt.grid()
                plt.savefig(os.path.join(linkLayerThroughputFolder, module + "-" + str(i) + "-LinkLayerThroughput.png"), bbox_inches = 'tight')
                plt.clf()

    linkLayerThroughputFolder = os.path.join(config_folder, 'ApplicationLayerThroughput')
    if not os.path.exists(linkLayerThroughputFolder):
        os.makedirs(linkLayerThroughputFolder)

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
                    plt.savefig(os.path.join(linkLayerThroughputFolder, module + "-" + str(i) + "-ApplicationLayerThroughput.png"), bbox_inches = 'tight')
                    plt.clf()

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

print('All simulations finished...')
