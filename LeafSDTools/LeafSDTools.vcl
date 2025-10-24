<html>
<body>
<pre>
<h1>Build Log</h1>
<h3>
--------------------Configuration: LeafSDTools - Win32 (WCE SH4) Release--------------------
</h3>
<h3>Command Lines</h3>
Creating command line "rc.exe /l 0x409 /fo"SH4Rel/LeafSDTools.res" /d UNDER_CE=500 /d _WIN32_WCE=500 /d "NDEBUG" /d "UNICODE" /d "_UNICODE" /d "WCE_PLATFORM_STANDARDSDK_500" /d "SHx" /d "SH4" /d "_SH4_" /r "E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\LeafSDTools.rc"" 
Creating temporary file "E:\DOCUME~1\LYLYKA~1\LOCALS~1\Temp\RSP1F.tmp" with contents
[
/nologo /W3 /D _WIN32_WCE=500 /D "WCE_PLATFORM_STANDARDSDK_500" /D "SHx" /D "SH4" /D "_SH4_" /D UNDER_CE=500 /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /FR"SH4Rel/" /Fp"SH4Rel/LeafSDTools.pch" /Yu"stdafx.h" /Fo"SH4Rel/" /Qsh4 /O2 /MC /c 
"E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\ExitUpdateMode.cpp"
"E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\ReadNAND.cpp"
"E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\SDPinControl.cpp"
"E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\UserSRAM.cpp"
"E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\WriteNAND.cpp"
"E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\Display.cpp"
"E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\Flash.cpp"
"E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\LeafSDTools.cpp"
"E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\Logger.cpp"
"E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\Touch.cpp"
]
Creating command line "clsh.exe @E:\DOCUME~1\LYLYKA~1\LOCALS~1\Temp\RSP1F.tmp" 
Creating temporary file "E:\DOCUME~1\LYLYKA~1\LOCALS~1\Temp\RSP20.tmp" with contents
[
/nologo /W3 /D _WIN32_WCE=500 /D "WCE_PLATFORM_STANDARDSDK_500" /D "SHx" /D "SH4" /D "_SH4_" /D UNDER_CE=500 /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /FR"SH4Rel/" /Fp"SH4Rel/LeafSDTools.pch" /Yc"stdafx.h" /Fo"SH4Rel/" /Qsh4 /O2 /MC /c 
"E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\StdAfx.cpp"
]
Creating command line "clsh.exe @E:\DOCUME~1\LYLYKA~1\LOCALS~1\Temp\RSP20.tmp" 
Creating temporary file "E:\DOCUME~1\LYLYKA~1\LOCALS~1\Temp\RSP21.tmp" with contents
[
commctrl.lib coredll.lib ceddk.lib /nologo /base:"0x00010000" /stack:0x10000,0x1000 /entry:"WinMainCRTStartup" /incremental:no /pdb:"SH4Rel/LeafSDTools.pdb" /nodefaultlib:"libc.lib /nodefaultlib:libcd.lib /nodefaultlib:libcmt.lib /nodefaultlib:libcmtd.lib /nodefaultlib:msvcrt.lib /nodefaultlib:msvcrtd.lib" /out:"SH4Rel/LeafSDTools.exe" /subsystem:windowsce,5.00 /MACHINE:SH4 
".\SH4Rel\ExitUpdateMode.obj"
".\SH4Rel\ReadNAND.obj"
".\SH4Rel\SDPinControl.obj"
".\SH4Rel\UserSRAM.obj"
".\SH4Rel\WriteNAND.obj"
".\SH4Rel\Display.obj"
".\SH4Rel\Flash.obj"
".\SH4Rel\LeafSDTools.obj"
".\SH4Rel\Logger.obj"
".\SH4Rel\StdAfx.obj"
".\SH4Rel\Touch.obj"
".\SH4Rel\LeafSDTools.res"
]
Creating command line "link.exe @E:\DOCUME~1\LYLYKA~1\LOCALS~1\Temp\RSP21.tmp"
<h3>Output Window</h3>
Compiling resources...
Compiling...
StdAfx.cpp
Compiling...
ExitUpdateMode.cpp
ReadNAND.cpp
SDPinControl.cpp
UserSRAM.cpp
E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\UserSRAM.cpp(152) : warning C4018: '>=' : signed/unsigned mismatch
WriteNAND.cpp
Display.cpp
E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\Display.cpp(40) : warning C4244: 'argument' : conversion from 'const double' to 'unsigned long', possible loss of data
E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\Display.cpp(390) : warning C4244: '=' : conversion from 'unsigned long' to 'unsigned short', possible loss of data
E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\Display.cpp(401) : warning C4018: '<' : signed/unsigned mismatch
E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\Display.cpp(403) : warning C4018: '<' : signed/unsigned mismatch
E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\Display.cpp(423) : warning C4800: 'int' : forcing value to bool 'true' or 'false' (performance warning)
E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\Display.cpp(428) : warning C4018: '<' : signed/unsigned mismatch
E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\Display.cpp(428) : warning C4018: '<' : signed/unsigned mismatch
E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\Display.cpp(432) : warning C4244: '=' : conversion from 'unsigned long' to 'unsigned short', possible loss of data
E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\Display.cpp(561) : warning C4244: 'argument' : conversion from 'unsigned long' to 'unsigned short', possible loss of data
E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\Display.cpp(678) : warning C4244: '=' : conversion from 'unsigned long' to 'unsigned short', possible loss of data
Flash.cpp
E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\Flash.cpp(295) : warning C4129: 'S' : unrecognized character escape sequence
LeafSDTools.cpp
Logger.cpp
Touch.cpp
E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\Touch.cpp(120) : warning C4800: 'int' : forcing value to bool 'true' or 'false' (performance warning)
Generating Code...
Linking...
Creating temporary file "E:\DOCUME~1\LYLYKA~1\LOCALS~1\Temp\RSP24.tmp" with contents
[
/nologo /o"SH4Rel/LeafSDTools.bsc" 
".\SH4Rel\StdAfx.sbr"
".\SH4Rel\ExitUpdateMode.sbr"
".\SH4Rel\ReadNAND.sbr"
".\SH4Rel\SDPinControl.sbr"
".\SH4Rel\UserSRAM.sbr"
".\SH4Rel\WriteNAND.sbr"
".\SH4Rel\Display.sbr"
".\SH4Rel\Flash.sbr"
".\SH4Rel\LeafSDTools.sbr"
".\SH4Rel\Logger.sbr"
".\SH4Rel\Touch.sbr"]
Creating command line "bscmake.exe @E:\DOCUME~1\LYLYKA~1\LOCALS~1\Temp\RSP24.tmp"
Creating browse info file...
<h3>Output Window</h3>





<h3>Results</h3>
LeafSDTools.exe - 0 error(s), 13 warning(s)
<h3>
--------------------Configuration: LeafSDTools - Win32 (WCE ARMV4I) Release--------------------
</h3>
<h3>Command Lines</h3>
Creating command line "rc.exe /l 0x409 /fo"ARMV4IRel/LeafSDTools.res" /d UNDER_CE=500 /d _WIN32_WCE=500 /d "UNICODE" /d "_UNICODE" /d "NDEBUG" /d "WCE_PLATFORM_STANDARDSDK_500" /d "THUMB" /d "_THUMB_" /d "ARM" /d "_ARM_" /d "ARMV4I" /r "E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\LeafSDTools.rc"" 
Creating temporary file "E:\DOCUME~1\LYLYKA~1\LOCALS~1\Temp\RSP25.tmp" with contents
[
/nologo /W3 /D _WIN32_WCE=500 /D "ARM" /D "_ARM_" /D "WCE_PLATFORM_STANDARDSDK_500" /D "ARMV4I" /D UNDER_CE=500 /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /D "QY8XXX" /FR"ARMV4IRel/" /Fp"ARMV4IRel/LeafSDTools.pch" /YX /Fo"ARMV4IRel/" /QRarch4T /QRinterwork-return /O2 /MC /c 
"E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\ExitUpdateMode.cpp"
"E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\ReadNAND.cpp"
"E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\SDPinControl.cpp"
"E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\UserSRAM.cpp"
"E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\WriteNAND_QY8.cpp"
"E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\DisplayDirectDraw.cpp"
"E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\Flash.cpp"
"E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\LeafSDTools.cpp"
"E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\Logger.cpp"
"E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\StdAfx.cpp"
"E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\Touch.cpp"
]
Creating command line "clarm.exe @E:\DOCUME~1\LYLYKA~1\LOCALS~1\Temp\RSP25.tmp" 
Creating temporary file "E:\DOCUME~1\LYLYKA~1\LOCALS~1\Temp\RSP26.tmp" with contents
[
commctrl.lib coredll.lib ceddk.lib ddraw.lib /nologo /base:"0x00010000" /stack:0x10000,0x1000 /entry:"WinMainCRTStartup" /incremental:no /pdb:"ARMV4IRel/LeafSDTools.pdb" /nodefaultlib:"libc.lib /nodefaultlib:libcd.lib /nodefaultlib:libcmt.lib /nodefaultlib:libcmtd.lib /nodefaultlib:msvcrt.lib /nodefaultlib:msvcrtd.lib" /out:"ARMV4IRel/LeafSDTools.exe" /subsystem:windowsce,5.00 /MACHINE:THUMB 
".\ARMV4IRel\ExitUpdateMode.obj"
".\ARMV4IRel\ReadNAND.obj"
".\ARMV4IRel\SDPinControl.obj"
".\ARMV4IRel\UserSRAM.obj"
".\ARMV4IRel\WriteNAND_QY8.obj"
".\ARMV4IRel\DisplayDirectDraw.obj"
".\ARMV4IRel\Flash.obj"
".\ARMV4IRel\LeafSDTools.obj"
".\ARMV4IRel\Logger.obj"
".\ARMV4IRel\StdAfx.obj"
".\ARMV4IRel\Touch.obj"
".\ARMV4IRel\LeafSDTools.res"
]
Creating command line "link.exe @E:\DOCUME~1\LYLYKA~1\LOCALS~1\Temp\RSP26.tmp"
<h3>Output Window</h3>
Compiling resources...
Compiling...
ExitUpdateMode.cpp
ReadNAND.cpp
SDPinControl.cpp
UserSRAM.cpp
e:\program files\microsoft embedded c++ 4.0\common\evc\myprojects\leafsdtools\usersram.cpp(152) : warning C4018: '>=' : signed/unsigned mismatch
WriteNAND_QY8.cpp
DisplayDirectDraw.cpp
e:\program files\microsoft embedded c++ 4.0\common\evc\myprojects\leafsdtools\displaydirectdraw.cpp(281) : warning C4244: '=' : conversion from 'unsigned long' to 'unsigned short', possible loss of data
e:\program files\microsoft embedded c++ 4.0\common\evc\myprojects\leafsdtools\displaydirectdraw.cpp(391) : warning C4244: '=' : conversion from 'unsigned long' to 'unsigned short', possible loss of data
e:\program files\microsoft embedded c++ 4.0\common\evc\myprojects\leafsdtools\displaydirectdraw.cpp(411) : warning C4018: '<' : signed/unsigned mismatch
e:\program files\microsoft embedded c++ 4.0\common\evc\myprojects\leafsdtools\displaydirectdraw.cpp(413) : warning C4018: '<' : signed/unsigned mismatch
e:\program files\microsoft embedded c++ 4.0\common\evc\myprojects\leafsdtools\displaydirectdraw.cpp(432) : warning C4800: 'int' : forcing value to bool 'true' or 'false' (performance warning)
e:\program files\microsoft embedded c++ 4.0\common\evc\myprojects\leafsdtools\displaydirectdraw.cpp(437) : warning C4018: '<' : signed/unsigned mismatch
e:\program files\microsoft embedded c++ 4.0\common\evc\myprojects\leafsdtools\displaydirectdraw.cpp(437) : warning C4018: '<' : signed/unsigned mismatch
e:\program files\microsoft embedded c++ 4.0\common\evc\myprojects\leafsdtools\displaydirectdraw.cpp(441) : warning C4244: '=' : conversion from 'unsigned long' to 'unsigned short', possible loss of data
e:\program files\microsoft embedded c++ 4.0\common\evc\myprojects\leafsdtools\displaydirectdraw.cpp(457) : warning C4244: 'argument' : conversion from 'unsigned long' to 'unsigned short', possible loss of data
Flash.cpp
e:\program files\microsoft embedded c++ 4.0\common\evc\myprojects\leafsdtools\flash.cpp(295) : warning C4129: 'S' : unrecognized character escape sequence
LeafSDTools.cpp
Logger.cpp
StdAfx.cpp
Touch.cpp
E:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\MyProjects\LeafSDTools\Touch.cpp(120) : warning C4800: 'int' : forcing value to bool 'true' or 'false' (performance warning)
Linking...
Creating temporary file "E:\DOCUME~1\LYLYKA~1\LOCALS~1\Temp\RSP29.tmp" with contents
[
/nologo /o"ARMV4IRel/LeafSDTools.bsc" 
".\ARMV4IRel\ExitUpdateMode.sbr"
".\ARMV4IRel\ReadNAND.sbr"
".\ARMV4IRel\SDPinControl.sbr"
".\ARMV4IRel\UserSRAM.sbr"
".\ARMV4IRel\WriteNAND_QY8.sbr"
".\ARMV4IRel\DisplayDirectDraw.sbr"
".\ARMV4IRel\Flash.sbr"
".\ARMV4IRel\LeafSDTools.sbr"
".\ARMV4IRel\Logger.sbr"
".\ARMV4IRel\StdAfx.sbr"
".\ARMV4IRel\Touch.sbr"]
Creating command line "bscmake.exe @E:\DOCUME~1\LYLYKA~1\LOCALS~1\Temp\RSP29.tmp"
Creating browse info file...
<h3>Output Window</h3>





<h3>Results</h3>
LeafSDTools.exe - 0 error(s), 12 warning(s)
</pre>
</body>
</html>
