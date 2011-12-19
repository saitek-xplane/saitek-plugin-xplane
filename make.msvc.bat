@ECHO OFF

:: Visual Studio 2010
if defined VS100COMNTOOLS  (
	if exist "%VS100COMNTOOLS%\vsvars32.bat" (
		echo -
		echo - Visual C++ 2010 found.
		echo -
		call "%VS100COMNTOOLS%\vsvars32.bat"
		goto STARTCOMPILING
	)
)

:: Toolkit 2003
if defined VCToolkitInstallDir (
	if exist "%VCToolkitInstallDir%\vcvars32.bat" (
		echo -
		echo - VC 7.1 Toolkit found.
		echo -
		call "%VCToolkitInstallDir%\vcvars32.bat"
		goto STARTCOMPILING
	)
)

:: Visual C++ 8.0
if defined VS80COMNTOOLS (
	if exist "%VS80COMNTOOLS%\vsvars32.bat" (
		echo -
		echo - Visual C++ 8.0 found.
		echo -
		call "%VS80COMNTOOLS%\vsvars32.bat"
		goto STARTCOMPILING
	)
)

:: Visual C++ 7.1
if defined VS71COMNTOOLS  (
	if exist "%VS71COMNTOOLS%\vsvars32.bat" (
		echo -
		echo - Visual C++ 7.1 found.
		echo -
		call "%VS71COMNTOOLS%\vsvars32.bat"
		goto STARTCOMPILING
	)
)

echo -
echo - No Visual C++ found, please set the enviroment variable 
echo - 
echo - VCToolkitInstallDir  or  VS71COMNTOOLS or VS80COMNTOOLS 
echo - 
echo - to your Visual Studio folder which contains vsvars32.bat.
echo - 
echo - Or call the vsvars32.bat.
echo -

goto ERROR

:STARTCOMPILING

:: buid process
set CL_INCLUDES=/I"./include" /I"./SDK/CHeaders/XPLM" /I"./include/ptypes"
set CL_OPTS=/c /nologo /W3 /Ox /Ob1 /Oi /GF /MT /Gy /MT
set CL_DEFS=/D "NDEBUG" /D "WIN32" /D "_MBCS"  /D "XPLM200" /D "_USRDLL" /D "_WINDLL" /D "APL=0" /D "IBM=1" /D "LIN=0" /D "WIN32" /D "_WINDOWS" /D "LOGPRINTF"
set CL_FILES="SaitekProPanelsWin.cpp" /TP "./hidapi/windows/hid.c" "multipanel.cpp" "PanelThreads.cpp" "radiopanel.cpp" "SaitekProPanels.cpp" "switchpanel.cpp" /TP "utils.c"

set LINK_OPTS=/OUT:SaitekProPanels.xpl /INCREMENTAL:NO /nologo /DLL /MACHINE:X86
set LINK_LIBS="user32.lib"  "libcmt.lib" "ptypes.lib" "XPLM.lib" "SetupAPI.lib"
set LINK_OBJS="SaitekProPanels.obj" "SaitekProPanelsWin.obj" "hid.obj" "multipanel.obj" "PanelThreads.obj" "radiopanel.obj" "switchpanel.obj" "utils.obj"

@ECHO ON

cl.exe  %CL_INCLUDES% %CL_OPTS% %CL_DEFS% %CL_FILES%
link.exe  %LINK_OPTS% %LINK_LIBS% %LINK_OBJS%

@ECHO OFF

del *.pdb *.exp *.obj

goto LEAVE

:ERROR
echo -
echo -
echo - An error occured. Compiling aborted.
echo - 
pause



:LEAVE

