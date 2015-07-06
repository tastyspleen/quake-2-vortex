from glob import glob
source = glob("*.c")

# vars described here: http://www.scons.org/doc/HTML/scons-user.html

env = Environment()
env['CFLAGS'] = '-m32'
env['CCFLAGS'] = '-m32'
env['LINKFLAGS'] = '-m32'
env['SHLIBPREFIX'] = ''

# print env.Dump()

env.SharedLibrary('gamei386', source)

