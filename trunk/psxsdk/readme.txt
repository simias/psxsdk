What is PSXSDK?

PSXSDK is an unofficial and homebrewed software development kit for the Sony PlayStation(TM).
Its programming tools (compiler, assembler, linker, etc.) are based on GNU Binutils and GNU
Compiler Collection (GCC) while the PlayStation-specific tools, which you need to convert data
to PSX format, were either written from scratch or adapted from already existing open source programs.

It is expected that you are already familiar with programming. 
If you want to learn how to program, I recommend starting programming for the operating system 
your computer uses and not here.

This readme is structured as a FAQ which also contains information which is not exclusive to the PSXSDK,
but that is generally informative.

Q: What do I need to run my programs on my PlayStation console?

A: There are several methods to do that, either a modchip, a boot disk, a cheating device
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

Q: How do I get started?

A: In most cases, you will simply need one of the precompiled toolchains tarballs which you can find
on the PSXSDK website (http://tails92.sepwich.com/psxsdk). Extract them so that the PSXSDK can
be found at /usr/local/psxsdk and you're ready.

If you want to do it the hard way, there is no precompiled tarball for your operating system or
you want to help in developing the PSXSDK, read below on how to build it from sources.

Q: How do I compile a program?

A: This is easy. It is done almost as you do it usually, but with an additional step:

psx-gcc -o myprogram myprogram.c
elf2exe myprogram myprogram.exe

Why the additional step? It is needed because the GNU compiler outputs an ELF executable file,
which can't be run on the PlayStation. elf2exe converts that ELF file to a PS-X EXE, which
is the PlayStation's executable format, and which can be booted.

Q: How do I make a CDROM image?

A: I will first describe how to make your executable boot.
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

mkpsxiso test.iso test.bin /usr/local/psxsdk/misc/infousa.dat

This will create test.bin and test.cue, and the license applied to the image will be American.

Now you can burn the .BIN/.CUE pair with your favourite burning application.

Q: Where can I find the function reference for the PSXSDK?

A: You can find it either in /usr/local/psxsdk/misc/docs or in the docs subdirectory in the source
code archive.

Q: Are there any examples which show how to use the PSXSDK library?

A: Yes. Download the examples archive from the PSXSDK website and compile the examples in it.
In this way you can also test that your PSXSDK compiler is working correctly.

Q: What does PSXSDK support?

A: It can use the GPU (or GS, Graphic Synthesizer), the SPU, handle the VBlank Interrupt, handle
the joysticks and read files from the CDROM drive. Which is more than enough to do a 2D game.

Q: What is missing from PSXSDK?

A: Geometry Transformation Engine (GTE) handling, root counter support and a lot more...
Even if the PlayStation was very common back in its golden days, there is scarce documentation about it.

Q: Why does the C library appear incomplete?

A: Many standard C library functions are wrapped around their implementation in the BIOS which keeps
code size low and eases implementation, but this also means that they have less features than
most C libraries of today. That isn't a big problem, because for games you don't need a lot of features.
Anyway, you can modify the PSXSDK to remove the wrapper and use your functions.
The PSXSDK already does that for BIOS functions which were found not working or too buggy (like memcpy).

Q: Help! My PlayStation crashed / something odd appeared on screen / other hardware bug

A: PSXSDK was primarily tested on a PAL SCPH-102 PSOne, on SCPH-5552, SCPH-7002 and SCPH-9002 PlayStations and on the PCSX emulator.
There are probably several hardware quirks and oddities I've not accounted for but which happen
on other models of the console. Please send an email if something like that happens.
Be sure to read realhw.txt in the documentation directory first, though, as it might be 
an easily fixable error on your part.

Q: What program can I use to convert my images to PlayStation format?

A: You can use bmp2tim which is included in the PSXSDK or any program which converts an image to
PlayStation TIM format. It is a fairly standard format in the PlayStation field.

Q: What program do I use in case I want to convert sound?

A: You can use the included wav2vag or any other program which converts to that format.
As it is the case for TIM, VAG is a quite standard format in the PlayStation environment.

VABs are just find compilations of more VAG files.

Q: Can I sell a game I make with my PSXSDK?

A: Yes, as you are not infringing any copyright.

Q: I want to use Caetla to upload and test programs on my PlayStation, but I cannot find a version of CatFlap for an operating system which is not DOS or Windows. Are there?

A: Very recently Fabio Battaglia (hkzlabs) released his CatFlap for Linux. You can use that on Linux, and use its source
code as a starting point for a port to other operating systems.

Q: When uploading and running programs with CatFlap, they hang after some PSXSDK initialization messages.
How can I make them not hang? I cannot run anything!

A: Insert a CD into the drive of your PlayStation. It doesn't have to be a PlayStation CD, any CD is good.
This happens due to the initialization routines of the CDROM subsystem. Hopefully it will be fixed someday.

Q: How can I change the writing and the logo at the "Licensed by Sony" PlayStation bootscreen?

A: That information is contained inside the license file that you use to license the CD-ROM image that you burn.
    Thus, to modify the information you must use the "lictool" program included in the PSXSDK tools.
    lictool modifies various information contained in the license file such as the writing, the logo, etc.
    
    For example, to output a new modified American license file which uses your logo contained in new.tmd:
    
    lictool /usr/local/psxsdk/share/licenses/infousa.dat my_new_license_file.dat -tmd=new.tmd
    
    Remember to use my_new_license_file.dat when you use mkpsxiso, otherwise you will use the standard
    American license file, and your new logo won't appear!
    
    Logos are in TMD format, the format for 3D models in Sony's original development kit.
    
Q: My music module sounds wrong or weird when I play it with libmodplay

A: Most music module formats have a lot of built-in effects, which are often poorly documented,
    very complex to implement and which are often thought for personal computer hardware which is less
    limited than the PlayStation SPU in some aspects. Luckily most modules don't really use them
    or sound quite right when those effects are not implemented.

Q: I heard that it is possible to make the PlayStation read CD-RW discs (CD-rewritable). Is this true?

A: Yes, it is true. But it is only really usable if you want to do hardware programming tests, as the amount
    that the CD drive of your PlayStation will be able to read will be around 128-150 kilobytes. You will
    not be able to read original PlayStation games or CD-Rs, and if you try to do it, your laser pickup will
    get stuck and you will have to unblock it by unscrewing it and sliding it with your hands...
    
    To read CD-RWs on later model PlayStation/cd drives you have to tune the potentiometer on the CD drive
    from its original position to 12-13 hours. It actually sort of looks like a clock. Remember the original
    position to not ruin anything. Use a precision screwdriver to tune the potentiometer.
    This will stress the laser in the CD drive more than it is in normal operation, and it will shorten its life.
    
    This is my (Giuseppe Gatta's) experience, and it might work much better on your PlayStation/PS cd drive
    or it might work much worse. PlayStation cd drives are very finicky, I myself have PlayStations which read
    like new and some which almost do not read at all. The CD drive is the lowest quality item inside
    a PlayStation.

Q: I don't use double buffering in my program/game. Nothing is shown on the emulator but it displays fine
    on the real hardware. Where is the issue?
    
A: You are using a buggy GPU plugin. If you're using version 1.0.16 of the P.E.Op.S SoftX Driver or most other
    P.E.Op.S GPU plugins, update to P.E.Op.S SoftX Driver 1.0.18 and the problem should go away.

Q: What is a .PSF music file?

A: A .PSF music file is in the Portable Sound Format which is used to store video game music data.
    Actually the .PSF just contains a tiny bit of information data in its header and then a compressed executable
    which can be executed by an emulator (or a video game console). A PSF player is just a stripped-down emulator,
    which supports only the SPU and the executable inside the PSF is an executable hacked to play only music.
    Originally it was meant only for the PlayStation but now there are music files in this platform-indipendent format
    from many consoles. You can play a .psf music just by using your console or emulator. Use the psfex tool shipped
    with the PSXSDK, extract the .exe from the PSF and then run the executable. 
    More info here: http://www.neillcorlett.com/psf/

Written by Giuseppe Gatta
E-mail: tails92@gmail.com
