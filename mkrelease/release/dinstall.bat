@echo off
echo Driver installatation folder: %~dp0
cd %~dp0

:CheckOS
IF EXIST "%PROGRAMFILES(X86)%" (GOTO 64BIT) ELSE (GOTO 32BIT)

:64BIT
echo Installing 64-bit driver version...
echo Removing previous driver versions...
devcon64.exe remove sbVirtualMIDIDevice
devcon64.exe install "sbvmidi.inf" sbVirtualMIDIDevice
GOTO END

:32BIT
echo Installing 32-bit driver version...
echo Removing previous driver versions...
devcon32.exe remove sbVirtualMIDIDevice
devcon32.exe install "sbvmidi.inf" sbVirtualMIDIDevice
GOTO END

:END
echo Driver installation complete
pause
