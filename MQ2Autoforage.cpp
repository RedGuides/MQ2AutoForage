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

#include <mq/Plugin.h>

constexpr const char* PLUGIN_NAME = "MQ2AutoForage";

PreSetup(PLUGIN_NAME);
PLUGIN_VERSION(2.3);

void StartForageCommand(PSPAWNINFO pChar, PCHAR szLine);
void StopForageCommand(PSPAWNINFO pChar, PCHAR szLine);
void KeepItemCommand(PSPAWNINFO pChar, PCHAR szLine);
void DestroyItemCommand(PSPAWNINFO pChar, PCHAR szLine);
void MyUseAbility(PCHAR szLine);
bool AbilityReady(PCHAR szSkillName);
inline bool InGame();
void HandleItem();
void Load_INI();
bool Check_INI();

// FIXME: Defaults here are different from the defaults in the INI settings.  Use this bool to inform the other setting rather than hardcode.
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
bool bHandleCalled = false;

bool SetININame()
{
	if (gGameState==GAMESTATE_INGAME && GetCharInfo())
	{
		sprintf_s(INIFileName,"%s\\MQ2Forage_%s_%s.ini", gPathConfig, GetCharInfo()->Name, EQADDR_SERVERNAME);
		Load_INI();
		return true;
	}

	return false;
}

PLUGIN_API void InitializePlugin()
{
	AddCommand("/startforage",StartForageCommand);
	AddCommand("/stopforage",StopForageCommand);
	AddCommand("/keepitem",KeepItemCommand);
	AddCommand("/destroyitem",DestroyItemCommand);
	MQ2ForageEnabled = SetININame();
}

PLUGIN_API void ShutdownPlugin()
{
	RemoveCommand("/startforage");
	RemoveCommand("/stopforage");
	RemoveCommand("/keepitem");
	RemoveCommand("/destroyitem");
}

PLUGIN_API void OnZoned()
{
	//If I switch characters and IAmCamping is still true and I finish zoning, and the gamestate is ingame...
	if (IAmCamping && GetGameState() == GAMESTATE_INGAME)
		IAmCamping = false;
	Load_INI();
}

PLUGIN_API void OnPulse()
{
	static int Pulse = 0;

	if (!InGame())
		return;

	// Process every 25 pulses
	if (++Pulse < 25)
		return;

	Pulse = 0;

	//WriteChatf("Pulsing");
	if (!MQ2ForageEnabled)
	{
		return;
	}
	PSPAWNINFO pChSpawn = GetCharInfo()->pSpawn;

	if ((IsForaging) && !(*EQADDR_ATTACK > 0) && !pSpellBookWnd->IsVisible() && !pGiveWnd->IsVisible() && !pBankWnd->IsVisible() && !pMerchantWnd->IsVisible() && !pTradeWnd->IsVisible() && !pLootWnd->IsVisible() && !IAmCamping)// && !GetCharInfo()->pSpawn->Mount) {
	{
		if (AbilityReady("Forage")) {
			if (pChSpawn->StandState == STANDSTATE_SIT) {
				DoCommand(pChSpawn, "/stand");
				WasSitting=true;
			} else if (((PSPAWNINFO)pLocalPlayer)->CastingData.SpellETA == 0 || (GetPcProfile()->Class == Bard)) {
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
		if (auto pProfile = GetPcProfile())
		{
			if (pProfile->GetInventorySlot(InvSlot_Cursor))
			{
				if (!bHandleCalled)
				{
					//WriteChatf("Turning ON bHandleCalled");
					bHandleCalled = true;
				}
				HandleItem();
			}
			else
			{
				if (bHandleCalled)
				{
					//WriteChatf("Turning off bHandleCalled and bWaitForCursor");
					bHandleCalled = false;
					bWaitForCursor = false;
				}
			}
		}
	}
}

PLUGIN_API bool OnIncomingChat(const char* Line, DWORD Color)
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


PLUGIN_API void SetGameState(int GameState)
{
	MQ2ForageEnabled = SetININame();
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
	if (MQ2ForageEnabled)
	{
		PCHARINFO pCharInfo = GetCharInfo();
		WriteChatf("%s::Now auto-keeping item [\ag%s\aw].",PLUGIN_NAME, szLine);
		WritePrivateProfileString(GetFullZone(pCharInfo->zoneId), szLine, "keep", INIFileName);
	}
}

void DestroyItemCommand(PSPAWNINFO pChar, PCHAR szLine)
{
	if (MQ2ForageEnabled)
	{
		PCHARINFO pCharInfo = GetCharInfo();
		WriteChatf("%s::Now auto-destroying item [\ag%s\aw].",PLUGIN_NAME, szLine);
		WritePrivateProfileString(GetFullZone(pCharInfo->zoneId), szLine, "destroy", INIFileName);
	}
}

void MyUseAbility(PCHAR szLine)
{
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
	for (int i = 0; i < NUM_SKILLS; i++) {
		if (HasSkill(i)) {
			if (const char* thename = pStringTable->getString(pCSkillMgr->GetNameToken(i), nullptr)) {
				if (!_stricmp(szSkillName, thename))
				{
					if (pCSkillMgr->IsActivatedSkill(i))
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
	return(GetGameState() == GAMESTATE_INGAME && GetCharInfo() && GetCharInfo()->pSpawn && GetPcProfile());
}

void VerifyINI(char* Section, char* Key, char* Default, char* ININame)
{
	char temp[MAX_STRING] = { 0 };
	if (GetPrivateProfileString(Section, Key, 0, temp, MAX_STRING, ININame) == 0)
	{
		WritePrivateProfileString(Section, Key, Default, ININame);
	}
}

void HandleItem()
{
	if (PSPAWNINFO pChSpawn = (PSPAWNINFO)pLocalPlayer)
	{
		if (const auto pChar2 = GetPcProfile())
		{
			ItemClient* pCont = pChar2->GetInventorySlot(InvSlot_Cursor);
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
						if (pCont->IsStackable())
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
				if (!pChar2->GetInventorySlot(InvSlot_Cursor))
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

void Load_INI()
{
	char temp[MAX_STRING];
	//AutoKeepEnabled/AutoKeepAll
	VerifyINI("General", "AutoKeepAll", "on", INIFileName);
	GetPrivateProfileString("General", "AutoKeepAll", "on", temp, MAX_STRING, INIFileName);
	AutoKeepEnabled = GetBoolFromString(temp, AutoKeepEnabled);

	//AutoAddAll
	VerifyINI("General", "AutoAddAll", "on", INIFileName);
	GetPrivateProfileString("General", "AutoAddAll", "on", temp, MAX_STRING, INIFileName);
	AutoAddEnabled = GetBoolFromString(temp, AutoAddEnabled);

	//Saving the last state of the /startforage /stopforage default is On.
	VerifyINI("General", "ForageOn", "on", INIFileName);
	GetPrivateProfileString("General", "ForageOn", "on", temp, MAX_STRING, INIFileName);
	IsForaging = GetBoolFromString(temp, IsForaging);
}

bool Check_INI()
{
	char szTemp[MAX_STRING];
	char szKeep[MAX_STRING];
	bool ItemSetting=false;
	PCHARINFO pChar = GetCharInfo();
	PITEMINFO pCursor = GetItemFromContents(GetPcProfile()->GetInventorySlot(InvSlot_Cursor));
	sprintf_s(szKeep, "%s", AutoKeepEnabled ? "keep" : "destroy");
	GetPrivateProfileString(GetFullZone(pChar->zoneId), pCursor->Name, "NULL", szTemp, MAX_STRING, INIFileName);
	if (strstr(szTemp,"NULL"))
	{
		if (AutoKeepEnabled)
		{
			ItemSetting=true;
		}

		if (AutoAddEnabled)
		{
			WritePrivateProfileString(GetFullZone(pChar->zoneId), pCursor->Name, szKeep, INIFileName);
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
		WriteChatf("%s::\arBad status in ini for item %s in zone %s. Using global setting\ax.", PLUGIN_NAME, pCursor->Name, GetFullZone(pChar->zoneId));
		if (AutoKeepEnabled)
		{
			ItemSetting=true;
		}
	}
	return ItemSetting;
}
