:: Tool to extract info for app crash
@ECHO OFF
SETLOCAL

:: CrashDebug: https://github.com/adamgreen/CrashDebug

:: Exe variables
SET GDB_TOOLPATH="C:\Firmware\arm_toochain\bin"
SET CRASH_DBG_APP="./CrashDebug.exe"
SET APP_FILE="../../build/MIDI4CV.elf"
SET DUMP_FILE="./CrashDump.txt"

:: Execute tool
%GDB_TOOLPATH%\arm-none-eabi-gdb %APP_FILE% -ex "set target-charset ASCII" -ex "target remote | %CRASH_DBG_APP% --elf %APP_FILE% --dump %DUMP_FILE%"

PAUSE