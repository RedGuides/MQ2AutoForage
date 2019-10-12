/* MQ2AutoForage

   Simple plugin to automate the tast of foraging.
   
   Syntax:
   /startforage         - commence autoforaging.
   /stopforage          - stop autoforaging.
   /keepitem {item}     - add/change the item in the .ini file to auto-keep.
   /destroyitem {item}  - add/change the item in the .ini file to auto-destroy.
   
   Changelog:
   7/26/2016 - v2.0 - Fixed for string safety patch and cleaned up code (watlol)
*/

#define   PLUGIN_NAME   "MQ2AutoForage"
#define   PLUGIN_DATE   20160726
#define   PLUGIN_VERS   2.00

#include "../MQ2Plugin.h"

PreSetup(PLUGIN_NAME);
PLUGIN_VERSION(PLUGIN_VERS);

void StartForageCommand(PSPAWNINFO pChar, PCHAR szLine);
void StopForageCommand(PSPAWNINFO pChar, PCHAR szLine);
void KeepItemCommand(PSPAWNINFO pChar, PCHAR szLine);
void DestroyItemCommand(PSPAWNINFO pChar, PCHAR szLine);
bool CheckAbilityReady(PCHAR szSkillName);
void HandleItem(PCHARINFO pCharInfo);
void Load_INI(VOID);
bool Check_INI(VOID);

bool IsForaging=false;
bool HasForaged=false;
bool ForageSuccess=false;
bool KeepDestroy=false;
bool KeepItem=false;
bool WasSitting=false;
bool AutoKeepEnabled=true;
bool AutoAddEnabled=true;
bool MQ2ForageEnabled=false;
bool IAmCamping = false;

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

DWORD __stdcall WaitForCursor(PVOID pData)
{
    PCHARINFO pCharInfo = (PCHARINFO)pData;
    DWORD counter = 0;
    while (!GetCharInfo2()->pInventoryArray->Inventory.Cursor && counter<200) //20 seconds should be enough even when laggy
    {
        Sleep(100);
        counter++;
    }
    while (GetCharInfo2()->pInventoryArray->Inventory.Cursor)
    {
        HandleItem(pCharInfo);
        Sleep(100);
    }
    return 0;
}
PLUGIN_API VOID OnPulse(VOID)
{
    if (!MQ2ForageEnabled)
    {
        return;
    }
    PSPAWNINFO pChSpawn = GetCharInfo()->pSpawn;
    PCHARINFO pCharInfo = NULL;

    if ((IsForaging) && !(*EQADDR_ATTACK > 0) && !(PCSIDLWND)pSpellBookWnd->IsVisible() && !(PCSIDLWND)pGiveWnd->IsVisible() && !(PCSIDLWND)pBankWnd->IsVisible() && !(PCSIDLWND)pMerchantWnd->IsVisible() && !(PCSIDLWND)pTradeWnd->IsVisible() && !(PCSIDLWND)pLootWnd->IsVisible() && !IAmCamping)// && !GetCharInfo()->pSpawn->Mount) {
    {
        //if (CheckAbilityReady("Tracking")) {
        //DoAbility(pChSpawn,"Tracking");
        //}
        if (CheckAbilityReady("Forage")) {
            if (pChSpawn->StandState == STANDSTATE_SIT) {
                DoCommand(pChSpawn, "/stand");
                WasSitting=true;
            } else if (((PSPAWNINFO)pLocalPlayer)->CastingData.SpellETA == 0 || (IsBardClass())) {
                HasForaged=true;
                DoAbility(pChSpawn,"Forage");
            }
        }
    }
    if (ForageSuccess && IsForaging)
    {
        if (NULL == (pCharInfo = GetCharInfo()))
            return;
        DWORD nThreadID = 0;
        CreateThread(NULL,0,WaitForCursor,pCharInfo,0,&nThreadID);
        ForageSuccess = false;
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


PLUGIN_API void SetGameState(DWORD GameState)
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
    }
}

void StopForageCommand(PSPAWNINFO pChar, PCHAR szLine)
{
    if (MQ2ForageEnabled)
    {
		WriteChatf("%s::Forage [\arDisabled\aw].",PLUGIN_NAME);
        IsForaging=false;
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

bool CheckAbilityReady(PCHAR szSkillName)
{
    for (DWORD nSkill=0;szSkills[nSkill];nSkill++)
    {
        if (!_stricmp(szSkillName,szSkills[nSkill]))
        {
            if (GetCharInfo2()->Skill[nSkill]>300)
            {
                return false;
            }
            for (DWORD nAbility=0;nAbility<10;nAbility++)
            {
                if (EQADDR_DOABILITYLIST[nAbility] == nSkill)
                {
                    if (nAbility<4)
                    {
                        nAbility+=7;
                    }
                    else
                    {
                        nAbility-=3;
                    }
                    //if (pSkillMgr->pSkill[nSkill]->SkillCombatType==2) {
                    //return gbAltTimerReady?true:false;
                    //} else {
                    return pCSkillMgr->IsAvailable(nSkill)?true:false;
                    //}
                }
            }
        }
    }
    return false;
}

void HandleItem(PCHARINFO pCharInfo)
{
    CHAR szItem[64] = {0};
    PSPAWNINFO pChSpawn = GetCharInfo()->pSpawn;
    PITEMINFO pCursor = GetItemFromContents(GetCharInfo2()->pInventoryArray->Inventory.Cursor);
    sprintf_s(szItem,"%s",pCursor->Name);
    KeepDestroy=true;
    KeepItem=Check_INI();
    if (KeepItem)
    {
		WriteChatf("%s::Keeping [\ag%s\aw].",PLUGIN_NAME, szItem);
        //DoCommand(pChSpawn, "/autoinventory");
        HideDoCommand(pChSpawn, "/autoinventory",0);

    }
    else
    {
		WriteChatf("%s::Destroying [\ag%s\aw].",PLUGIN_NAME, szItem);
        DoCommand(pChSpawn, "/destroy");
    }
    if (!GetCharInfo2()->pInventoryArray->Inventory.Cursor)
    {
        KeepDestroy=false;
        KeepItem=false;
        HasForaged=false;
        ForageSuccess=false;
    }
    if (!ForageSuccess && WasSitting)
    {
        WasSitting=false;
        DoCommand(pChSpawn, "/sit");
    }
}

void Load_INI(VOID)
{
    char szTemp[MAX_STRING];
    GetPrivateProfileString("General","AutoKeepAll","NULL",szTemp,MAX_STRING,INIFileName);
    if (strstr(szTemp,"NULL"))
    {
		WriteChatf("%s::INI not found, creating defaults.",PLUGIN_NAME);
        WritePrivateProfileString("General","AutoKeepAll","on",INIFileName);
        WritePrivateProfileString("General","AutoAddAll","on",INIFileName);
    }
    else
    {
        GetPrivateProfileString("General","AutoKeepAll","NULL",szTemp,MAX_STRING,INIFileName);
        if ((strcmp(szTemp,"NULL"))||strcmp(szTemp,"on"))
        {
            AutoKeepEnabled=true;
        }
        else
        {
            AutoKeepEnabled=false;
        }
    }
    GetPrivateProfileString("General","AutoAddAll","NULL",szTemp,MAX_STRING,INIFileName);
    if ((strcmp(szTemp,"NULL"))||strcmp(szTemp,"on"))
    {
        AutoAddEnabled=true;
    }
    else
    {
        AutoAddEnabled=false;
    }
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