:: Flasher script to use st-link
@ECHO OFF
SETLOCAL ENABLEEXTENSIONS
SET me=%~n0
SET parent=%~dp0

:: Call parameters:
:: arg1: path to STM32_Programmer_CLI app
:: arg2: action, <FLASH> / <ERASE>
:: arg3: path to image to load, only for FLASH action
:: arg4: destination address in hex, only for FLASH action

:: FLASH Example:
:: C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe
:: FLASH
:: C:\Firmware\git\STM32G070x_BASE/build/STM32G070x_BASE.bin
:: 0x08000000 

:: ERASE Example:
:: C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe
:: ERASE

:: Show input args
SET stlink_path=%~1
SET action=%~2
SET app_bin=%~3
SET app_addr=%~4
ECHO %me%: ST-LINK PATH: %stlink_path%
ECHO %me%: ST-LINK ACTION: %action%
:: Proocess action
IF /I "%action%"=="FLASH" (
    CALL :action_flash
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
ECHO %me%: APP: %app_bin% %app_addr%
"%stlink_path%" -c port=SWD freq=4000 -d %app_bin% %app_addr% -v -rst
EXIT /B %ERRORLEVEL%


:: Erase action
:action_erase
"%stlink_path%" -c port=SWD freq=4000 -e all
EXIT /B %ERRORLEVEL%

:: EOF