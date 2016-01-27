
# About

libwlocate is a library that can be used to evaluate a geographical position out of the
WLAN networks that are available near to a user. It is (c) 2010-2014 by Oxygenic/VWP. You
can contact me at virtual_worlds(at)gmx.de but please only in case this README and the
included documentation does not answer your question ;-)
Contributions, feedback and suggestion are always welcome.

# Intended audience

This library does not come with an GUI and offers a programming interface only.
Therefore the intended audience are software developers that want to use this library
out of own applications to provide WLAN-based location services out of their software.

Although there is a small test application that demonstrates the usage principle of the
library I afraid it is more or less useless for people that do not program own
applications.

# Licensing

The sources are located in directory libwlocate and are licensed unter the terms of the
GPL. For more details please refer to document COPYING.

# Building and usage

There are several possibilites provided to build the library using different operating
systems and building environments:



## Building wiht Linux/Arch

1. Install dependencies (mingw-w64-gcc is only needed for Windows cross compiling):

	`yaourt -S wireless_tools mingw-w64-gcc binutils gcc make git`

2. Run git clone:

	`git clone https://github.com/openwifi-su/lwlocate.git`

	`cd lwlocate/`

3. Run make:

	`make help`

After building you will get the shared library libwlocate.so and a test and trace application lwtrace for Linux. The resulting position trace file when you run lwtrace can be loaded by LocDemo for later position evaluation.
For Windows cross compiling you will get libwlocate.dll and lwtrace.exe.

## Building wiht QNX and Windows

Makelib.QNX

- file for building the shared library libwlocate.so for QNX using standard "make"; here scanning of WLAN networks is currently not implemented, the compiled library will not work at the moment!

Makefile.QNX

- file for building a QNX test and trace application using libwlocate.so and standard make libwlocate.dsp/

- project file for building the shared DLL libwlocate.dll for Windows libwlocate.vcxproj using Microsoft Visual Studio

# libwlocate

The library provides two major function that perform the full location job:

int wloc_get_location() and int wloc_get_location_from()

These functions scan the currently available WLAN networks, send the resulting
information to the server of a open WLAN map project and return the evaulated
position. Here the first function always uses openwlanmap.org for position retrieval,
the second one offers an additional parameter where an alternative project can be
specified with.
Additional information about the quality of the result is given, it uses a
range of 0..100 where 100 is the maximum possible quality and accuracy of the result.
Here 100% means the location can't be evaluated more exact, it does not mean
necessarily that the returned position does not differ by more than a fixed deviation.
So this value is only some kind of probability that tells you if the position is really
near to the given geographic coordinates. The smaller this value is, the bigger the
deviation can be.
The value returned by the functions informs about the success of the operation, only
in case WLOC_OK is given, the operation was successful and the returned position data
are valid and can be used.
In all other cases the return value informs about the kind of error that occured
(please refer to libwlocate.h for details).

For a more detailled description of all available library functions, their parameters
and usage please refer to file libwlocate.h, there you can find an inline documentation
for all of them.
