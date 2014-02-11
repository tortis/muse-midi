@echo off
cd %~dp0

echo Uninstalling driver...
:CheckOS
IF EXIST "%PROGRAMFILES(X86)%" (GOTO 64BIT) ELSE (GOTO 32BIT)

:64BIT
devcon64.exe remove sbVirtualMIDIDevice
GOTO END

:32BIT
devcon32.exe remove sbVirtualMIDIDevice
GOTO END

:END
echo Driver uninstalled
pause