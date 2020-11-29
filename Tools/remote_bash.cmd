:: Initial setup
@echo off
SETLOCAL ENABLEEXTENSIONS
SET me=%~n0
SET parent=%~dp0

:: Transfer variables
SET remote_bash_file=%~1
SET remote_user=%~2
SET remote_pass=%~3
SET remote_host=%~4
@REM SET remote_bash_file=/home/pi/shared/devtools/remote_flasher/flasher_jlink.sh
@REM SET remote_user=pi
@REM SET remote_pass=pipassword
@REM SET remote_host=192.168.1.50

:: Copy binary into remote machine (plink required)
plink -batch -pw %remote_pass% %remote_user%@%remote_host% %remote_bash_file%