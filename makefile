!include "../global.mak"

ALL : "$(OUTDIR)\MQ2Autoforage.dll"

CLEAN :
	-@erase "$(INTDIR)\MQ2Autoforage.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\MQ2Autoforage.dll"
	-@erase "$(OUTDIR)\MQ2Autoforage.exp"
	-@erase "$(OUTDIR)\MQ2Autoforage.lib"
	-@erase "$(OUTDIR)\MQ2Autoforage.pdb"


LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\Detours\lib60\detours.lib ..\Release\MQ2Main.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\MQ2Autoforage.pdb" /debug /machine:I386 /out:"$(OUTDIR)\MQ2Autoforage.dll" /implib:"$(OUTDIR)\MQ2Autoforage.lib" /OPT:NOICF /OPT:NOREF 
LINK32_OBJS= \
	"$(INTDIR)\MQ2Autoforage.obj" \
	"$(OUTDIR)\MQ2Main.lib"

"$(OUTDIR)\MQ2Autoforage.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("MQ2Autoforage.dep")
!INCLUDE "MQ2Autoforage.dep"
!ELSE 
!MESSAGE Warning: cannot find "MQ2Autoforage.dep"
!ENDIF 
!ENDIF 


SOURCE=.\MQ2Autoforage.cpp

"$(INTDIR)\MQ2Autoforage.obj" : $(SOURCE) "$(INTDIR)"

