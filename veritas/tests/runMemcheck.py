#!/usr/bin/python
# run valgrind's memory error checker on all tests.
# filter uninteresting errors and known false positives
# eg staticly initialized memory from libraries like libfontconfig
#

from os import system, remove
from sys import exit, stdout
from subprocess import Popen, PIPE
from xml.dom.minidom import parse, parseString

def garbage(line):
    ''' filter for valgridn output'''
    return not line.startswith('<unknown program name>') and \
           not line.startswith('profiling:')

def memcheck(test):
    ''' run valgrind-memcheck on test in testdir. return xml output as string '''
    #proc = Popen(["valgrind", "--tool=memcheck", "--leak-check=full", "--xml=yes", test], stdout=PIPE, stderr=PIPE)
    #proc.wait()
    #out = proc.stderr.readlines()
    system("valgrind --tool=memcheck --leak-check=full --xml=yes --num-callers=75 " + test + " 1>/dev/null 2>.memcheck.tmp")
    out = open(".memcheck.tmp").readlines()
    remove(".memcheck.tmp")
    out = filter(garbage, out)
    return ''.join(out)

def xml_child_data(dom,tag):
    ''' extract child data for tag. return None if not found'''
    elem = dom.getElementsByTagName(tag)
    val = None
    if len(elem) != 0:
        val = elem[0].firstChild.data
    return val

class Frame:
    ''' single entry in a memory error backtrace '''
    def __init__(self, dom_frame):
        '''<frame>
        <ip>0x62ACDBF</ip>
        <obj>/home/nix/KdeDev/kde4/lib/libkdevplatformlanguage.so.1.0.0</obj>
        <fn>KDevelop::ParamIterator::ParamIterator(QString, QString, int)</fn>
        <dir>/home/nix/KdeDev/kdevplatform/language/duchain</dir>
        <file>stringhelpers.cpp</file>
        <line>292</line>
        </frame>'''
        self.obj   = xml_child_data(dom_frame, 'obj')
        self.func  = xml_child_data(dom_frame, 'fn')
        self.sfile = xml_child_data(dom_frame, 'file')
        self.sline = xml_child_data(dom_frame, 'line')

    def __str__(self):
        out = ""
        if self.func:
            out += "\t" + self.func
        if self.sfile and self.sline:
            out += " (" + self.sfile + ":" + self.sline + ")"
        #if self.obj:
            #out += "\t" + self.obj + "\n"
        out += "\n"
        return out

class BackTrace:
    ''' valgrind memcheck stack trace '''
    def __init__(self, errordom):
        self.dom = errordom
        self.kind = self.dom.getElementsByTagName('kind')[0].firstChild.data
        stack = self.dom.getElementsByTagName('frame')
        self.stack = []
        for frame in stack:
            if xml_child_data(frame, 'fn'): # filter anonymous frames out
                self.stack.append(Frame(frame))
        self.what = xml_child_data(self.dom, 'what')

    def is_definitely_lost(self):
        return self.kind == u'Leak_DefinitelyLost'

    def is_qtest(self):
        is_interesting = False
        for frame in self.stack:
            if frame.obj:
                if frame.obj.find("fontconfig") != -1:
                    return False # google says libfontconfig related 'definitly losts' are  OK
            if frame.func:
                if frame.func.find("QTest") != -1 or frame.func.find("Veritas") != -1:
                    is_interesting = True
                if frame.func.find('__nss_database_lookup') != -1:
                    return False # nothing to worry about
                if frame.func.find('XRegisterIMInstantiateCallback') != -1:
                    return False # X-related static memory allocation, no leak
            if frame.sfile:
                if frame.sfile.find("xtest") != -1 or frame.sfile.find("veritas") != -1:
                    is_interesting = True
        return is_interesting

    def __str__(self):
        out = self.what + "\n"
        for frame in self.stack:
            out += str(frame)
        return out

def parse_errors(out):
    ''' extract the interesting memcheck errors from the xml-string input 'out'.
    return these as a list '''
    xmldoc = parseString(out)
    errors = xmldoc.getElementsByTagName('error')
    errors_ = []
    for error in errors:
        bt = BackTrace(error)
        if bt.is_definitely_lost() and bt.is_qtest():
            errors_.append(bt)
    return errors_

def run_all_tests():
    tests=['veritas-unit-resultsmodel', 'veritas-unit-resultsproxymodel', \
       'veritas-unit-runnermodel', 'veritas-unit-selectionstoretest', \
       'veritas-unit-test', 'veritas-unit-testexecutor', \
       'veritas-unit-treetraverse', 'veritas-sys-runnerwindow']

    root='/home/nix/KdeDev/kdevplatform/build/'
    testdir=root + 'veritas/tests/'
    system("export LD_LIBRARY_PATH="+root+"lib/:$LD_LIBRARY_PATH")

    print ">> running valgrind memcheck"
    all = len(tests)
    curr = 1
    found_error = False
    for test in tests:
        print str(curr) + "/" + str(all) + " " + test + "\t",
        if len(test) < 28: print "\t",
        if len(test) < 20: print "\t",
        stdout.flush()
        curr+=1
        out = memcheck(testdir + test)
        errors = parse_errors(out)
        if len(errors) == 0:
            print "OK"
        else:
            found_error = True
            log = open(test+".memcheck", 'w')
            for trace in errors:
                log.write(str(trace))
                log.write("---------------------------------------------------\n")
            log.close()
            print "NOK (see " + test + ".memcheck)"
    if found_error: exit(-1)

def run_single_test(exe_name):
    print ">> running valgrind memcheck on " + exe_name
    system("export LD_LIBRARY_PATH="+sys.argv[2]+"/lib/:$LD_LIBRARY_PATH")
    out = memcheck(exe_name)
    errors = parse_errors(out)
    if len(errors) == 0:
        print "PASS"
        exit(0)
    else:
        for trace in errors:
            print trace,
            print "---------------------------------------------------"
        exit(-1)

################### ENTRY ####################################################

if __name__ == '__main__':
    import sys
    if len(sys.argv) == 1: run_all_tests()
    else: run_single_test(sys.argv[1])
