@echo off

cl /arch:AVX2 /O2 /Ot src/*.c /link /entry:_entry /SUBSYSTEM:NATIVE /OUT:main.exe ntdll.lib 
del *obj;