---------------------- Notes on the sample sources ------------------------

1) All of the example programs have been pre-built, and are in the .\BIN
directory.  The sources for individual chapters are in the appropriately
named subdirectory beneath this directory (e.g., .\CHAP5 contains the sources
for chapter 5.)

2) These programs were compiled with Visual C++ 2.0, and where appropriate,
Borland C++ 4.5.

3) Many of these examples assume the existance of the "combined" Win32SDK
that encompasses both NT and Win95.  At the time these examples were created,
Visual C++ had not been updated to include the Win95 specific header files
and libraries.  If you don't have the Win32 SDK, but have a Win95 aware
version of Visual C++, you can probably make minor changes to the .MAK files
in order to make the sources compile.

4) Whereever possible, I tried to keep all the program code in straight
'C' code, and use as little assemble as possible.  However, some of the
sample programs absolutely require .ASM files.  For these files, you'll
need MASM 6.11 (or TASM32 if you're building with the Borland C++ tools).
There is no way around this.

5) Some of the programs are so simple that a full blown .MAK file
isn't necessary to build them.  Instead, they can be compiled from
the command line.  Programs of this type can be built using the .BAT
files in the same directory as the program sources.

6) Where a .MAK file is provided, simply type "NMAKE <project.mak>", where
<project.mak> is the name of the .MAK file.

7) I'm not a big fan of Integrated Development Environments (IDEs),
and I did not create any IDE project files for these programs.  For
the most part, the programs are very straightforward (i.e., one or
more .C files, some .H files, and maybe a .RC or .DEF file).  It
shouldn't be hard to create a project on your own if you want to work
with these sources in your IDE.

8) Most of the programs have been also built and tested with Borland C++.
Where I had current Borland C++ .MAK files, they are included in the
source directory, with the extension ".BOR".

