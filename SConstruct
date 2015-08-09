from glob import glob
source = glob("*.c")

# vars described here: http://www.scons.org/doc/HTML/scons-user.html

env = Environment()
env['CFLAGS'] = '-O2 -m32'
env['CCFLAGS'] = '-O2 -m32'
env['LINKFLAGS'] = '-m32'
env['SHLIBPREFIX'] = ''

# print env.Dump()

env.SharedLibrary('gamei386', source)

