/* MQ2AutoForage

   Simple plugin to automate the tast of foraging.
   
   Syntax:
   /startforage         - commence autoforaging.
   /stopforage          - stop autoforaging.
   /keepitem {item}     - add/change the item in the .ini file to auto-keep.
   /destroyitem {item}  - add/change the item in the .ini file to auto-destroy.
   
   Changelog:
   7/26/2016 - v2.0 - Fixed for string safety patch and cleaned up code (watlol)
   10/26/2019 - 2.1 - ChatWithThisName && Sic
						No longer requires a hotkey in "Actions" to use forage.
						Added new functions MyUseAbility, AbilityReady, atob, InGame
						and VerifyINI. 
						Added Keeping the last state for /startforage and /stopforage
						Defaults to on. It's assumed if they loaded the plugin that they
						wanted to start foraging.
						Significantly slowed down the plugin, to only check forage to be
						ready once every 100 pulses. 
	11/13/19 - 2.2 - Sic - Lowered pulsedelay to better avoid conflicts with other plugins and macros
	11/26/19 - 2.3 - Eqmule - Removed Thread. Please don't create threads that call eq functions, they will race condition and crash eq.
	                        - Fixed crash on full inventory.
*/

#define   PLUGIN_NAME   "MQ2AutoForage"
#define   PLUGIN_DATE   20191126
#define   PLUGIN_VERS   2.3

#include "../MQ2Plugin.h"

PreSetup(PLUGIN_NAME);
PLUGIN_VERSION(PLUGIN_VERS);

void StartForageCommand(PSPAWNINFO pChar, PCHAR szLine);
void StopForageCommand(PSPAWNINFO pChar, PCHAR szLine);
void KeepItemCommand(PSPAWNINFO pChar, PCHAR szLine);
void DestroyItemCommand(PSPAWNINFO pChar, PCHAR szLine);
void MyUseAbility(PCHAR szLine);
bool AbilityReady(PCHAR szSkillName);
bool atob(char x[MAX_STRING]);
inline bool InGame();
void HandleItem();
void Load_INI(VOID);
bool Check_INI(VOID);
void VerifyINI(char Section[MAX_STRING], char Key[MAX_STRING], char Default[MAX_STRING], char ININame[MAX_STRING]);

bool IsForaging=false;
bool HasForaged=false;
bool ForageSuccess=false;
bool bWaitForCursor = false;
bool KeepDestroy=false;
bool KeepItem=false;
bool WasSitting=false;
bool AutoKeepEnabled=true;
bool AutoAddEnabled=true;
bool MQ2ForageEnabled=false;
bool IAmCamping = false;
int Pulse = 99;
const int PulseDelay = 25;

// Added by Jaq -- Ripped off from mq2MoveUtils
bool IsBardClass(void);

PLUGIN_API VOID InitializePlugin(VOID)
{
    AddCommand("/startforage",StartForageCommand);
    AddCommand("/stopforage",StopForageCommand);
    AddCommand("/keepitem",KeepItemCommand);
    AddCommand("/destroyitem",DestroyItemCommand);

    if (MQ2Globals::gGameState==GAMESTATE_INGAME)
    {
        if (GetCharInfo())
        {
            sprintf_s(INIFileName,"%s\\MQ2Forage_%s_%s.ini",gszINIPath,GetCharInfo()->Name,EQADDR_SERVERNAME);
            Load_INI();
            MQ2ForageEnabled=true;
        }
    }
    else
    {
        MQ2ForageEnabled=false;
    }
}

PLUGIN_API VOID ShutdownPlugin(VOID)
{
    RemoveCommand("/startforage");
    RemoveCommand("/stopforage");
    RemoveCommand("/keepitem");
    RemoveCommand("/destroyitem");
}

PLUGIN_API VOID OnZoned(VOID)
{
	//If I switch characters and IAmCamping is still true and I finish zoning, and the gamestate is ingame...
	if (IAmCamping && GetGameState() == GAMESTATE_INGAME)
		IAmCamping = false;
    Load_INI();
}

bool bHandleCalled = false;
PLUGIN_API VOID OnPulse(VOID)
{
	if (!InGame())
		return;
	if (++Pulse < PulseDelay)
		return;
	Pulse = 0;
	//WriteChatf("Pulsing");
    if (!MQ2ForageEnabled)
    {
        return;
    }
    PSPAWNINFO pChSpawn = GetCharInfo()->pSpawn;
    PCHARINFO pCharInfo = NULL;

    if ((IsForaging) && !(*EQADDR_ATTACK > 0) && !(PCSIDLWND)pSpellBookWnd->IsVisible() && !(PCSIDLWND)pGiveWnd->IsVisible() && !(PCSIDLWND)pBankWnd->IsVisible() && !(PCSIDLWND)pMerchantWnd->IsVisible() && !(PCSIDLWND)pTradeWnd->IsVisible() && !(PCSIDLWND)pLootWnd->IsVisible() && !IAmCamping)// && !GetCharInfo()->pSpawn->Mount) {
    {
        if (AbilityReady("Forage")) {
            if (pChSpawn->StandState == STANDSTATE_SIT) {
                DoCommand(pChSpawn, "/stand");
                WasSitting=true;
            } else if (((PSPAWNINFO)pLocalPlayer)->CastingData.SpellETA == 0 || (IsBardClass())) {
                HasForaged=true;
                MyUseAbility("Forage");
            }
        }
    }
    if (!bWaitForCursor && ForageSuccess && IsForaging)
    {
		//WriteChatf("Turning ON bWaitForCursor");
		bWaitForCursor = true;
        ForageSuccess = false;
    }
	if (bWaitForCursor)
	{
		if (PCHARINFO2 pCharInfo2 = GetCharInfo2())
		{
			if (pCharInfo2->pInventoryArray->Inventory.Cursor)
			{
				if (!bHandleCalled)
				{
					//WriteChatf("Turning ON bHandleCalled");
					bHandleCalled = true;
				}
				HandleItem();
				return;
			}
			if (bHandleCalled && !pCharInfo2->pInventoryArray->Inventory.Cursor)
			{
				//WriteChatf("Turning off bHandleCalled and bWaitForCursor");
				bHandleCalled = false;
				bWaitForCursor = false;
			}
		}
	}
}

PLUGIN_API DWORD OnIncomingChat(PCHAR Line, DWORD Color)
{
    if (HasForaged && strstr(Line, "You have scrounged up"))
    {
        ForageSuccess=true;
    }
    else if (HasForaged && strstr(Line, "You fail to locate"))
    {
        ForageSuccess=false;
        HasForaged=false;
        KeepItem=false;
        KeepDestroy=false;
    }
    else if (!IAmCamping && strstr(Line, "It will take you about 30 seconds to prepare your camp.")) {
		IAmCamping = true;
	}
	else if (IAmCamping && strstr(Line, "You abandon your preparations to camp.")) {
		IAmCamping = false;
	}
    return 0;
}


PLUGIN_API VOID SetGameState(DWORD GameState)
{
    if (GameState==GAMESTATE_INGAME)
    {
        if (GetCharInfo())
        {
            sprintf_s(INIFileName,"%s\\MQ2Forage_%s_%s.ini",gszINIPath,GetCharInfo()->Name,EQADDR_SERVERNAME);
            Load_INI();
            MQ2ForageEnabled=true;
        }
    }
    else
    {
        MQ2ForageEnabled=false;
    }
}

void StartForageCommand(PSPAWNINFO pChar, PCHAR szLine)
{
    if (MQ2ForageEnabled)
    {
		WriteChatf("%s::Forage [\agEnabled\aw].",PLUGIN_NAME);
        IsForaging=true;
		WritePrivateProfileString("General", "ForageOn", "On", INIFileName);
    }
}

void StopForageCommand(PSPAWNINFO pChar, PCHAR szLine)
{
    if (MQ2ForageEnabled)
    {
		WriteChatf("%s::Forage [\arDisabled\aw].",PLUGIN_NAME);
        IsForaging=false;
		WritePrivateProfileString("General", "ForageOn", "Off", INIFileName);
    }
}

void KeepItemCommand(PSPAWNINFO pChar, PCHAR szLine)
{
    char szZone[64];
    PCHARINFO pCharInfo = GetCharInfo();

    if (MQ2ForageEnabled)
    {
		WriteChatf("%s::Now auto-keeping item [\ag%s\aw].",PLUGIN_NAME, szLine);
        sprintf_s(szZone,"%s",GetFullZone(pCharInfo->zoneId));
        WritePrivateProfileString(szZone,szLine,"keep",INIFileName);
    }
}

void DestroyItemCommand(PSPAWNINFO pChar, PCHAR szLine)
{
    char szZone[64];
    PCHARINFO pCharInfo = GetCharInfo();

    if (MQ2ForageEnabled)
    {
		WriteChatf("%s::Now auto-destroying item [\ag%s\aw].",PLUGIN_NAME, szLine);
        sprintf_s(szZone,"%s",GetFullZone(pCharInfo->zoneId));
        WritePrivateProfileString(szZone,szLine,"destroy",INIFileName);
    }
}

void MyUseAbility(PCHAR szLine) {
	if (PSPAWNINFO pChSpawn = GetCharInfo()->pSpawn) {
		char temp[MAX_STRING] = "";
		sprintf_s(temp, "\"%s", szLine);
		strcat_s(temp, MAX_STRING, "\"");
		DoAbility(pChSpawn, temp);
	}
}

bool AbilityReady(PCHAR szSkillName)
{
	if (!InGame())
		return false;
	PSPAWNINFO me = GetCharInfo()->pSpawn;
	for (int i = 0; i < NUM_SKILLS; i++) {
		DWORD nToken = pCSkillMgr->GetNameToken(i);
		if (HasSkill(i)) {
			if (char* thename = pStringTable->getString(nToken, 0)) {
				if (!_stricmp(szSkillName, thename))
				{
					if (bool bActivated = pCSkillMgr->IsActivatedSkill(i))
					{
						return pCSkillMgr->IsAvailable(i);
					}
					break;
				}
			}
		}
	}
	return false;
}

inline bool InGame()
{
	return(GetGameState() == GAMESTATE_INGAME && GetCharInfo() && GetCharInfo()->pSpawn && GetCharInfo2());
}

void VerifyINI(char Section[MAX_STRING], char Key[MAX_STRING], char Default[MAX_STRING], char ININame[MAX_STRING])
{
	char temp[MAX_STRING] = { 0 };
	if (GetPrivateProfileString(Section, Key, 0, temp, MAX_STRING, ININame) == 0)
	{
		WritePrivateProfileString(Section, Key, Default, ININame);
	}
}

bool atob(char x[MAX_STRING])
{
	for (int i = 0; i < 4; i++)
		x[i] = tolower(x[i]);
	if (!_stricmp(x, "true") || atoi(x) != 0 || !_stricmp(x, "on"))
		return true;
	return false;
}

void HandleItem()
{
	if (PSPAWNINFO pChSpawn = (PSPAWNINFO)pLocalPlayer)
	{
		if (PCHARINFO2 pChar2 = GetCharInfo2())
		{
			PCONTENTS pCont = pChar2->pInventoryArray->Inventory.Cursor;
			if (PITEMINFO pCursor = GetItemFromContents(pCont))
			{
				CHAR szItem[64] = { 0 };
				sprintf_s(szItem, "%s", pCursor->Name);
				KeepDestroy = true;
				KeepItem = Check_INI();
				if (KeepItem)
				{
					int freeslots = GetFreeInventory(pCursor->Size);
					if (freeslots <=1)
					{
						bool bTurnItOff = true;
						//check if we can stack it
						if (((EQ_Item*)pCont)->IsStackable())
						{
							int freestack = GetFreeStack(pCont);
							if (freestack >= pCont->StackCount)
							{
								bTurnItOff = false;
							}
						}
						if(bTurnItOff)
						{
							//Beep(1000, 100);
							WriteChatf("%s::Turning \arOFF\aw Foraging, your inventory is FULL.", PLUGIN_NAME);
							StopForageCommand((PSPAWNINFO)pLocalPlayer, "");
							if (freeslots == 1)
							{
								HideDoCommand(pChSpawn, "/autoinventory", 0);
							}
							bWaitForCursor = false;
							bHandleCalled = false;
							return;
						}
					}
					WriteChatf("%s::Keeping [\ag%s\aw].", PLUGIN_NAME, szItem);
					HideDoCommand(pChSpawn, "/autoinventory", 0);
				}
				else
				{
					WriteChatf("%s::Destroying [\ag%s\aw].", PLUGIN_NAME, szItem);
					DoCommand(pChSpawn, "/destroy");
				}
				if (!pChar2->pInventoryArray->Inventory.Cursor)
				{
					KeepDestroy = false;
					KeepItem = false;
					HasForaged = false;
					ForageSuccess = false;
				}
				if (!ForageSuccess && WasSitting)
				{
					WasSitting = false;
					DoCommand(pChSpawn, "/sit");
				}
			}
		}
	}
}

void Load_INI(VOID)
{
    char temp[MAX_STRING];
	//AutoKeepEnabled/AutoKeepAll
	VerifyINI("General", "AutoKeepAll", "on", INIFileName);
	GetPrivateProfileString("General", "AutoKeepAll", "on", temp, MAX_STRING, INIFileName);
	AutoKeepEnabled = atob(temp);

	//AutoAddAll
	VerifyINI("General", "AutoAddAll", "on", INIFileName);
	GetPrivateProfileString("General", "AutoAddAll", "on", temp, MAX_STRING, INIFileName);
	AutoAddEnabled = atob(temp);

	//Saving the last state of the /startforage /stopforage default is On.
	VerifyINI("General", "ForageOn", "on", INIFileName);
	GetPrivateProfileString("General", "ForageOn", "on", temp, MAX_STRING, INIFileName);
	IsForaging = atob(temp);
}

bool Check_INI(VOID)
{
    char szTemp[MAX_STRING];
    char szKeep[MAX_STRING];
    char szItem[64];
    char szZone[64];
    bool ItemSetting=false;
    PCHARINFO pChar = GetCharInfo();
    PITEMINFO pCursor = GetItemFromContents(GetCharInfo2()->pInventoryArray->Inventory.Cursor);
    sprintf_s(szItem,"%s",pCursor->Name);
    sprintf_s(szZone,"%s",GetFullZone(pChar->zoneId));
    sprintf_s(szKeep,"%s",AutoKeepEnabled?"keep":"destroy");
    GetPrivateProfileString(szZone,szItem,"NULL",szTemp,MAX_STRING,INIFileName);
    if (strstr(szTemp,"NULL"))
    {
        if (AutoKeepEnabled)
        {
            ItemSetting=true;
        }

        if (AutoAddEnabled)
        {
            WritePrivateProfileString(szZone,szItem,szKeep,INIFileName);
        }
    }
    else if (strstr(szTemp,"keep"))
    {
        ItemSetting=true;
    }
    else if (strstr(szTemp,"destroy"))
    {
        ItemSetting=false;
    }
    else
    {
		WriteChatf("%s::\arBad status in ini for item %s in zone %s. Using global setting\ax.",PLUGIN_NAME, szItem, szZone);
        if (AutoKeepEnabled)
        {
            ItemSetting=true;
        }
    }
    return ItemSetting;
}

// Added by Jaq -- Ripped off from mq2MoveUtils
bool IsBardClass()
{
    if (strncmp(pEverQuest->GetClassDesc(GetCharInfo2()->Class & 0xff),"Bard",5))
    {
        return false;
    }
    else
    {
        return true;
    }

}