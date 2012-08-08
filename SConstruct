from glob import glob

env = Environment()

env.Append(CCFLAGS='-g')

sources = glob("*.c") + glob ("ai/*.c")

env.SharedLibrary('gamex86', source=sources)
