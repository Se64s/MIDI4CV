:: Flasher script to use jlink
@ECHO OFF
SETLOCAL ENABLEEXTENSIONS
SET me=%~n0
SET parent=%~dp0

:: Call parameter example
:: "C:\Program Files (x86)\SEGGER\JLink\jlink.exe" 
:: LPC54608J512 
:: "C:\Firmware\01_projects\00_TOOLS\test\bootstopper.bin" 
:: 0x00000000

:: Call parameters:
:: arg1: path to jlink app
:: arg2: action, <FLASH> / <ERASE>
:: arg3: device name
:: arg4: path to image to load, only for FLASH action
:: arg5: destination address in hex, only for FLASH action

:: FLASH Example:
:: "C:\Program Files (x86)\SEGGER\JLink\jlink.exe" 
:: FLASH
:: LPC54608J512
:: "C:\Firmware\01_projects\00_TOOLS\test\bootstopper.bin" 
:: 0x00000000

:: ERASE Example:
:: "C:\Program Files (x86)\SEGGER\JLink\jlink.exe" 
:: ERASE
:: LPC54608J512

:: Show input args
SET jlink_path=%~1
SET action=%~2
SET device_target=%~3
SET app_bin=%~4
SET app_addr=%~5
ECHO %me%: JLINK PATH: %jlink_path%
ECHO %me%: JLINK ACTION: %action%
ECHO %me%: JLINK DEVICE: %device_target%
:: Proocess action
IF /I "%action%"=="FLASH" (
    CALL :action_flash %app_bin% %app_addr%
) ELSE (
    IF /I "%action%"=="ERASE" (
        CALL :action_erase
    ) ELSE (
        ECHO %me%: ACTION NOT VALID: %action%
    )
)
:: Finish script
EXIT /B %ERRORLEVEL%


:: Flash action
:action_flash
:: Create tmp file with cmd to execute
SET bin_data=%1
SET bin_addr=%2
ECHO %me%: APP: %bin_data% %bin_addr%
SET tmp_cmd_file=jlink_cmd.tmp
TYPE nul > %tmp_cmd_file%
ECHO r >> %tmp_cmd_file%
ECHO sleep 250 >> %tmp_cmd_file%
ECHO loadfile %bin_data% %bin_addr% >> %tmp_cmd_file%
ECHO r >> %tmp_cmd_file%
ECHO sleep 100 >> %tmp_cmd_file%
ECHO g >> %tmp_cmd_file%
ECHO exit >> %tmp_cmd_file%
CALL :process_cmd_file %tmp_cmd_file%
:: Execute cmd file
EXIT /B %ERRORLEVEL%


:: Erase action
:action_erase
:: Create tmp file with cmd to execute
SET tmp_cmd_file=jlink_cmd.tmp
TYPE nul > %tmp_cmd_file%
ECHO r >> %tmp_cmd_file%
ECHO sleep 250 >> %tmp_cmd_file%
ECHO erase >> %tmp_cmd_file%
ECHO exit >> %tmp_cmd_file%
CALL :process_cmd_file %tmp_cmd_file%
:: Execute cmd file
EXIT /B %ERRORLEVEL%


:: Execute cmd file
:process_cmd_file
SET cmd_file=%1
SET /A retry_count=0
:process_cmd_file_start
:: Execute flash command
"%jlink_path%" -Device %device_target% -If SWD -Speed 4000 -ExitOnError -CommandFile "%cmd_file%"
:: Process output to loop on error
IF /I "%ERRORLEVEL%" NEQ "0" (
    ECHO %me%: EXE RETRY %retry_count%
    IF /I "%retry_count%" LEQ "3" (
        SET /A retry_count=%retry_count% + 1
        ECHO - TRY TO FLASH AGAIN
        GOTO process_cmd_file_start
    ) ELSE (
        ECHO - MAXIMUN RETRY COUNT ACHIEVE
    )
)
:: Delete tmp file
DEL %cmd_file%
EXIT /B 0

:: EOF