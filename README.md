# quake-2-vortex

This is a fork of Q2 Vortex from the TastySpleen repository.
I've updated most of it and it compiles on Windows and Linux distros
at /W4 and -Wall -O3 respectively. I've fixed a lot of style issues and
lots of possible null pointer dereference issues.
Consider this the first pass of a general code cleanup. (2020/01/01)

<b>It runs. I do not guarantee correctness.</b>

This appears to be the Chilean version. Refer to the documents
for more information. There is no documentation on how to
install and run Vortex servers. 

The Vortex developers, I am told, kept the sources and the servers 
"centrally" managed. The problem at the time was exploits and cheats. 
The code had links to mySQL so that was a huge vuln right there 
but it looks like users didn't have direct control of the input.

If anyone has legacy Vortex server archives somewhere I would
appreciate it if you can share it so this collection can be
preserved and completed.

My intent is to finish cleaning up the warnings, lint and general
cruft in the code until it compiles cleanly, fixing bugs in the
game as I find them.

First cleanup complete 1/2/2020.
100 commits later, 2020.03.30, compiles without warnings or errors on Linux, Windows, OSX.
Several logical errors spotted along the way.

//QW//

Automatically exported from code.google.com/p/quake-2-vortex
