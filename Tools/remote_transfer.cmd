:: Initial setup
@echo off
SETLOCAL ENABLEEXTENSIONS
SET me=%~n0
SET parent=%~dp0

:: Transfer variables
SET local_file_path=%~1
SET remote_file_path=%~2
SET remote_user=%~3
SET remote_pass=%~4
SET remote_host=%~5
@REM SET local_file_path=C:\Firmware\git\MIDI4CV\test\test_file.txt
@REM SET remote_file_path=/home/pi/shared/test_file.txt
@REM SET remote_user=pi
@REM SET remote_pass=pipassword
@REM SET remote_host=192.168.1.50

:: Copy binary into remote machine (pscp required)
pscp -pw %remote_pass% %local_file_path% %remote_user%@%remote_host%:%remote_file_path%