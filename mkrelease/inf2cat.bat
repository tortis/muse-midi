xcopy /y "..\sbvmidi\libfre_wnet_amd64\amd64\sbvmidi.sys" "release\"
ren "release\sbvmidi.sys" sbvmidi64.sys
xcopy /y "..\sbvmidi\libfre_wxp_x86\i386\sbvmidi.sys" "release\"
xcopy /y "..\sbvmidi\sbvmidi.inf" "release\"
"C:\Program Files (x86)\Windows Kits\8.1\bin\x86\Inf2Cat.exe" /driver:./release /os:8_X64,8_X86,Server8_x64,Server2008R2_x64,7_x64,7_x86,server2008_x64,server2008_x86,vista_x64,vista_x86,server2003_x64,server2003_x86,xp_x64,xp_x86