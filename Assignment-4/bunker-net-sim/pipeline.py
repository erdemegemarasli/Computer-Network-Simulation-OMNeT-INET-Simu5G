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
        '-psimu5g',
        '-KINET4_4_PROJ=' + INET_path,
        '-KSIMU5G_1_2_1_PROJ=' + Simu5G_path,
        '-DINET_IMPORT',
        '-I' + os.path.join(INET_path, 'src'),
        '-I' + os.path.join(Simu5G_path, 'src'),
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
            os.path.join('..', 'bunker-net-sim'),
            '-r',
            '0',
            '-m',
            '-u',
            'Cmdenv',
            '-n',
            '{}:{}:{}:{}'.format('.', os.path.join('..', 'src'), os.path.join(INET_path, 'src'), os.path.join(Simu5G_path, 'src')),
            '-l',
            os.path.join(INET_path, 'src', 'INET'),
            '-l',
            os.path.join(Simu5G_path, 'src', 'simu5g'),
            '-c',
            config,
            os.path.join('.', 'omnetpp.ini')],
            stderr = sys.stderr, stdout = sys.stdout, cwd = os.path.join('.', 'simulations'))
        
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

    # vectors = clean_vectors(vectors)

    # with open(os.path.join('.', 'simulations', 'results', config, config + '.clean.vec.json'), 'w') as f:
    #     f.write(json.dumps(vectors, indent = 4))

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

    first_key = list(vectors.keys())[0]
    vector_data = vectors[first_key]['vectors']
    vector_data = list(filter(lambda item: (item['module'] == "Network.server.ppp[0].queue" and item['name'] == "outgoingDataRate:vector"), vector_data))[0]
    time = np.array(vector_data['time'])
    value = np.array(vector_data['value'])
    
    plt.title("Link Layer Throughput of Server")
    plt.xlabel('Time (s)')
    plt.ylabel('Throughput (Mbps)')
    xpoints = time
    ypoints = value / 1000000
    plt.plot(xpoints, ypoints, linestyle = 'solid')
    plt.grid()
    plt.savefig(os.path.join(linkLayerThroughputFolder, "server-LinkLayerThroughput.png"), bbox_inches = 'tight')
    plt.clf()
############################################################################################################################################
    print('Plotting LinkUtilization of ' + config + '...')
    linkUtilizationFolder = os.path.join(config_folder, 'LinkUtilization')
    if not os.path.exists(linkUtilizationFolder):
        os.makedirs(linkUtilizationFolder)

    first_key = list(vectors.keys())[0]
    vector_data = vectors[first_key]['vectors']
    vector_data = list(filter(lambda item: (item['module'] == "Network.server.app[0]" and item['name'] == "throughput:vector"), vector_data))[0]
    time = np.array(vector_data['time'])
    value = np.array(vector_data['value'])
    
    plt.title("Link Utilization of Server")
    plt.xlabel('Time (s)')
    plt.ylabel('Link Utilization (%)')
    xpoints = time
    ypoints = value * 100 / bandwidth
    plt.plot(xpoints, ypoints, linestyle = 'solid')
    plt.grid()
    plt.savefig(os.path.join(linkUtilizationFolder, "server-LinkUtilization.png"), bbox_inches = 'tight')
    plt.clf()
############################################################################################################################################
    print('Plotting Lookup of ' + config + '...')
    lookupFolder = os.path.join(config_folder, 'Lookup')
    if not os.path.exists(lookupFolder):
        os.makedirs(lookupFolder)

    first_key = list(vectors.keys())[0]
    vector_data = vectors[first_key]['vectors']
    vector_data = list(filter(lambda item: (item['module'] == "Network.server.app[0]" and item['name'] == "successfulLookup:vector"), vector_data))[0]
    time1 = np.array(vector_data['time'])
    value1 = np.array(vector_data['value'])

    first_key = list(vectors.keys())[0]
    vector_data = vectors[first_key]['vectors']
    vector_data = list(filter(lambda item: (item['module'] == "Network.server.app[0]" and item['name'] == "unsuccessfulLookup:vector"), vector_data))[0]
    time2 = np.array(vector_data['time'])
    value2 = np.array(vector_data['value'])

    plt.title('Successful and Unsuccessful Lookups')
    plt.xlabel('Time (s)')
    plt.ylabel('# of Lookups')

    xpoints = time1
    ypoints = value1
    plt.plot(xpoints, ypoints, linestyle = 'solid', label = 'Successful Lookups')

    xpoints = time2
    ypoints = value2
    plt.plot(xpoints, ypoints, linestyle = 'solid', label = 'Unsuccessful Lookups')
    plt.legend()
    plt.grid()

    plt.savefig(os.path.join(lookupFolder, "server-Lookups.png"), bbox_inches = 'tight')
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