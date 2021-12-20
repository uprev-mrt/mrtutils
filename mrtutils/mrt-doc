#!/usr/bin/env python

import sys
import os
from mrtutils.modsync import *
from mrtutils.mrtDocHelper import DocHelper
import argparse
import tempfile
import shutil
import csv

from kconfiglib import Kconfig, \
                       Symbol, MENU, COMMENT, \
                       BOOL, TRISTATE, STRING, INT, HEX, UNKNOWN, \
                       expr_value, \
                       TRI_TO_STR

from menuconfig import menuconfig

localRepo = None #Repo(os.getcwd())


args = None
parser = None



# Initialize the argument parser
def init_args():
    global parser
    parser = argparse.ArgumentParser("Tool to Manage MrT modules")
    parser.add_argument('localRoot', type=str, nargs='?', help='Local Root of MrT modules', default=".")
    parser.add_argument('-d', '--doc', type=str, help='path to store doc index', default="doc")
    parser.add_argument('-c', '--device-csv', action='store_true', help='creates csv file of devices', default= False)



def main():
    global localRepo

    info={}
    init_args()
    args= parser.parse_args()
   

    #check for git repo
    if not os.path.exists(".git"):
        print("This is not a  valid git repo")
        exit(0)

    localRepo = Repo(os.getcwd())
    localRepo.setRelativePath(args.localRoot + "/")
    localRepo.getSubModules()



    with open( 'mrt.yml', 'w') as file:
        info = localRepo.gatherModuleYamls()
        doc = yaml.dump(info, file)

    if args.device_csv:
        headers = ["Date Added","Category", "Author", "Manufacturer", "MFR_PN", "Interface", "Verified", "Description","Repo"]
        fields = ["created", "category", "author", "mfr", "mfr_pn", "bus", "validated", "description", "url"]
        with open('devices.csv', 'w') as f: 
            write = csv.writer(f) 
            write.writerow(headers)
            for key, value in info.items():
                if "Devices" in value["path"]:
                    row = []
                    for x in fields:
                        if x in value:
                            row.append(value[x])
                        else:
                            row.append("")
                        
                    write.writerow(row) 
            f.close()

    if not os.path.isdir(args.doc):
        d = DocHelper()
        d.buildDocStructure(args.doc)
        

    if os.path.isdir(args.doc +"/moddocs"):
        shutil.rmtree(args.doc +"/moddocs")
    os.makedirs(args.doc +"/moddocs",exist_ok=True)


    modDir = localRepo.getDir(args.localRoot + "/Modules")
    modDir.copyDocStruct(0, args.doc +"/moddocs")

                



        

if __name__ == "__main__":
    main()