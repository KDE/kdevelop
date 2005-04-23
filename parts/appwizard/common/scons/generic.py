#! /usr/bin/env python
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
YELLOW ="\033[1m"  #"\033[93m" # replaced by bold because yellow is unreadable on white
CYAN   ="\033[96m"
NORMAL ="\033[0m"

import os

def exists(env):
	return true

def generate(env):
	env.Help("""
"""+BOLD+
"""*** Generic options ***
-----------------------"""+NORMAL+"""
"""+BOLD+"""* debug  """+NORMAL+""": debug=1 (-g) or debug=full (-g3, slower) else use environment CXXFLAGS, or -O2 by default
"""+BOLD+"""* prefix """+NORMAL+""": the installation path
"""+BOLD+"""* extraincludes """+NORMAL+""": a list of paths separated by ':'
ie: """+BOLD+"""scons configure debug=full prefix=/usr/local extraincludes=/tmp/include:/usr/local
"""+NORMAL)

	# load the options
	from SCons.Options import Options, PathOption
	opts = Options('generic.cache.py')
	opts.AddOptions(
		( 'KDECXXFLAGS', 'debug level for the project : full or just anything' ),
		( 'PREFIX', 'prefix for installation' ),
		( 'EXTRAINCLUDES', 'extra include paths for the project' ),
	)
	opts.Update(env)
	
	# use this to avoid an error message 'how to make target configure ?'
	env.Alias('configure', None)

	# configure the environment if needed
	if 'configure' in env['TARGS'] or not env.has_key('KDECXXFLAGS'):
		# need debugging ?
		if env.has_key('KDECXXFLAGS'):
			env.__delitem__('KDECXXFLAGS')
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
				import SCons.Util
				env['KDECXXFLAGS'] = SCons.Util.CLVar( os.environ['CXXFLAGS'] )
				env.Append( KDECXXFLAGS = ['-DNDEBUG', '-DNO_DEBUG'] )
			else:
				env.Append(KDECXXFLAGS = ['-O2', '-DNDEBUG', '-DNO_DEBUG'])

		# user-specified prefix
		if env['ARGS'].get('prefix', None):
			env['PREFIX'] = env['ARGS'].get('prefix', None)
			print CYAN+'** set the installation prefix for the project : ' + env['PREFIX'] +' **'+ NORMAL
		elif env.has_key('PREFIX'):
			env.__delitem__('PREFIX')

		# user-specified include paths
		env['EXTRAINCLUDES'] = env['ARGS'].get('extraincludes', None)
		if env['ARGS'].get('extraincludes', None):
			print CYAN+'** set extra include paths for the project : ' + env['EXTRAINCLUDES'] +' **'+ NORMAL
		elif env.has_key('EXTRAINCLUDES'):
			env.__delitem__('EXTRAINCLUDES')

		# and finally save the options in a cache
		opts.Save('generic.cache.py', env)

	if env.has_key('KDECXXFLAGS'):
		# load the flags
                env.AppendUnique( CPPFLAGS = env['KDECXXFLAGS'] )

	if env.has_key('EXTRAINCLUDES'):
		incpaths = []
		for dir in str(env['EXTRAINCLUDES']).split(':'):
			incpaths.append( dir )
		env.Append(CPPPATH = incpaths)

