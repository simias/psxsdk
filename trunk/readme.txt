What is PSXSDK?

PSXSDK is an unofficial and homebrewn software development kit for the Sony PlayStation(TM).
Its programming tools (compiler, assembler, linker, etc.) are based on GNU Binutils and GNU
Compiler Collection (GCC) while the PlayStation-specific tools, which you need to convert data
to PSX format, were either written from scratch or adapted from already existing open source programs.

It is expected that you are already familiar with programming. 
If you want to learn how to program, I recommend starting programming for the operating system 
your computer uses and not here.

What do I need to run my programs on my PlayStation console?

There are several methods to do that, either a modchip, a boot disk, a cheating device
like Caetla, or the "swap trick".

The "modchip" method involves soldering a chip on the motherboard of your PlayStation to make it
think it's reading an original licensed disc. The kind of modchip depends on the model of your console,
and they are quite hard to find nowadays (as of 2010). You might already have one, as it was very
common to install it in many parts of the world.

I might be actually telling you to ignore your laws (in case you have EUCD and DMCA), and I recommend
to do it. The constant eroding of our rights as citizens is getting very annoying, and these
laws need to be fought. Let's not be hypocrites.
Big money influences law (TM).

The "boot disk" method uses a specially-made CD-ROM which looks like it is an original licensed disc
to the PlayStation. Once booted they show you a menu where you can boot an unlicensed disc.
You might be able to still find one for sale.

The "cheating device" method was very used back in the golden days of the PlayStation scene.
I have never used this method so I cannot describe it here. Search for "caetla" on your favourite
search engine and something interesting about that should come up.

The "swap trick" consists in first putting an original licensed disc in the PlayStation, so that
the PlayStation can authenticate, then when the drive is still spinning you replace it with an
unlicensed disc, and the PlayStation boots the disc as it already authenticated earlier.
The exact method of the "swap trick" varies between different models of the console.
It ruins your CD-ROM drive in the long run and spare PSX CD-ROM drives aren't very easy to find
(maybe harder to find than entire consoles), so I recommend to use something else if you can.
Search for this on your favourite search engine, something will surely come up.

On SCPH1001 PlayStations (Early American models) there seems to be an easier way to do the swap trick, also called audio menu exploit.
Make the SCPH1001 PlayStation go into the BIOS menu by not inserting any CD,
then go to the cd player, let it read an original PS1 disc until it stops spinning,
swap the disc with a burned CD-R disc, and exit the cd player.

This trick might also work on early Japanese consoles but I'm not sure of that.

How do I get started?

In most cases, you will simply need one of the precompiled toolchains tarballs which you can find
on the PSXSDK website (http://tails92.sepwich.com/psxsdk). Extract them so that the PSXSDK can
be found at /usr/local/psxcross and you're ready.

If you want to do it the hard way, there is no precompiled tarball for your operating system or
you want to help in developing the PSXSDK, read below on how to build it from sources.

How do I compile a program?

This is easy. It is done almost as you do it usually, but with an additional step:

psx-gcc -o myprogram myprogram.c
elf2exe myprogram myprogram.exe

Why the additional step? It is needed because the GNU compiler outputs an ELF executable file,
which can't be run on the PlayStation. elf2exe converts that ELF file to a PS-X EXE, which
is the PlayStation's executable format, and which can be booted.

How do I make a CDROM image?

I will first describe how to make your executable boot.
There are two ways to make your executable boot: you either name it "PSX.EXE" and you
put it in the CDROM filesystem's root directory, or you set a file named "SYSTEM.CNF" up
which describes a bit of information about your executable.

A SYSTEM.CNF file looks like this:

BOOT = cdrom:PROGRAM.EXE;1
TCB = 4
EVENT = 16
STACK = 801FFFF0

The BOOT field contains the filename of your program (in this case PROGRAM.EXE in the
root directory of the CDROM filesystem). I would advise you to not touch the other fields.
Modifying STACK is even useless (as the starting address of the stack is reset by the PSXSDK
library to 0x801FFFF0)

Now, you can make the ISO9660 filesystem for the CDROM with any program which does that.
This, for example, is the command you would use to do it with the cdrtools:

mkisofs -o test.iso -V TEST -sysid PLAYSTATION cd_root

Then you will have to use mkpsxiso to convert the ISO image to a format the PlayStation understands
and to license it. mkpsxiso outputs a .BIN/.CUE pair, which is supported by most CD burning
applications.

mkpsxiso test.iso test.bin /usr/local/psxcross/misc/infousa.dat

This will create test.bin and test.cue, and the license applied to the image will be American.

Now you can burn the .BIN/.CUE pair with your favourite burning application.

Where can I find the function reference for the PSXSDK?

You can find it either in /usr/local/psxcross/misc/docs or in the docs subdirectory in the source
code archive.

Are there any examples which show how to use the PSXSDK library?

Yes. Download the examples archive from the PSXSDK website and compile the examples in it.
In this way you can also test that your PSXSDK compiler is working correctly.

What does PSXSDK support?

It can use the GPU (or GS, Graphic Synthesizer), the SPU, handle the VBlank Interrupt, handle
the joysticks and read files from the CDROM drive. Which is more than enough to do a 2D game.

What is missing from PSXSDK?

Geometry Transformation Engine (GTE) handling, root counter support and a lot more...
Even if the PlayStation was very common back in its golden days, there is scarce documentation about it.

Why does the C library appear incomplete?

Many standard C library functions are wrapped around their implementation in the BIOS which keeps
code size low and eases implementation, but this also means that they have less features than
most C libraries of today. That isn't a big problem, because for games you don't need a lot of features.
Anyway, you can modify the PSXSDK to remove the wrapper and use your functions.
The PSXSDK already does that for BIOS functions which were found not working or too buggy (like memcpy).

Help! My PlayStation crashed / something odd appeared on screen / other hardware bug

PSXSDK was primarily tested on a PAL SCPH-102 PSOne, on SCPH-5552, SCPH-7002 and SCPH-9002 PlayStations and on the PCSX emulator.
There are probably several hardware quirks and oddities I've not accounted for but which happen
on other models of the console. Please send an email if something like that happens.
Be sure to read realhw.txt in the documentation directory first, though, as it might be 
an easily fixable error on your part.

What program can I use to convert my images to PlayStation format?

You can use bmp2tim which is included in the PSXSDK or any program which converts an image to
PlayStation TIM format. It is a fairly standard format in the PlayStation field.

What program do I use in case I want to convert sound?

You can use the included wav2vag or any other program which converts to that format.
As it is the case for TIM, VAG is a quite standard format in the PlayStation environment.

VABs are just find compilations of more VAG files.

Can I sell a game I make with my PSXSDK?

Yes, as you are not infringing any copyright.

Written by Giuseppe Gatta
E-mail: tails92@gmail.com
