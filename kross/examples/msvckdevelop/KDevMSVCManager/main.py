#import sys;
from VCProjManager import VCProjManager
from SolutionReader import SolutionReader

def main():
#    file = sys.argv[1]
    s=SolutionReader('/home/apol/beca/objectManager/src.sln')
    
    for project in s.projects().values():
        vc=VCProjManager()
        vc.importProject(project)
        
        print "*********** %s" % (project)
        for filter in vc.files():
            print "Filter: %s" % (filter)
            for file in vc.files()[filter]:
                print "--> %s" % (file) 
                
        print vc.configurations()
        print vc.defines(vc.configurations()[0])

main()
