1. Use buildxpx86.bat to build the driver for Windows XP 32 bits. 
	This will generate a .sys file under sbvmidi/libfre_wxp_x86/i386
2. Use buildxpx64.bat to generate the 64 bits .sys. Rename it to sbvmidi64.sys
3. Copy the 2 .sys files to the release directory. 
4. Make sure the .inf under /sbvmidi/ is up to date. Remember to update driverver with date
	and version if necessary.
5. Run inf2cat.bat to generate the catalog file.
6. The driver should be ready for signing.