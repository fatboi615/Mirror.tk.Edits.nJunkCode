#include "EdgyLagComp.h"
#include "backtracking.h"
#include "Backtracking Rage.h"
//#include "Resolver.h"
#include "Hooks.h"
#include "Hacks.h"
#include "Chams.h"
#include "Menu.h"
#include "Interfaces.h"
#include "RenderManager.h"
#include "MiscHacks.h"
#include "Entities.h"
#include "CRC32.h"
#include "hitmarker.h"
#include <intrin.h>
#include "fakelaghelper.h"
#include "GlowManager.h"

extern float lineLBY;
extern float lineRealAngle;
extern float lineFakeAngle;
Vector LastAngleAAReal;
Vector LBYThirdpersonAngle;



#define STUDIO_NONE						0x00000000
#define STUDIO_RENDER					0x00000001
#define STUDIO_VIEWXFORMATTACHMENTS		0x00000002
#define STUDIO_DRAWTRANSLUCENTSUBMODELS 0x00000004
#define STUDIO_TWOPASS					0x00000008
#define STUDIO_STATIC_LIGHTING			0x00000010
#define STUDIO_WIREFRAME				0x00000020
#define STUDIO_ITEM_BLINK				0x00000040
#define STUDIO_NOSHADOWS				0x00000080
#define STUDIO_WIREFRAME_VCOLLIDE		0x00000200
#define STUDIO_NOLIGHTING_OR_CUBEMAP	0x00000200
#define STUDIO_SKIP_FLEXES				0x00000400
#define STUDIO_DONOTMODIFYSTENCILSTATE	0x00000800	// TERROR

// Not a studio flag, but used to flag model as a non-sorting brush model
#define STUDIO_TRANSPARENCY				0x80000000

// Not a studio flag, but used to flag model as using shadow depth material override
#define STUDIO_SHADOWDEPTHTEXTURE		0x40000000

// Not a studio flag, but used to flag model as doing custom rendering into shadow texture
#define STUDIO_SHADOWTEXTURE			0x20000000

#define STUDIO_SKIP_DECALS				0x20000000


float fakeangle;
float realyaw;
QAngle realangle;


#define MakePtr(cast, ptr, addValue) (cast)( (DWORD)(ptr) + (DWORD)(addValue))

#ifdef NDEBUG
#define strenc( s ) std::string( cx_make_encrypted_string( s ) )
#define charenc( s ) strenc( s ).c_str()
#define wstrenc( s ) std::wstring( strenc( s ).begin(), strenc( s ).end() )
#define wcharenc( s ) wstrenc( s ).c_str()
#else
#define strenc( s ) ( s )
#define charenc( s ) ( s )
#define wstrenc( s ) ( s )
#define wcharenc( s ) ( s )
#endif

#ifdef NDEBUG
#define XorStr( s ) ( XorCompileTime::XorString< sizeof( s ) - 1, __COUNTER__ >( s, std::make_index_sequence< sizeof( s ) - 1>() ).decrypt() )
#else
#define XorStr( s ) ( s )
#endif




int currentfov;
Vector LastAngleAA;
bool Resolver::didhitHS;
CUserCmd* Globals::UserCmd;
IClientEntity* Globals::Target;
int Globals::Shots;
bool Globals::change;
int Globals::TargetID;
std::map<int, QAngle>Globals::storedshit;
int Globals::missedshots;

typedef void(__thiscall* DrawModelEx_)(void*, void*, void*, const ModelRenderInfo_t&, matrix3x4*);
typedef void(__thiscall* PaintTraverse_)(PVOID, unsigned int, bool, bool);
typedef bool(__thiscall* InPrediction_)(PVOID);
typedef void(__stdcall *FrameStageNotifyFn)(ClientFrameStage_t);
typedef bool(__thiscall *FireEventClientSideFn)(PVOID, IGameEvent*);
typedef void(__thiscall* RenderViewFn)(void*, CViewSetup&, CViewSetup&, int, int);
using OverrideViewFn = void(__fastcall*)(void*, void*, CViewSetup*);
typedef float(__stdcall *oGetViewModelFOV)();
typedef void(__thiscall *SceneEnd_t)(void *pEcx);


SceneEnd_t pSceneEnd;
PaintTraverse_ oPaintTraverse;
DrawModelEx_ oDrawModelExecute;
FrameStageNotifyFn oFrameStageNotify;
OverrideViewFn oOverrideView;
FireEventClientSideFn oFireEventClientSide;
RenderViewFn oRenderView;


void __fastcall PaintTraverse_Hooked(PVOID pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce);
bool __stdcall Hooked_InPrediction();
bool __fastcall Hooked_FireEventClientSide(PVOID ECX, PVOID EDX, IGameEvent *Event);
void __fastcall Hooked_DrawModelExecute(void* thisptr, int edx, void* ctx, void* state, const ModelRenderInfo_t &pInfo, matrix3x4 *pCustomBoneToWorld);
bool __stdcall CreateMoveClient_Hooked(float frametime, CUserCmd* pCmd);
void  __stdcall Hooked_FrameStageNotify(ClientFrameStage_t curStage);
void __fastcall Hooked_OverrideView(void* ecx, void* edx, CViewSetup* pSetup);
float __stdcall GGetViewModelFOV();
void __fastcall Hooked_RenderView(void* ecx, void* edx, CViewSetup &setup, CViewSetup &hudViewSetup, int nClearFlags, int whatToDraw);
void __fastcall	hkSceneEnd(void *pEcx, void *pEdx);

namespace Hooks
{
	Utilities::Memory::VMTManager VMTPanel;
	Utilities::Memory::VMTManager VMTClient;
	Utilities::Memory::VMTManager VMTClientMode;
	Utilities::Memory::VMTManager VMTModelRender;
	Utilities::Memory::VMTManager VMTPrediction;
	Utilities::Memory::VMTManager VMTRenderView;
	Utilities::Memory::VMTManager VMTEventManager;
};

void Hooks::UndoHooks()
{
	VMTPanel.RestoreOriginal();
	VMTPrediction.RestoreOriginal();
	VMTModelRender.RestoreOriginal();
	VMTClientMode.RestoreOriginal();
}

void Hooks::Initialise()
{

	Interfaces::Engine->ExecuteClientCmd("clear");

	VMTPanel.Initialise((DWORD*)Interfaces::Panels);
	oPaintTraverse = (PaintTraverse_)VMTPanel.HookMethod((DWORD)&PaintTraverse_Hooked, Offsets::VMT::Panel_PaintTraverse);

	VMTPrediction.Initialise((DWORD*)Interfaces::Prediction);
	VMTPrediction.HookMethod((DWORD)&Hooked_InPrediction, 14);

	VMTModelRender.Initialise((DWORD*)Interfaces::ModelRender);
	oDrawModelExecute = (DrawModelEx_)VMTModelRender.HookMethod((DWORD)&Hooked_DrawModelExecute, Offsets::VMT::ModelRender_DrawModelExecute);

	VMTClientMode.Initialise((DWORD*)Interfaces::ClientMode);
	VMTClientMode.HookMethod((DWORD)CreateMoveClient_Hooked, 24);

	VMTRenderView.Initialise((DWORD*)Interfaces::RenderView);
	pSceneEnd = (SceneEnd_t)VMTRenderView.HookMethod((DWORD)&hkSceneEnd, 9);

	oOverrideView = (OverrideViewFn)VMTClientMode.HookMethod((DWORD)&Hooked_OverrideView, 18);
	VMTClientMode.HookMethod((DWORD)&GGetViewModelFOV, 35);

	VMTClient.Initialise((DWORD*)Interfaces::Client);
	oFrameStageNotify = (FrameStageNotifyFn)VMTClient.HookMethod((DWORD)&Hooked_FrameStageNotify, 36);
	;
	VMTEventManager.Initialise((DWORD*)Interfaces::EventManager);
	oFireEventClientSide = (FireEventClientSideFn)VMTEventManager.HookMethod((DWORD)&Hooked_FireEventClientSide, 9);







	Interfaces::Engine->ClientCmd_Unrestricted("toggleconsole");
	Interfaces::Engine->ClientCmd_Unrestricted("clear");
	Interfaces::Engine->ClientCmd_Unrestricted("echo [Mirror.tk] Hello! Thank you for downloading Mirror.tk Update #4");

	Interfaces::Engine->ClientCmd_Unrestricted("echo [Mirror.tk] This cheat is a community project and will keep recieving updates as time goes");
	Sleep(150);
	Interfaces::Engine->ClientCmd_Unrestricted("echo [Mirror.tk] PLEASE NOTE THE FOLLOWING:");
	Sleep(400);
	Interfaces::Engine->ClientCmd_Unrestricted("echo [Mirror.tk] 1- If you want to import a config, place the .xml files in the csgo folder (where you installed it)");
	Sleep(800);
	Interfaces::Engine->ClientCmd_Unrestricted("echo [Mirror.tk] 2- This cheat is in BETA, please don't toggle everything on at once as it is still being worked on");
	Sleep(900);
	Interfaces::Engine->ClientCmd_Unrestricted("echo [Mirror.tk] 3- Fullscreen Windowed / Windowed is NECESSARY to play with this cheat without crashing");
	Sleep(1100);
	Interfaces::Engine->ClientCmd_Unrestricted("echo [Mirror.tk] 4- This is a PUBLIC CHEAT, so unless you add Junkcode to the functions, or redo something yourself, it is DETECTED!");
	Sleep(1000);
	Interfaces::Engine->ClientCmd_Unrestricted("echo [Mirror.tk] Now, please enjoy Mirror.tk V4");
	Sleep(2000);
	Interfaces::Engine->ClientCmd_Unrestricted("echo [Mirror.tk] FreaKratsKids");
	Sleep(200);
	Interfaces::Engine->ClientCmd_Unrestricted("toggleconsole");




}

void MovementCorrection(CUserCmd* pCmd)
{

}

float clip(float n, float lower, float upper)
{

	return (std::max)(lower, (std::min)(n, upper));
}

int LagCompBreak() {
	IClientEntity *pLocalPlayer = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	Vector velocity = pLocalPlayer->GetVelocity();
	velocity.z = 0;
	float speed = velocity.Length();
	if (speed > 0.f) {
		auto distance_per_tick = speed *
			Interfaces::Globals->interval_per_tick;
		int choked_ticks = std::ceilf(65.f / distance_per_tick);
		return std::min<int>(choked_ticks, 14);
	}
	return 1;
}




void Prediction(CUserCmd* pCmd, IClientEntity* LocalPlayer)
{
	BYTE bMoveData[0x200];
	// Interfaces::MoveHelper && 
	if (Menu::Window.RageBotTab.AimbotEnable.GetState())
	{

		float curtime = Interfaces::Globals->curtime;
		float frametime = Interfaces::Globals->frametime;
		int iFlags = LocalPlayer->GetFlags();

		Interfaces::Globals->curtime = (float)LocalPlayer->GetTickBase() * Interfaces::Globals->interval_per_tick;
		Interfaces::Globals->frametime = Interfaces::Globals->interval_per_tick;
		Interfaces::MoveHelper->SetHost(LocalPlayer);

		Interfaces::Prediction1->SetupMove(LocalPlayer, pCmd, nullptr, bMoveData);
		Interfaces::GameMovement->ProcessMovement(LocalPlayer, bMoveData);
		Interfaces::Prediction1->FinishMove(LocalPlayer, pCmd, bMoveData);

		Interfaces::MoveHelper->SetHost(0);

		Interfaces::Globals->curtime = curtime;
		Interfaces::Globals->frametime = frametime;
		*LocalPlayer->GetPointerFlags() = iFlags;
	}
}



int kek = 0;
int autism = 0;

namespace esketit
{
	bool match;
	bool isfake;
	bool isreal;




}

void SetClanTag(const char* tag, const char* name)//190% paste
{

	static auto pSetClanTag = reinterpret_cast<void(__fastcall*)(const char*, const char*)>(((DWORD)Utilities::Memory::FindPattern("engine.dll", (PBYTE)"\x53\x56\x57\x8B\xDA\x8B\xF9\xFF\x15\x00\x00\x00\x00\x6A\x24\x8B\xC8\x8B\x30", "xxxxxxxxx????xxxxxx")));
	pSetClanTag(tag, name);

}
void NoClantag()
{

	SetClanTag("", "");

}

void ClanTag()
{
	static int counter = 0;
	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	switch (Menu::Window.MiscTab.OtherClantag.GetIndex())
	{
	case 0:
		break;
	case 1:
	{
		static int motion = 0;
		int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * 1.3;
		if (counter % 48 == 0)
			motion++;
		int value = ServerTime % 1;
		switch (value)
		{ //mirror.tk
		case 0:SetClanTag("[Mirror.tk]", "Mirror.tk V4 - by FreaK"); break;
		}
	}

	break;
	case 2:
	{
		static int motion = 0;
		int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * 1.3;
		if (counter % 48 == 0)
			motion++;
		int value = ServerTime % 21;
		switch (value) { //mirror.tk
		case 0:SetClanTag("           ", "MIRROR.TK"); break;
		case 1:SetClanTag("M          ", "M1RR0R.TK"); break;
		case 2:SetClanTag("MI        ", "MIRROR.TK"); break;
		case 3:SetClanTag("M1R      ", "M1RR0R.TK"); break;
		case 4:SetClanTag("MIRR    ", "MIRROR.TK"); break;
		case 5:SetClanTag("MIRRO  ", "M1RR0R.TK"); break;
		case 6:SetClanTag("MIRR0R", "MIRROR.TK"); break;
		case 7:SetClanTag("MIRRO", "M1RR0R.TK"); break;
		case 8:SetClanTag("M1RR", "MIRROR.TK"); break;
		case 9:SetClanTag("MIR", "M1RR0R.TK"); break;
		case 10:SetClanTag("MI", "MIRROR.TK"); break;
		case 11:SetClanTag("M            ", "M1RR0R.TK"); break;
		case 12:SetClanTag("Mi           ", "MIRROR.TK"); break;
		case 13:SetClanTag("Mir          ", "M1RR0R.TK"); break;
		case 14:SetClanTag("Mirr         ", "MIRROR.TK"); break;
		case 15:SetClanTag("Mirro        ", "M1RR0R.TK"); break;
		case 16:SetClanTag("Mirror       ", "MIRROR.TK"); break;
		case 17:SetClanTag("Mirror.      ", "M1RR0R.TK"); break;
		case 18:SetClanTag("Mirror.t     ", "MIRROR.TK"); break;
		case 19:SetClanTag("Mirror.tk    ", "M1RR0R.TK"); break;
		case 20:SetClanTag(" Mirror.tk   ", "MIRROR.TK"); break;
		case 21:SetClanTag("  Mirror.tk  ", "M1RR0R.TK"); break;
		}
		counter++;

		break;
	}
	case 3:
	{
		
			if (pLocal->IsDefusing())
			{
				static int motion = 0;
				int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * 1.5;
				if (counter % 48 == 0)
					motion++;
				int value = ServerTime % 9;
				switch (value) {
				case 0:SetClanTag("_____________", "Mirror.tk V4"); break;
				case 1:SetClanTag("FreaK________", "Mirror.tk V4"); break;
				case 2:SetClanTag("_____rats____", "Mirror.tk V4"); break;
				case 3:SetClanTag("_________Kids", "Mirror.tk V4"); break;
				case 4:SetClanTag("_____rats____", "Mirror.tk V4"); break;
				case 5:SetClanTag("FreaK________", "Mirror.tk V4"); break;
				case 6:SetClanTag("_____rats____", "Mirror.tk V4"); break;
				case 7:SetClanTag("_________Kids", "Mirror.tk V4"); break;
				case 8:SetClanTag("FreaKratsKids", "Mirror.tk V4"); break;


				}
				counter++;
			}
			else
			{

				static int motion = 0;
				int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * 2.5;
				if (counter % 48 == 0)
					motion++;
				int value = ServerTime % 9;
				switch (value) {
				case 0:SetClanTag("OmgNoNamerRat", "Mirror.tk V4"); break;
				case 1:SetClanTag("FreaK________", "Mirror.tk V4"); break;
				case 2:SetClanTag("_____rats____", "Mirror.tk V4"); break;
				case 3:SetClanTag("_________Kids", "Mirror.tk V4"); break;
				case 4:SetClanTag("_____rats____", "Mirror.tk V4"); break;
				case 5:SetClanTag("FreaK________", "Mirror.tk V4"); break;
				case 6:SetClanTag("_____rats____", "Mirror.tk V4"); break;
				case 7:SetClanTag("_________Kids", "Mirror.tk V4"); break;
				case 8:SetClanTag("FreaKratsKids", "Mirror.tk V4"); break;


				}
				counter++;
		}
		
	

		

		break;
	}
	case 4:
	{
		if (pLocal->IsAlive())
		{
			static int motion = 0;



			int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * 1.4;
			if (counter % 48 == 0)
				motion++;
			int value = ServerTime % 17;
			switch (value)
			{
			case 0:SetClanTag("Mirror.tk    ", "Mirror.tk v4"); break;
			case 1:SetClanTag(" Mirror.tk   ", "/c/freakcsgohacker"); break;
			case 2:SetClanTag("  Mirror.tk  ", "UnknownCheats.me"); break;
			case 3:SetClanTag("   Mirror.tk ", "Mirror.tk v4"); break;
			case 4:SetClanTag("    Mirror.tk", "/c/freakcsgohacker"); break;
			case 5:SetClanTag("   M irror.tk", "Hxppy Thoughts"); break;
			case 6:SetClanTag("  M i rror.tk", " M i r r o r . t k "); break;
			case 7:SetClanTag(" M i r ror.tk", "FREAKWARE"); break;
			case 8:SetClanTag("M i r r or.tk", "FREAKWARE"); break;
			case 9:SetClanTag("Mi r r o r.tk", "FREAKWARE"); break;
			case 10:SetClanTag("Mir r o r .tk", "FREAKWARE"); break;
			case 11:SetClanTag("Mirr o r . tk", "FREAKWARE"); break;
			case 12:SetClanTag("Mirro r . t k", "FREAKWARE"); break;
			case 13:SetClanTag("Mirror . t k ", "FREAKWARE"); break;
			case 14:SetClanTag("Mirror. t k  ", "FREAKWARE"); break;
			case 15:SetClanTag("Mirror.t k   ", "FREAKWARE"); break;
			case 16:SetClanTag("Mirror.tk    ", "FREAKWARE"); break;




			}
			counter++;
		}

		else 
		{
			static int motion = 0;
			int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * 2.5;
			if (counter % 48 == 0)
				motion++;
			int value = ServerTime % 12;
			switch (value)
			{
			case 0:SetClanTag(" M i r r o r ", "Mirror.tk v4"); break;
			case 1:SetClanTag("M i r r o r  ", "/c/freakcsgohacker"); break;
			case 2:SetClanTag("Mi r r o r   ", "UnknownCheats.me"); break;
			case 3:SetClanTag("Mir r o r    ", "Mirror.tk v4"); break;
			case 4:SetClanTag("Mirr o r     ", "/c/freakcsgohacker"); break;
			case 5:SetClanTag("Mirro r      ", "Hxppy Thoughts"); break;
			case 6:SetClanTag("Mirror       ", " M i r r o r . t k "); break;
			case 7:SetClanTag("Mirro r      ", "FREAKWARE"); break;
			case 8:SetClanTag("Mirr o r     ", "/c/freakcsgohacker"); break;
			case 9:SetClanTag("Mir r o r    ", "UnknownCheats.me"); break;
			case 10:SetClanTag("Mi r r o r  ", "Mirror.tk v4"); break;
			case 11:SetClanTag("M i r r o r  ", "Mirror.tk v4"); break;




			}
			counter++;
		}
		break;
	}
	case 5:
	{

		static int motion = 0;

		int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * 2;
		if (counter % 48 == 0)
			motion++;
		int value = ServerTime % 10;
		switch (value)
		{
		case 0:SetClanTag("           ", "Mirror.tk v4"); break;
		case 1:SetClanTag("          M", "Mirror.tk v4"); break;
		case 2:SetClanTag("         M", "Mirror.tk v4"); break;
		case 3:SetClanTag("        M", "Mirror.tk v4"); break;
		case 4:SetClanTag("       M", "Mirror.tk v4"); break;
		case 5:SetClanTag("      M", "Mirror.tk v4"); break;
		case 6:SetClanTag("     M", "Mirror.tk v4"); break;
		case 7:SetClanTag("    M", "Mirror.tk v4"); break;
		case 8:SetClanTag("   M", "Mirror.tk v4"); break;
		case 9:SetClanTag("  M", "Mirror.tk v4"); break;
		case 10:SetClanTag(" M", "Mirror.tk v4"); break;
		case 11:SetClanTag("M", "Mirror.tk v4"); break;
		case 12:SetClanTag("M         u", "Mirror.tk v4"); break;
		case 13:SetClanTag("M        u", "Mirror.tk v4"); break;
		case 14:SetClanTag("M       u", "Mirror.tk v4"); break;
		case 15:SetClanTag("M      u", "Mirror.tk v4"); break;
		case 16:SetClanTag("M     u", "Mirror.tk v4"); break;
		case 17:SetClanTag("M    u", "Mirror.tk v4"); break;
		case 18:SetClanTag("M   u", "Mirror.tk v4"); break;
		case 19:SetClanTag("M  u", "Mirror.tk v4"); break;
		case 20:SetClanTag("M u", "Mirror.tk v4"); break;
		case 21:SetClanTag("Mu", "Mirror.tk v4"); break;
		case 22:SetClanTag("Mu        t", "Mirror.tk v4"); break;
		case 23:SetClanTag("Mu       t", "Mirror.tk v4"); break;
		case 24:SetClanTag("Mu      t", "Mirror.tk v4"); break;
		case 25:SetClanTag("Mu     t", "Mirror.tk v4"); break;
		case 26:SetClanTag("Mu    t", "Mirror.tk v4"); break;
		case 27:SetClanTag("Mu   t", "Mirror.tk v4"); break;
		case 28:SetClanTag("Mu  t", "Mirror.tk v4"); break;
		case 29:SetClanTag("Mu t", "Mirror.tk v4"); break;
		case 30:SetClanTag("Mut", "Mirror.tk v4"); break;
		case 31:SetClanTag("Mut       i", "Mirror.tk v4"); break;
		case 32:SetClanTag("Mut      i", "Mirror.tk v4"); break;
		case 33:SetClanTag("Mut     i", "Mirror.tk v4"); break;
		case 34:SetClanTag("Mut    i", "Mirror.tk v4"); break;
		case 35:SetClanTag("Mut   i", "Mirror.tk v4"); break;
		case 36:SetClanTag("Mut  i", "Mirror.tk v4"); break;
		case 37:SetClanTag("Mut i", "Mirror.tk v4"); break;
		case 38:SetClanTag("Muti", "Mirror.tk v4"); break;
		case 39:SetClanTag("Muti      n", "Mirror.tk v4"); break;
		case 40:SetClanTag("Muti     n", "Mirror.tk v4"); break;
		case 41:SetClanTag("Muti    n", "Mirror.tk v4"); break;
		case 42:SetClanTag("Muti   n", "Mirror.tk v4"); break;
		case 43:SetClanTag("Muti  n", "Mirror.tk v4"); break;
		case 44:SetClanTag("Muti n", "Mirror.tk v4"); break;
		case 45:SetClanTag("Mutin", "Mirror.tk v4"); break;
		case 46:SetClanTag("Mutin     y", "Mirror.tk v4"); break;
		case 47:SetClanTag("Mutin    y", "Mirror.tk v4"); break;
		case 48:SetClanTag("Mutin   y", "Mirror.tk v4"); break;
		case 49:SetClanTag("Mutin  y", "Mirror.tk v4"); break;
		case 50:SetClanTag("Mutin y", "Mirror.tk v4"); break;
		case 51:SetClanTag("Mutiny", "Mirror.tk v4"); break;
		case 52:SetClanTag("Mutiny    .", "Mirror.tk v4"); break;
		case 53:SetClanTag("Mutiny   .", "Mirror.tk v4"); break;
		case 54:SetClanTag("Mutiny  .", "Mirror.tk v4"); break;
		case 55:SetClanTag("Mutiny .", "Mirror.tk v4"); break;
		case 56:SetClanTag("Mutiny.", "Mirror.tk v4"); break;
		case 57:SetClanTag("Mutiny.   p", "Mirror.tk v4"); break;
		case 58:SetClanTag("Mutiny.  p", "Mirror.tk v4"); break;
		case 59:SetClanTag("Mutiny. p", "Mirror.tk v4"); break;
		case 60:SetClanTag("Mutiny.p", "Mirror.tk v4"); break;
		case 61:SetClanTag("Mutiny.p  w", "Mirror.tk v4"); break;
		case 62:SetClanTag("Mutiny.p w", "Mirror.tk v4"); break;
		case 63:SetClanTag("Mutiny.pw ", "Mirror.tk v4"); break;
		case 64:SetClanTag("Mutiny.pw", "Mirror.tk v4"); break;
		case 65:SetClanTag("utiny.pw ", "Mirror.tk v4"); break;
		case 66:SetClanTag("tiny.pw  ", "Mirror.tk v4"); break;
		case 67:SetClanTag("iny.pw   ", "Mirror.tk v4"); break;
		case 68:SetClanTag("ny.pw    ", "Mirror.tk v4"); break;
		case 69:SetClanTag("y.pw     ", "Mirror.tk v4"); break;
		case 70:SetClanTag(".pw      ", "Mirror.tk v4"); break;
		case 71:SetClanTag("pw       ", "Mirror.tk v4"); break;
		case 72:SetClanTag("w        ", "Mirror.tk v4"); break;
		}
		counter++;
		break;
	}
	case 6:
	{

		static int motion = 0;

		int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * 1;
		if (counter % 48 == 0)
			motion++;
		int value = ServerTime % 4;
		switch (value)
		{
		case 0:SetClanTag("HookTronic.TOP", "Mirror.tk v4"); break;
		case 1:SetClanTag("HookTronic.TOP", "Mirror.tk v4"); break;
		case 2:SetClanTag("HookTronic.TOP", "Mirror.tk v4"); break;
		case 3:SetClanTag("HookTronic.T0P", "Mirror.tk v4"); break;
		}
		counter++;
		break;

	}
	case 7:
	{

		static int motion = 0;

		int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * 1;
		if (counter % 48 == 0)
			motion++;
		int value = ServerTime % 18;
		switch (value)
		{
		case 0:SetClanTag("         ", "Mirror.tk v4"); break;
		case 1:SetClanTag("p        ", "Mirror.tk v4"); break;
		case 2:SetClanTag("pa       ", "Mirror.tk v4"); break;
		case 3:SetClanTag("pas      ", "Mirror.tk v4"); break;
		case 4:SetClanTag("past     ", "Mirror.tk v4"); break;
		case 5:SetClanTag("paste    ", "Mirror.tk v4"); break;
		case 6:SetClanTag("pastew   ", "Mirror.tk v4"); break;
		case 7:SetClanTag("pastewa  ", "Mirror.tk v4"); break;
		case 8:SetClanTag("pastewar ", "Mirror.tk v4"); break;
		case 9:SetClanTag("pasteware", "Mirror.tk v4"); break;
		case 10:SetClanTag("pastewar ", "Mirror.tk v4"); break;
		case 11:SetClanTag("pastewa  ", "Mirror.tk v4"); break;
		case 12:SetClanTag("pastew   ", "Mirror.tk v4"); break;
		case 13:SetClanTag("paste    ", "Mirror.tk v4"); break;
		case 14:SetClanTag("past     ", "Mirror.tk v4"); break;
		case 15:SetClanTag("pas      ", "Mirror.tk v4"); break;
		case 16:SetClanTag("pa       ", "Mirror.tk v4"); break;
		case 17:SetClanTag("p        ", "Mirror.tk v4"); break;

		}
		counter++;
		break;

	}
	case 8:
	{

		static int motion = 0;

		int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * 1;
		if (counter % 48 == 0)
			motion++;
		int value = ServerTime % 2;
		switch (value)
		{
		case 0:SetClanTag("-INTERWEBZ", "Mirror.tk v4"); break;
		case 1:SetClanTag("INTERWEBZ-", "Mirror.tk v4"); break;
		}
		counter++;
		break;

	}
	break;
	case 9:
	{

		static int motion = 0;
		int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * 1;
		if (counter % 48 == 0)
			motion++;
		int value = ServerTime % 1;
		switch (value)
		{
		case 0:SetClanTag("SUPREMACY $", "Mirror.tk v4"); break;
		}
	}
	break;
	case 10:
	{

		static int motion = 0;
		int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * 1;
		if (counter % 48 == 0)
			motion++;
		int value = ServerTime % 1;
		switch (value)
		{
		case 0:SetClanTag("X22", "Mirror.tk v4"); break;
		}
	}
	break;
	case 11:
	{

		static int motion = 0;

		int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * 1;
		if (counter % 48 == 0)
			motion++;
		int value = ServerTime % 12;
		switch (value)
		{
		case 0:SetClanTag("AIMWARE.NET", "Mirror.tk v4"); break;
		case 1:SetClanTag("IMWARE.NET ", "Mirror.tk v4"); break;
		case 2:SetClanTag("MWARE.NET  ", "Mirror.tk v4"); break;
		case 3:SetClanTag("WARE.NET  A", "Mirror.tk v4"); break;
		case 4:SetClanTag("ARE.NET  AI", "Mirror.tk v4"); break;
		case 5:SetClanTag("RE.NET  AIM", "Mirror.tk v4"); break;
		case 6:SetClanTag("E.NET  AIMW", "Mirror.tk v4"); break;
		case 7:SetClanTag(".NET  AIMWA", "Mirror.tk v4"); break;
		case 8:SetClanTag("NET AIMWARE", "Mirror.tk v4"); break;
		case 9:SetClanTag("ET AIMWARE.", "Mirror.tk v4"); break;
		case 10:SetClanTag("T AIMWARE.N", "Mirror.tk v4"); break;
		case 11:SetClanTag(" AIMWARE.NE", "Mirror.tk v4"); break;


		}
		counter++;
		break;

	}
	break;
	case 12:
	{

		static int motion = 0;

		int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * 1;
		if (counter % 48 == 0)
			motion++;
		int value = ServerTime % 11;
		switch (value)
		{
		case 0:SetClanTag("", "Mirror.tk v4"); break;
		case 1:SetClanTag("e", "Mirror.tk v4"); break;
		case 2:SetClanTag("ev", "Mirror.tk v4"); break;
		case 3:SetClanTag("ev0", "Mirror.tk v4"); break;
		case 4:SetClanTag("ev0l", "Mirror.tk v4"); break;
		case 5:SetClanTag("ev0lv", "Mirror.tk v4"); break;
		case 6:SetClanTag("ev0lve", "Mirror.tk v4"); break;
		case 7:SetClanTag("ev0lve.", "Mirror.tk v4"); break;
		case 8:SetClanTag("ev0lve.x", "Mirror.tk v4"); break;
		case 9:SetClanTag("ev0lve.xy", "Mirror.tk v4"); break;
		case 10:SetClanTag("ev0lve.xyz", "Mirror.tk v4"); break;

		}
		counter++;
		break;

	}
	break;
	case 13:
	{

		static int motion = 0;

		int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * 1;
		if (counter % 48 == 0)
			motion++;
		int value = ServerTime % 30;
		switch (value)
		{
		case 0:SetClanTag("               ", "Mirror.tk v4"); break;
		case 1:SetClanTag("              b", "Mirror.tk v4"); break;
		case 2:SetClanTag("             bi", "Mirror.tk v4"); break;
		case 3:SetClanTag("            bit", "Mirror.tk v4"); break;
		case 4:SetClanTag("           bitw", "Mirror.tk v4"); break;
		case 5:SetClanTag("          bitwi", "Mirror.tk v4"); break;
		case 6:SetClanTag("         bitwis", "Mirror.tk v4"); break;
		case 7:SetClanTag("        bitwise", "Mirror.tk v4"); break;
		case 8:SetClanTag("       bitwise.", "Mirror.tk v4"); break;
		case 9:SetClanTag("      bitwise.s", "Mirror.tk v4"); break;
		case 10:SetClanTag("     bitwise.sy", "Mirror.tk v4"); break;
		case 11:SetClanTag("    bitwise.sys", "Mirror.tk v4"); break;
		case 12:SetClanTag("   bitwise.syst", "Mirror.tk v4"); break;
		case 13:SetClanTag("  bitwise.syste", "Mirror.tk v4"); break;
		case 14:SetClanTag(" bitwise.system", "Mirror.tk v4"); break;
		case 15:SetClanTag("bitwise.systems", "Mirror.tk v4"); break;
		case 16:SetClanTag("itwise.systems ", "Mirror.tk v4"); break;
		case 17:SetClanTag("twise.systems  ", "Mirror.tk v4"); break;
		case 18:SetClanTag("wise.systems   ", "Mirror.tk v4"); break;
		case 19:SetClanTag("ise.systems    ", "Mirror.tk v4"); break;
		case 20:SetClanTag("se.systems     ", "Mirror.tk v4"); break;
		case 21:SetClanTag("e.systems      ", "Mirror.tk v4"); break;
		case 22:SetClanTag(".systems       ", "Mirror.tk v4"); break;
		case 23:SetClanTag("systems        ", "Mirror.tk v4"); break;
		case 24:SetClanTag("ystems         ", "Mirror.tk v4"); break;
		case 25:SetClanTag("stems          ", "Mirror.tk v4"); break;
		case 26:SetClanTag("tems           ", "Mirror.tk v4"); break;
		case 27:SetClanTag("ems            ", "Mirror.tk v4"); break;
		case 28:SetClanTag("ms             ", "Mirror.tk v4"); break;
		case 29:SetClanTag("s              ", "Mirror.tk v4"); break;
	

		}
		counter++;
		break;

	}
	break;


	}

}

bool __stdcall CreateMoveClient_Hooked(float frametime, CUserCmd* pCmd)
{
	if (!pCmd->command_number)
		return true;

	IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && pLocal)
	{

		PVOID pebp;
		__asm mov pebp, ebp;
		bool* pbSendPacket = (bool*)(*(DWORD*)pebp - 0x1C);
		bool& bSendPacket = *pbSendPacket;

		if (Menu::Window.MiscTab.OtherClantag.GetIndex() > 0)
			ClanTag();
		Vector origView = pCmd->viewangles;
		Vector viewforward, viewright, viewup, aimforward, aimright, aimup;
		Vector qAimAngles;
		qAimAngles.Init(0.0f, pCmd->viewangles.y, 0.0f);
		AngleVectors(qAimAngles, &viewforward, &viewright, &viewup);

		for (int i = 1; i < Interfaces::Engine->GetMaxClients(); i++)
		{
			IClientEntity* pBaseEntity = Interfaces::EntList->GetClientEntity(i);
			if (pBaseEntity && !pBaseEntity->IsDormant() && pBaseEntity != hackManager.pLocal()) globalsh.OldSimulationTime[i] = pBaseEntity->GetSimulationTime();
			if (pBaseEntity && !pBaseEntity->IsDormant() && pBaseEntity != hackManager.pLocal() && Menu::Window.RageBotTab.ragetrack.GetKey()) DataManager.UpdatePlayerPos();
		}

		IClientEntity* pEntity;
		if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && pLocal && pLocal->IsAlive())
		{
			Hacks::MoveHacks(pCmd, bSendPacket);
			ResoSetup::GetInst().CM(pEntity);
		}
		backtracking->legitBackTrack(pCmd, pLocal);
		backtracking->rageBackTrack(pCmd, pLocal);




		static bool abc = false;

		static bool freakware = false;
		if (Menu::Window.VisualsTab.colmodupdate.GetState())
		{
			if (!freakware)
			{
				ConVar* staticdrop = Interfaces::CVar->FindVar("r_DrawSpecificStaticProp");
				SpoofedConvar* staticdrop_spoofed = new SpoofedConvar(staticdrop);
				staticdrop_spoofed->SetInt(0);

				{
					for (MaterialHandle_t i = Interfaces::MaterialSystem->FirstMaterial(); i != Interfaces::MaterialSystem->InvalidMaterial(); i = Interfaces::MaterialSystem->NextMaterial(i))
					{
						IMaterial *pMaterial = Interfaces::MaterialSystem->GetMaterial(i);

						if (!pMaterial)
							continue;

						ConVar* sky = Interfaces::CVar->FindVar("sv_skyname");
						SpoofedConvar* sky_spoofed = new SpoofedConvar(sky);
				

						float amountr = Menu::Window.VisualsTab.colmodr.GetValue();
						float test = Menu::Window.VisualsTab.asusamount.GetValue();
						if (amountr <= 0.9999)
						{
							sky_spoofed->SetString("sky_csgo_night02");
							if (!strcmp(pMaterial->GetTextureGroupName(), "World textures"))  // walls	
							{
								pMaterial->ColorModulation(amountr, amountr, amountr);

							}
							if (!strcmp(pMaterial->GetTextureGroupName(), "StaticProp textures"))
							{
								pMaterial->ColorModulation(amountr, amountr, amountr);   // values of boxes n barrels n shit

								// ---- the || below me means "or" ---- //
								if (strstr(pMaterial->GetName(), "door") || strstr(pMaterial->GetName(), "box") || strstr(pMaterial->GetName(), "crate") || strstr(pMaterial->GetName(), "window") || strstr(pMaterial->GetName(), "wood") || strstr(pMaterial->GetName(), "glass") || strstr(pMaterial->GetName(), "sheet_metal_panel") || strstr(pMaterial->GetName(), "rocka") || strstr(pMaterial->GetName(), "rockb") || strstr(pMaterial->GetName(), "portapotty01") || strstr(pMaterial->GetName(), "window_frame_a"))
									pMaterial->AlphaModulate(test);

							}
						}
						if (amountr > 0.9999)
						{
							sky_spoofed->SetString("");
							if (!strcmp(pMaterial->GetTextureGroupName(), "World textures"))  // walls	
							{
								pMaterial->ColorModulation(amountr, amountr, amountr);

							}
							if (!strcmp(pMaterial->GetTextureGroupName(), "StaticProp textures"))
							{
								pMaterial->ColorModulation(amountr, amountr, amountr);   // values of boxes n barrels n shit

																						 // ---- the || below me means "or" ---- //
								if (strstr(pMaterial->GetName(), "door") || strstr(pMaterial->GetName(), "box") || strstr(pMaterial->GetName(), "crate") || strstr(pMaterial->GetName(), "window") || strstr(pMaterial->GetName(), "wood") || strstr(pMaterial->GetName(), "glass") || strstr(pMaterial->GetName(), "sheet_metal_panel") || strstr(pMaterial->GetName(), "rocka") || strstr(pMaterial->GetName(), "rockb") || strstr(pMaterial->GetName(), "portapotty01") || strstr(pMaterial->GetName(), "window_frame_a"))
									pMaterial->AlphaModulate(test);

							}
						}
						
					}
				}
			}
			freakware = true; // Update for anti fps suicide 
			Menu::Window.VisualsTab.colmodupdate.SetState(false);
		}
		else
		{
			freakware = false;
		}



	

		qAimAngles.Init(0.0f, GetAutostrafeView().y, 0.0f);
		AngleVectors(qAimAngles, &viewforward, &viewright, &viewup);
		qAimAngles.Init(0.0f, pCmd->viewangles.y, 0.0f);
		AngleVectors(qAimAngles, &aimforward, &aimright, &aimup);
		Vector vForwardNorm;		Normalize(viewforward, vForwardNorm);
		Vector vRightNorm;			Normalize(viewright, vRightNorm);
		Vector vUpNorm;				Normalize(viewup, vUpNorm);

		float forward = pCmd->forwardmove;
		float right = pCmd->sidemove;
		float up = pCmd->upmove;
		if (forward > 450) forward = 450;
		if (right > 450) right = 450;
		if (up > 450) up = 450;
		if (forward < -450) forward = -450;
		if (right < -450) right = -450;
		if (up < -450) up = -450;
		pCmd->forwardmove = DotProduct(forward * vForwardNorm, aimforward) + DotProduct(right * vRightNorm, aimforward) + DotProduct(up * vUpNorm, aimforward);
		pCmd->sidemove = DotProduct(forward * vForwardNorm, aimright) + DotProduct(right * vRightNorm, aimright) + DotProduct(up * vUpNorm, aimright);
		pCmd->upmove = DotProduct(forward * vForwardNorm, aimup) + DotProduct(right * vRightNorm, aimup) + DotProduct(up * vUpNorm, aimup);

		if (Menu::Window.MiscTab.OtherSafeMode.GetState())
		{
			GameUtils::NormaliseViewAngle(pCmd->viewangles);

			if (pCmd->viewangles.z != 0.0f)
			{
				pCmd->viewangles.z = 0.00;
			}

			if (pCmd->viewangles.x < -89 || pCmd->viewangles.x > 89 || pCmd->viewangles.y < -180 || pCmd->viewangles.y > 180)
			{
				Utilities::Log(" Re-calculating angles");
				GameUtils::NormaliseViewAngle(pCmd->viewangles);
				Beep(750, 800);
				if (pCmd->viewangles.x < -89 || pCmd->viewangles.x > 89 || pCmd->viewangles.y < -180 || pCmd->viewangles.y > 180)
				{
					pCmd->viewangles = origView;
					pCmd->sidemove = right;
					pCmd->forwardmove = forward;
				}
			}
		}

		if (pCmd->viewangles.x > 90)
		{
			pCmd->forwardmove = -pCmd->forwardmove;
		}

		if (bSendPacket == true)
		{
			fakeangle = pCmd->viewangles.y;
		}

		if (pCmd->viewangles.x < -90)
		{
			pCmd->forwardmove = -pCmd->forwardmove;
		}
		// LBY
		LBYThirdpersonAngle = Vector(pLocal->GetEyeAnglesXY()->x, pLocal->GetLowerBodyYaw(), pLocal->GetEyeAnglesXY()->z);

		if (bSendPacket == true) {
			LastAngleAA = pCmd->viewangles;
		}
		else if (bSendPacket == false) {
			LastAngleAAReal = pCmd->viewangles;
		}

		lineLBY = pLocal->GetLowerBodyYaw();
		if (bSendPacket == true) {
			lineFakeAngle = pCmd->viewangles.y;
		}
		else if (bSendPacket == false) {
			lineRealAngle = pCmd->viewangles.y;
		}

	}
	return false;
}




void __fastcall PaintTraverse_Hooked(PVOID pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce)
{
	if (Menu::Window.VisualsTab.Active.GetState() && Menu::Window.VisualsTab.OtherNoScope.GetState() && strcmp("HudZoom", Interfaces::Panels->GetName(vguiPanel)) == 0)
		return;
	int w, h;
	int centerW, centerh, topH;
	Interfaces::Engine->GetScreenSize(w, h);
	centerW = w / 2;
	centerh = h / 2;
	static unsigned int FocusOverlayPanel = 0;
	static bool FoundPanel = false;
	int angles = Menu::Window.VisualsTab.angleindicator.GetIndex();
	if (!FoundPanel)
	{

		PCHAR szPanelName = (PCHAR)Interfaces::Panels->GetName(vguiPanel);
		if (strstr(szPanelName, XorStr("MatSystemTopPanel")))
		{

			FocusOverlayPanel = vguiPanel;
			FoundPanel = true;
		}

	}
	else if (FocusOverlayPanel == vguiPanel)
	{
	
		float markr = Menu::Window.colourtab.markr.GetValue();
		float markg = Menu::Window.colourtab.markg.GetValue();
		float markb = Menu::Window.colourtab.markb.GetValue();

		if (Menu::Window.VisualsTab.Watermark.GetState())
		{
			Render::Text((centerW - 19), 15, Color(markr, markg, markb, 255), Render::Fonts::ESP, ("Mirror.tk V4 "));
			Render::Text((centerW - 19), 15, Color(0, 255, 60, 255), Render::Fonts::ESP, ("Mirror.tk V"));
			Render::Text((centerW - 19), 15, Color(255, 255, 255, 255), Render::Fonts::ESP, ("Mirror.tk"));
			Render::Text((centerW - 19), 15, Color(2, 160, 255, 255), Render::Fonts::ESP, ("Mirror"));
			Render::Text((centerW - 19), 15, Color(255, 10, 205, 255), Render::Fonts::ESP, ("Mir"));
		}

		if (angles == 0)
		{

		}
		else if (angles == 1)
		{
			char bufferlineRealAngle[64];
			sprintf_s(bufferlineRealAngle, "Real:  %.1f", lineRealAngle);
			Render::Text(9, (centerh + 40), Color(10, 140, 200, 255), Render::Fonts::ESP, bufferlineRealAngle);

			char bufferlineFakeAngle[64];
			sprintf_s(bufferlineFakeAngle, "Fake:  %.1f", lineFakeAngle);
			Render::Text(9, (centerh + 55), Color(0, 200, 110, 255), Render::Fonts::ESP, bufferlineFakeAngle);

			char bufferLineLBY[64];
			sprintf_s(bufferLineLBY, "LBY:  %.1f", lineLBY);
			Render::Text(9, (centerh + 70), Color(200, 10, 100, 255), Render::Fonts::ESP, bufferLineLBY);

		}

		else if (angles == 2)
		{

			if (lineRealAngle > 0)
			{
				Render::Text((centerW - 24), (centerh - 20), Color(0, 165, 255, 255), Render::Fonts::ESP, ("REAL: Right"));
			}
			else if (lineRealAngle < 0)
			{
				Render::Text((centerW - 24), (centerh - 20), Color(0, 165, 255, 255), Render::Fonts::ESP, ("REAL: Left"));
			}
			else
				Render::Text((centerW - 24), (centerh - 20), Color(0, 165, 255, 255), Render::Fonts::ESP, ("REAL: Front"));


		}
	

		if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
		{
			Hacks::DrawHacks();

		}

		Menu::DoUIFrame();

		if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && Menu::Window.VisualsTab.OtherHitmarker.GetState())
			hitmarker::singleton()->on_paint();
	}
	oPaintTraverse(pPanels, vguiPanel, forceRepaint, allowForce);
}

bool __stdcall Hooked_InPrediction()
{

	bool result;
	static InPrediction_ origFunc = (InPrediction_)Hooks::VMTPrediction.GetOriginalFunction(14);
	static DWORD *ecxVal = Interfaces::Prediction;
	result = origFunc(ecxVal);

	if (Menu::Window.VisualsTab.OtherNoVisualRecoil.GetState() && (DWORD)(_ReturnAddress()) == Offsets::Functions::dwCalcPlayerView)
	{

		IClientEntity* pLocalEntity = NULL;

		float* m_LocalViewAngles = NULL;

		__asm
		{
			MOV pLocalEntity, ESI
			MOV m_LocalViewAngles, EBX
		}

		Vector viewPunch = pLocalEntity->localPlayerExclusive()->GetViewPunchAngle();
		Vector aimPunch = pLocalEntity->localPlayerExclusive()->GetAimPunchAngle();

		m_LocalViewAngles[0] -= (viewPunch[0] + (aimPunch[0] * 2 * 0.4499999f));
		m_LocalViewAngles[1] -= (viewPunch[1] + (aimPunch[1] * 2 * 0.4499999f));
		m_LocalViewAngles[2] -= (viewPunch[2] + (aimPunch[2] * 2 * 0.4499999f));
		return true;
	}
	return result;
}

player_info_t GetInfo(int Index) {
	player_info_t Info;
	Interfaces::Engine->GetPlayerInfo(Index, &Info);
	return Info;
}

typedef void(__cdecl* MsgFn)(const char* msg, va_list);
void Msg(const char* msg, ...)
{

	if (msg == nullptr)
		return; //If no string was passed, or it was null then don't do anything
	static MsgFn fn = (MsgFn)GetProcAddress(GetModuleHandle("tier0.dll"), "Msg"); //This gets the address of export "Msg" in the dll "tier0.dll". The static keyword means it's only called once and then isn't called again (but the variable is still there)
	char buffer[989];
	va_list list; //Normal varargs stuff http://stackoverflow.com/questions/10482960/varargs-to-printf-all-arguments
	va_start(list, msg);

	vsprintf(buffer, msg, list);
	va_end(list);

	fn(buffer, list); //Calls the function, we got the address above.
}

int Kills2 = 0;
int Kills = 0;
bool RoundInfo = false;
size_t Delay = 0;
bool flipAA;
bool __fastcall Hooked_FireEventClientSide(PVOID ECX, PVOID EDX, IGameEvent *Event)
{
	IClientEntity* pEntity;


	if (Menu::Window.RageBotTab.AimbotResolver.GetIndex() != 0)
	{

		if (!strcmp(Event->GetName(), "player_hurt"))
		{

			int deadfag = Event->GetInt("userid");
			int attackingfag = Event->GetInt("attacker");
			IClientEntity* pLocal = hackManager.pLocal();
			if (Interfaces::Engine->GetPlayerForUserID(deadfag) != Interfaces::Engine->GetLocalPlayer() && Interfaces::Engine->GetPlayerForUserID(attackingfag) == Interfaces::Engine->GetLocalPlayer())
			{
				IClientEntity* hittedplayer = (IClientEntity*)(Interfaces::Engine->GetPlayerForUserID(deadfag));
				int hit = Event->GetInt("hitgroup");
				if (hit == 1 && hittedplayer && deadfag && attackingfag)
				{
					Resolver::didhitHS = true;
					Globals::missedshots = 0;
				}

				else
				{
					Resolver::didhitHS = false;
					Globals::missedshots++;
				}
			}
		}

	}

	if (Menu::Window.RageBotTab.FlipAA.GetState())
	{

		if (!strcmp(Event->GetName(), "player_hurt"))
		{

			int deadfag = Event->GetInt("userid");
			int attackingfag = Event->GetInt("attacker");
			IClientEntity* pLocal = hackManager.pLocal();
			if (Interfaces::Engine->GetPlayerForUserID(deadfag) == Interfaces::Engine->GetLocalPlayer() && Interfaces::Engine->GetPlayerForUserID(attackingfag) != Interfaces::Engine->GetLocalPlayer())
			{

				flipAA = true;

			}
			else
			{

				flipAA = false;
			}

		}
	}
	if (Menu::Window.VisualsTab.Logs.GetState())
	{
		if (!strcmp(Event->GetName(), "item_purchase"))
		{

			int nUserID = Event->GetInt("attacker");
			int nDead = Event->GetInt("userid");
			if (nUserID || nDead)
			{
				player_info_t killed_info = GetInfo(Interfaces::Engine->GetPlayerForUserID(nDead));
				player_info_t killer_info = GetInfo(Interfaces::Engine->GetPlayerForUserID(nUserID));
				std::string before = ("  [Mirror.tk] ");
				std::string one = killed_info.name;
				std::string two = ("bought ");
				std::string three = Event->GetString("weapon");
				std::string six = "\n";
				if (Menu::Window.VisualsTab.Logs.GetState())
				{

					Msg((before + one + two + three + six).c_str());

				}

			}
		}
		if (Menu::Window.VisualsTab.Logs.GetState())
		{
			if (!strcmp(Event->GetName(), "player_hurt"))
			{

				int attackerid = Event->GetInt("attacker");
				int entityid = Interfaces::Engine->GetPlayerForUserID(attackerid);


				if (entityid == Interfaces::Engine->GetLocalPlayer())
				{

					int nUserID = Event->GetInt("attacker");
					int nDead = Event->GetInt("userid");
					if (nUserID || nDead)
					{

						player_info_t killed_info = GetInfo(Interfaces::Engine->GetPlayerForUserID(nDead));
						player_info_t killer_info = GetInfo(Interfaces::Engine->GetPlayerForUserID(nUserID));
						std::string before = ("  [Mirror.tk] ");
						std::string two = ("Hit ");
						std::string three = killed_info.name;
						std::string foura = " for ";
						std::string fivea = Event->GetString("dmg_health");
						std::string damage = " damage";
						std::string fourb = " (";
						std::string fiveb = Event->GetString("health");
						std::string six = " health remaining)";
						std::string newline = "\n";

						std::string meme = "Missed Shot Due to Bad Resolver :(";
						std::string isbrute1 = "Estimating Target Angle";
						std::string isbrute2 = "Angry Brute Forcing >:[";





						if (Menu::Window.VisualsTab.Logs.GetState())
						{
							if (Globals::missedshots <= 5)
								Msg((before + two + three + foura + fivea + damage + fourb + fiveb + six + newline).c_str());
							else if (Globals::missedshots = Menu::Window.RageBotTab.forceback.GetValue())
								Msg((before + isbrute1 + newline).c_str());
							else if (Globals::missedshots = Menu::Window.RageBotTab.forceback.GetValue() + 1 && Menu::Window.RageBotTab.forceback.GetValue() > 3)
								Msg((before + isbrute2 + newline).c_str());
							else if (Globals::missedshots >= 5)
								Msg((before + meme + newline).c_str());

						}

					}
				}

			}

		}


	}
	return oFireEventClientSide(ECX, Event);
}


void __fastcall Hooked_DrawModelExecute(void* thisptr, int edx, void* ctx, void* state, const ModelRenderInfo_t &pInfo, matrix3x4 *pCustomBoneToWorld)
{
	if (!Interfaces::Engine->IsConnected() || !Interfaces::Engine->IsInGame())
		return;

	Color color;
	float flColor[3] = { 0.f };
	static IMaterial* CoveredLit = CreateMaterial(true);
	static IMaterial* OpenLit = CreateMaterial(false);
	static IMaterial* CoveredFlat = CreateMaterial(true, false);
	static IMaterial* OpenFlat = CreateMaterial(false, false);
	bool DontDraw = false;
	const char* ModelName = Interfaces::ModelInfo->GetModelName((model_t*)pInfo.pModel);
	IClientEntity* pModelEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(pInfo.entity_index);
	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	Vector cross1 = pLocal->GetHeadAngle(), screen8;
	int ChamsStyle = Menu::Window.VisualsTab.OptionsChams.GetIndex();
	IMaterial *covered = ChamsStyle == 1 ? CoveredLit : CoveredFlat;
	IClientEntity *local = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (Menu::Window.VisualsTab.Active.GetState())
	{
		if (pLocal->IsScoped())
		{
			if (Menu::Window.VisualsTab.pLocalOpacity.GetState() && strstr(ModelName, "models/player"))
			{
				Interfaces::RenderView->SetBlend(0.05f);
			}
			else if (strstr(ModelName, "models/player"))
			{
				if (pLocal->IsPlayer())
				{
					flColor[0] = 10 / 255.f;
					flColor[1] = 10 / 255.f;
					flColor[2] = 10 / 255.f;
					Interfaces::RenderView->SetColorModulation(flColor);
					Interfaces::RenderView->SetBlend(0.45);
					Interfaces::ModelRender->ForcedMaterialOverride(covered);
					oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
				}
				else
				{
					flColor[0] = 10 / 255.f;
					flColor[1] = 200 / 255.f;
					flColor[2] = 60 / 255.f;
					Interfaces::RenderView->SetColorModulation(flColor);
					Interfaces::RenderView->SetBlend(0.30);
					Interfaces::ModelRender->ForcedMaterialOverride(covered);
					oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
				
				}
	
			}


			else if (pModelEntity)
			{
				if (pModelEntity == nullptr)
					return;


				IClientEntity *local = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
				if (local)
				{


					
					float alpha = 1.f;

					if (pModelEntity->HasGunGameImmunity())
						alpha = 0.5f;

					if (pModelEntity->GetTeamNum() == 2)
					{
						flColor[0] = 70 / 255.f;
						flColor[1] = 5 / 255.f;
						flColor[2] = 170 / 255.f;
						Interfaces::RenderView->SetColorModulation(flColor);
						Interfaces::RenderView->SetBlend(0.60);
						Interfaces::ModelRender->ForcedMaterialOverride(covered);
						oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
					}
					
				}

			}
		}

	}
	else
	{
		if (Menu::Window.VisualsTab.pLocalOpacity.GetState() && pLocal->IsAlive() && strstr(ModelName, "models/player"))
		{
			Interfaces::RenderView->SetBlend(0.2f);
		}

	}


	int HandsStyle = Menu::Window.VisualsTab.OtherNoHands.GetIndex();

	//-----------------------visible-chams-----------------------//

	float vtr = Menu::Window.colourtab.vchamtr.GetValue();
	float vtg = Menu::Window.colourtab.vchamtg.GetValue();
	float vtb = Menu::Window.colourtab.vchamtb.GetValue();

	float vctr = Menu::Window.colourtab.vchamctr.GetValue();
	float vctg = Menu::Window.colourtab.vchamctg.GetValue();
	float vctb = Menu::Window.colourtab.vchamctb.GetValue();

	//-----------------------invisible-chams-----------------------//

	float tr = Menu::Window.colourtab.chamtr.GetValue();
	float tg = Menu::Window.colourtab.chamtg.GetValue();
	float tb = Menu::Window.colourtab.chamtb.GetValue();

	float ctr = Menu::Window.colourtab.chamctr.GetValue();
	float ctg = Menu::Window.colourtab.chamctg.GetValue();
	float ctb = Menu::Window.colourtab.chamctb.GetValue();
	//-----------------------invisible-chams-----------------------//


	if (ChamsStyle != 0 && Menu::Window.VisualsTab.FiltersPlayers.GetState() && strstr(ModelName, "models/player"))
	{
		if (pModelEntity == nullptr)
			return;

		if (pLocal && (!Menu::Window.VisualsTab.FiltersEnemiesOnly.GetState() ||
			pModelEntity->GetTeamNum() != pLocal->GetTeamNum()))
		{
			IMaterial *covered = ChamsStyle == 1 ? CoveredLit : CoveredFlat;
			IMaterial *open = ChamsStyle == 1 ? OpenLit : OpenFlat;

			IClientEntity* pModelEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(pInfo.entity_index);
			if (pModelEntity)
			{
				if (pModelEntity == nullptr)
					return;


				IClientEntity *local = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
				if (local)
				{

					if (local == nullptr)
						return;

					if (pModelEntity->IsAlive() && pModelEntity->GetTeamNum() != local->GetTeamNum())
					{
						float alpha = 1.f;

						if (pModelEntity->HasGunGameImmunity())
							alpha = 0.5f;

						if (pModelEntity->GetTeamNum() == 2)
						{

							flColor[0] = vtr / 255.f;
							flColor[1] = vtg / 255.f;
							flColor[2] = vtb / 255.f;
						}
						else
						{

							flColor[0] = vctr / 255.f;
							flColor[1] = vctg / 255.f;
							flColor[2] = vctb / 255.f;

						}

						if (Menu::Window.VisualsTab.ChamsVisibleOnly.GetState())
						{
							Interfaces::RenderView->SetColorModulation(flColor);
							Interfaces::RenderView->SetBlend(1);
					//		Interfaces::ModelRender->ForcedMaterialOverride(covered);
							oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
						}
						else
						{

							Interfaces::RenderView->SetColorModulation(flColor);
							Interfaces::RenderView->SetBlend(1);

							Interfaces::ModelRender->ForcedMaterialOverride(covered);
							oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
						}

						if (pModelEntity->GetTeamNum() == 2)
						{

							flColor[0] = tr / 255.f;
							flColor[1] = tg / 255.f;
							flColor[2] = tb / 255.f;
						}
						else
						{
							flColor[0] = ctr / 255.f;
							flColor[1] = ctg / 255.f;
							flColor[2] = ctb / 255.f;
						}

						Interfaces::RenderView->SetColorModulation(flColor);

						Interfaces::ModelRender->ForcedMaterialOverride(open);
					}
					else
					{
						Interfaces::RenderView->SetColorModulation(flColor);
						static IMaterial* wire = CreateMaterial(true, false, true);
						IMaterial *modl = wire;
						Interfaces::RenderView->SetBlend(0.4);
						color.SetColor(40, 30, 30, 100);
						ForceMaterial(color, open);
					
					}
				}
			}



		}

	}

	else if (HandsStyle != 0 && strstr(ModelName, XorStr("arms")))
	{
		IMaterial *covered = ChamsStyle == 1 ? CoveredLit : CoveredFlat;
		IMaterial *open = ChamsStyle == 1 ? OpenLit : OpenFlat;
		if (HandsStyle == 1)
		{

			DontDraw = true;
		}
		else if (HandsStyle == 2)
		{
			flColor[0] = 20.f / 255.f;
			flColor[1] = 20.f / 255.f;
			flColor[2] = 20.f / 255.f;
			color.SetColor(20, 20, 20, 100);
			Interfaces::RenderView->SetBlend(0.5);
			Interfaces::ModelRender->ForcedMaterialOverride(OpenLit);

		}

		else if (HandsStyle == 4)
		{


			if (pLocal)
			{

			
				int alpha = pLocal->HasGunGameImmunity() ? 150 : 255;

				if (pLocal->GetTeamNum() == 2)
					color.SetColor(20, 130, 235, alpha);
				else
					color.SetColor(10, 180, 245, alpha);

				ForceMaterial(color, covered);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);

				if (pLocal->GetTeamNum() == 2)
					color.SetColor(247, 40, 60, alpha);
				else
					color.SetColor(32, 100, 254, alpha);
			}
			else
			{

				color.SetColor(255, 255, 255, 255);
			}

			ForceMaterial(color, open);
			

		}
		else
		{
			static int counter = 0;
			static float colors[3] = { 1.f, 0.f, 0.f };

			if (colors[counter] >= 1.0f)
			{
				colors[counter] = 1.0f;
				counter += 1;
				if (counter > 2)
					counter = 0;
			}
			else
			{

				int prev = counter - 1;
				if (prev < 0) prev = 2;
				colors[prev] -= 0.05f;
				colors[counter] += 0.05f;
			}

			Interfaces::RenderView->SetColorModulation(colors);
			Interfaces::RenderView->SetBlend(0.5);
			Interfaces::ModelRender->ForcedMaterialOverride(OpenLit);
		}
	}




	if (!DontDraw)
		oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
	Interfaces::ModelRender->ForcedMaterialOverride(NULL);
}

int RandomInt(int min, int max)
{
	return rand() % max + min;

}

std::vector<const char*> smoke_materials = {
	"particle/vistasmokev1/vistasmokev1_smokegrenade",
	"particle/vistasmokev1/vistasmokev1_emods",
	"particle/vistasmokev1/vistasmokev1_emods_impactdust",
	"particle/vistasmokev1/vistasmokev1_fire",

};


float Bolbilize(float Yaw)
{
	if (Yaw > 180)
	{
		Yaw -= (round(Yaw / 360) * 360.f);
	}
	else if (Yaw < -180)
	{
		Yaw += (round(Yaw / 360) * -360.f);
	}
	return Yaw;
}

Vector CalcAngle69(Vector dst, Vector src)
{
	Vector angles;

	double delta[3] = { (src.x - dst.x), (src.y - dst.y), (src.z - dst.z) };
	double hyp = sqrt(delta[1] * delta[1] + delta[2] * delta[2]);
	angles.x = (float)(atan(delta[2] / hyp) * 180.0 / 3.14159265);
	angles.y = (float)(atanf(delta[1] / delta[0]) * 57.295779513082f);
	angles.z = 0.0f;

	if (delta[0] >= 1.9)
	{
		angles.y += 180.0f;
	}

	return angles;
}
//Vector LastAngleAA;




namespace resolvokek
{
	int Shots;
	int missedshots;
	float RealAngle;
	float FakeAngle;
	Vector AimPoint;
	bool shouldflip;
	bool ySwitch;
	float NextTime;
	int resom = 1;
	float fakeAngle;
	float OldSimulationTime[65];
	bool error;
}

#define SETANGLE 180
void  __stdcall Hooked_FrameStageNotify(ClientFrameStage_t curStage)
{

	DWORD eyeangles = NetVar.GetNetVar(0xBFEA4E7B);
	IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	IClientEntity* pEntity = nullptr;

	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && curStage == FRAME_RENDER_START)
	{
		static bool rekt1 = false;
		if (Menu::Window.MiscTab.OtherThirdperson.GetState() && pLocal->IsAlive() && pLocal->IsScoped() == 0)
		{
			if (!rekt1)
			{
				Interfaces::Engine->ClientCmd_Unrestricted("thirdperson");
				rekt1 = true;

			}
		}
		else if (!Menu::Window.MiscTab.OtherThirdperson.GetState())
		{
			rekt1 = false;

		}

		static bool rekt = false;
		if (!Menu::Window.MiscTab.OtherThirdperson.GetState() || pLocal->IsAlive() == 0)
		{
			if (!rekt)
			{
				Interfaces::Engine->ClientCmd_Unrestricted("firstperson");
				rekt = true;

			}
		}
		else if (Menu::Window.MiscTab.OtherThirdperson.GetState() || pLocal->IsAlive() == 0)
		{
			rekt = false;
		}

		static bool meme = false;
		if (Menu::Window.MiscTab.OtherThirdperson.GetState() && pLocal->IsScoped() == 0)
		{
			if (!meme)
			{
				Interfaces::Engine->ClientCmd_Unrestricted("thirdperson");
				meme = true;
			}
		}
		else if (pLocal->IsScoped())
		{

			meme = false;
		}

		static bool kek = false;
		if (Menu::Window.MiscTab.OtherThirdperson.GetState() && pLocal->IsAlive())
		{
			if (!kek)
			{
				Interfaces::Engine->ClientCmd_Unrestricted("thirdperson");
				kek = true;
			}
		}
		else if (pLocal->IsAlive() == 0)
		{
			kek = false;
		}	if (pLocal->IsAlive() && Menu::Window.MiscTab.OtherThirdperson.GetState())
		{
			Vector thirdpersonMode;

			switch (Menu::Window.MiscTab.ThirdpersonAngle.GetIndex())
			{
			case 0:

				thirdpersonMode = LastAngleAAReal;
				break;
			case 1:
				thirdpersonMode = LastAngleAA;
				break;
			case 2:
				thirdpersonMode = LBYThirdpersonAngle;
				break;
			}


			static bool rekt = false;
			if (!rekt)
			{
				ConVar* sv_cheats = Interfaces::CVar->FindVar("sv_cheats");
				SpoofedConvar* sv_cheats_spoofed = new SpoofedConvar(sv_cheats);
				sv_cheats_spoofed->SetInt(1);
				rekt = true;
			}


			static bool kek = false;

			if (!kek)
			{
				Interfaces::Engine->ClientCmd_Unrestricted("thirdperson");
				kek = true;
			}

			static bool toggleThirdperson;
			static float memeTime;
			int ThirdPersonKey = Menu::Window.MiscTab.ThirdPersonKeyBind.GetKey();
			if (ThirdPersonKey >= 0 && GUI.GetKeyState(ThirdPersonKey) && abs(memeTime - Interfaces::Globals->curtime) > 0.5)
			{
				toggleThirdperson = !toggleThirdperson;
				memeTime = Interfaces::Globals->curtime;
			}


			if (toggleThirdperson)
			{
				Interfaces::pInput->m_fCameraInThirdPerson = true;
				if (*(bool*)((DWORD)Interfaces::pInput + 0xA5))
					*(Vector*)((DWORD)pLocal + 0x31C8) = thirdpersonMode;
			}
			else {
				// No Thirdperson
				static Vector vecAngles;
				Interfaces::Engine->GetViewAngles(vecAngles);
				Interfaces::pInput->m_fCameraInThirdPerson = false;
				Interfaces::pInput->m_vecCameraOffset = Vector(vecAngles.x, vecAngles.y, 0);
			}


		}
		else if (pLocal->IsAlive() == 0)
		{

			kek = false;

		}

		if (!Menu::Window.MiscTab.OtherThirdperson.GetState()) {

			// No Thirdperson
			static Vector vecAngles;
			Interfaces::Engine->GetViewAngles(vecAngles);
			Interfaces::pInput->m_fCameraInThirdPerson = false;
			Interfaces::pInput->m_vecCameraOffset = Vector(vecAngles.x, vecAngles.y, 0);
		}

		if (curStage == FRAME_RENDER_START)
		{
			for (int i = 1; i <= Interfaces::Globals->maxClients; i++)
			{
				if (i == Interfaces::Engine->GetLocalPlayer()) continue;

				IClientEntity* pCurEntity = Interfaces::EntList->GetClientEntity(i);
				if (!pCurEntity) continue;

				*(int*)((uintptr_t)pCurEntity + 0xA30) = Interfaces::Globals->framecount;
				*(int*)((uintptr_t)pCurEntity + 0xA28) = 0;
			}
			for (auto matName : smoke_materials)
			{
				IMaterial* mat = Interfaces::MaterialSystem->FindMaterial(matName, "Other textures");
				mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, Menu::Window.VisualsTab.OtherNoSmoke.GetState());
				mat->ColorModulation(20, 20, 20);
				mat->AlphaModulate(10);
			}
		}

	}


	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_START) {
		{



		}


		/*

		static bool abc = false;

		static bool freakware = false;
		if (Menu::Window.VisualsTab.colmodupdate.GetState())
		{
		if (!freakware)
		{
		ConVar* staticdrop = Interfaces::CVar->FindVar("r_DrawSpecificStaticProp");
		SpoofedConvar* staticdrop_spoofed = new SpoofedConvar(staticdrop);
		staticdrop_spoofed->SetInt(0);

		{
		for (MaterialHandle_t i = Interfaces::MaterialSystem->FirstMaterial(); i != Interfaces::MaterialSystem->InvalidMaterial(); i = Interfaces::MaterialSystem->NextMaterial(i))
		{
		IMaterial *pMaterial = Interfaces::MaterialSystem->GetMaterial(i);

		if (!pMaterial)
		continue;

		ConVar* sky = Interfaces::CVar->FindVar("sv_skyname");
		SpoofedConvar* sky_spoofed = new SpoofedConvar(sky);
		sky_spoofed->SetString("sky_csgo_night02");

		float amountr = Menu::Window.VisualsTab.colmodr.GetValue();

		if (!strcmp(pMaterial->GetTextureGroupName(), "World textures"))  // walls
		{
		pMaterial->ColorModulation(amountr, amountr, amountr);
		}
		if (!strcmp(pMaterial->GetTextureGroupName(), "StaticProp textures"))
		{
		pMaterial->ColorModulation(amountr, amountr, amountr);   // values of boxes n barrels n shit
		}
		}
		}
		}
		freakware = true; // Update for anti fps suicide (?)
		Menu::Window.VisualsTab.colmodupdate.SetState(false);
		}
		else
		{
		freakware = false;
		}
		*/

		if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
		{

			IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
			ResoSetup::GetInst().FSN(pEntity, curStage);

			//	int Skin = Menu::Window.MiscTab.KnifeSkin.GetIndex();

			int iBayonet = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_bayonet.mdl");
			int iButterfly = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_butterfly.mdl");
			int iFlip = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_flip.mdl");
			int iGut = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_gut.mdl");
			int iKarambit = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_karam.mdl");
			int iM9Bayonet = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_m9_bay.mdl");
			int iHuntsman = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_tactical.mdl");
			int iFalchion = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_falchion_advanced.mdl");
			int iDagger = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_push.mdl");
			int iBowie = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_survival_bowie.mdl");
			int iGunGame = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_gg.mdl");

			for (int i = 0; i <= Interfaces::EntList->GetHighestEntityIndex(); i++)
			{
				IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);

				if (pEntity)
				{
					if (pEntity == nullptr)
						return;

					ULONG hOwnerEntity = *(PULONG)((DWORD)pEntity + 0x148);

					IClientEntity* pOwner = Interfaces::EntList->GetClientEntityFromHandle((HANDLE)hOwnerEntity);

					if (pOwner)
					{

						if (pOwner == nullptr)
							return;

						if (pOwner == pLocal)
						{
							CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)pEntity;

							ClientClass *pClass = Interfaces::Client->GetAllClasses();

							if (Menu::Window.MiscTab.SkinEnable.GetState())
							{
								int Model = Menu::Window.MiscTab.KnifeModel.GetIndex();
								if (pEntity->GetClientClass()->m_ClassID == (int)CSGOClassID::CKnife)
								{
									auto pCustomName1 = MakePtr(char*, pWeapon, 0x301C);


									if (Model == 0) // Bayonet
									{
										*pWeapon->ModelIndex() = iBayonet;
										*pWeapon->ViewModelIndex() = iBayonet;
										*pWeapon->WorldModelIndex() = iBayonet + 1;
										*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 500;
										*pWeapon->GetEntityQuality() = 3;

									}
									else if (Model == 1) // Bowie Knife
									{
										*pWeapon->ModelIndex() = iBowie;
										*pWeapon->ViewModelIndex() = iBowie;
										*pWeapon->WorldModelIndex() = iBowie + 1;
										*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 514;
										*pWeapon->GetEntityQuality() = 3;

									}
									else if (Model == 2) // Butterfly Knife
									{
										*pWeapon->ModelIndex() = iButterfly;
										*pWeapon->ViewModelIndex() = iButterfly;
										*pWeapon->WorldModelIndex() = iButterfly + 1;
										*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 515;
										*pWeapon->GetEntityQuality() = 3;

									}
									else if (Model == 3) // Falchion Knife
									{
										*pWeapon->ModelIndex() = iFalchion;
										*pWeapon->ViewModelIndex() = iFalchion;
										*pWeapon->WorldModelIndex() = iFalchion + 1;
										*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 512;
										*pWeapon->GetEntityQuality() = 3;

									}
									else if (Model == 4) // Flip Knife
									{
										*pWeapon->ModelIndex() = iFlip;
										*pWeapon->ViewModelIndex() = iFlip;
										*pWeapon->WorldModelIndex() = iFlip + 1;
										*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 505;

									}
									else if (Model == 5) // Gut Knife
									{
										*pWeapon->ModelIndex() = iGut;
										*pWeapon->ViewModelIndex() = iGut;
										*pWeapon->WorldModelIndex() = iGut + 1;
										*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 506;
										*pWeapon->GetEntityQuality() = 3;

									}
									else if (Model == 6) // Huntsman Knife
									{
										*pWeapon->ModelIndex() = iHuntsman;
										*pWeapon->ViewModelIndex() = iHuntsman;
										*pWeapon->WorldModelIndex() = iHuntsman + 1;
										*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 509;
										*pWeapon->GetEntityQuality() = 3;

									}
									else if (Model == 7) // Karambit
									{
										*pWeapon->ModelIndex() = iKarambit;
										*pWeapon->ViewModelIndex() = iKarambit;
										*pWeapon->WorldModelIndex() = iKarambit + 1;
										*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 507;
										*pWeapon->GetEntityQuality() = 3;

									}
									else if (Model == 8) // M9 Bayonet
									{
										*pWeapon->ModelIndex() = iM9Bayonet;
										*pWeapon->ViewModelIndex() = iM9Bayonet;
										*pWeapon->WorldModelIndex() = iM9Bayonet + 1;
										*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 508;
										*pWeapon->GetEntityQuality() = 3;

									}
									else if (Model == 10) // Shadow Daggers
									{
										*pWeapon->ModelIndex() = iDagger;
										*pWeapon->ViewModelIndex() = iDagger;
										*pWeapon->WorldModelIndex() = iDagger + 1;
										*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 516;
										*pWeapon->GetEntityQuality() = 3;

									}
								}

								*pWeapon->OwnerXuidLow() = 0;//crash?
								*pWeapon->OwnerXuidHigh() = 0;
								*pWeapon->FallbackWear() = 0.001f;
								*pWeapon->m_AttributeManager()->m_Item()->ItemIDHigh() = 1;

							}
						}
					}

				}/*
				 for (int i = 1; i <= Interfaces::Engine->GetMaxClients(); ++i)
				 {
				 IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);

				 if (!pEntity || !pLocal) {
				 continue;
				 }

				 if (pEntity->GetTeamNum() == pLocal->GetTeamNum()) {
				 continue;
				 }

				 if (pEntity->IsDormant() || !pLocal->IsAlive() || !pEntity->IsAlive()) {
				 continue;
				 }
				 //lagComp->log(curStage);
				 }*/

			}
		}
	}

	oFrameStageNotify(curStage);
}


void __fastcall Hooked_OverrideView(void* ecx, void* edx, CViewSetup* pSetup)
{

	IClientEntity* localplayer1 = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (!localplayer1)
		return;

	if (!Interfaces::Engine->IsConnected())
		return;
	if (!Interfaces::Engine->IsInGame())
		return;

	if (localplayer1)
	{
		if (!localplayer1->IsScoped() && Menu::Window.VisualsTab.OtherFOV.GetValue() > 0)
			pSetup->fov += Menu::Window.VisualsTab.OtherFOV.GetValue();
	}
	currentfov = pSetup->fov;
	oOverrideView(ecx, edx, pSetup);
}

void GetViewModelFOV(float& fov)
{
	IClientEntity* localplayer = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
	{

		if (!localplayer)
			return;


		if (Menu::Window.VisualsTab.Active.GetState())
			fov += Menu::Window.VisualsTab.OtherViewmodelFOV.GetValue();
	}
}

float __stdcall GGetViewModelFOV()
{
	float fov = Hooks::VMTClientMode.GetMethod<oGetViewModelFOV>(35)();


	return fov;
}

void __fastcall  hkSceneEnd(void *pEcx, void *pEdx)
{
	float espA = Menu::Window.colourtab.alpEsp.GetValue();

	IClientEntity* pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());


	if (Menu::Window.VisualsTab.Active.GetState())
	{
		if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
		{
		if (Menu::Window.VisualsTab.ghostcham.GetIndex() == 1)
			{


				if (pLocal)
				{
					static  IMaterial* CoveredLit = CreateMaterial(true);


					if (CoveredLit)
					{
						Vector OrigAng;
						OrigAng = pLocal->GetEyeAngles();
						pLocal->SetAngle2(Vector(0, fakeangle, 0));
						float	NormalColor[3] = { 0.9, 0.4 , 0.5 };

						bool LbyColor = false;

						float lbyUpdateColor[3] = { 0, 1, 0 };
						Interfaces::RenderView->SetColorModulation(LbyColor ? lbyUpdateColor : NormalColor);
						Interfaces::ModelRender->ForcedMaterialOverride(CoveredLit);
						pLocal->draw_model(STUDIO_RENDER, 255);
						Interfaces::RenderView->SetBlend(0.5f);
						Interfaces::ModelRender->ForcedMaterialOverride(nullptr);
						pLocal->SetAngle2(OrigAng);

					}
				}
			}
			else if (Menu::Window.VisualsTab.ghostcham.GetIndex() == 2)
			{

				if (pLocal)
				{
					static  IMaterial* CoveredLit = CreateMaterial(true);
					if (CoveredLit)
					{
						Vector OrigAng;
						OrigAng = pLocal->GetEyeAngles();
						pLocal->SetAngle2(Vector(0, lineLBY, 0));
						bool LbyColor = false;
						float NormalColor[3] = { 0.05, 0.35, 0.8 };
						float lbyUpdateColor[3] = { 0, 1, 0 };
						Interfaces::RenderView->SetColorModulation(LbyColor ? lbyUpdateColor : NormalColor);
						Interfaces::ModelRender->ForcedMaterialOverride(CoveredLit);
						pLocal->draw_model(STUDIO_RENDER, 255);
						Interfaces::RenderView->SetBlend(0.3f);
						Interfaces::ModelRender->ForcedMaterialOverride(nullptr);
						pLocal->SetAngle2(OrigAng);
						pSceneEnd(pEcx);
					}
				}
			}

		}

	}
	pSceneEnd(pEcx);
}

void __fastcall Hooked_RenderView(void* ecx, void* edx, CViewSetup &setup, CViewSetup &hudViewSetup, int nClearFlags, int whatToDraw)
{
	static DWORD oRenderView = Hooks::VMTRenderView.GetOriginalFunction(6);

	IClientEntity* pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	__asm
	{
		PUSH whatToDraw
		PUSH nClearFlags
		PUSH hudViewSetup
		PUSH setup
		MOV ECX, ecx
		CALL oRenderView
	}
}