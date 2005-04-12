# Copyright (c) 2001, 2002, 2003, 2004 The SCons Foundation
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
# KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

# Shamelessly stolen from qt.py and (heavily) modified into kde.py :)
# Thomas Nagy, 2004, 2005 <tnagy2^8@yahoo.fr>

"""
Here follow the basic rules for building kde programs
The detection is done in detect_kde when needed
We wan to use the cached variables as much as possible

The variables used when configuring are :
* prefix     : base install path,         eg: /usr/local
* execprefix : install path for binaries, eg: /usr/bin
* datadir    : install path for the data, eg: /usr/local/share
* libdir     : install path for the libs, eg: /usr/lib

* kdeincludes: path to the kde includes (/usr/include/kde on debian, ...)
* qtincludes : same punishment, for qt includes (/usr/include/qt on debian, ...)

* kdelibs    : path to the kde libs, for linking the programs
* qtlibs     : same punishment, for qt libraries

eg: scons configure libdir=/usr/local/lib qtincludes=/usr/include/qt
"""

BOLD   ="\033[1m"
RED    ="\033[91m"
GREEN  ="\033[92m"
YELLOW ="\033[93m"
CYAN   ="\033[96m"
NORMAL ="\033[0m"

def exists(env):
	return True

def detect_kde(env):
	""" Detect the qt and kde environment using kde-config mostly """
	import os, sys, re

	prefix      = env['ARGS'].get('prefix', None)
	execprefix  = env['ARGS'].get('execprefix', None)
	datadir     = env['ARGS'].get('datadir', None)
	libdir      = env['ARGS'].get('libdir', None)
	kdeincludes = env['ARGS'].get('kdeincludes', None)
	kdelibs     = env['ARGS'].get('kdelibs', None)
	qtincludes  = env['ARGS'].get('qtincludes', None)
	qtlibs      = env['ARGS'].get('qtlibs', None)

	## Detect the kde libraries
	print "Checking for kde-config           : ",
	kde_config = os.popen("which kde-config 2>/dev/null").read().strip()
	if len(kde_config):
		print GREEN + "kde-config was found" + NORMAL
	else:
		print RED + "kde-config was NOT found in your PATH"+ NORMAL
		print "Make sure kde is installed properly"
		print "(missing package kdebase-devel?)"
		# TODO : prompt the user for the path of kde-config ?
		sys.exit(1)
	env['KDEDIR'] = os.popen('kde-config -prefix').read().strip()

	print "Checking for kde version          : ",
	kde_version = os.popen("kde-config --version|grep KDE").read().strip().split()[1]
	if int(kde_version[0]) != 3 or int(kde_version[2]) < 2:
		print RED + kde_version
		print RED + "Your kde version can be too old" + NORMAL
		print RED + "Please make sure kde is at least 3.2" + NORMAL
	else:
		print GREEN + kde_version + NORMAL

	## Detect the qt library
	print "Checking for the qt library       : ",
	qtdir = os.getenv("QTDIR")
	if qtdir:
		print GREEN + "qt is in " + qtdir + NORMAL
	else:
		m = re.search('(.*)/lib/libqt.*', os.popen('ldd `kde-config --expandvars --install lib`' + '/libkdeui.so.4 | grep libqt').read().strip().split()[2])
		if m:
			qtdir = m.group(1)
			print YELLOW + "qt was found as " + m.group(1) + NORMAL
		else:
			print RED + "qt was not found" + NORMAL
			print RED + "Please set QTDIR first (/usr/lib/qt3?)" + NORMAL
			sys.exit(1)
	env['QTDIR'] = qtdir.strip()

	## Find the necessary programs uic and moc
	print "Checking for uic                  : ",
	uic = qtdir + "/bin/uic"
	if os.path.isfile(uic):
		print GREEN + "uic was found as " + uic + NORMAL
	else:
		uic = os.popen("which uic 2>/dev/null").read().strip()
		if len(uic):
			print YELLOW + "uic was found as " + uic + NORMAL
		else:
			uic = os.popen("which uic 2>/dev/null").read().strip()
			if len(uic):
				print YELLOW + "uic was found as " + uic + NORMAL
			else:
				print RED + "uic was not found - set QTDIR put it in your PATH ?" + NORMAL
				sys.exit(1)
	env['QT_UIC'] = uic

	print "Checking for moc                  : ",
	moc = qtdir + "/bin/moc"
	if os.path.isfile(moc):
		print GREEN + "moc was found as " + moc + NORMAL
	else:
		moc = os.popen("which moc 2>/dev/null").read().strip()
		if len(moc):
			print YELLOW + "moc was found as " + moc + NORMAL
		elif os.path.isfile("/usr/share/qt3/bin/moc"):
			moc = "/usr/share/qt3/bin/moc"
			print YELLOW + "moc was found as " + moc + NORMAL
		else:
			print RED + "moc was not found - set QTDIR or put it in your PATH ?" + NORMAL
			sys.exit(1)
	env['QT_MOC'] = moc

	## check for the qt and kde includes
	print "Checking for the qt includes      : ",
	if qtincludes and os.path.isfile(qtincludes + "/qlayout.h"):
		# The user told where to look for and it looks valid
		print GREEN + "ok " + qtincludes + NORMAL
	else:
		if os.path.isfile(qtdir + "/include/qlayout.h"):
			# Automatic detection
			print GREEN + "ok " + qtdir + "/include/ " + NORMAL
			qtincludes = qtdir + "/include/"
		elif os.path.isfile("/usr/include/qt3/qlayout.h"):
			# Debian probably
			print YELLOW + "the qt headers were found in /usr/include/qt3/ " + NORMAL
			qtincludes = "/usr/include/qt3"
		else:
			print RED + "the qt headers were not found" + NORMAL
			sys.exit(1)

	print "Checking for the kde includes     : ",
	kdeprefix = os.popen("kde-config --prefix").read().strip()
	if not kdeincludes:
		kdeincludes = kdeprefix+"/include/"
	if os.path.isfile(kdeincludes + "/klineedit.h"):
		print GREEN + "ok " + kdeincludes + NORMAL
	else:
		if os.path.isfile(kdeprefix+"/include/kde/klineedit.h"):
			# Debian, Fedora probably
			print YELLOW + "the kde headers were found in " + kdeprefix + "/include/kde/" + NORMAL
			kdeincludes = kdeprefix + "/include/kde/"
		else:
			print RED + "The kde includes were NOT found" + NORMAL
			sys.exit(1)

	if prefix:
		## use the user-specified prefix
		if not execprefix:
			execprefix = prefix
		if not datadir:
			datadir = prefix  + "/share"
		if not libdir:
			libdir = execprefix + "/lib"

		subst_vars = lambda x: x.replace('${exec_prefix}', execprefix).replace('${datadir}', 
			datadir).replace('${libdir}', libdir)
		debian_fix = lambda x: x.replace('/usr/share', '${datadir}')
		env['KDEBIN']   = subst_vars(os.popen('kde-config --install exe').read().strip())
		env['KDEAPPS']  = subst_vars(os.popen('kde-config --install apps').read().strip())		
		env['KDEDATA']  = subst_vars(os.popen('kde-config --install data').read().strip())
		env['KDEMODULE']= subst_vars(os.popen('kde-config --install module').read().strip())
		env['KDELOCALE']= subst_vars(os.popen('kde-config --install locale').read().strip())
		env['KDEDOC']   = subst_vars( debian_fix(os.popen('kde-config --install html').read().strip()) )
		env['KDEKCFG']  = subst_vars(os.popen('kde-config --install kcfg').read().strip())
		env['KDEXDG']   = subst_vars(os.popen('kde-config --install xdgdata-apps').read().strip())
		env['KDEMENU']  = subst_vars(os.popen('kde-config --install apps').read().strip())
		env['KDEMIME']  = subst_vars(os.popen('kde-config --install mime').read().strip())
		env['KDEICONS'] = subst_vars(os.popen('kde-config --install icon').read().strip())
		env['KDESERV']  = subst_vars(os.popen('kde-config --install services').read().strip())
	else:
		# the user has given no prefix, install as a normal kde app
		env['PREFIX'] = os.popen('kde-config --prefix').read().strip()
	
		env['KDEBIN']   = os.popen('kde-config --expandvars --install exe').read().strip()
		env['KDEAPPS']  = os.popen('kde-config --expandvars --install apps').read().strip()
		env['KDEDATA']  = os.popen('kde-config --expandvars --install data').read().strip()
		env['KDEMODULE']= os.popen('kde-config --expandvars --install module').read().strip()
		env['KDELOCALE']= os.popen('kde-config --expandvars --install locale').read().strip()
		env['KDEDOC']   = os.popen('kde-config --expandvars --install html').read().strip()
		env['KDEKCFG']  = os.popen('kde-config --expandvars --install kcfg').read().strip()
		env['KDEXDG']   = os.popen('kde-config --expandvars --install xdgdata-apps').read().strip()
		env['KDEMENU']  = os.popen('kde-config --expandvars --install apps').read().strip()
		env['KDEMIME']  = os.popen('kde-config --expandvars --install mime').read().strip()
		env['KDEICONS'] = os.popen('kde-config --expandvars --install icon').read().strip()
		env['KDESERV']  = os.popen('kde-config --expandvars --install services').read().strip()

	env['QTPLUGINS']=os.popen('kde-config --expandvars --install qtplugins').read().strip()

	## kde libs and includes
	env['KDEINCLUDEPATH']= kdeincludes
	if not kdelibs:
		kdelibs = os.popen('kde-config --expandvars --install lib').read().strip()
	env['KDELIBPATH']= kdelibs

	## qt libs and includes
	env['QTINCLUDEPATH']= qtincludes
	if not qtlibs:
		qtlibs = qtdir+ "/lib"
	env['QTLIBPATH']= qtlibs


def generate(env):
	""""Set up the qt and kde environment and builders - the moc part is difficult to understand """

	env.Help("""
"""+BOLD+
"""*** KDE options ***
-------------------"""
+NORMAL+"""
"""+BOLD+"""* prefix     """+NORMAL+""": base install path,         ie: /usr/local
"""+BOLD+"""* execprefix """+NORMAL+""": install path for binaries, ie: /usr/bin
"""+BOLD+"""* datadir    """+NORMAL+""": install path for the data, ie: /usr/local/share
"""+BOLD+"""* libdir     """+NORMAL+""": install path for the libs, ie: /usr/lib
"""+BOLD+"""* kdeincludes"""+NORMAL+""": path to the kde includes (/usr/include/kde on debian, ...)
"""+BOLD+"""* qtincludes """+NORMAL+""": same punishment, for qt includes (/usr/include/qt on debian, ...)
"""+BOLD+"""* kdelibs    """+NORMAL+""": path to the kde libs, for linking the programs
"""+BOLD+"""* qtlibs     """+NORMAL+""": same punishment, for qt libraries
ie: """+BOLD+"""scons configure libdir=/usr/local/lib qtincludes=/usr/include/qt
"""+NORMAL)

	import os.path
	import re

	import SCons.Defaults
	import SCons.Tool
	import SCons.Util

	ui_extensions = [".ui", ".Ui", ".UI"]
	header_extensions = [".h", ".hxx", ".hpp", ".hh", ".H", ".HH"]
	source_extensions = [".cpp", ".cxx", ".cc", ".CPP", ".CXX", ".CC"]

	def find_file(filename, paths, node_factory):
		retval = None
		for dir in paths:
			node = node_factory(filename, dir)
			if node.rexists():
				return node
		return None

	class _Metasources:
		""" Callable class, which works as an emitter for Programs, SharedLibraries 
		and StaticLibraries."""
	
		def __init__(self, objBuilderName):
			self.objBuilderName = objBuilderName
	
		def __call__(self, target, source, env):
			""" Smart autoscan function. Gets the list of objects for the Program
			or Lib. Adds objects and builders for the special qt files. """
			try:
				if int(env.subst('$QT_AUTOSCAN')) == 0:
					return target, source
			except ValueError:
				pass

			try:
				qtdebug = int(env.subst('$QT_DEBUG'))
			except ValueError:
				qtdebug = 0

			# some shortcuts used in the scanner
			FS = SCons.Node.FS.default_fs
			splitext = SCons.Util.splitext
			objBuilder = getattr(env, self.objBuilderName)
	
			# some regular expressions:
			# Q_OBJECT detection
			q_object_search = re.compile(r'[^A-Za-z0-9]Q_OBJECT[^A-Za-z0-9]')

			# cxx and c comment 'eater'
			#comment = re.compile(r'(//.*)|(/\*(([^*])|(\*[^/]))*\*/)')
			# CW: something must be wrong with the regexp. See also bug #998222
			# CURRENTLY THERE IS NO TEST CASE FOR THAT

			# The following is kind of hacky to get builders working properly (FIXME)
			objBuilderEnv = objBuilder.env
			objBuilder.env = env
			mocBuilderEnv = env.Moc.env
			env.Moc.env = env

			# make a deep copy for the result; MocH objects will be appended
			out_sources = source[:]

			for obj in source:
				if not obj.has_builder():
					# binary obj file provided
					if qtdebug:
						print "scons: qt: '%s' seems to be a binary. Discarded." % str(obj)
					continue
				cpp = obj.sources[0]
				if not splitext(str(cpp))[1] in source_extensions:
					if qtdebug:
						print "scons: qt: '%s' is no cxx file. Discarded." % str(cpp)
					# c or fortran source
					continue
				#cpp_contents = comment.sub('', cpp.get_contents())
				cpp_contents = cpp.get_contents()
	
				h = None
				ui = None
	
				for ui_ext in ui_extensions:
					# try to find the ui file in the corresponding source directory
					uiname = splitext(cpp.name)[0] + ui_ext
					ui = find_file(uiname, (cpp.get_dir(),), FS.File)
					if ui:
						if qtdebug:
							print "scons: qt: found .ui file of header" #% (str(h), str(cpp))
							#h_contents = comment.sub('', h.get_contents())
						break
	
				# if we have a .ui file, do not continue, it is automatically handled by Uic
				if ui:
					continue

				for h_ext in header_extensions:
					# try to find the header file in the corresponding source
					# directory
					hname = splitext(cpp.name)[0] + h_ext
					h = find_file(hname, (cpp.get_dir(),), FS.File)
					if h:
						if qtdebug:
							print "scons: qt: Scanning '%s' (header of '%s')" % (str(h), str(cpp))
						#h_contents = comment.sub('', h.get_contents())
						h_contents = h.get_contents()
						break
	
				if not h and qtdebug:
					print "scons: qt: no header for '%s'." % (str(cpp))
				if h and q_object_search.search(h_contents):
					# h file with the Q_OBJECT macro found -> add .moc or _moc.cpp file
					moc_cpp = None

					if env.has_key('NOMOCSCAN'):
						moc_cpp = env.Moc(h)
					else:
						reg = '\n\s*#include\s+"'+splitext(cpp.name)[0]+'.moc"'
						meta_object_search = re.compile(reg)
						if meta_object_search.search(cpp_contents):
							moc_cpp = env.Moc(h)
						else:
							moc_cpp = env.Moccpp(h)
							moc_o = objBuilder(moc_cpp)
							out_sources.append(moc_o)
					if qtdebug:
						print "scons: qt: found Q_OBJECT macro in '%s', moc'ing to '%s'" % (str(h), str(moc_cpp[0]))
	
				if cpp and q_object_search.search(cpp_contents):
					print "error, bksys cannot handle cpp files with Q_OBJECT classes"
					print "if you are sure this is a feature worth the effort, "
					print "report this to the authors tnagyemail-mail yahoo.fr"

			# restore the original env attributes (FIXME)
			objBuilder.env = objBuilderEnv
			env.Moc.env = mocBuilderEnv

			return (target, out_sources)
	
	MetasourcesShared = _Metasources('SharedObject')
	MetasourcesStatic = _Metasources('StaticObject')

	CLVar = SCons.Util.CLVar
	splitext = SCons.Util.splitext
	Builder = SCons.Builder.Builder
	
	# Detect the environment - replaces ./configure implicitely
	# and store the options into a cache
	from SCons.Options import Options
	opts = Options('kde.cache.py')
	opts.AddOptions(
		( 'QTDIR', 'root of qt directory' ),
		( 'QTLIBPATH', 'path to the qt libraries' ),
		( 'QTINCLUDEPATH', 'path to the qt includes' ),
		( 'QT_UIC', 'moc directory'),
		( 'QT_MOC', 'moc executable command'),
		( 'QTPLUGINS', 'uic executable command'),
		( 'KDEDIR', 'root of kde directory' ),
		( 'KDELIBPATH', 'path to the kde libs' ),
		( 'KDEINCLUDEPATH', 'path to the kde includes' ),

		( 'PREFIX', 'root of the program installation'),

		( 'KDEBIN', 'installation path of the kde binaries'),
		( 'KDEMODULE', 'installation path of the parts and libs'),
		( 'KDEAPPS', ''),
		( 'KDEDATA', 'installation path of the application data'),
		( 'KDELOCALE', ''),
		( 'KDEDOC', 'installation path of the application documentation'),
		( 'KDEKCFG', 'installation path of the .kcfg files'),
		( 'KDEXDG', 'installation path of the service types'),
		( 'KDEMENU', ''),
		( 'KDEMIME', 'installation path of to the mimetypes'),
		( 'KDEICONS', ''),
		( 'KDESERV', ''),
	)
	opts.Update(env)

	# reconfigure when things are missing
	if 'configure' in env['TARGS'] or not env.has_key('QTDIR') or not env.has_key('KDEDIR'):
		detect_kde(env)

	# finally save the configuration
	opts.Save('kde.cache.py', env)

	## set default variables, one can override them in sconscript files
	env.Append(CXXFLAGS = ['-I'+env['KDEINCLUDEPATH'], '-I'+env['QTINCLUDEPATH'] ])
	env.Append(LIBPATH = [env['KDELIBPATH'], env['QTLIBPATH'] ])
	
	env['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME'] = 1
	
	env['QT_AUTOSCAN']     = 1
	env['QT_DEBUG']        = 0
	
	env['QT_UIC_HFLAGS']   = '-L $QTPLUGINS -nounload'
	env['QT_UIC_CFLAGS']   = '$QT_UIC_HFLAGS -tr tr2i18n'
	env['QT_LIBS']         = 'qt-mt'
	
	env['LIBTOOL_FLAGS']   = '--silent --mode=compile --tag=CXX'
	
	env['QT_UICIMPLPREFIX'] = ''
	env['QT_UICIMPLSUFFIX'] = '.cpp'
	env['QT_MOCHPREFIX']    = ''
	env['QT_MOCHSUFFIX']    = '.moc'
	env['KDE_KCFG_IMPLPREFIX'] = ''
	env['KDE_KCFG_IMPL_HSUFFIX'] = '.h'
	env['KDE_KCFG_IMPL_CSUFFIX'] = '.cpp'
	env['KDE_SKEL_IMPL_SUFFIX']  = '.skel'
	env['MEINPROC']              = 'meinproc'
	env['MSGFMT']                = 'msgfmt'


	###### ui file processing
	def uicGenerator(target, source, env, for_signature):
		act=[]
		act.append('$QT_UIC $QT_UIC_HFLAGS -o '+target[0].path+' '+source[0].path)
		act.append('rm -f '                           +target[1].path)
		act.append('echo \'#include <klocale.h>\' >> '+target[1].path)
		act.append('echo \'#include <kdialog.h>\' >> '+target[1].path)
		act.append('$QT_UIC $QT_UIC_CFLAGS -impl '+target[0].path+' -o '+target[1].path+'.tmp '+source[0].path)
		act.append('cat '+target[1].path+'.tmp >> '+target[1].path)
		act.append('rm -f '+target[1].path+'.tmp')
		act.append('echo \'#include "' + target[2].name + '"\' >> '+target[1].path)
		act.append('$QT_MOC -o '+target[2].path+' '+target[0].path)
		return act
	
	def uicEmitter(target, source, env):
		adjustixes = SCons.Util.adjustixes
		bs = SCons.Util.splitext(str(source[0].name))[0]
		bs = os.path.join(str(target[0].get_dir()),bs)
		# first target is automatically added by builder (.h file)
		if len(target) < 2:
			# second target is .cpp file
			target.append(adjustixes(bs,
				env.subst('$QT_UICIMPLPREFIX'),
				env.subst('$QT_UICIMPLSUFFIX')))
		if len(target) < 3:
			# third target is .moc file
			target.append(adjustixes(bs,
				env.subst('$QT_MOCHPREFIX'),
				env.subst('$QT_MOCHSUFFIX')))
		return target, source
	
	UIC_BUILDER = Builder(
		generator  = uicGenerator,
		emitter    = uicEmitter,
		suffix     = '.h',
		src_suffix = '.ui' )
	
	###### moc file processing
	env['QT_MOCCOM'] = ('$QT_MOC -o ${TARGETS[0]} $SOURCE')
	
	MOC_BUILDER = Builder(
		action     = '$QT_MOCCOM',
		suffix     = '.moc',
		src_suffix = '.h' )
	
	MOCCPP_BUILDER = Builder(
		action     = '$QT_MOCCOM',
		suffix     = '_moc.cpp',
		src_suffix = '.h' )
	
	###### kcfg file processing
	def kcfgGenerator(target, source, env, for_signature):
		act=[]
		act.append('kconfig_compiler -d'+str(source[0].get_dir())+' '+source[1].path+' '+source[0].path)
		return act
	
	def kcfgEmitter(target, source, env):
		adjustixes = SCons.Util.adjustixes
		bs = SCons.Util.splitext(str(source[0].name))[0]
		bs = os.path.join(str(target[0].get_dir()),bs)
		# first target is automatically added by builder (.h file)
		if len(target) < 2:
			# second target is .cpp file
			target.append(adjustixes(bs, env.subst('$KDE_KCFG_IMPLPREFIX'), env.subst('$KDE_KCFG_IMPL_CSUFFIX')))
	
		#    find_file(kcfgfile, (source[0].get_dir(),) ,SCons.Node.FS.default_fs)
		if len(source) <2:
			if not os.path.isfile(str(source[0])):
				print RED+'kcfg file given'+str(source[0])+' does not exist !'+NORMAL
				return target, source
			kcfgfilename = os.popen("cat "+str(source[0])+"|grep File|sed 's/File.*=//i'").read().rstrip()
			source.append( str(source[0].get_dir())+'/'+kcfgfilename )
		return target, source

	KCFG_BUILDER = Builder(
		generator  = kcfgGenerator,
		emitter    = kcfgEmitter,
		suffix     = '.h',
		src_suffix = '.kcfgc' )
	
	###### dcop processing
	def dcopGenerator(target, source, env, for_signature):
		act=[]
		act.append('dcopidl '+source[0].path+' > '+target[1].path+'|| ( rm -f '+target[1].path+' ; false )')
		act.append('dcopidl2cpp --c++-suffix cpp --no-signals --no-stub '+target[1].path)
		return act

	def dcopEmitter(target, source, env):
		bs = SCons.Util.splitext(str(source[0].name))[0]
		bs = os.path.join(str(target[0].get_dir()),bs)
		target.append(bs+'.kidl')
		#target.append(bs+'_skel.cpp')
		return target, source

	DCOP_BUILDER = Builder(
		generator  = dcopGenerator,
		emitter    = dcopEmitter,
		suffix     = '_skel.cpp',
		src_suffix = '.h' )

	###### documentation (meinproc) processing
	MEINPROC_BUILDER = Builder(
		action     = '$MEINPROC --check --cache $TARGET $SOURCE',
		suffix     = '.cache.bz2',
		src_suffix = '.docbook' )

	###### translation files builder
	TRANSFILES_BUILDER = Builder(
		action     = '$MSGFMT $SOURCE -o $TARGET',
		suffix     = '.gmo',
		src_suffix = '.po' )

	###### libtool file builder
	def laGenerator(target, source, env, for_signature):
		act=[]
		act.append('echo "dlname=\''+source[0].name+'\'" > '+target[0].path)
		act.append('echo "library_names=\''+source[0].name+' '+source[0].name+' '+source[0].name+'\'" >> '+target[0].path)
		act.append('echo "old_library=\'\'">> '+target[0].path)
		act.append('echo "dependency_libs=\'\'">> '+target[0].path)
		act.append('echo "current=0">> '+target[0].path)
		act.append('echo "age=0">> '+target[0].path)
		act.append('echo "revision=0">> '+target[0].path)
		act.append('echo "installed=yes">> '+target[0].path)
		act.append('echo "shouldnotlink=no">> '+target[0].path)
		act.append('echo "dlopen=\'\'">> '+target[0].path)
		act.append('echo "dlpreopen=\'\'">> '+target[0].path)
		act.append('echo "libdir=\''+env['KDEMODULE']+'\'" >> '+target[0].path)
		return act
	
	LA_BUILDER = Builder(
		generator  = laGenerator,
		suffix     = '.la',
		src_suffix = '.so' )
	
####### TODO : real libtool builder (but i hate libtool - ita)
#	def libtoolGenerator(target, source, env, for_signature):
#		act=[]
#		act.append('libtool $LIBTOOL_FLAGS $CXX $CXXFLAGS $CPPFLAGS $_CPPDEFFLAGS $_CPPINCFLAGS -c -o '+target[0].path+' '+source[0].path)
#		return act
#	LIBTOOL_BUILDER = Builder(
#		generator  = libtoolGenerator,
#		suffix     = '.lo',
#		src_suffix = '.cpp' )

	##### register the builders
	env['BUILDERS']['Uic']        = UIC_BUILDER
	env['BUILDERS']['Moc']        = MOC_BUILDER
	env['BUILDERS']['Moccpp']     = MOCCPP_BUILDER
	env['BUILDERS']['Dcop']       = DCOP_BUILDER
	env['BUILDERS']['Kcfg']       = KCFG_BUILDER
	env['BUILDERS']['LaFile']     = LA_BUILDER
	#env['BUILDERS']['Libtool']    = LIBTOOL_BUILDER
	env['BUILDERS']['Meinproc']   = MEINPROC_BUILDER
	env['BUILDERS']['Transfiles'] = TRANSFILES_BUILDER

	static_obj, shared_obj = SCons.Tool.createObjBuilders(env)
	static_obj.src_builder.append('Uic')
	shared_obj.src_builder.append('Uic')
	static_obj.src_builder.append('Kcfg')
	shared_obj.src_builder.append('Kcfg')
	static_obj.src_builder.append('LaFile')
	shared_obj.src_builder.append('LaFile')
	static_obj.src_builder.append('Meinproc')
	shared_obj.src_builder.append('Meinproc')
	static_obj.src_builder.append('Transfiles')
	shared_obj.src_builder.append('Transfiles')

	## find the files to moc, dcop, and link against kde and qt
	env.AppendUnique(PROGEMITTER = [MetasourcesStatic], SHLIBEMITTER=[MetasourcesShared], LIBEMITTER =[MetasourcesStatic])

	###########################################
	## Handy helpers for building kde programs
	## You should not have to modify them ..

	import SCons.Util
	skel_ext = [".skel", ".SKEL"]
	def KDEfiles(target, source, env):
		"""
		Returns a list of files for scons (handles kde tricks like .skel) 
		It also makes custom checks against double includes like : ['file.ui', 'file.cpp']
		(file.cpp is already included because of file.ui)
		"""
		src=[]
		ui_files=[]
		kcfg_files=[]
		skel_files=[]
		other_files=[]

		# For each file, check wether it is a dcop file or not, and create the complete list of sources
		for file in source:
			bs  = SCons.Util.splitext(file)[0]
			ext = SCons.Util.splitext(file)[1]
			if ext in skel_ext:
				env.Dcop(bs+'.h')
				src.append(bs+'_skel.cpp')
			else:
				src.append(file)

			if ext == '.ui':
				ui_files.append(bs)
			elif ext == '.kcfgc':
				kcfg_files.append(bs)
			elif ext == '.skel':
				skel_files.append(bs)
			else:
				other_files.append(bs)	
		
		# Now check against newbie errors
		for file in ui_files:
			for ofile in other_files:
				if ofile == file:
					print RED+"WARNING: You have included "+file+".ui and another file of the same prefix"+NORMAL
					print "Files generated by uic (file.h, file.cpp must not be included"
		for file in kcfg_files:
			for ofile in other_files:
				if ofile == file:
					print RED+"WARNING: You have included "+file+".kcfg and another file of the same prefix"+NORMAL
					print "Files generated by kconfig_compiler (settings.h, settings.cpp) must not be included"
		#for file in skel_files:
		#	for ofile in other_files:
		#		if ofile == file:
		#			print RED+"WARNING: source contain "+file+".skel and another file of the same prefix"+NORMAL
		#			print "Files generated automatically from .skel file must not be included (file.h, file.idl)"
		
		return src

	def KDEprogram(target, source, lenv):
		""" Makes a kde program """
		src = KDEfiles(target, source, lenv)
		lenv.Program(target, src)
		if 'install' in env['TARGS']:
			env.Alias('install', env.Install( env['KDEBIN'], target ) )

	def KDEshlib(target, source, lenv):
		""" Makes a shared library for kde (.la file for klibloader) """
		src = KDEfiles(target, source, lenv)
		lenv.SharedLibrary( target, src )
		lenv.LaFile( target, target+'.so' )
		if 'install' in env['TARGS']:
			env.Alias('install', env.Install( env['KDEMODULE'], target+'.so' ) )
			env.Alias('install', env.Install( env['KDEMODULE'], target+'.la' ) )

	def KDEstaticlib(target, source, lenv):
		""" Makes a static library for kde - in practice you should not use static libraries 
		1. they take more memory than shared ones
		2. makefile.am needed it because of stupid limitations
		(cannot handle sources in separate folders - takes extra processing) """
		src = KDEfiles(target, source, lenv)
		lenv.StaticLibrary( target, src )
#		# do not install static libraries
#		if 'install' in env['TARGS']:
#			env.Alias('install', env.Install( env['KDEMODULE'], target+'.a' ) )

	def KDEaddlibs(libs, env):
		""" Helper function """
		env.AppendUnique(LIBS = libs)

	def KDEaddpaths(paths, env):
		""" Helper function """
		env.AppendUnique(CPPPATH = paths)

	def KDEinstall(path, file, lenv):
		""" Quick wrapper """
		if 'install' in env['TARGS']:
			env.Alias('install', env.Install( path, file ) )

	def KDEinstallas(destfile, file, lenv):
		""" Quick wrapper """
		if 'install' in env['TARGS']:
			env.Alias('install', env.InstallAs( destfile, file ) )

	def KDElang(transfiles, lenv):
		""" Process translations (.po files) in a po/ dir """
		if not lenv['APPNAME']:
			print "define lenv['APPNAME'] before using KDElang !!"
			return
		for lang in transfiles:
			lenv.Transfiles( lang+'.po' )
			KDEinstallas( lenv['KDELOCALE']+'/'+lang+'/LC_MESSAGES/'+lenv['APPNAME']+'.mo', 
				lang+'.gmo', lenv )

	def KDEdoc(lang, file, lenv):
		""" Install the documentation """
		if not lenv['APPNAME']:
			print "define lenv['APPNAME'] before using KDEdoc !!"
			return
		KDEinstall( lenv['KDEDOC']+'/'+lang+'/'+lenv['APPNAME'], file, lenv )

	# Export variables so that sconscripts in subdirectories can use them
	env.Export("KDEprogram KDEshlib KDEaddpaths KDEaddlibs KDEinstall KDEinstallas KDElang KDEdoc")
