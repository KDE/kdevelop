## Thomas Nagy, 2005

"""
Detect and store the most common options
* kdecxxflags  : debug=1 (-g) or debug=full (-g3, slower)
  else use the user CXXFLAGS if any, - or -O2 by default
* prefix : the installation path
* extraincludes : a list of paths separated by ':'
ie: scons configure debug=full prefix=/usr/local extraincludes=/tmp/include:/usr/local
"""

BOLD   ="\033[1m"
RED    ="\033[91m"
GREEN  ="\033[92m"
YELLOW ="\033[1m" #"\033[93m" # unreadable on white backgrounds
CYAN   ="\033[96m"
NORMAL ="\033[0m"

import os

def exists(env):
	return true

def generate(env):
	## Bksys requires scons 0.96
	env.EnsureSConsVersion(0, 96)
	
	import sys
	env['HELP']=0
	if '--help' in sys.argv or '-h' in sys.argv or 'help' in sys.argv:
		env['HELP']=1
	
	if env['HELP']:
		print """
"""+BOLD+"""*** Generic options ***
-----------------------"""+NORMAL+"""
"""+BOLD+"""* debug  """+NORMAL+""": debug=1 (-g) or debug=full (-g3, slower) else use environment CXXFLAGS, or -O2 by default
"""+BOLD+"""* prefix """+NORMAL+""": the installation path
"""+BOLD+"""* extraincludes """+NORMAL+""": a list of paths separated by ':'
ie: """+BOLD+"""scons configure debug=full prefix=/usr/local extraincludes=/tmp/include:/usr/local
"""+NORMAL
	
	## Global cache directory
	## Put all project files in it so a rm -rf cache will clean up the config
	if not env.has_key('CACHEDIR'):
		env['CACHEDIR'] = os.getcwd()+'/cache/'
	if not os.path.isdir(env['CACHEDIR']):
		os.mkdir(env['CACHEDIR'])
	
	## SCons cache directory
	## this avoids recompiling the same files over and over again: very handy when working with cvs
	env.CacheDir(os.getcwd()+'/cache/objects')

	## Avoid spreading .sconsign files everywhere - keep this line
	env.SConsignFile(env['CACHEDIR']+'/scons_signatures')
	
	# Special trick for installing rpms ...
	env['DESTDIR']=''
	if 'install' in sys.argv and os.environ.has_key('DESTDIR'):
		env['DESTDIR']=os.environ['DESTDIR']+'/'
		print CYAN+'** Enabling DESTDIR for the project ** ' + NORMAL + env['DESTDIR']

	# load the options
	from SCons.Options import Options, PathOption
	cachefile=env['CACHEDIR']+'generic.cache.py'
	opts = Options(cachefile)
	opts.AddOptions(
		( 'KDECCFLAGS', 'C flags' ),
		( 'KDECXXFLAGS', 'debug level for the project : full or just anything' ),
		( 'KDELINKFLAGS', 'additional link flags' ),
		( 'PREFIX', 'prefix for installation' ),
		( 'EXTRAINCLUDES', 'extra include paths for the project' ),
		( 'ISCONFIGURED', 'is the project configured' ),
	)
	opts.Update(env)
	
	# use this to avoid an error message 'how to make target configure ?'
	env.Alias('configure', None)

	import SCons.Util

	if 'install' in sys.argv:
		env['_INSTALL']=1
	else:
		env['_INSTALL']=0
	if 'configure' in sys.argv:
		env['_CONFIGURE']=1
	else:
		env['_CONFIGURE']=0

	# configure the environment if needed
	if not env['HELP'] and (env['_CONFIGURE'] or not env.has_key('ISCONFIGURED')):

		import re
		def makeHashTable(args):
			table = { }
			for arg in args:
				if len(arg) > 1:
					lst=arg.split('=')
					if len(lst) < 2:
						continue
					key=lst[0]
					value=lst[1]
					if len(key) > 0 and len(value) >0:
						table[key] = value
 			return table

		env['ARGS']=makeHashTable(sys.argv)

		# be paranoid, unset existing variables
		if env.has_key('KDECXXFLAGS'):
			env.__delitem__('KDECXXFLAGS')
		if env.has_key('KDECCFLAGS'):
			env.__delitem__('KDECCFLAGS')
		if env.has_key('KDELINKFLAGS'):
			env.__delitem__('KDELINKFLAGS')
		if env.has_key('PREFIX'):
			env.__delitem__('PREFIX')
		if env.has_key('EXTRAINCLUDES'):
			env.__delitem__('EXTRAINCLUDES')
		if env.has_key('ISCONFIGURED'):
			env.__delitem__('ISCONFIGURED')

		if env['ARGS'].get('debug', None):
			debuglevel = env['ARGS'].get('debug', None)
			print CYAN+'** Enabling debug for the project **' + NORMAL
			if (debuglevel == "full"):
				env['KDECXXFLAGS'] = ['-DDEBUG', '-g3']
			else:
				env['KDECXXFLAGS'] = ['-DDEBUG', '-g']
		else:
			if os.environ.has_key('CXXFLAGS'):
				# user-defined flags (gentooers will be elighted)
				env['KDECXXFLAGS'] = SCons.Util.CLVar( os.environ['CXXFLAGS'] )
				env.Append( KDECXXFLAGS = ['-DNDEBUG', '-DNO_DEBUG'] )
			else:
				env.Append(KDECXXFLAGS = ['-O2', '-DNDEBUG', '-DNO_DEBUG'])

		if os.environ.has_key('CFLAGS'):
			env['KDECCFLAGS'] = SCons.Util.CLVar( os.environ['CFLAGS'] )

		## FreeBSD settings (contributed by will at freebsd dot org)
		if os.uname()[0] == "FreeBSD":
		        if os.environ.has_key('PTHREAD_LIBS'):
		                env.AppendUnique( KDELINKFLAGS = SCons.Util.CLVar( os.environ['PTHREAD_LIBS'] ) )
		        else:
		                syspf = os.popen('/sbin/sysctl kern.osreldate')
		                osreldate = int(syspf.read().split()[1])
		                syspf.close()
		                if osreldate < 500016:
		                        env.AppendUnique( KDELINKFLAGS = ['-pthread'])
		                        env.AppendUnique( KDECXXFLAGS = ['-D_THREAD_SAFE'])
		                elif osreldate < 502102:
		                        env.AppendUnique( KDELINKFLAGS = ['-lc_r'])
		                        env.AppendUnique( KDECXXFLAGS = ['-D_THREAD_SAFE'])
		                else:
		                        env.AppendUnique( KDELINKFLAGS = ['-pthread'])

		# User-specified prefix
		if env['ARGS'].get('prefix', None):
			env['PREFIX'] = env['ARGS'].get('prefix', None)
			print CYAN+'** set the installation prefix for the project : ' + env['PREFIX'] +' **'+ NORMAL
		elif env.has_key('PREFIX'):
			env.__delitem__('PREFIX')

		# User-specified include paths
		env['EXTRAINCLUDES'] = env['ARGS'].get('extraincludes', None)
		if env['ARGS'].get('extraincludes', None):
			print CYAN+'** set extra include paths for the project : ' + env['EXTRAINCLUDES'] +' **'+ NORMAL
		elif env.has_key('EXTRAINCLUDES'):
			env.__delitem__('EXTRAINCLUDES')

		env['ISCONFIGURED']=1

		# And finally save the options in the cache
		opts.Save(cachefile, env)

	if env.has_key('KDECXXFLAGS'):
                env.AppendUnique( CPPFLAGS = env['KDECXXFLAGS'] )

	if env.has_key('KDECCFLAGS'):
		env.AppendUnique( CCFLAGS = env['KDECCFLAGS'] )

	if env.has_key('KDELINKFLAGS'):
		env.AppendUnique( LINKFLAGS = env['KDELINKFLAGS'] )

	if env.has_key('EXTRAINCLUDES'):
		incpaths = []
		for dir in str(env['EXTRAINCLUDES']).split(':'):
			incpaths.append( dir )
		env.Append(CPPPATH = incpaths)

	env.Export("env")

