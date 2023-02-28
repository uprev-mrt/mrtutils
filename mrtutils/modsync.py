#!/usr/bin/python3

import ctypes


#
# class disable_file_system_redirection:
#     _disable = ctypes.windll.kernel32.Wow64DisableWow64FsRedirection
#     _revert = ctypes.windll.kernel32.Wow64RevertWow64FsRedirection
#     def __enter__(self):
#         self.old_value = ctypes.c_long()
#         self.success = self._disable(ctypes.byref(self.old_value))
#     def __exit__(self, type, value, traceback):
#         if self.success:
#             self._revert(self.old_value)


import sys
import os
import glob 
import shutil
from sys import platform
import os.path
import re
import subprocess
import markdown
from threading import Thread
from urllib.request import urlopen
import yaml

devObjBlacklist= ['registers', 'fields', 'packages', 'configs', 'variants', 'pins']

def getRemoteFileText(url, file):
    txt=""

    repo= Repo(url, False)
    return repo.getFileText(file)



def copyRepoFile(url, file, outfile):
    txt = getRemoteFileText(url,file)
    dst = open( outfile , "w")
    dst.write(txt)
    dst.close()
    #print("Pulled {0} from {1}".format(file,url))


class Submodule:
    def __init__(self, path, url):
        self.url = url
        self.mrtprops = {} 
        self.git_url = url
        self.path = path
        self.nodes = path.split('/')
        self.name = self.nodes[-1]
        self.exists = False
        self.requirements = []
        self.readme="x"
        self.repo_name = url.split('/')[-1].replace(".git","")
        self.yaml = {}  
        self.repo = None
        self.submodrepo = Repo(self.url, True, False)


    def checkFor(self,mods):
        global relativePath
        self.exists = False
        for mod in mods:
            if(self.name == mod.name):
                self.exists = True


    def getReadMe(self):

        return self.submodrepo.getReadMe()


    def loadYaml(self, yamlObj):

        if 'requires' in yamlObj:
            for req in yamlObj['requires']:
                if req == 'Platform':
                    self.requirements.append('Common')
                else:
                    self.requirements.append(req)
        
        self.mrtprops = yamlObj

    def getOtherYaml(self, file):

        yamlText = ""
        ret = {}
        yamlObj = {}

        ret['url'] = self.url
        ret['path'] = self.path 
        ret['name'] = self.name

        if os.path.isfile(self.path+"/README.rst"):
            ret['doc'] = self.path+"/README.md"
        
        elif os.path.isfile(self.path+"/README.md"):
            ret['doc'] = self.path+"/README.md"


        try:
            yamlText = self.submodrepo.getFileText(file)

        except:
            print("couldnt load yaml")

        #Get device file if it exists
        try:
            devText = self.submodrepo.getFileText(file)
        except:
            print("couldnt load yaml")

        
        try:
            yamlText = yamlText.replace('\t',' ')
            yamlObj = yaml.load(yamlText,  Loader=yaml.FullLoader)
        except:
            print("Parsing error: " +self.name )

        

        if not yamlObj is None:
            for key, value in yamlObj.items():
                if not key == 'name':
                    ret[key] = value
        
        return ret


    def getYaml(self):

        yamlText = ""
        ret = {}
        yamlObj = {}

        devText =""
        devObj = {}

        ret['url'] = self.url
        ret['path'] = self.path 
        ret['name'] = self.name

        if os.path.isfile(self.path+"/README.rst"):
            ret['doc'] = self.path+"/README.md"
        
        elif os.path.isfile(self.path+"/README.md"):
            ret['doc'] = self.path+"/README.md"


        yamlText = self.submodrepo.getFileText('mrt.yml')
        
        
        try:
            yamlText = yamlText.replace('\t',' ')
            yamlObj = yaml.load(yamlText,  Loader=yaml.FullLoader)
        except:
            print("Parsing error: " +self.name )

        

        if not yamlObj is None:
            for key, value in yamlObj.items():
                if not key == 'name':
                    ret[key] = value
        

        if 'Devices' in self.path:
            devObj = self.getOtherYaml("device.yml")
            for key, value in devObj.items():
                if not key in ret and not key in devObjBlacklist:
                    ret[key] = value
                    

        #get other yaml files 

        
        return ret

    
    def getProp(self,key):
        if key in self.mrtprops:
            return self.mrtprops[key]
        else:
            return None




    def gatherRequirements(self):
            self.requirements = []
            #read in all modules
            regex = re.compile(r'>Requires: (.*?)')
            reqs = regex.findall(self.readme)
            for req in requirements:
                self.requirements.append(req[1])


class RepoDirectory:
    def __init__(self, name,lvl):
        self.name = name
        self.dirs = {}
        self.mods = {}
        self.mod = Submodule('','')
        self.isMod = False
        self.lvl = lvl

    def setMod(self,mod):
        self.mod = mod
        self.isMod = True


    def add(self, mod, lvl):

        nodes = mod.path.split('/')[lvl:]

        #print nodes[lvl:]

        if(len(nodes) > 1):
            node = nodes[0]
            if not node in self.dirs:
                self.dirs[node] = RepoDirectory(node, lvl)
            self.dirs[node].add(mod,lvl+1)
            if node in self.mods:
                self.dirs[node].setMod(self.mods[node])
                del self.mods[node]
        else:
            if mod.name in self.dirs:
                self.dirs[mod.name].set(mod)
            else:
                self.mods[mod.name] = mod

    # def addModToTree(self, mod):
    #     nodes = mod.path.split('/')
    #     lvl = 0
    #     dir = self 
    #     for node in nodes:
    #         if node in dir.dirs:

    def printout(self,lvl):
        start =""
        for i in range(lvl):
            start = start+"  "
            sys.stdout.write("──")

        sys.stdout.write(self.name + "\n")

        for mod in self.mods.values():
            if mod == list(self.mods)[-1]:
                sys.stdout.write(start + "└─%─" +mod.name +"\n\n")
            else:
                sys.stdout.write(start + "├──" +mod.name +"\n")

        for dir in self.dirs.values():
            dir.printout(lvl+1)

    def copyDocStruct(self, lvl, root , root_index_text = "" ):

        if lvl == 0:
            path = root
        else:
            path = root+"/"+self.name




        os.makedirs(path,exist_ok=True)
        indexText =""

        if lvl == 0 and root_index_text != "":
            indexText = root_index_text 
        else:
            indexText = self.name+"\n"
            for c in self.name:
                indexText+= "="

        indexText+= "\n"
        indexText+= ".. toctree::\n"
        indexText+= "\t:caption: " + self.name+"\n"
        indexText+= "\t:titlesonly:\n"
        indexText+= "\n"

        for dir in self.dirs.values():
            indexText+= "\t"+dir.name +"/index\n"
            dir.copyDocStruct(lvl+1,path)
        
        for mod in self.mods.values():
            os.makedirs(path+"/"+mod.name,exist_ok=True)

            for file in glob.glob(mod.path+"/*.rst"):
                fileName = shutil.copy(file, path+"/"+mod.name+"/")  

            for file in glob.glob(mod.path+"/*.md"):
                fileName = shutil.copy(file, path+"/"+mod.name+"/")  

            indexText+= "\t"+mod.name +"/README\n"


        f = open(path+"/index.rst", 'w')
        f.write(indexText)
        f.close()


    def getKConfigString(self, lvl, title = "Config"):
        fileTxt = ""
        if lvl == 0:
            fileTxt += "mainmenu  \"{0}\"\n".format(title)
        else:
            fileTxt += "menu \"{0}\"\n".format(self.name)

        
        for dir in self.dirs.values():
            fileTxt+= dir.getKConfigString(lvl+1)

        for mod in self.mods.values():
            fileTxt += "config ENABLE_{0}\n".format(mod.name.upper())
            fileTxt += "\tbool \"{0}\"\n".format(mod.name)

            
            if mod.exists:
                fileTxt += "\tdefault y\n"
            else:
                fileTxt += "\tdefault n\n"
            
            for req in mod.requirements:
                fileTxt+="\tselect ENABLE_{0}\n".format(req.upper())
  
            # strYaml = yaml.dump(mod.mrtprops)
            # strYaml = strYaml.replace("\n", "\n\t\t")
            # fileTxt +="\thelp\n\t\t{0}\n".format(strYaml)

            if not mod.getProp("description") == None:
                brief = mod.getProp("description")
                brief = brief.replace("\n", "\n\t\t")
                fileTxt +="\thelp\n\t\t{0}\n".format(brief)


            fileTxt += "\n"

        if not lvl == 0:
            fileTxt += "endmenu\n\n"
        
        return fileTxt


class Repo:
    def __init__(self, path : str, createdir=True):

        self.url = path
        self.path = path
        self.mods =[]
        self.isRemote = False

        if path.startswith("https:") or path.startswith("git@"):
            self.isRemote = True


        self.rootYaml = {}
        normalUrl = path.replace(':','/')
        nodes = normalUrl.split('/')
        self.name = nodes[-1].replace(".git","")
        self.host = ''

        if "bitbucket" in path:
            self.host = 'bitbucket'
            self.account =  nodes[-2]
        if "github" in path:
            self.host = 'github'
            self.account =  nodes[-2]

        self.dir = None 
        if createdir:
            self.dir = RepoDirectory("root",0)

    def setRelativePath(self,path):
        self.relativePath = path

    def getDir(self, path):

        nodes = path.split('/')
        curDir = self.dir 
        for n in nodes:
            if n != '.' and n in curDir.dirs:
                curDir = curDir.dirs[n]


        return curDir


    
    def crossCheckMods(self,repo):
        for mod in self.mods:
            mod.checkFor(repo.mods)
    
    def getRootYaml(self):
        
        yamlText = ""
        yamlObj = {}

        yamlText = self.getFileText('mrt.yml')
        
        
        try:
            yamlText = yamlText.replace('\t',' ')
            yamlObj = yaml.load(yamlText,  Loader=yaml.FullLoader)
        except:
            print("Parsing error: " +self.name )
        
        return yamlObj
        


    def gatherModuleYamls(self):
        dict_yaml = {}
        for mod in self.mods:
            dict_yaml[mod.name] = mod.getYaml()
        
        return dict_yaml
        

    def getSubModules(self):
        data = ""

        self.rootYaml = self.getRootYaml()

        if self.isRemote:

            data = self.getFileText('.gitmodules').replace('\r', '').replace('\n', '').replace('\t', '')
            #read in all modules
            regex = re.compile(r'\[(.*?)].*?path = (.*?)url = (.*?\.git)')
            modules = regex.findall(data)
            for mod in modules:
                if(mod[1] != "Config"):
                    self.mods.append(Submodule(mod[1], mod[2]))
        else:
            if os.path.isfile(self.path + "/.gitmodules"):
                file = open(self.path + "/.gitmodules", "r")
                data = file.read().replace('\r', '').replace('\n', '').replace('\t', '')
                #read in all modules
                regex = re.compile(r'\[(.*?)].*?path = (.*?)url = (.*?\.git)')
                modules = regex.findall(data)
                for mod in modules:
                    self.mods.append(Submodule(mod[1], mod[2]))
        
        for mod in self.mods:
            mod.repo = self
            if not self.rootYaml is None:
                if mod.name in self.rootYaml:
                    mod.loadYaml(self.rootYaml[mod.name])
            self.dir.add(mod, 0)
    
    def getFileText(self,file):

        ret = ""
        if self.isRemote:

            req_url = ""

            if(self.host== 'github'):
                req_url = "https://raw.githubusercontent.com/" + self.account + "/" + self.name + "/master/" + file
            elif(self.host == 'bitbucket'):
                req_url = "https://bitbucket.org/" + self.account + "/" + self.name + "/raw/master/" + file
  
            
            try:
                response = urlopen(req_url)
                html_content = response.read()
                encoding = response.headers.get_content_charset('utf-8')
                ret = html_content.decode(encoding)
            except:
                print(req_url +"  not found")
        else:

            path = self.path + '/' + file
            if os.path.isfile(path):
                try:
                    file = open(path)
                    ret = file.read()
                    file.close()
                except:
                    file.close()
                    print( "Error reading " + path)

        return ret


    def getReadMe(self):

        ret = self.getFileText('README.md')

        if(ret == ""):
            ret = self.getFileText('README.rst')
        
        return ret

    def findMod(self, name):
        for mod in self.mods: 
            if mod.name.lower() == name.lower():
                return mod 
        
        return None

    def fetchReadmes(self):
        print ("Fetching README.md from modules")
        for mod in self.mods:
            mod.getReadMe()

    def addSubModule(self,mod):
        prev_path = os.getcwd()
        os.chdir(self.path)
        print("Adding Submodule: " + mod.git_url)
        subprocess.check_output(['git','submodule','add', '-f',mod.git_url,  self.relativePath + mod.path] )
        os.chdir(prev_path)
        mod.exists = True

    def removeSubModule(self,mod):
        prev_path = os.getcwd()
        os.chdir(self.path)
        print("Removing Submodule: " + mod.git_url)
        subprocess.check_output(['git','rm', '-f', self.relativePath + mod.path ])
        os.chdir(prev_path)
        mod.exists = False
    
    def getKConfigString(self):
        return self.dir.getKConfigString(0)

    def getConfigString(self):
        text = ""
        for mod in self.mods:
            if mod.exists:
                text += "CONFIG_ENABLE_{0}=y\n".format(mod.name.upper())
            else:
                text += "CONFIG_ENABLE_{0}=n\n".format(mod.name.upper())
        return text



