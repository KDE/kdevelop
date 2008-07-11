#!/usr/bin/env kdevelop
# coding: utf-8

class SolutionReader:
    theProjects={}
    
    def __init__(self, url):
        lastdash=url.rfind('/')
        baseurl=url[0:lastdash]
        f=open(url, 'r')
        #f = codecs.open( url, "r", "utf-8" )
        for line in f:
            print line
            if line.startswith("Project(\"{"):
                startName=line.find('=')+3
                endName=line.find('\"', startName)
                startPath=endName+4
                endPath=line.find('\"', startPath+1)
                name=line[startName:endName]
                path=line[startPath:endPath]
                path=path.replace("\\", "/")
                self.theProjects[name]=baseurl+"/"+path
    
    def projects(self):
        return self.theProjects
                
                