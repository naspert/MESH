# Microsoft Developer Studio Project File - Name="rawview" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=rawview - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "rawview.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "rawview.mak" CFG="rawview - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "rawview - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "rawview - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "rawview - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\include" /I "include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "READ_TIME" /YX /FD /c
# ADD BASE RSC /l 0x100c /d "NDEBUG"
# ADD RSC /l 0x100c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "rawview - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "READ_TIME" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x100c /d "_DEBUG"
# ADD RSC /l 0x100c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "rawview - Win32 Release"
# Name "rawview - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\3dmodel_io.c

!IF  "$(CFG)" == "rawview - Win32 Release"

# SUBTRACT CPP /I "..\include"

!ELSEIF  "$(CFG)" == "rawview - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\block_list.c

!IF  "$(CFG)" == "rawview - Win32 Release"

# SUBTRACT CPP /I "..\include"

!ELSEIF  "$(CFG)" == "rawview - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\curvature.c

!IF  "$(CFG)" == "rawview - Win32 Release"

# SUBTRACT CPP /I "..\include"

!ELSEIF  "$(CFG)" == "rawview - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\geomutils.c

!IF  "$(CFG)" == "rawview - Win32 Release"

# SUBTRACT CPP /I "..\include"

!ELSEIF  "$(CFG)" == "rawview - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\gl2ps.c

!IF  "$(CFG)" == "rawview - Win32 Release"

# SUBTRACT CPP /I "..\include"

!ELSEIF  "$(CFG)" == "rawview - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\image.c

!IF  "$(CFG)" == "rawview - Win32 Release"

# SUBTRACT CPP /I "..\include"

!ELSEIF  "$(CFG)" == "rawview - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\kobbelt_sqrt3.c
# End Source File
# Begin Source File

SOURCE=.\src\model_in.c

!IF  "$(CFG)" == "rawview - Win32 Release"

# SUBTRACT CPP /I "..\include"

!ELSEIF  "$(CFG)" == "rawview - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\model_in_ply.c

!IF  "$(CFG)" == "rawview - Win32 Release"

# SUBTRACT CPP /I "..\include"

!ELSEIF  "$(CFG)" == "rawview - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\model_in_raw.c

!IF  "$(CFG)" == "rawview - Win32 Release"

# SUBTRACT CPP /I "..\include"

!ELSEIF  "$(CFG)" == "rawview - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\model_in_smf.c

!IF  "$(CFG)" == "rawview - Win32 Release"

# SUBTRACT CPP /I "..\include"

!ELSEIF  "$(CFG)" == "rawview - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\model_in_vrml_iv.c

!IF  "$(CFG)" == "rawview - Win32 Release"

# SUBTRACT CPP /I "..\include"

!ELSEIF  "$(CFG)" == "rawview - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\normals.c

!IF  "$(CFG)" == "rawview - Win32 Release"

# SUBTRACT CPP /I "..\include"

!ELSEIF  "$(CFG)" == "rawview - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\rawview.c

!IF  "$(CFG)" == "rawview - Win32 Release"

# SUBTRACT CPP /I "..\include"

!ELSEIF  "$(CFG)" == "rawview - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\rawview_disp.c

!IF  "$(CFG)" == "rawview - Win32 Release"

# SUBTRACT CPP /I "..\include"

!ELSEIF  "$(CFG)" == "rawview - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\rawview_grab.c

!IF  "$(CFG)" == "rawview - Win32 Release"

# SUBTRACT CPP /I "..\include"

!ELSEIF  "$(CFG)" == "rawview - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\rawview_utils.c

!IF  "$(CFG)" == "rawview - Win32 Release"

# SUBTRACT CPP /I "..\include"

!ELSEIF  "$(CFG)" == "rawview - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\ring.c

!IF  "$(CFG)" == "rawview - Win32 Release"

# SUBTRACT CPP /I "..\include"

!ELSEIF  "$(CFG)" == "rawview - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\subdiv.c

!IF  "$(CFG)" == "rawview - Win32 Release"

# SUBTRACT CPP /I "..\include"

!ELSEIF  "$(CFG)" == "rawview - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\subdiv_butterfly.c

!IF  "$(CFG)" == "rawview - Win32 Release"

# SUBTRACT CPP /I "..\include"

!ELSEIF  "$(CFG)" == "rawview - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\subdiv_loop.c

!IF  "$(CFG)" == "rawview - Win32 Release"

# SUBTRACT CPP /I "..\include"

!ELSEIF  "$(CFG)" == "rawview - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\subdiv_sph.c

!IF  "$(CFG)" == "rawview - Win32 Release"

# SUBTRACT CPP /I "..\include"

!ELSEIF  "$(CFG)" == "rawview - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\subdiv_sqrt3.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
