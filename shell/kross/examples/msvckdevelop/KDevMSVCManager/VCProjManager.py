#!/usr/bin/env kdevelop
# coding: utf-8

import xml.dom.minidom

class VCProjManager:
	"""hoooola"""
	fileList={}
	configs={}
	projectData=0
	
	def files(self):
		return self.fileList
	
	def titolito(self, par):
		return par+par
	
	def importProject(self, url):
		lastdash=url.rfind('/')
		baseurl=url[0:lastdash]
		
		self.projectData = xml.dom.minidom.parse(url).documentElement
		
		filesNode=self.projectData.getElementsByTagName("Files")[0]
		
		for afilter in filesNode.getElementsByTagName("Filter"): #provar de fer igual amb l'altre
			filterName=afilter.attributes["Name"].value
			
			self.fileList[filterName]=[]
			#print afilter.getElementsByTagName("File")
			for f in afilter.getElementsByTagName("File"):
				path=f.attributes["RelativePath"].value
				path=path.replace("\\","/")
				self.fileList[filterName].append(baseurl+"/"+path)
		
		
		cfgsNode=self.projectData.getElementsByTagName("Configurations")[0]
		
		for config in cfgsNode.getElementsByTagName("Configuration"):
			name = config.attributes["Name"].value
			self.configs[name]=config
	
	def configurations(self):
		return self.configs.keys()
	
	def defines(self, cfgName):
		cfgData=self.configs[cfgName]
		toolsData=cfgData.getElementsByTagName("Tool")
		for tool in toolsData:
			if tool.attributes["Name"].value=="VCCLCompilerTool":
				toolData=tool
		if toolData is not None:
			defs=toolData.attributes["PreprocessorDefinitions"].value.split(";")
		return defs
	
	def includes(self, cfgName):
		cfgData=self.configs[cfgName]
		toolsData=cfgData.getElementsByTagName("Tool")
		toolData=defs=None
		for tool in toolsData:
			if tool.attributes["Name"].value=="VCCLCompilerTool":
				toolData=tool
				break
		if toolData is not None:
			defs=toolData.attributes["AdditionalIncludeDirectories"].value.split(";")
		
		return defs
