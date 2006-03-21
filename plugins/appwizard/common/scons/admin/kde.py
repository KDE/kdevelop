# Made from scons qt.py and (heavily) modified into kde.py
# Thomas Nagy, 2004, 2005 <tnagy2^8@yahoo.fr>

"""
Run scons -h to display the associated help, or look below ..
"""

BOLD   ="\033[1m"
RED    ="\033[91m"
GREEN  ="\033[92m"
YELLOW ="\033[1m" #"\033[93m" # unreadable on white backgrounds
CYAN   ="\033[96m"
NORMAL ="\033[0m"

import os, re, types

# Returns the name of the shared object (i.e. libkdeui.so.4)
# referenced by a libtool archive (like libkdeui.la)
def getSOfromLA(lafile):
	contents = open(lafile, 'r').read()
	match = re.search("^dlname='([^']*)'$", contents, re.M)
	if match:
		return match.group(1)
	return None

def exists(env):
	return True

def detect_kde(env):
	""" Detect the qt and kde environment using kde-config mostly """

	prefix      = env['ARGS'].get('prefix', None)
	execprefix  = env['ARGS'].get('execprefix', None)
	datadir     = env['ARGS'].get('datadir', None)
	libdir      = env['ARGS'].get('libdir', None)
	libsuffix   = env['ARGS'].get('libsuffix', '')
	kdeincludes = env['ARGS'].get('kdeincludes', None)
	kdelibs     = env['ARGS'].get('kdelibs', None)
	qtincludes  = env['ARGS'].get('qtincludes', None)
	qtlibs      = env['ARGS'].get('qtlibs', None)

	if libdir:
		libdir = libdir+libsuffix

	## Detect the kde libraries
	print "Checking for kde-config           : ",
	kde_config = os.popen("which kde-config 2>/dev/null").read().strip()
	if len(kde_config):
		print GREEN+"kde-config was found"+NORMAL
	else:
		print RED+"kde-config was NOT found in your PATH"+NORMAL
		print "Make sure kde is installed properly"
		print "(missing package kdebase-devel?)"
		env.Exit(1)
	env['KDEDIR'] = os.popen('kde-config -prefix').read().strip()

	print "Checking for kde version          : ",
	kde_version = os.popen("kde-config --version|grep KDE").read().strip().split()[1]
	if int(kde_version[0]) != 3 or int(kde_version[2]) < 2:
		print RED+kde_version
		print RED+"Your kde version can be too old"+NORMAL
		print RED+"Please make sure kde is at least 3.2"+NORMAL
	else:
		print GREEN+kde_version+NORMAL

	## Detect the qt library
	print "Checking for the qt library       : ",
	qtdir = os.getenv("QTDIR")
	if qtdir:
		print GREEN+"qt is in "+qtdir+NORMAL
	else:
		libdir = os.popen('kde-config --expandvars --install lib').read().strip()
		libkdeuiSO = libdir+'/'+getSOfromLA(libdir+'/libkdeui.la')
		m = re.search('(.*)/lib/libqt.*', os.popen('ldd ' + libkdeuiSO + ' | grep libqt').read().strip().split()[2])
		if m:
			qtdir = m.group(1)
			print YELLOW+"qt was found as "+m.group(1)+NORMAL
		else:
			print RED+"qt was not found"+NORMAL
			print RED+"Please set QTDIR first (/usr/lib/qt3?)"+NORMAL
			env.Exit(1)
	env['QTDIR'] = qtdir.strip()

	## Find the necessary programs uic and moc
	print "Checking for uic                  : ",
	uic = qtdir + "/bin/uic"
	if os.path.isfile(uic):
		print GREEN+"uic was found as "+uic+NORMAL
	else:
		uic = os.popen("which uic 2>/dev/null").read().strip()
		if len(uic):
			print YELLOW+"uic was found as "+uic+NORMAL
		else:
			uic = os.popen("which uic 2>/dev/null").read().strip()
			if len(uic):
				print YELLOW+"uic was found as "+uic+NORMAL
			else:
				print RED+"uic was not found - set QTDIR put it in your PATH ?"+NORMAL
				env.Exit(1)
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
			env.Exit(1)
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
			env.Exit(1)

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
			env.Exit(1)

	if prefix:
		## use the user-specified prefix
		if not execprefix:
			execprefix = prefix
		if not datadir:
			datadir=prefix+"/share"
		if not libdir:
			libdir=execprefix+"/lib"+libsuffix

		subst_vars = lambda x: x.replace('${exec_prefix}', execprefix).replace('${datadir}',datadir).replace('${libdir}', libdir)
		debian_fix = lambda x: x.replace('/usr/share', '${datadir}')
		env['KDEBIN']   = subst_vars(os.popen('kde-config --install exe').read().strip())
		env['KDEAPPS']  = subst_vars(os.popen('kde-config --install apps').read().strip())		
		env['KDEDATA']  = subst_vars(os.popen('kde-config --install data').read().strip())
		env['KDEMODULE']= subst_vars(os.popen('kde-config --install module').read().strip())
		env['KDELOCALE']= subst_vars(os.popen('kde-config --install locale').read().strip())
		env['KDEDOC']   = subst_vars( debian_fix(os.popen('kde-config --install html').read().strip()) )
		env['KDEKCFG']  = subst_vars(os.popen('kde-config --install kcfg').read().strip())
		env['KDEXDG']   = subst_vars(os.popen('kde-config --install xdgdata-apps').read().strip())
		env['KDEXDGDIR']= subst_vars(os.popen('kde-config --install xdgdata-dirs').read().strip())
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
		env['KDEXDGDIR']= os.popen('kde-config --expandvars --install xdgdata-dirs').read().strip()
		env['KDEMENU']  = os.popen('kde-config --expandvars --install apps').read().strip()
		env['KDEMIME']  = os.popen('kde-config --expandvars --install mime').read().strip()
		env['KDEICONS'] = os.popen('kde-config --expandvars --install icon').read().strip()
		env['KDESERV']  = os.popen('kde-config --expandvars --install services').read().strip()

	env['QTPLUGINS']=os.popen('kde-config --expandvars --install qtplugins').read().strip()

	## kde libs and includes
	env['KDEINCLUDEPATH']=kdeincludes
	if not kdelibs:
		kdelibs=os.popen('kde-config --expandvars --install lib').read().strip()
	env['KDELIBPATH']=kdelibs

	## qt libs and includes
	env['QTINCLUDEPATH']=qtincludes
	if not qtlibs:
		qtlibs=qtdir+"/lib"
	env['QTLIBPATH']=qtlibs

def generate(env):
	""""Set up the qt and kde environment and builders - the moc part is difficult to understand """
	if env['HELP']:
		print """
"""+BOLD+"""*** KDE options ***
-------------------"""+NORMAL+"""
"""+BOLD+"""* prefix     """+NORMAL+""": base install path,         ie: /usr/local
"""+BOLD+"""* execprefix """+NORMAL+""": install path for binaries, ie: /usr/bin
"""+BOLD+"""* datadir    """+NORMAL+""": install path for the data, ie: /usr/local/share
"""+BOLD+"""* libdir     """+NORMAL+""": install path for the libs, ie: /usr/lib
"""+BOLD+"""* libsuffix  """+NORMAL+""": suffix of libraries on amd64, ie: 64, 32
"""+BOLD+"""* kdeincludes"""+NORMAL+""": path to the kde includes (/usr/include/kde on debian, ...)
"""+BOLD+"""* qtincludes """+NORMAL+""": same punishment, for qt includes (/usr/include/qt on debian, ...)
"""+BOLD+"""* kdelibs    """+NORMAL+""": path to the kde libs, for linking the programs
"""+BOLD+"""* qtlibs     """+NORMAL+""": same punishment, for qt libraries
ie: """+BOLD+"""scons configure libdir=/usr/local/lib qtincludes=/usr/include/qt
"""+NORMAL

	import SCons.Defaults
	import SCons.Tool
	import SCons.Util

	ui_extensions = [".ui"]
	header_extensions = [".h", ".hxx", ".hpp", ".hh"]
	source_extensions = [".cpp", ".cxx", ".cc"]

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

			# The following is kind of hacky to get builders working properly (FIXME) ??
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
						reg = '\n\s*#include\s*("|<)'+splitext(cpp.name)[0]+'.moc("|>)'
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
	
	# Detect the environment - replaces ./configure implicitely and store the options into a cache
	from SCons.Options import Options
	cachefile=env['CACHEDIR']+'kde.cache.py'
	opts = Options(cachefile)
	opts.AddOptions(
		('PREFIX', 'root of the program installation'),

		('QTDIR', 'root of qt directory'),
		('QTLIBPATH', 'path to the qt libraries'),
		('QTINCLUDEPATH', 'path to the qt includes'),
		('QT_UIC', 'moc directory'),
		('QT_MOC', 'moc executable command'),
		('QTPLUGINS', 'uic executable command'),
		('KDEDIR', 'root of kde directory'),
		('KDELIBPATH', 'path to the kde libs'),
		('KDEINCLUDEPATH', 'path to the kde includes'),

		('KDEBIN', 'installation path of the kde binaries'),
		('KDEMODULE', 'installation path of the parts and libs'),
		('KDEAPPS', ''),
		('KDEDATA', 'installation path of the application data'),
		('KDELOCALE', ''),
		('KDEDOC', 'installation path of the application documentation'),
		('KDEKCFG', 'installation path of the .kcfg files'),
		('KDEXDG', 'installation path of the service types'),
		('KDEXDGDIR', 'installation path of the xdg service directories'),
		('KDEMENU', ''),
		('KDEMIME', 'installation path of to the mimetypes'),
		('KDEICONS', ''),
		('KDESERV', ''),
	)
	opts.Update(env)

	# reconfigure when things are missing
	if not env['HELP'] and (env['_CONFIGURE'] or not env.has_key('QTDIR') or not env.has_key('KDEDIR')):
		detect_kde(env)

		# finally save the configuration to the cache file
		opts.Save(cachefile, env)

	## set default variables, one can override them in sconscript files
	env.Append(CXXFLAGS = ['-I'+env['KDEINCLUDEPATH'], '-I'+env['QTINCLUDEPATH'] ])
	env.Append(LIBPATH = [env['KDELIBPATH'], env['QTLIBPATH'] ])
	
	env['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME'] = 1
	
	env['QT_AUTOSCAN'] = 1
	env['QT_DEBUG']    = 0
	env['QT_UIC_HFLAGS']   = '-L $QTPLUGINS -nounload'
	env['QT_UIC_CFLAGS']   = '$QT_UIC_HFLAGS -tr tr2i18n'
	env['QT_LIBS']      = 'qt-mt'
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

	## ui file processing
	def uic_processing(target, source, env):
		inc_kde  = '#include <klocale.h>\n#include <kdialog.h>\n'
		inc_moc  = '#include "%s"\n' % target[2].name
		comp_h   = '$QT_UIC $QT_UIC_HFLAGS -o %s %s' % (target[0].path, source[0].path)
		comp_c   = '$QT_UIC $QT_UIC_CFLAGS -impl %s %s' % (target[0].path, source[0].path)
		comp_moc = '$QT_MOC -o %s %s' % (target[2].path, target[0].path)
		
		ret = env.Execute(comp_h)
		if ret:
			return ret
		
		dest = open( target[1].path, "w" )
		dest.write(inc_kde)
		dest.close()

		ret = env.Execute( comp_c+" >> "+target[1].path )
		if ret:
			return ret

		dest = open( target[1].path, "a" )
		dest.write(inc_moc)
		dest.close()

		ret = env.Execute( comp_moc )
		return ret
	
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
		action     = uic_processing,
		emitter    = uicEmitter,
		suffix     = '.h',
		src_suffix = '.ui')
	
	## moc file processing
	env['QT_MOCCOM'] = ('$QT_MOC -o ${TARGETS[0]} $SOURCE')
	
	MOC_BUILDER = Builder(
		action     = '$QT_MOCCOM',
		suffix     = '.moc',
		src_suffix = '.h')
	
	MOCCPP_BUILDER = Builder(
		action     = '$QT_MOCCOM',
		suffix     = '_moc.cpp',
		src_suffix = '.h')
	
	## kcfg file processing
	def kcfgGenerator(target, source, env, for_signature):
		act=[]
		act.append('kconfig_compiler -d'+str(source[0].get_dir())+' '+source[1].path+' '+source[0].path)
		return act
	
	def kcfgEmitter(target, source, env):
		adjustixes = SCons.Util.adjustixes
		bs = SCons.Util.splitext(str(source[0].name))[0]
		bs = os.path.join(str(target[0].get_dir()),bs)
		# .h file is automatically added
		if len(target) < 2:
			# add .cpp file
			target.append(adjustixes(bs, env.subst('$KDE_KCFG_IMPLPREFIX'), env.subst('$KDE_KCFG_IMPL_CSUFFIX')))
	
		if len(source) <2:
			if not os.path.isfile(str(source[0])):
				print RED+'kcfg file given'+str(source[0])+' does not exist !'+NORMAL
				return target, source
			kfcgfilename=""
			kcfgFileDeclRx = re.compile("^[fF]ile\s*=\s*(.+)\s*$")
			for line in file(str(source[0]), "r").readlines():
				match = kcfgFileDeclRx.match(line.strip())
				if match:
					kcfgfilename = match.group(1)
					break
			source.append(str(source[0].get_dir())+'/'+kcfgfilename)
		return target, source

	KCFG_BUILDER = Builder(
		generator  = kcfgGenerator,
		emitter    = kcfgEmitter,
		suffix     = '.h',
		src_suffix = '.kcfgc')
	
	## dcop processing
	def dcopGenerator(target, source, env, for_signature):
		act=[]
		act.append('dcopidl '+source[0].path+' > '+target[1].path+'|| ( rm -f '+target[1].path+' ; false)')
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
		src_suffix = '.h')

	## documentation processing
	MEINPROC_BUILDER = Builder(
		action     = '$MEINPROC --check --cache $TARGET $SOURCE',
		suffix     = '.cache.bz2')

	## translation files builder
	TRANSFILES_BUILDER = Builder(
		action     = '$MSGFMT $SOURCE -o $TARGET',
		suffix     = '.gmo',
		src_suffix = '.po')

	## libtool file builder
	def la_file(target, source, env):
		dest=open(target[0].path, 'w')
		sname=source[0].name
		dest.write("dlname='%s'\n" % sname)
		dest.write("library_names='%s %s %s'\n" % (sname, sname, sname))
		dest.write("old_library=''\n")
		dest.write("dependency_libs=''\n")
		dest.write("current=0\n")
		dest.write("age=0\n")
		dest.write("revision=0\n")
		dest.write("installed=yes\n")
		dest.write("shouldnotlink=no\n")
		dest.write("dlopen=''\n")
		dest.write("dlpreopen=''\n")
		dest.write("libdir='%s'" % env['KDEMODULE'])
		dest.close()
		return 0

	LA_BUILDER = Builder(
		action     = la_file,
		suffix     = '.la',
		src_suffix = env['SHLIBSUFFIX'])
	
	## register the builders
	env['BUILDERS']['Uic']       = UIC_BUILDER
	env['BUILDERS']['Moc']       = MOC_BUILDER
	env['BUILDERS']['Moccpp']    = MOCCPP_BUILDER
	env['BUILDERS']['Dcop']      = DCOP_BUILDER
	env['BUILDERS']['Kcfg']      = KCFG_BUILDER
	env['BUILDERS']['LaFile']    = LA_BUILDER
	env['BUILDERS']['Meinproc']  = MEINPROC_BUILDER
	env['BUILDERS']['Transfiles']= TRANSFILES_BUILDER

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

	## Find the files to moc, dcop, and link against kde and qt
	env.AppendUnique(PROGEMITTER = [MetasourcesStatic], SHLIBEMITTER=[MetasourcesShared], LIBEMITTER =[MetasourcesStatic])

	## Handy helpers for building kde programs
	## You should not have to modify them ..

	## return a list of things
	def make_list(e):
		if type(e) is types.ListType:
			return e
		else:
			return e.split()

	#import SCons.Util
	skel_ext = [".skel", ".SKEL"]
	def KDEfiles(lenv, target, source):
		""" Returns a list of files for scons (handles kde tricks like .skel) 
		It also makes custom checks against double includes like : ['file.ui', 'file.cpp']
		(file.cpp is already included because of file.ui) """

		src=[]
		ui_files=[]
		kcfg_files=[]
		skel_files=[]
		other_files=[]

		source_=make_list(source)

		# For each file, check wether it is a dcop file or not, and create the complete list of sources
		for file in source_:
			bs  = SCons.Util.splitext(file)[0]
			ext = SCons.Util.splitext(file)[1]
			if ext in skel_ext:
				lenv.Dcop(bs+'.h')
				src.append(bs+'_skel.cpp')
			elif ext == ".moch":
				lenv.Moccpp(bs+'.h')
				src.append(bs+'_moc.cpp')
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
		
		# Now check against typical newbie errors
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
		return src


	"""
	In the future, these functions will contain the code that will dump the
	configuration for re-use from an IDE
	"""
	import glob
	def KDEinstall(lenv, restype, subdir, files):
		if not env['_INSTALL']:
			return
		basedir=env['DESTDIR']
		if len(restype)>0:
			if not lenv.has_key(restype):
				print RED+"unknown resource type "+restype+NORMAL
			else:
				basedir += lenv[restype]+'/'
		#print file # <- useful to trace stuff :)
		install_list =  env.Install(basedir+subdir+'/', files)
		env.Alias('install', install_list)
		return install_list

	def KDEinstallas(lenv, restype, destfile, file):
		if not env['_INSTALL']:
			return
		basedir=env['DESTDIR']
		if len(restype)>0:
			if not lenv.has_key(restype):
				print RED+"unknown resource type "+restype+NORMAL
			else:
				basedir += lenv[restype]+'/'
		install_list = env.InstallAs(basedir+destfile, file)
		env.Alias('install', install_list)
		return install_list

	def KDEprogram(lenv, target, source):
		""" Makes a kde program 
		The program is installed except if one sets env['NOAUTOINSTALL'] """
		src = KDEfiles(lenv, target, source)
		program_list = lenv.Program(target, src)
		if not lenv.has_key('NOAUTOINSTALL'):
			KDEinstall(lenv, 'KDEBIN', '', target)
		return program_list

	def KDEshlib(lenv, target, source, kdelib=0, libprefix='lib'):
		""" Makes a shared library for kde (.la file for klibloader)
		The library is installed except if one sets env['NOAUTOINSTALL'] """
		src = KDEfiles(lenv, target, source)
		lenv['LIBPREFIX']=libprefix
		library_list = lenv.SharedLibrary(target, src)
		lafile_list  = lenv.LaFile(target, library_list)
		if not lenv.has_key('NOAUTOINSTALL'):
			install_dir = 'KDEMODULE'
			if kdelib==1:
				install_dir = 'KDELIBPATH'
			KDEinstall(lenv, install_dir, '', library_list)
			KDEinstall(lenv, install_dir, '', lafile_list)
		return library_list

	def KDEstaticlib(lenv, target, source):
		""" Makes a static library for kde - in practice you should not use static libraries 
		1. they take more memory than shared ones
		2. makefile.am needed it because of limitations
		(cannot handle sources in separate folders - takes extra processing) """
		src = KDEfiles(lenv, target, source)
		return lenv.StaticLibrary(target, src)
		# do not install static libraries by default

	def KDEaddflags_cxx(lenv, fl):
		""" Compilation flags for C++ programs """
		lenv.AppendUnique(CXXFLAGS = make_list(fl))
	
	def KDEaddflags_c(lenv, fl):
		""" Compilation flags for C programs """
		lenv.AppendUnique(CFLAGS = make_list(fl))

	def KDEaddflags_link(lenv, fl):
		""" Add link flags - Use this if KDEaddlibs below is not enough """
		lenv.AppendUnique(LINKFLAGS = make_list(fl))

	def KDEaddlibs(lenv, libs):
		""" Helper function """
		lenv.AppendUnique(LIBS = make_list(libs))

	def KDEaddpaths_includes(lenv, paths):
		""" Add new include paths """
		lenv.AppendUnique(CPPPATH = make_list(paths))

	def KDEaddpaths_libs(lenv, paths):
		""" Add paths to libraries """
		lenv.AppendUnique(LIBPATH = make_list(paths))

	def KDElang(lenv, folder, appname):
		""" Process translations (.po files) in a po/ dir """
		transfiles = glob.glob(folder+'/*.po')
		for lang in transfiles:
			result = lenv.Transfiles(lang)
			country = SCons.Util.splitext(result[0].name)[0]
			KDEinstallas(lenv, 'KDELOCALE', country+'/LC_MESSAGES/'+appname+'.mo', result)

	def subdirs(lenv, folderlist):
		flist=make_list(folderlist)
		for i in flist:
			lenv.SConscript(i+"/SConscript")

	def KDEicon(lenv, icname='*', path='./'):
		""" Emulates the behaviour of Makefile.am to install icons
		Contributed by: "Andrey Golovizin" <grooz@gorodok@net> """
		type_dic = { 'action' : 'actions', 'app' : 'apps', 'device' : 
			'devices', 'filesys' : 'filesystems', 'mime' : 'mimetypes' } 
		dir_dic = {
		'los'  :'locolor/16x16',
		'lom'  :'locolor/32x32',
		'him'  :'hicolor/32x32',
		'hil'  :'hicolor/48x48',
		'lo16' :'locolor/16x16',
		'lo22' :'locolor/22x22',
		'lo32' :'locolor/32x32',
		'hi16' :'hicolor/16x16',
		'hi22' :'hicolor/22x22',
		'hi32' :'hicolor/32x32',
		'hi48' :'hicolor/48x48',
		'hi64' :'hicolor/64x64',
		'hi128':'hicolor/128x128',
		'hisc' :'hicolor/scalable',
		'cr16' :'crystalsvg/16x16',
		'cr22' :'crystalsvg/22x22',
		'cr32' :'crystalsvg/32x32',
		'cr48' :'crystalsvg/48x48',
		'cr64' :'crystalsvg/64x64',
		'cr128':'crystalsvg/128x128',
		'crsc' :'crystalsvg/scalable'
		}

		iconfiles = []
		for ext in "png xpm mng svg svgz".split():
			files = glob.glob(path+'/'+'*-*-%s.%s' % (icname, ext))
			iconfiles += files
		for iconfile in iconfiles:
			lst = iconfile.split('/')
			filename = lst[ len(lst) - 1 ]
			tmp = filename.split('-')
			if len(tmp)!=3:
				print RED+'WARNING: icon filename has unknown format: '+iconfile+NORMAL
				continue
			[icon_dir, icon_type, icon_filename]=tmp
			try:
				destfile = '%s/%s/%s/%s' % (lenv['KDEICONS'], dir_dic[icon_dir], type_dic[icon_type], icon_filename)
			except KeyError:
				print RED+'WARNING: unknown icon type: '+iconfile+NORMAL
				continue
			## Do not use KDEinstallas here, as parsing from an ide will be necessary
			if env['_INSTALL']: 
				env.Alias('install', env.InstallAs( env['DESTDIR']+'/'+destfile, iconfile ) )

	def KDEuse(lenv, flags):
		_flags=make_list(flags)
		if 'environ' in _flags:
			## The scons developers advise against using this but it is mostly innocuous :)
			import os
			lenv.AppendUnique( ENV = os.environ )
		if not 'lang_qt' in _flags:
			## Use this define if you are using the kde translation scheme (.po files)
			lenv.Append( CPPFLAGS = '-DQT_NO_TRANSLATION' )
		if 'rpath' in _flags:
			## Use this to set rpath - this may cause trouble if folders are moved (chrpath)
			lenv.Append( RPATH = [env['QTLIBPATH'], env['KDELIBPATH'], env['KDEMODULE']] )
		if 'thread' in _flags:
			## Uncomment the following if you need threading support
			lenv.KDEaddflags_cxx( ['-DQT_THREAD_SUPPORT', '-D_REENTRANT'] )
		if not 'nohelp' in _flags:
			if lenv['_CONFIGURE'] or lenv['HELP']:
				env.Exit(0)

		## To use kDebug(intvalue)<<"some trace"<<endl; you need to define -DDEBUG
		## it is done in admin/generic.py automatically when you do scons configure debug=1

	# Attach the functions to the environment so that sconscripts can use them
	from SCons.Script.SConscript import SConsEnvironment
	SConsEnvironment.KDEprogram = KDEprogram
	SConsEnvironment.KDEshlib = KDEshlib
	SConsEnvironment.KDEstaticlib = KDEstaticlib
	SConsEnvironment.KDEinstall = KDEinstall
	SConsEnvironment.KDEinstallas = KDEinstallas
	SConsEnvironment.KDElang = KDElang
	SConsEnvironment.KDEicon = KDEicon

	SConsEnvironment.KDEaddflags_cxx = KDEaddflags_cxx
	SConsEnvironment.KDEaddflags_c = KDEaddflags_c
	SConsEnvironment.KDEaddflags_link = KDEaddflags_link
	SConsEnvironment.KDEaddlibs = KDEaddlibs
	SConsEnvironment.KDEaddpaths_includes = KDEaddpaths_includes
	SConsEnvironment.KDEaddpaths_libs = KDEaddpaths_libs

	SConsEnvironment.subdirs = subdirs
	SConsEnvironment.KDEuse = KDEuse

