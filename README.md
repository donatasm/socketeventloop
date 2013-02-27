Requirements for building libuv:
- svn
- python

Build libuv: 

    cd libuv
    vcbuild release static x64 

To make libuv clr compatible:
- for Debug configuration: Properties -> Runtime Library = Multi-threaded Debug DLL (/MDd)
- for Release configuration: Properties -> Runtime Library = Multi-threaded DLL (/MD)