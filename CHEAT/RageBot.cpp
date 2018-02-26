#include "RageBot.h"
#include "RenderManager.h"
#include "Resolver.h"
#include "Autowall.h"
#include <iostream>
#include "UTIL Functions.h"
#include "fakelaghelper.h"
#include "interfaces.h"
#include "quickmaths.h" // 2+2 = selfharm
#include "movedata.h"
#include "nospreadhelper.h"

void CRageBot::Init()
{

	IsAimStepping = false;
	IsLocked = false;
	TargetID = -1;
}

#define BEGIN_NAMESPACE( x ) namespace x {
#define END_NAMESPACE }

BEGIN_NAMESPACE(XorCompileTime)
constexpr auto time = __TIME__;
constexpr auto seed = static_cast<int>(time[7]) + static_cast<int>(time[6]) * 10 + static_cast<int>(time[4]) * 60 + static_cast<int>(time[3]) * 600 + static_cast<int>(time[1]) * 3600 + static_cast<int>(time[0]) * 36000;

// 1988, Stephen Park and Keith Miller
// "Random Number Generators: Good Ones Are Hard To Find", considered as "minimal standard"
// Park-Miller 31 bit pseudo-random number generator, implemented with G. Carta's optimisation:
// with 32-bit math and without division

template <int N>
struct RandomGenerator
{
private:
	static constexpr unsigned a = 16807; // 7^5
	static constexpr unsigned m = 2147483647; // 2^31 - 1

	static constexpr unsigned s = RandomGenerator<N - 1>::value;
	static constexpr unsigned lo = a * (s & 0xFFFF); // Multiply lower 16 bits by 16807
	static constexpr unsigned hi = a * (s >> 16); // Multiply higher 16 bits by 16807
	static constexpr unsigned lo2 = lo + ((hi & 0x7FFF) << 16); // Combine lower 15 bits of hi with lo's upper bits
	static constexpr unsigned hi2 = hi >> 15; // Discard lower 15 bits of hi
	static constexpr unsigned lo3 = lo2 + hi;

public:
	static constexpr unsigned max = m;
	static constexpr unsigned value = lo3 > m ? lo3 - m : lo3;
};

template <>
struct RandomGenerator<0>
{
	static constexpr unsigned value = seed;
};

template <int N, int M>
struct RandomInt
{
	static constexpr auto value = RandomGenerator<N + 1>::value % M;
};

template <int N>
struct RandomChar
{
	static const char value = static_cast<char>(1 + RandomInt<N, 0x7F - 1>::value);
};

template <size_t N, int K>
struct XorString
{
private:
	const char _key;
	std::array<char, N + 1> _encrypted;

	constexpr char enc(char c) const
	{
		return c ^ _key;
	}

	char dec(char c) const
	{
		return c ^ _key;
	}

public:
	template <size_t... Is>
	constexpr __forceinline XorString(const char* str, std::index_sequence<Is...>) : _key(RandomChar<K>::value), _encrypted{ enc(str[Is])... }
	{
	}

	__forceinline decltype(auto) decrypt(void)
	{
		for (size_t i = 0; i < N; ++i)
		{
			_encrypted[i] = dec(_encrypted[i]);
		}
		_encrypted[N] = '\0';
		return _encrypted.data();
	}
};

//--------------------------------------------------------------------------------
//-- Note: XorStr will __NOT__ work directly with functions like printf.
//         To work with them you need a wrapper function that takes a const char*
//         as parameter and passes it to printf and alike.
//
//         The Microsoft Compiler/Linker is not working correctly with variadic 
//         templates!
//  
//         Use the functions below or use std::cout (and similar)!
//--------------------------------------------------------------------------------

static auto w_printf = [](const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf_s(fmt, args);
	va_end(args);
};

static auto w_printf_s = [](const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf_s(fmt, args);
	va_end(args);
};

static auto w_sprintf = [](char* buf, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int len = _vscprintf(fmt, args) // _vscprintf doesn't count  
		+ 1; // terminating '\0' 
	vsprintf_s(buf, len, fmt, args);
	va_end(args);
};

static auto w_sprintf_s = [](char* buf, size_t buf_size, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsprintf_s(buf, buf_size, fmt, args);
	va_end(args);
};
static bool w_strcmp(const char* str1, const char* str2)
{
	return strcmp(str1, str2);
};
//#define XorStr( s ) ( XorCompileTime::XorString< sizeof( s ) - 1, __COUNTER__ >( s, std::make_index_sequence< sizeof( s ) - 1>() ).decrypt() )
/*#ifdef NDEBUG //nDEBUG
#define XorStr( s ) ( XorCompileTime::XorString< sizeof( s ) - 1, __COUNTER__ >( s, std::make_index_sequence< sizeof( s ) - 1>() ).decrypt() )
#elif*/
#define XorStr( s ) ( s )
//#endif

END_NAMESPACE

void CRageBot::Prediction(CUserCmd* pCmd, IClientEntity* LocalPlayer)
{
	if (Interfaces::MoveHelper && Menu::Window.RageBotTab.AimbotEnable.GetState() && Menu::Window.RageBotTab.posadjust.GetState() )
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
	}
}



float InterpFix()
{

		static ConVar* cvar_cl_interp = Interfaces::CVar->FindVar("cl_interp");
		static ConVar* cvar_cl_updaterate = Interfaces::CVar->FindVar("cl_updaterate");
		static ConVar* cvar_sv_maxupdaterate = Interfaces::CVar->FindVar("sv_maxupdaterate");
		static ConVar* cvar_sv_minupdaterate = Interfaces::CVar->FindVar("sv_minupdaterate");
		static ConVar* cvar_cl_interp_ratio = Interfaces::CVar->FindVar("cl_interp_ratio");

		IClientEntity* pLocal = hackManager.pLocal();
		CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

		float cl_interp = cvar_cl_interp->GetFloat();
		int cl_updaterate = cvar_cl_updaterate->GetInt();
		int sv_maxupdaterate = cvar_sv_maxupdaterate->GetInt();
		int sv_minupdaterate = cvar_sv_minupdaterate->GetInt();
		int cl_interp_ratio = cvar_cl_interp_ratio->GetInt();

		if (sv_maxupdaterate <= cl_updaterate)
			cl_updaterate = sv_maxupdaterate;

		if (sv_minupdaterate > cl_updaterate)
			cl_updaterate = sv_minupdaterate;

		float new_interp = (float)cl_interp_ratio / (float)cl_updaterate;

		if (new_interp > cl_interp)
			cl_interp = new_interp;

		return max(cl_interp, cl_interp_ratio / cl_updaterate);
	
	// 
}

void FakeWalk(CUserCmd * pCmd, bool & bSendPacket)
{

	IClientEntity* pLocal = hackManager.pLocal();
	if (GetAsyncKeyState(VK_SHIFT))
	{

		static int iChoked = -1;
		iChoked++;

		if (iChoked < 1)
		{
			bSendPacket = false;



			pCmd->tick_count += 7.95; // 10.95
			pCmd->command_number += 3.07 + pCmd->tick_count % 2 ? 0 : 1; // 5
			pCmd->buttons |= pLocal->GetMoveType() == IN_FORWARD;
			pCmd->buttons |= pLocal->GetMoveType() == IN_LEFT;
			pCmd->buttons |= pLocal->GetMoveType() == IN_BACK;
			pCmd->forwardmove = pCmd->sidemove = 0.f;
		}
		else
		{
			bSendPacket = true;
			iChoked = -1;

			Interfaces::Globals->frametime *= (pLocal->GetVelocity().Length2D()) / 6; // 10
			pCmd->buttons |= pLocal->GetMoveType() == IN_FORWARD;
		}
	}
}

void CRageBot::Draw()
{

}

bool IsAbleToShoot(IClientEntity* pLocal)
{
	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());
	if (!pLocal)return false;
	if (!pWeapon)return false;
	float flServerTime = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;
	return (!(pWeapon->GetNextPrimaryAttack() > flServerTime));
}

float hitchance(IClientEntity* pLocal, CBaseCombatWeapon* pWeapon)
{
	float hitchance = 101;
	if (!pWeapon) return 0;
	if (Menu::Window.RageBotTab.AccuracyHitchance.GetValue() > 1)
	{
		float inaccuracy = pWeapon->GetInaccuracy();
		if (inaccuracy == 0) inaccuracy = 0.0000000000001;
		inaccuracy = 1 / inaccuracy;
		hitchance = inaccuracy;
	}
	return hitchance;
}

bool CanOpenFire() 
{
	IClientEntity* pLocalEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (!pLocalEntity)
		return false;

	CBaseCombatWeapon* entwep = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocalEntity->GetActiveWeaponHandle());

	float flServerTime = (float)pLocalEntity->GetTickBase() * Interfaces::Globals->interval_per_tick;
	float flNextPrimaryAttack = entwep->GetNextPrimaryAttack();

	std::cout << flServerTime << " " << flNextPrimaryAttack << std::endl;

	return !(flNextPrimaryAttack > flServerTime);
}

void CRageBot::Move(CUserCmd *pCmd, bool &bSendPacket)
{

	IClientEntity* pLocalEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (!pLocalEntity || !Menu::Window.RageBotTab.Active.GetState() || !Interfaces::Engine->IsConnected() || !Interfaces::Engine->IsInGame())
		return;

	if (Menu::Window.RageBotTab.AntiAimEnable.GetState())
	{
		static int ChokedPackets = -1;

		CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());
		if (!pWeapon)
			return;

		if (ChokedPackets < 1 && pLocalEntity->IsAlive() && pCmd->buttons & IN_ATTACK && CanOpenFire() && GameUtils::IsBallisticWeapon(pWeapon))
		{
			bSendPacket = false;
		}
		else
		{
			if (pLocalEntity->IsAlive())
			{
				DoAntiAim(pCmd, bSendPacket);

			}
			ChokedPackets = 1;
		}
	}

	if (Menu::Window.RageBotTab.AimbotEnable.GetState())
		DoAimbot(pCmd, bSendPacket);

	if (Menu::Window.RageBotTab.AccuracyRecoil.GetIndex() == 0)
	{

	}
	else if (Menu::Window.RageBotTab.AccuracyRecoil.GetIndex() == 1)
	{
		DoNoRecoil(pCmd);
	}
	else if (Menu::Window.RageBotTab.AccuracyRecoil.GetIndex() == 2)
	{
		DoNoSpread(pCmd); // Not Yet
	}

	if (Menu::Window.RageBotTab.positioncorrect.GetState())
		pCmd->tick_count = TIME_TO_TICKS(InterpFix());

	if (Menu::Window.RageBotTab.AimbotAimStep.GetState())
	{
		Vector AddAngs = pCmd->viewangles - LastAngle;
		if (AddAngs.Length2D() > 25.f)
		{
			Normalize(AddAngs, AddAngs);
			AddAngs *= 23;
			pCmd->viewangles = LastAngle + AddAngs;
			GameUtils::NormaliseViewAngle(pCmd->viewangles);
		}
	}

	LastAngle = pCmd->viewangles;
}

Vector BestPoint(IClientEntity *targetPlayer, Vector &final)
{
	IClientEntity* pLocal = hackManager.pLocal();
	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());


	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;

	filter.pSkip = targetPlayer;
	ray.Init(final + Vector(0, 0,10), final); // all 0 or 0,0,10, either way, should hit the top of the head, but its weird af.
	Interfaces::Trace->TraceRay(ray, MASK_SHOT, &filter, &tr);

	// if you're actually one to look through this stuff, do tag me @ discord and tell me.

	final = tr.endpos;
	return final;
}


void CRageBot::DoAimbot(CUserCmd *pCmd, bool &bSendPacket)
{


	static int ChokedPackets = -1;
	ChokedPackets++;
	IClientEntity* pTarget = nullptr;
	IClientEntity* pLocal = hackManager.pLocal();
	Vector Start = pLocal->GetViewOffset() + pLocal->GetOrigin();
	bool FindNewTarget = true;
	CSWeaponInfo* weapInfo = ((CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle()))->GetCSWpnData();
	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());

	Vector Point;



	if (Menu::Window.RageBotTab.AutoRevolver.GetState())
		if (GameUtils::IsRevolver(pWeapon))
		{
			static int delay = 0;
			delay++;
			if (delay <= 15)pCmd->buttons |= IN_ATTACK;
			else delay = 0;
		}
	if (pWeapon)
	{
		if (pWeapon->GetAmmoInClip() == 0 || !GameUtils::IsBallisticWeapon(pWeapon)) return;
	}
	else return;
	if (IsLocked && TargetID >= 0 && HitBox >= 0)
	{
		pTarget = Interfaces::EntList->GetClientEntity(TargetID);
		if (pTarget  && TargetMeetsRequirements(pTarget))
		{
			HitBox = HitScan(pTarget);
			if (HitBox >= 0)
			{
				Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset(), View;
				Interfaces::Engine->GetViewAngles(View);
				float FoV = FovToPlayer(ViewOffset, View, pTarget, HitBox);
				if (FoV < Menu::Window.RageBotTab.AimbotFov.GetValue())	FindNewTarget = false;
			}
		}
	}


	if (FindNewTarget)
	{
		TargetID = 0;
		pTarget = nullptr;
		HitBox = -1;
		switch (Menu::Window.RageBotTab.TargetSelection.GetIndex())
		{
		case 0:TargetID = GetTargetCrosshair(); break;
		case 1:TargetID = GetTargetDistance(); break;
		case 2:TargetID = GetTargetHealth(); break;
		case 3:TargetID = GetTargetThreat(pCmd); break;
		case 4:TargetID = GetTargetNextShot(); break;
		}
		if (TargetID >= 0) pTarget = Interfaces::EntList->GetClientEntity(TargetID);
		else
		{


			pTarget = nullptr;
			HitBox = -1;
		}
	} 
	Globals::Target = pTarget;
	Globals::TargetID = TargetID;
	if (TargetID >= 0 && pTarget)
	{
		HitBox = HitScan(pTarget);

		if (!CanOpenFire()) return;

		if (Menu::Window.RageBotTab.AimbotKeyPress.GetState())
		{


			int Key = Menu::Window.RageBotTab.AimbotKeyBind.GetKey();
			if (Key >= 0 && !GUI.GetKeyState(Key))
			{
				TargetID = -1;
				pTarget = nullptr;
				HitBox = -1;
				return;
			}
		}
		float pointscale = Menu::Window.RageBotTab.TargetPointscale.GetValue() + 0.1f; 
		Vector AimPoint = GetHitboxPosition(pTarget, HitBox) + Vector(0, 0, pointscale);
		if (Menu::Window.RageBotTab.TargetMultipoint.GetState()) Point = BestPoint(pTarget, AimPoint);
		else Point = AimPoint;

		if (GameUtils::IsScopedWeapon(pWeapon) && !pWeapon->IsScoped() && Menu::Window.RageBotTab.AccuracyAutoScope.GetState()) pCmd->buttons |= IN_ATTACK2;
		else if ((Menu::Window.RageBotTab.AccuracyHitchance.GetValue() * 1.5 <= hitchance(pLocal, pWeapon)) || Menu::Window.RageBotTab.AccuracyHitchance.GetValue() == 0 || *pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() == 64)
			{
			if (pTarget->GetChokedTicks() > 4)
			{
				if (Menu::Window.RageBotTab.AimbotAutoFire.GetState() && !(pCmd->buttons & IN_ATTACK))
				{
					pCmd->buttons |= IN_ATTACK;
					AimPoint -= pTarget->GetAbsOrigin();
					AimPoint += pTarget->GetNetworkOrigin();
				}
				else if (pCmd->buttons & IN_ATTACK || pCmd->buttons & IN_ATTACK2)return;
			}
			else if (AimAtPoint(pLocal, Point, pCmd, bSendPacket))
					if (Menu::Window.RageBotTab.AimbotAutoFire.GetState() && !(pCmd->buttons & IN_ATTACK))pCmd->buttons |= IN_ATTACK;
					else if (pCmd->buttons & IN_ATTACK || pCmd->buttons & IN_ATTACK2)return;
			}
		if (IsAbleToShoot(pLocal) && pCmd->buttons & IN_ATTACK) { Globals::Shots += 1;  ChokedPackets = 1; }
	}

}

bool CRageBot::TargetMeetsRequirements(IClientEntity* pEntity)
{
	if (pEntity && pEntity->IsDormant() == false && pEntity->IsAlive() && pEntity->GetIndex() != hackManager.pLocal()->GetIndex())
	{

		ClientClass *pClientClass = pEntity->GetClientClass();
		player_info_t pinfo;
		if (pClientClass->m_ClassID == (int)CSGOClassID::CCSPlayer && Interfaces::Engine->GetPlayerInfo(pEntity->GetIndex(), &pinfo))
		{
			if (pEntity->GetTeamNum() != hackManager.pLocal()->GetTeamNum() || Menu::Window.RageBotTab.TargetFriendlyFire.GetState())
			{
				if (!pEntity->HasGunGameImmunity())
				{
					return true;
				}
			}
		}

	}

	return false;
}

float CRageBot::FovToPlayer(Vector ViewOffSet, Vector View, IClientEntity* pEntity, int aHitBox)
{
	CONST FLOAT MaxDegrees = 180.0f;

	Vector Angles = View;

	Vector Origin = ViewOffSet;

	Vector Delta(0, 0, 0);

	Vector Forward(0, 0, 0);

	AngleVectors(Angles, &Forward);
	Vector AimPos = GetHitboxPosition(pEntity, aHitBox);

	VectorSubtract(AimPos, Origin, Delta);

	Normalize(Delta, Delta);

	FLOAT DotProduct = Forward.Dot(Delta);

	return (acos(DotProduct) * (MaxDegrees / PI));
}

int CRageBot::GetTargetCrosshair()
{

	int target = -1;
	float minFoV = Menu::Window.RageBotTab.AimbotFov.GetValue();

	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);

	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (fov < minFoV)
				{
					minFoV = fov;
					target = i;
				}
			}

		}
	}

	return target;
}

int CRageBot::GetTargetDistance()
{

	int target = -1;
	int minDist = 99999;

	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);

	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{

			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				Vector Difference = pLocal->GetOrigin() - pEntity->GetOrigin();
				int Distance = Difference.Length();
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (Distance < minDist && fov < Menu::Window.RageBotTab.AimbotFov.GetValue())
				{
					minDist = Distance;
					target = i;
				}
			}

		}
	}

	return target;
}

int CRageBot::GetTargetNextShot()
{
	int target = -1;
	int minfov = 361;

	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);

	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++)
	{

		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				int Health = pEntity->GetHealth();
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (fov < minfov && fov < Menu::Window.RageBotTab.AimbotFov.GetValue())
				{
					minfov = fov;
					target = i;
				}
				else
					minfov = 361;
			}

		}
	}

	return target;
}

float GetFov(const QAngle& viewAngle, const QAngle& aimAngle)
{
	Vector ang, aim;

	AngleVectors(viewAngle, &aim);
	AngleVectors(aimAngle, &ang);

	return RAD2DEG(acos(aim.Dot(ang) / aim.LengthSqr()));
}

double inline __declspec (naked) __fastcall FASTSQRT(double n)
{
	_asm fld qword ptr[esp + 4]
		_asm fsqrt
	_asm ret 8
}

float VectorDistance(Vector v1, Vector v2)
{
	return FASTSQRT(pow(v1.x - v2.x, 2) + pow(v1.y - v2.y, 2) + pow(v1.z - v2.z, 2));
}

int CRageBot::GetTargetThreat(CUserCmd* pCmd)
{
	auto iBestTarget = -1;
	float flDistance = 8192.f;

	IClientEntity* pLocal = hackManager.pLocal();

	for (int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			auto vecHitbox = pEntity->GetBonePos(NewHitBox);
			if (NewHitBox >= 0)
			{

				Vector Difference = pLocal->GetOrigin() - pEntity->GetOrigin();
				QAngle TempTargetAbs;
				CalcAngle(pLocal->GetEyePosition(), vecHitbox, TempTargetAbs);
				float flTempFOVs = GetFov(pCmd->viewangles, TempTargetAbs);
				float flTempDistance = VectorDistance(pLocal->GetOrigin(), pEntity->GetOrigin());
				if (flTempDistance < flDistance && flTempFOVs < Menu::Window.RageBotTab.AimbotFov.GetValue())
				{
					flDistance = flTempDistance;
					iBestTarget = i;
				}
			}
		}
	}
	return iBestTarget;
}

int CRageBot::GetTargetHealth()
{

	int target = -1;
	int minHealth = 101;

	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);


	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				int Health = pEntity->GetHealth();
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (Health < minHealth && fov < Menu::Window.RageBotTab.AimbotFov.GetValue())
				{
					minHealth = Health;
					target = i;
				}
			}
		}

	}

	return target;
}

int CRageBot::HitScan(IClientEntity* pEntity)
{
	IClientEntity* pLocal = hackManager.pLocal();
	std::vector<int> HitBoxesToScan;
	
#pragma region GetHitboxesToScan
	float huso = (pEntity->GetHealth());
	int health = Menu::Window.RageBotTab.BaimIfUnderXHealth.GetValue();

	bool AWall = Menu::Window.RageBotTab.AccuracyAutoWall.GetState();
	bool Multipoint = Menu::Window.RageBotTab.TargetMultipoint.GetState();
	int TargetHitbox = Menu::Window.RageBotTab.TargetHitbox.GetIndex();
	static bool enemyHP = false;
	IClientEntity* WeaponEnt = Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());
	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());
	int xs;
	int ys;
	Interfaces::Engine->GetScreenSize(xs, ys);
	xs /= 2; ys /= 2;

	auto accuracy = pWeapon->GetInaccuracy() * 550.f; //3000


	int AimbotBaimOnKey = Menu::Window.RageBotTab.AimbotBaimOnKey.GetKey();
	if (AimbotBaimOnKey >= 0 && GUI.GetKeyState(AimbotBaimOnKey))
	{
		HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach); // 4
		HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftThigh); // 9
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightThigh); // 8
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot); // 13
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot); // 12
	}


	if (huso < health)
	{
		HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
		HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
		HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
		HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LowerChest);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightUpperArm);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftThigh);
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightThigh);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftHand);
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightHand);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftLowerArm);
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightLowerArm);
	}

	else if (Globals::Shots >= Menu::Window.RageBotTab.xaneafterX.GetValue())
	{
		HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
		HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
		HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
		HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);

	}
	else if (Menu::Window.RageBotTab.AWPAtBody.GetState() && GameUtils::AWP(pWeapon))
	{
		HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
		HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
		HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LowerChest);
		HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
	}
	else if (TargetHitbox)
	{
		switch (Menu::Window.RageBotTab.TargetHitbox.GetIndex())
		{
		case 0:
		{ /* wont shoot anything at all. Peace and love bruuuuh */ } break;
		case 1:
			HitBoxesToScan.push_back((int)CSGOHitboxID::Head); // 1
			HitBoxesToScan.push_back((int)CSGOHitboxID::Neck); // lets compensate a bit
			if (Menu::Window.RageBotTab.baimfactor.GetIndex() != 0)
			{
				HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
			}
			else
			{
				HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
			}
			break;
		case 2:
		{
			switch (Menu::Window.RageBotTab.baimfactor.GetIndex())
			{
			case 0:
			{
				HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightLowerArm);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
			}
			break;
			case 1:
			{
				if (accuracy > 60)
				{

					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				}
				else
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightLowerArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
				}
			}
			break;
			case 2:
			{
				if (Globals::Shots >= 2)
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				}
				else
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightLowerArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
				}

			}
			break;
			case 3:
			{
				if (accuracy >= 40)
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				}
				else if (accuracy >= 40 && Globals::Shots >= 2)
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				}
				else
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightLowerArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
				}

			}
			break;
			}

			break;

		}

		break;
	
		case 3:
		{
		
			switch (Menu::Window.RageBotTab.baimfactor.GetIndex())
			{
			case 0:
			{
				HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
				HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightLowerArm);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
			}
				break;
			case 1:
			{
				if (accuracy > 60)
				{

					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				}
				else
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightLowerArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
				}
			}
			break;
			case 2:
			{
				if (Globals::Shots >= 2)
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				}
				else
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightLowerArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
				}

			}
			break;
			case 3:
			{
				if (accuracy >= 40)
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				}
				else if (accuracy >= 40 && Globals::Shots >= 2)
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				}
				else
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightLowerArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
				}

			}
			break;
			}

			break;
		
		}
		
			break;
		case 4:
		{
		

			switch (Menu::Window.RageBotTab.baimfactor.GetIndex())
			{
			case 0: 
			{
				HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
				HitBoxesToScan.push_back((int)CSGOHitboxID::NeckLower);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftLowerArm);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightUpperArm);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftThigh);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightThigh);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
				HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightHand);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftHand);
			}

				break;
			case 1:
			{
				if (accuracy > 60)
				{

					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				}
				else
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::NeckLower);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftLowerArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightUpperArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftThigh);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightThigh);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightHand);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftHand);
				}
			}
			break;
			case 2:
			{
				if (Globals::Shots >= 2)
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				}
				else
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::NeckLower);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftLowerArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightUpperArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftThigh);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightThigh);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightHand);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftHand);
				}

			}
			case 3:
			{
				if (accuracy >= 25)
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				}
				else if (accuracy >= 24 && Globals::Shots >= 2)
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				}
				else
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::NeckLower);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftLowerArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightUpperArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftThigh);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightThigh);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightHand);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftHand);
				}

			}
			}

			break;
		}
	
		}
	}
#pragma endregion Get the list of shit to scan
	for (auto HitBoxID : HitBoxesToScan)
	{
		if (AWall)
		{
			Vector Point = GetHitboxPosition(pEntity, HitBoxID);
			float Damage = 0.f;
			Color c = Color(255, 255, 255, 255);
			if (CanHit(Point, &Damage))
			{
				c = Color(0, 255, 0, 255);
				if (Damage >= Menu::Window.RageBotTab.AccuracyMinimumDamage.GetValue())
				{
					return HitBoxID;
				}
			}
		}
		else
		{
			if (GameUtils::IsVisible(hackManager.pLocal(), pEntity, HitBoxID))
				return HitBoxID;
		}
	}

	return -1;
}

void CRageBot::DoNoRecoil(CUserCmd *pCmd)
{

	IClientEntity* pLocal = hackManager.pLocal();
	if (pLocal)
	{
		Vector AimPunch = pLocal->localPlayerExclusive()->GetAimPunchAngle();
		if (AimPunch.Length2D() > 0 && AimPunch.Length2D() < 150)
		{
			pCmd->viewangles -= AimPunch * 2;
			GameUtils::NormaliseViewAngle(pCmd->viewangles);
		}
	}

}

void CRageBot::DoNoSpread(CUserCmd *pCmd)
{

	IClientEntity* pLocal = hackManager.pLocal();
	if (pLocal)
	{
		Vector AimPunch = pLocal->localPlayerExclusive()->GetAimPunchAngle();
		if (AimPunch.Length2D() > 0 && AimPunch.Length2D() < 3000)
		{
			pCmd->viewangles -= AimPunch * 4;
			GameUtils::NormaliseViewAngle(pCmd->viewangles);
		}
	}

}



#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class uffsrpe {
public:
	int vrxksjstwrxe;
	double wedjyqqaze;
	int fbjxmujtahozgom;
	string xozxgo;
	bool qjsvzbwighacgx;
	uffsrpe();
	int qfgemouxxajhyybglzvtdhr(double rtntfr, string xmenhv, string teuwejm, bool hzfkcfevndgyroq, bool yzrrq, bool yvuavebac, double lvykq, double gvfgtactvcaoed);
	int dpvnubniavcuispstypgigy(bool diqdzauhgs, bool edodqdfuwxtrs, int leoglorargy, bool prvwbehtefzyhj, string vjcexwli);
	double pvtecqvzenmduer(double kcdrbqrthhwk, bool goehpwuqqioc, int htzgszjjbegh, double adhaghdvhnnzw, double ryghehczlwfhjrs, string brfzbjgzyox, bool efmzqjpdbydrp, bool qjrasvfyzwbhgs);
	int naxwddvjldc(int ykewjzao, bool swygrvdew, double qjuzxawi, bool wjpcbowv, double yzdvwkmpi, int vljbiizgtsqkyl, string kjqhgdo, string firdjuloxzktxq, double zpkdboiraxigrjm, bool ugrmfkpvxkqeeta);

protected:
	double edoeyy;
	bool ekszxjjnd;
	int wibxhsrpciamc;
	int shethsak;

	void bkbhutxlrntbied(int rppkbc, string kkbmfm, double ortssbjmyvhh, double iljmwddeyh, int myuccput, double tkizxcujnamnk, string giddvjjfbvn, bool gkcjfximla, string sswvcfaaqaltu);
	bool vkyuoyyvzpjsta(bool msdjoiuopjiwejl, bool xjosdlpsvpnkpj, int qauvv, int nfpxddeivxxhpbc, string tfotktsjyqso);
	bool jhandztkwdcoycxspbgcw();
	int qljrinyurcfqsxtwypalttr(string pixdv, double csfnnkpmnse, bool zwviqjeqcxx, double juztibv, int czgamtpnp, string csvclrszb);

private:
	string eupxyflf;
	int rpoqddzlajfive;

	double bgjwmufyyqduoubh(string dcgnjytoqb, string wsbysmp);
	bool jpoguawjxtvmevuwkd(string aabivmmrovfpvw, double riifg, string qnltqczssfjdryn, string zhgwceufafwigc);
	string jldpselvllkwlo(int kdfwlvo, string sihjneseaawmwdd, int uvsrywiouhhv, int irjnaq, double crkrhloqod, string sycly, string nhygfjwwcqm, bool nkzsfzlgscptb);
	void cfqfwygucokuai(string ehrlz);

};



double uffsrpe::bgjwmufyyqduoubh(string dcgnjytoqb, string wsbysmp) {
	int czwbh = 5858;
	double ugxwlojj = 21378;
	bool xvaizrvgyrsi = false;
	int gdaatytkl = 612;
	bool kwxnnqkudquh = true;
	double giakdvib = 96420;
	int ggkjlvbcteqtq = 8161;
	if (5858 == 5858) {
		int slgqf;
		for (slgqf = 61; slgqf > 0; slgqf--) {
			continue;
		}
	}
	if (false == false) {
		int mvgnvke;
		for (mvgnvke = 12; mvgnvke > 0; mvgnvke--) {
			continue;
		}
	}
	if (5858 == 5858) {
		int nbtyjugrye;
		for (nbtyjugrye = 59; nbtyjugrye > 0; nbtyjugrye--) {
			continue;
		}
	}
	if (5858 != 5858) {
		int vgix;
		for (vgix = 46; vgix > 0; vgix--) {
			continue;
		}
	}
	return 51447;
}

bool uffsrpe::jpoguawjxtvmevuwkd(string aabivmmrovfpvw, double riifg, string qnltqczssfjdryn, string zhgwceufafwigc) {
	int wzmyjsqeqxsrit = 3920;
	double slfqbpcos = 14166;
	int qzgjewpu = 2911;
	int aswerh = 1321;
	double ehryjkpnzcump = 22312;
	string csxyptg = "hkqebinfowhnciydkyveyrmimaebrvmqozuqfehiomg";
	double zqxgukqa = 1512;
	double xdjibn = 53841;
	bool aaqxk = true;
	bool sckgsksgzv = false;
	if (3920 != 3920) {
		int klgufpxkfo;
		for (klgufpxkfo = 0; klgufpxkfo > 0; klgufpxkfo--) {
			continue;
		}
	}
	return true;
}

string uffsrpe::jldpselvllkwlo(int kdfwlvo, string sihjneseaawmwdd, int uvsrywiouhhv, int irjnaq, double crkrhloqod, string sycly, string nhygfjwwcqm, bool nkzsfzlgscptb) {
	bool irsgsysi = true;
	string lklanoobdqscovj = "fgyljvonszxhpmwfpysfipbmqvrvms";
	if (true != true) {
		int csiibvr;
		for (csiibvr = 8; csiibvr > 0; csiibvr--) {
			continue;
		}
	}
	return string("jcxszuhvoqrr");
}

void uffsrpe::cfqfwygucokuai(string ehrlz) {
	double pdstaqjqtxbgugx = 85774;
	double kgytmxzlwugf = 3897;
	int tusjqyzivqd = 368;
	string sggmjaruafl = "mjoxphzfocgftozyrsugopxopgyozdejwcmyknzkfboelqvggwpqmjcjxgwhvvpjyqfuurckmvzk";
	int lhxlyaqhb = 231;
	double ihjylvbhmrsmexv = 36412;
	if (85774 == 85774) {
		int glwxakqmzl;
		for (glwxakqmzl = 43; glwxakqmzl > 0; glwxakqmzl--) {
			continue;
		}
	}

}

void uffsrpe::bkbhutxlrntbied(int rppkbc, string kkbmfm, double ortssbjmyvhh, double iljmwddeyh, int myuccput, double tkizxcujnamnk, string giddvjjfbvn, bool gkcjfximla, string sswvcfaaqaltu) {
	string ohczzdaprmohrc = "wwhwzluxweurmgzwutzmupccpccocpcwbeujplt";
	double ecpmbezkyduwfcv = 53907;
	bool fgtdeznajh = true;
	string dzafgn = "cjionsblzopswnnxhrezuicqsnstsqykgokogzsdlinxnqzatftexzkebnawxxajzkgtv";
	double ieyisiqgsfu = 11297;
	int mysddu = 2089;
	bool rdkqrhruovbfi = true;
	string kfgdlb = "mbmpsynvkqjgagmavzbf";
	int jltqkr = 2289;
	if (string("wwhwzluxweurmgzwutzmupccpccocpcwbeujplt") != string("wwhwzluxweurmgzwutzmupccpccocpcwbeujplt")) {
		int zwrpcaewcy;
		for (zwrpcaewcy = 97; zwrpcaewcy > 0; zwrpcaewcy--) {
			continue;
		}
	}
	if (string("cjionsblzopswnnxhrezuicqsnstsqykgokogzsdlinxnqzatftexzkebnawxxajzkgtv") != string("cjionsblzopswnnxhrezuicqsnstsqykgokogzsdlinxnqzatftexzkebnawxxajzkgtv")) {
		int ldauqaovqb;
		for (ldauqaovqb = 68; ldauqaovqb > 0; ldauqaovqb--) {
			continue;
		}
	}
	if (string("cjionsblzopswnnxhrezuicqsnstsqykgokogzsdlinxnqzatftexzkebnawxxajzkgtv") == string("cjionsblzopswnnxhrezuicqsnstsqykgokogzsdlinxnqzatftexzkebnawxxajzkgtv")) {
		int vprkn;
		for (vprkn = 77; vprkn > 0; vprkn--) {
			continue;
		}
	}

}

bool uffsrpe::vkyuoyyvzpjsta(bool msdjoiuopjiwejl, bool xjosdlpsvpnkpj, int qauvv, int nfpxddeivxxhpbc, string tfotktsjyqso) {
	return true;
}

bool uffsrpe::jhandztkwdcoycxspbgcw() {
	bool xfxzpoouersj = true;
	double hxklxoopm = 72559;
	bool dfrqrvsirfcnuh = true;
	if (true == true) {
		int tkrysw;
		for (tkrysw = 52; tkrysw > 0; tkrysw--) {
			continue;
		}
	}
	if (72559 != 72559) {
		int zknsicxsow;
		for (zknsicxsow = 38; zknsicxsow > 0; zknsicxsow--) {
			continue;
		}
	}
	if (true != true) {
		int joeotle;
		for (joeotle = 87; joeotle > 0; joeotle--) {
			continue;
		}
	}
	if (true != true) {
		int mzgyiugb;
		for (mzgyiugb = 53; mzgyiugb > 0; mzgyiugb--) {
			continue;
		}
	}
	return false;
}

int uffsrpe::qljrinyurcfqsxtwypalttr(string pixdv, double csfnnkpmnse, bool zwviqjeqcxx, double juztibv, int czgamtpnp, string csvclrszb) {
	bool ollndbaspb = false;
	double fxkfbbbblnijn = 23848;
	string xlxnzfyidj = "roiipwiohzkzlbxzqsqicyqhhygbxemmmgpqcaaowfrrkrckhulszszqounxbxqlysrwmofotrwnclepbgeevp";
	bool xuwnznvm = false;
	if (false == false) {
		int sbl;
		for (sbl = 8; sbl > 0; sbl--) {
			continue;
		}
	}
	if (false == false) {
		int jotktxbwk;
		for (jotktxbwk = 74; jotktxbwk > 0; jotktxbwk--) {
			continue;
		}
	}
	if (false != false) {
		int akamvy;
		for (akamvy = 92; akamvy > 0; akamvy--) {
			continue;
		}
	}
	if (false == false) {
		int lpnqam;
		for (lpnqam = 91; lpnqam > 0; lpnqam--) {
			continue;
		}
	}
	return 23461;
}

int uffsrpe::qfgemouxxajhyybglzvtdhr(double rtntfr, string xmenhv, string teuwejm, bool hzfkcfevndgyroq, bool yzrrq, bool yvuavebac, double lvykq, double gvfgtactvcaoed) {
	string aktmtt = "btmftgebgoohsfqqajtrluscbqelievfoekfpqkytbnghneslgtrtkupnsbltnumeuglikgvsut";
	double asttelbm = 7206;
	bool hupgc = true;
	bool zsodojep = false;
	string izrgvfraacdoj = "vaafjaubvlpwrefwjfskcrybwuknlhvpbhawhiatfjwsdjydfwyizlsdblydhpittyobbikbclheg";
	int mzerswya = 2385;
	double qjkmhjqbtsdphjg = 13697;
	if (string("vaafjaubvlpwrefwjfskcrybwuknlhvpbhawhiatfjwsdjydfwyizlsdblydhpittyobbikbclheg") != string("vaafjaubvlpwrefwjfskcrybwuknlhvpbhawhiatfjwsdjydfwyizlsdblydhpittyobbikbclheg")) {
		int ytbcizcfx;
		for (ytbcizcfx = 92; ytbcizcfx > 0; ytbcizcfx--) {
			continue;
		}
	}
	if (false != false) {
		int mrf;
		for (mrf = 35; mrf > 0; mrf--) {
			continue;
		}
	}
	if (13697 == 13697) {
		int qdfwrodfbz;
		for (qdfwrodfbz = 91; qdfwrodfbz > 0; qdfwrodfbz--) {
			continue;
		}
	}
	return 44622;
}

int uffsrpe::dpvnubniavcuispstypgigy(bool diqdzauhgs, bool edodqdfuwxtrs, int leoglorargy, bool prvwbehtefzyhj, string vjcexwli) {
	string tmczrtdgvhih = "vaucbpgjkjvynvapqzwbsqnxidjynzfrxzdmvfezvyzdsvpwndyjzsptckdhjoibgbcvwfhglphstlekc";
	double vbeiepyipwu = 87062;
	string zlrkrc = "lnrunoreckkzckhwejhepgqalnymnxtnqhxiikafgxiuhpveqnikzxbc";
	int wyvovivlfkmvkwj = 3940;
	int kblzelcxyd = 824;
	double zpqtfqmtopxj = 94449;
	if (824 != 824) {
		int ayyq;
		for (ayyq = 87; ayyq > 0; ayyq--) {
			continue;
		}
	}
	if (string("vaucbpgjkjvynvapqzwbsqnxidjynzfrxzdmvfezvyzdsvpwndyjzsptckdhjoibgbcvwfhglphstlekc") == string("vaucbpgjkjvynvapqzwbsqnxidjynzfrxzdmvfezvyzdsvpwndyjzsptckdhjoibgbcvwfhglphstlekc")) {
		int dfkee;
		for (dfkee = 24; dfkee > 0; dfkee--) {
			continue;
		}
	}
	if (87062 == 87062) {
		int rqwjb;
		for (rqwjb = 54; rqwjb > 0; rqwjb--) {
			continue;
		}
	}
	if (string("vaucbpgjkjvynvapqzwbsqnxidjynzfrxzdmvfezvyzdsvpwndyjzsptckdhjoibgbcvwfhglphstlekc") != string("vaucbpgjkjvynvapqzwbsqnxidjynzfrxzdmvfezvyzdsvpwndyjzsptckdhjoibgbcvwfhglphstlekc")) {
		int kessdwbngg;
		for (kessdwbngg = 14; kessdwbngg > 0; kessdwbngg--) {
			continue;
		}
	}
	if (3940 != 3940) {
		int ihrvth;
		for (ihrvth = 24; ihrvth > 0; ihrvth--) {
			continue;
		}
	}
	return 41962;
}

double uffsrpe::pvtecqvzenmduer(double kcdrbqrthhwk, bool goehpwuqqioc, int htzgszjjbegh, double adhaghdvhnnzw, double ryghehczlwfhjrs, string brfzbjgzyox, bool efmzqjpdbydrp, bool qjrasvfyzwbhgs) {
	bool hrmxjckffrt = false;
	int lezbwrvxygvlm = 3166;
	double kewmmalyosx = 35253;
	double pqythl = 23278;
	double vzjdehy = 12863;
	string bkmctwm = "gewlnmxngnejlreynemirl";
	return 47114;
}

int uffsrpe::naxwddvjldc(int ykewjzao, bool swygrvdew, double qjuzxawi, bool wjpcbowv, double yzdvwkmpi, int vljbiizgtsqkyl, string kjqhgdo, string firdjuloxzktxq, double zpkdboiraxigrjm, bool ugrmfkpvxkqeeta) {
	int colglecgojwgbp = 2049;
	int ewlbzjjgdi = 4310;
	string bspivpcwg = "qvbmvfhj";
	bool laurotjfsn = false;
	int cuowslyszrf = 2184;
	bool gdphlqxsiuzm = false;
	if (false != false) {
		int gfqck;
		for (gfqck = 70; gfqck > 0; gfqck--) {
			continue;
		}
	}
	return 905;
}

uffsrpe::uffsrpe() {
	this->qfgemouxxajhyybglzvtdhr(27716, string("gvbhusmcsdnymoeuigueafvffgprsfflaiymgfkjejsaiyusqylpfxcubjtjpbeciqjuygdnjlvzvwcdqnusybblnihqyvemvg"), string("sbvqdqdfsbvhvumdmmcbiucpjlnnkpomxbksjhldliywvmxzymdxhyafbjmuajkqaqvidfqfkxlselvzworknjqjlhumgkwk"), true, false, true, 8317, 15762);
	this->dpvnubniavcuispstypgigy(false, true, 971, true, string("pvwhonkyfgyyhzqfunkydkuzjxyqdiivcayqedikussgqkedanixcinotvuzlhfgrxmjhaxrgdiktnlacdxukkoozs"));
	this->pvtecqvzenmduer(16661, false, 2027, 31358, 66824, string("dsqpnlukywubnlomerhvkbvsyqcbkikyrqkubvhqajmjksrotpjjcbbjtvlocnscrozsnolfbbijywbwhyd"), true, false);
	this->naxwddvjldc(366, true, 4714, false, 12998, 690, string("xizhmdqbmgmmisikkqdzoexnayolqercunsgianwhgjtxmgnhyli"), string("szvtpfawfkxzgerjprgnrvebuoxfvxvfrgdgktergcylahpewtnkdrcykxxazgoblrkpfdpnwithacbkrxdelbowervxqrgpqw"), 53035, false);
	this->bkbhutxlrntbied(3026, string("osepcgbqukvqgkgmwlgocdtcsomlausdvuzfbludoqzmfkvqryfwv"), 76370, 10721, 4612, 12112, string("emqpdfpddetlddevjg"), false, string("hgwmsroro"));
	this->vkyuoyyvzpjsta(true, false, 2579, 1675, string("fnhfjwimhlmcudyiyjukduhdqejixtno"));
	this->jhandztkwdcoycxspbgcw();
	this->qljrinyurcfqsxtwypalttr(string("gvsnfnyifyfznpzgvkbrpdbmyzsbjabytawoauzrwdl"), 19503, true, 3670, 2454, string("dlheywgahdhgrbetahlvhmpwzxtuqxqkxnsonyxueoyarvnfgatemoucihupvnbhmqukmfmt"));
	this->bgjwmufyyqduoubh(string("qnprqskfprolredmepausguneouxehgorhqsgygh"), string("anaiftpfdhzyptzigvyaemqjcutseonoxlbifouqqgnzdvhoamhwgvcymeafiqhzhbwpmepfvlzilqkomrjdkhxz"));
	this->jpoguawjxtvmevuwkd(string("abkjinkbrqpzzgoakuoygrxkunzsxpcmvjspflfgvuphpisgaqzsojvxiqrcvrbtmbojcnzfnzuk"), 17027, string("pxlviwoxdwwbpevhryjhfklmeqvplhhmtiitucwzliwohyaeerajvittaxlidsvdbpmmobzaqzmnprnjdjliyvxqdmjshx"), string("qilzmtvirkpfxacvzafdvelbykusjtoabjaaeoaqhjkocjrivymbzpjrnjuezdgxqhywvrl"));
	this->jldpselvllkwlo(1495, string("luutpbzz"), 4522, 458, 47082, string("eupdenjsymolqjbgesiehclxxbccteevqav"), string("rdt"), true);
	this->cfqfwygucokuai(string("rpjhwycdmstjfbvhfntgkweawqpzrszartdhp"));
}





CAntiSpread* NoSpread = new CAntiSpread;


void CAntiSpread::CalcServer(Vector vSpreadVec, Vector ViewIn, Vector &vecSpreadDir)
{
	Vector vecViewForward, vecViewRight, vecViewUp;

	angvec(ViewIn, vecViewForward, vecViewRight, vecViewUp);

	vecSpreadDir = vecViewForward + vecViewRight * vSpreadVec.x + vecViewUp * vSpreadVec.y;
}

void CAntiSpread::antispread(CUserCmd* pCmd)
{
	Vector vecForward, vecRight, vecDir, vecUp, vecAntiDir;
	float flSpread, flInaccuracy;
	Vector qAntiSpread;

//	glb::mainwep->UpdateAccuracyPenalty();

	flSpread = glb::mainwep->GetSpread();

	flInaccuracy = glb::mainwep->GetInaccuracy();
	Globals::UserCmd->random_seed = MD5_PseudoRandom(Globals::UserCmd->command_number) & 0x7FFFFFFF;
	RandomSeed((Globals::UserCmd->random_seed & 0xFF) + 1);


	float fRand1 = RandFloat(0.f, 1.f);
	float fRandPi1 = RandFloat(0.f, 2.f * (float)M_PI);
	float fRand2 = RandFloat(0.f, 1.f);
	float fRandPi2 = RandFloat(0.f, 2.f * (float)M_PI);

	float m_flRecoilIndex = glb::mainwep->GetFloatRecoilIndex();


	if (glb::mainwep->WeaponID() == 64)
	{
		if (Globals::UserCmd->buttons & IN_ATTACK2)
		{
			fRand1 = 1.f - fRand1 * fRand1;
			fRand2 = 1.f - fRand2 * fRand2;
		}
	}
	else if (glb::mainwep->WeaponID() == NEGEV && m_flRecoilIndex < 3.f)
	{
		for (int i = 3; i > m_flRecoilIndex; --i)
		{
			fRand1 *= fRand1;
			fRand2 *= fRand2;
		}

		fRand1 = 1.f - fRand1;
		fRand2 = 1.f - fRand2;
	}

	float fRandInaccuracy = fRand1 * glb::mainwep->GetInaccuracy();
	float fRandSpread = fRand2 * glb::mainwep->GetSpread();

	float fSpreadX = cos(fRandPi1) * fRandInaccuracy + cos(fRandPi2) * fRandSpread;
	float fSpreadY = sin(fRandPi1) * fRandInaccuracy + sin(fRandPi2) * fRandSpread;


	pCmd->viewangles.x += RAD2DEG(atan(sqrt(fSpreadX * fSpreadX + fSpreadY * fSpreadY)));
	pCmd->viewangles.z = RAD2DEG(atan2(fSpreadX, fSpreadY));
}

void CRageBot::ns1(CUserCmd *pCmd, IClientEntity* LocalPlayer)
{
	float recoil_value = 2;
//	glb::mainwep->UpdateAccuracyPenalty();
	QAngle punch = LocalPlayer->GetPunchAngle();

	if (Interfaces::CVar->FindVar(XorStr("weapon_recoil_scale")))
	{
		ConVar* recoil_value = Interfaces::CVar->FindVar(XorStr("weapon_recoil_scale"));
		recoil_value->SetValue("2");
	}
	Globals::UserCmd->viewangles -= punch * recoil_value;
}

void CRageBot::nospread(CUserCmd *pCmd)
{
	IClientEntity* LocalPlayer= hackManager.pLocal();
	if (Menu::Window.RageBotTab.AccuracyRecoil.GetIndex() == 2)
	{
		ns1(pCmd, LocalPlayer);
		Globals::UserCmd->viewangles = NoSpread->SpreadFactor(Globals::UserCmd->random_seed);
	}


	

}

// 	float m_fValue;
void CRageBot::aimAtPlayer(CUserCmd *pCmd)
{
	IClientEntity* pLocal = hackManager.pLocal();

	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

	if (!pLocal || !pWeapon)
		return;

	Vector eye_position = pLocal->GetEyePosition();

	float best_dist = pWeapon->GetCSWpnData()->m_flRange;

	IClientEntity* target = nullptr;

	for (int i = 0; i < Interfaces::Engine->GetMaxClients(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			if (Globals::TargetID != -1)
				target = Interfaces::EntList->GetClientEntity(Globals::TargetID);
			else
				target = pEntity;

			Vector target_position = target->GetEyePosition();

			float temp_dist = eye_position.DistTo(target_position);

			if (best_dist > temp_dist)
			{
				best_dist = temp_dist;
				CalcAngle(eye_position, target_position, pCmd->viewangles);
			}
		}

	}
}


float GetBestHeadAngle(float yaw)
{
	float Back, Right, Left;
	IClientEntity* pLocal = hackManager.pLocal();
	Vector src3D, dst3D, forward, right, up, src, dst;
	trace_t tr;
	Ray_t ray, ray2, ray3, ray4, ray5;
	CTraceFilter filter;
	IEngineTrace trace;
	QAngle angles;
	QAngle engineViewAngles;
	Interfaces::Engine->GetViewAngles(angles);

	engineViewAngles.x = 0;

	Math::AngleVectors(engineViewAngles, &forward, &right, &up);

	filter.pSkip = pLocal;
	src3D = pLocal->GetEyeAngles();
	dst3D = src3D + (forward * 384);

	ray.Init(src3D, dst3D);

	Interfaces::Trace->TraceRay(ray, MASK_SHOT, &filter, &tr);

	Back = (tr.endpos - tr.startpos).Length();

	ray2.Init(src3D + right * 35, dst3D + right * 35);

	Interfaces::Trace->TraceRay(ray2, MASK_SHOT, &filter, &tr);

	Right = (tr.endpos - tr.startpos).Length();

	ray3.Init(src3D - right * 35, dst3D - right * 35);

	Interfaces::Trace->TraceRay(ray3, MASK_SHOT, &filter, &tr);

	Left = (tr.endpos - tr.startpos).Length();

	if (Left > Right)
	{
		return (yaw - 90);
	}
	else if (Right > Left)
	{
		return (yaw + 90);
	}
	else if (Back > Right || Back > Left)
	{
		return (yaw - 180);
	}
	return 0;
}


bool ShouldPredict()
{
	INetChannelInfo* nci = Interfaces::Engine->GetNetChannelInfo();

	IClientEntity* pLocal = hackManager.pLocal();
	float server_time = Interfaces::Globals->curtime + nci->GetLatency(FLOW_OUTGOING);

	float PredictedTime = 0.f;
	static bool initialized;

	bool will_update = false;

	if (!initialized && pLocal->IsMoving())
	{
		initialized = true;
		PredictedTime = server_time + 0.22f;
	}
	else if (pLocal->IsMoving())
	{
		initialized = false;
	}

	if (server_time >= (PredictedTime) && pLocal->GetFlags() & FL_ONGROUND)
	{
		PredictedTime = server_time + 1.1f;
		will_update = true;
	}
	return will_update;
}


bool CRageBot::AimAtPoint(IClientEntity* pLocal, Vector point, CUserCmd *pCmd, bool &bSendPacket)
{
	bool ReturnValue = false;

	if (point.Length() == 0) return ReturnValue;

	Vector angles;
	Vector src = pLocal->GetOrigin() + pLocal->GetViewOffset();

	CalcAngle(src, point, angles);
	GameUtils::NormaliseViewAngle(angles);

	if (angles[0] != angles[0] || angles[1] != angles[1])
	{
		return ReturnValue;
	}

	IsLocked = true;

	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	if (!IsAimStepping)
		LastAimstepAngle = LastAngle; 

	float fovLeft = FovToPlayer(ViewOffset, LastAimstepAngle, Interfaces::EntList->GetClientEntity(TargetID), 0);

	if (fovLeft > 25.0f && Menu::Window.RageBotTab.AimbotAimStep.GetState())
	{

		Vector AddAngs = angles - LastAimstepAngle;
		Normalize(AddAngs, AddAngs);
		AddAngs *= 25;
		LastAimstepAngle += AddAngs;
		GameUtils::NormaliseViewAngle(LastAimstepAngle);
		angles = LastAimstepAngle;
	}
	else
	{
		ReturnValue = true;
	}

	if (Menu::Window.RageBotTab.AimbotSilentAim.GetState())
	{
		pCmd->viewangles = angles;

	}

	if (!Menu::Window.RageBotTab.AimbotSilentAim.GetState())
	{

		Interfaces::Engine->SetViewAngles(angles);
	}

	return ReturnValue;
}

namespace AntiAims 
{
	

	void FastSpin(CUserCmd *pCmd)
	{
		static int y2 = -179;
		int spinBotSpeedFast = 75;

		y2 += spinBotSpeedFast;

		if (y2 >= 179)
			y2 = -179;

		pCmd->viewangles.y = y2;
	}



	void FastSpint(CUserCmd *pCmd)
	{
		int r1 = rand() % 100;
		int r2 = rand() % 1000;

		static bool dir;
		static float current_y = pCmd->viewangles.y;

		if (r1 == 1) dir = !dir;

		if (dir)
			current_y += 15 + rand() % 10;
		else
			current_y -= 15 + rand() % 10;

		pCmd->viewangles.y = current_y;

		if (r1 == r2)
			pCmd->viewangles.y += r1;
	}

	void Up(CUserCmd *pCmd)
	{
		pCmd->viewangles.x = -89.0f;
	}

	void Zero(CUserCmd *pCmd)
	{
		pCmd->viewangles.x = 0.f;
	}

	void fforward(CUserCmd *pCmd, bool &bSendPacket)
	{
		pCmd->viewangles.y -= 0.f;
	}

	void StaticRealYaw(CUserCmd *pCmd, bool &bSendPacket) {
		bSendPacket = false;
		pCmd->viewangles.y += 90;
	}
	
	

	void freak(IClientEntity* pLocal, CUserCmd *pCmd, bool &bSendPacket) 
	{
		if (pLocal->GetFlags()  & pLocal->GetVelocity().Length() <= 0.f & FL_ONGROUND)
		{
			QAngle angles;
			Interfaces::Engine->GetViewAngles(angles);

			float BestHeadPosition = GetBestHeadAngle(angles.y);

			float LowerbodyDelta = 160;

			if (bSendPacket)
			{
				pCmd->viewangles.y = BestHeadPosition + LowerbodyDelta + Math::RandomFloat2(-40.f, 40.f);
			}
			else
			{
				if (ShouldPredict())
					pCmd->viewangles.y = BestHeadPosition + LowerbodyDelta;
				else
					pCmd->viewangles.y = BestHeadPosition;
			}
		}
		


	}


	void freakmove(IClientEntity* pLocal, CUserCmd *pCmd, bool &bSendPacket)
	{
		if (pLocal->GetFlags() && pLocal->GetVelocity().Length() <= 245.f & FL_ONGROUND)
		{
			QAngle angles;
			Interfaces::Engine->GetViewAngles(angles);

			float BestHeadPosition = GetBestHeadAngle(angles.y);

			float LowerbodyDelta = 160;

			if (bSendPacket)
			{
				pCmd->viewangles.y = BestHeadPosition + LowerbodyDelta + Math::RandomFloat2(-40.f, 40.f);
			}
			else
			{
				if (ShouldPredict())
					pCmd->viewangles.y = BestHeadPosition + LowerbodyDelta;
				else
					pCmd->viewangles.y = BestHeadPosition;
			}
		}
		else
			pCmd->viewangles.y += 180;


	}
	void custombuildPitch(IClientEntity* pLocal, CUserCmd *pCmd, bool &bSendPacket)
	{
		static bool quickmathematics = false;
		float custombase = (Menu::Window.aabuild.pitchbase.GetValue());

		float pitch_jiiter_from = (Menu::Window.aabuild.pitch_jitter_from.GetValue());
		float pitch_jiiter_to = (Menu::Window.aabuild.pitch_jitter_to.GetValue());

		float pitch_safe_fake = (Menu::Window.aabuild.pitch_safe_fake.GetValue());
		float pitch_safe_real = (Menu::Window.aabuild.pitch_safe_real.GetValue());

		float pitch_unsafe_fake = (Menu::Window.aabuild.pitch_unsafe_fake.GetValue());
		float pitch_unsafe_real = (Menu::Window.aabuild.pitch_unsafe_real.GetValue());


		if (Menu::Window.aabuild.pitchpick.GetIndex() == 0)
		{
			pCmd->viewangles.x = custombase;
		}
		else if (Menu::Window.aabuild.pitchpick.GetIndex() == 1)
		{

			if (quickmathematics)
			{
				pCmd->viewangles.x = pitch_jiiter_from;
			}
			else
			{
				pCmd->viewangles.x = pitch_jiiter_to;
			}
			quickmathematics = !quickmathematics;
		}
		else if (Menu::Window.aabuild.pitchpick.GetIndex() == 2)
		{
			if (bSendPacket)
				pitch_safe_fake;
			else
				pitch_safe_real;
		}
		else
		{
			if (bSendPacket)
				pitch_unsafe_fake;
			else
				pitch_unsafe_real;
		}
	}


	void custombuildYaw(IClientEntity* pLocal, CUserCmd *pCmd, bool &bSendPacket)
	{
		float value;
		float rndy = rand() % 60;
		static bool switch3 = false;

		if (rndy < 15)
			rndy = 15;


		float custombase = (Menu::Window.aabuild.yawbase.GetValue()); // Starting point

		float switch1 = (Menu::Window.aabuild.yaw_switch_from.GetValue());
		float switch2 = (Menu::Window.aabuild.yaw_switch_to.GetValue());

		float jit1 = (Menu::Window.aabuild.yaw_jitter_from.GetValue());
		float jit2 = (Menu::Window.aabuild.yaw_jitter_to.GetValue());

		float spinspeed = (Menu::Window.aabuild.spinspeed.GetValue());

		//--------------------lby-trash-idk-lmao--------------------//
		int flip = (int)floorf(Interfaces::Globals->curtime / 1.1) % 2;
		static bool bFlipYaw;
		static bool wilupdate;
		float flInterval = Interfaces::Globals->interval_per_tick;
		float flTickcount = pCmd->tick_count;
		float flTime = flInterval * flTickcount;
		if (std::fmod(flTime, 1) == 0.f)
			bFlipYaw = !bFlipYaw;
		static float LastLBYUpdateTime = 0;

		float server_time = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;
		//--------------------lby-trash-idk-lmao--------------------//


		if (Menu::Window.aabuild.yawpick.GetIndex() == 0)
		{
			pCmd->viewangles.y -= custombase;
		}
		else if (Menu::Window.aabuild.yawpick.GetIndex() == 1) // jitter AA
		{

			if (switch3)
				pCmd->viewangles.y = jit1;
			else
				pCmd->viewangles.y = jit2;
			switch3 = !switch3;

		}
		else if (Menu::Window.aabuild.yawpick.GetIndex() == 2) // Switch AA
		{

			if (Menu::Window.aabuild.yaw_add_jitter.GetState() == false) // if we do not add jitter
			{
				if (flip)
				{
					pCmd->viewangles.y += bFlipYaw ? switch1 : switch2;

				}
				else
				{
					pCmd->viewangles.y -= hackManager.pLocal()->GetLowerBodyYaw() + bFlipYaw ? switch1 : switch2;
				}
			}
			else // if we add jitter
			{
				if (flip)
				{
					pCmd->viewangles.y += bFlipYaw ? switch1 + rndy : switch2 - rndy;

				}
				else
				{
					pCmd->viewangles.y -= hackManager.pLocal()->GetLowerBodyYaw() + bFlipYaw ? switch1 + rndy : switch2 - rndy;
				}
			}

		}
		else
		{
			static int y2 = -170;
			int spinBotSpeedFast = spinspeed;

			y2 += spinBotSpeedFast;

			if (y2 >= 170)
				y2 = -170;

			pCmd->viewangles.y = y2;

		}

	}
	void custombuildFake(IClientEntity* pLocal, CUserCmd *pCmd, bool &bSendPacket)
	{
		float value;
		float rndy = rand() % 60;
		static bool switch3 = false;

		if (rndy < 15)
			rndy = 15;


		float custombase = (Menu::Window.aabuild.fyawbase.GetValue()); // Starting point

		float switch1 = (Menu::Window.aabuild.fyaw_switch_from.GetValue());
		float switch2 = (Menu::Window.aabuild.fyaw_switch_to.GetValue());

		float jit1 = (Menu::Window.aabuild.fyaw_jitter_from.GetValue());
		float jit2 = (Menu::Window.aabuild.fyaw_jitter_to.GetValue());

		float spinspeed = (Menu::Window.aabuild.fspinspeed.GetValue());

		//--------------------lby-trash-idk-lmao--------------------//
		int flip = (int)floorf(Interfaces::Globals->curtime / 1.1) % 2;
		static bool bFlipYaw;
		static bool wilupdate;
		float flInterval = Interfaces::Globals->interval_per_tick;
		float flTickcount = pCmd->tick_count;
		float flTime = flInterval * flTickcount;
		if (std::fmod(flTime, 1) == 0.f)
			bFlipYaw = !bFlipYaw;
		static float LastLBYUpdateTime = 0;

		float server_time = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;
		//--------------------lby-trash-idk-lmao--------------------//


		if (Menu::Window.aabuild.fyawpick.GetIndex() == 0)
		{
			if (bSendPacket)
				pCmd->viewangles.y -= custombase;
		}
		else if (Menu::Window.aabuild.fyawpick.GetIndex() == 1) // jitter AA
		{
			if (bSendPacket)
			{
				if (switch3)
					pCmd->viewangles.y = jit1;
				else
					pCmd->viewangles.y = jit2;
				switch3 = !switch3;
			}

		}
		else if (Menu::Window.aabuild.fyawpick.GetIndex() == 2) // Switch AA
		{

			if (Menu::Window.aabuild.yaw_add_jitter.GetState() == false) // if we do not add jitter
			{
				if (bSendPacket)
				{
					if (flip)
					{
						pCmd->viewangles.y += bFlipYaw ? switch1 : switch2;

					}
					else
					{
						pCmd->viewangles.y -= hackManager.pLocal()->GetLowerBodyYaw() + bFlipYaw ? switch1 : switch2;
					}
				}
			}
			else // if we add jitter
			{
				if (bSendPacket)
				{
					if (flip)
					{
						pCmd->viewangles.y += bFlipYaw ? switch1 + rndy : switch2 + rndy;

					}
					else
					{
						pCmd->viewangles.y -= hackManager.pLocal()->GetLowerBodyYaw() + bFlipYaw ? switch1 - rndy : switch2 - rndy;
					}
				}
			}

		}
		else
		{
			if (bSendPacket)
			{
				static int y2 = -170;
				int spinBotSpeedFast = spinspeed;

				y2 += spinBotSpeedFast;

				if (y2 >= 170)
					y2 = -170;

				pCmd->viewangles.y = y2;
			}

		}
	}






	void LBYSide(CUserCmd *pCmd, bool &bSendPacket)
	{
		int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); ++i;
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

		static bool isMoving;
		float PlayerIsMoving = abs(pLocal->GetVelocity().Length());
		if (PlayerIsMoving > 0.1) isMoving = true;
		else if (PlayerIsMoving <= 0.1) isMoving = false;
		int meme = rand() % 75;
		if (meme < 17)
			meme = 17;
		int flip = (int)floorf(Interfaces::Globals->curtime / 1.1) % 2;
		static bool bFlipYaw;
		float flInterval = Interfaces::Globals->interval_per_tick;
		float flTickcount = pCmd->tick_count;
		float flTime = flInterval * flTickcount;
		if (std::fmod(flTime, 1) == 0.f)
			bFlipYaw = !bFlipYaw;

		if (PlayerIsMoving <= 0.4)
		{
			if (bSendPacket)
			{
				pCmd->viewangles.y += bFlipYaw ? (90.f + meme) : (-90 - meme);
			}
			else
			{
				if (flip)
				{
					pCmd->viewangles.y += bFlipYaw ? (90.f + meme) : (-90 - meme);

				}
				else
				{
					pCmd->viewangles.y -= hackManager.pLocal()->GetLowerBodyYaw() + bFlipYaw ? (90.f + meme) : (-90 - meme);
				}
			}
		}
		else
		{
			if (bSendPacket)
			{
				pCmd->viewangles.y += 5.f - meme;
			}
			else
			{
				pCmd->viewangles.y += 160.f - meme;
			}
		}
	}


	void SideJitter(CUserCmd *pCmd)
	{
		static bool Fast2 = false;
		if (Fast2)
		{
			pCmd->viewangles.y += 75;
		}
		else
		{
			pCmd->viewangles.y += 105;
		}
		Fast2 = !Fast2;
	}

	void SlowSpin(CUserCmd *pCmd)
	{
		int r1 = rand() % 100;
		int r2 = rand() % 1000;

		static bool dir;
		static float current_y = pCmd->viewangles.y;

		if (r1 == 1) dir = !dir;

		if (dir)
			current_y += 4 + rand() % 10;
		else
			current_y -= 4 + rand() % 10;

		pCmd->viewangles.y = current_y;

		if (r1 == r2)
			pCmd->viewangles.y += r1;
	}
}

void CorrectMovement(Vector old_angles, CUserCmd* cmd, float old_forwardmove, float old_sidemove)
{
	float delta_view, first_function, second_function;

	if (old_angles.y < 0.f) first_function = 360.0f + old_angles.y;
	else first_function = old_angles.y;
	if (cmd->viewangles.y < 0.0f) second_function = 360.0f + cmd->viewangles.y;
	else second_function = cmd->viewangles.y;

	if (second_function < first_function) delta_view = abs(second_function - first_function);
	else delta_view = 360.0f - abs(first_function - second_function);

	delta_view = 360.0f - delta_view;

	cmd->forwardmove = cos(DEG2RAD(delta_view)) * old_forwardmove + cos(DEG2RAD(delta_view + 90.f)) * old_sidemove;
	cmd->sidemove = sin(DEG2RAD(delta_view)) * old_forwardmove + sin(DEG2RAD(delta_view + 90.f)) * old_sidemove;
}

float GetLatency()
{
	INetChannelInfo *nci = Interfaces::Engine->GetNetChannelInfo();
	if (nci)
	{

		float Latency = nci->GetAvgLatency(FLOW_OUTGOING) + nci->GetAvgLatency(FLOW_INCOMING);
		return Latency;
	}
	else
	{

		return 0.0f;
	}
}
float GetOutgoingLatency()
{
	INetChannelInfo *nci = Interfaces::Engine->GetNetChannelInfo();
	if (nci)
	{

		float OutgoingLatency = nci->GetAvgLatency(FLOW_OUTGOING);
		return OutgoingLatency;
	}
	else
	{

		return 0.0f;
	}
}
float GetIncomingLatency()
{
	INetChannelInfo *nci = Interfaces::Engine->GetNetChannelInfo();
	if (nci)
	{
		float IncomingLatency = nci->GetAvgLatency(FLOW_INCOMING);
		return IncomingLatency;
	}
	else
	{

		return 0.0f;
	}
}

float OldLBY;
float LBYBreakerTimer;
float LastLBYUpdateTime;
bool bSwitch;

float CurrentVelocity(IClientEntity* LocalPlayer)
{
	int vel = LocalPlayer->GetVelocity().Length2D();
	return vel;
}

bool NextLBYUpdate()
{
	IClientEntity* LocalPlayer = hackManager.pLocal();

	float flServerTime = (float)(LocalPlayer->GetTickBase()  * Interfaces::Globals->interval_per_tick);


	if (OldLBY != LocalPlayer->GetLowerBodyYaw())
	{

		LBYBreakerTimer++;
		OldLBY = LocalPlayer->GetLowerBodyYaw();
		bSwitch = !bSwitch;
		LastLBYUpdateTime = flServerTime;
	}

	if (CurrentVelocity(LocalPlayer) > 1)
	{
		LastLBYUpdateTime = flServerTime;
		return false;
	}
	int choked;
	if (Menu::Window.aabuild.FakeLagTyp.GetIndex() == 1)
		choked = 0.0625;
	else if (Menu::Window.aabuild.FakeLagTyp.GetIndex() == 2)
		choked = 0.1406;

	if ((LastLBYUpdateTime + 1 - (GetLatency() * 2) < flServerTime) && (LocalPlayer->GetFlags() & FL_ONGROUND))
	{
		if (LastLBYUpdateTime + (1.1 - choked) - (GetLatency() * 2) < flServerTime)
		{
			LastLBYUpdateTime += (1.1 - choked);
		}
		return true;
	}
	return false;
}

bool NextMovingLBYUpdate()
{
	IClientEntity* LocalPlayer = hackManager.pLocal();

	float flServerTime = (float)(LocalPlayer->GetTickBase()  * Interfaces::Globals->interval_per_tick);


	if (OldLBY != LocalPlayer->GetLowerBodyYaw())
	{

		LBYBreakerTimer++;
		OldLBY = LocalPlayer->GetLowerBodyYaw();
		bSwitch = !bSwitch;
		LastLBYUpdateTime = flServerTime;
	}

	if (CurrentVelocity(LocalPlayer) > 1)
	{
		LastLBYUpdateTime = flServerTime;
		return false;
	}

	if ((LastLBYUpdateTime + 1 - (GetLatency() * 2) < flServerTime) && (LocalPlayer->GetFlags() & FL_ONGROUND))
	{
		if (LastLBYUpdateTime + 0.2171 - (GetLatency() * 2) < flServerTime)
		{
			LastLBYUpdateTime += 0.21712;
		}
		return true;
	}
	return false;
}




#define RandomInt(min, max) (rand() % (max - min + 1) + min)
void DoLBYBreak(CUserCmd * pCmd, IClientEntity* pLocal, bool& bSendPacket)
{
	if (!bSendPacket)
	{
		pCmd->viewangles.y -= 90;
	}
	else
	{
		pCmd->viewangles.y += 90;
	}
}

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class vvjnhom {
public:
	int apzronq;
	vvjnhom();
	bool utavasgnltzhuxpklkfjd(bool fiyxt, int azsgagqvcy, int kcdrxanqmcw, bool zlxagzixduridq, bool tozqftmfunh, int erbbciwyuj, int tovopwa, bool laumd, bool rvgqhdqnvhookva);
	void njdzejpyxqcefbulgoi(bool mfbccaeljvp, double opaegxoksi, string gkeojssw, double dzridg, double nolatwbsuokf, string vmitcvjzqikpewz);
	void znsllldevqbhgzlvalcq(int wyfpu, string hijgxqorzqqcs, bool wgvhdjwgfl, bool neehjxkodulhii, double bfrmbfoeqnpynse);
	double pnhneepxknhuhyylcia(bool acnonpuxm, int jrzilqhbg, double igjrsiqnay, string djfnxypltll, int fcaww, int aulgwkkisxfauvf, int utwnobnizgj, double mzsgszea);
	string zuhhcmvehjumfxgwpctwtdf(int qkuhjl, int uehdohxenajykrr);

protected:
	double rpiagfupsuzay;
	bool clhrxwlrwrq;
	int wxzqgffmn;

	double dlqitbzcchhstuw(string aoqymglvv, int czolmpfxndo, string tprqsj, string cmivffopkqqcnpf, double bgfnhiamnsuvkv, double yzcvqnm, bool nrkudnbagdlnq);
	double jffkthlotfmfxfq(double kphelghget, double dgmrotwngtqayvm, double shayre, double lxwejqugmclo);
	bool nakvwtvdaubyyb(bool ejmvpxq, string rcetao, int dgmhelo, string lxweugdgyhynb, double vwtvkjqeven);
	bool gtufnctaydlljkkztv(string ogttmktdw, bool dlpynffsox);
	string pqzwrzhlbtjhmrkdtnxzai(string drpvujrjng, string htcxcbpifexcj);
	void nqfrlnqeifupt(bool wqtjinoiwgkl, int oqawetsodoagvuf, int cjmbbpmtsdws, double rsgysdvi, bool hnrahscxtpx, double vtyybqt);
	int xpmahomjrwgdrcnxgwmkggsf(double hvpvshviz, int vkusb, int rjigbysamrra, bool pdgmltwxzcuxa, string hrdbkhhngkj, int uubvry, bool bvcsbl);
	bool yigwiqaluszkdlkr(double vfrfcpxgopqsh, double liebhzxsw, int dotyaeeoiqgtjvm);
	double rnpkouwllcdjlbdezn(double fhlydknwvo, int ypvquori, string mlzdhaqkmot, bool cgqpqwsoacjmkzr, string orlvq, int wsrsmojk, string qqneoqsl, int uywvqde, int jwvilfe);
	void nsdorttkzizfwdwn();

private:
	string mmgmt;
	int smnlmrhgudesffp;

	string fhhgabotkryttpatp(string ssultubpxyub, string xrdnhmsd, double vuyqobmqvidtpx, bool gftxlrduuvifnr, double eqummz, string fesaaeug, double ldhkqngxi);
	int fjhdmzcdefx();

};



string vvjnhom::fhhgabotkryttpatp(string ssultubpxyub, string xrdnhmsd, double vuyqobmqvidtpx, bool gftxlrduuvifnr, double eqummz, string fesaaeug, double ldhkqngxi) {
	bool ewspirj = true;
	int ekdkbzfwzmjm = 555;
	int nbihog = 3309;
	int wqfxihuurb = 4307;
	string edwge = "gpafwlmhqeqwgqukdezdeznczqvkltevugxubxabim";
	int zrodthexhmmwej = 901;
	double hrrglelcvqo = 13068;
	bool efsmi = true;
	double qgddexf = 10642;
	bool rjmdkgyq = false;
	if (true != true) {
		int lnevmxxo;
		for (lnevmxxo = 9; lnevmxxo > 0; lnevmxxo--) {
			continue;
		}
	}
	if (901 != 901) {
		int pfhep;
		for (pfhep = 45; pfhep > 0; pfhep--) {
			continue;
		}
	}
	if (555 == 555) {
		int repziizki;
		for (repziizki = 37; repziizki > 0; repziizki--) {
			continue;
		}
	}
	return string("");
}

int vvjnhom::fjhdmzcdefx() {
	int trmeoortky = 1042;
	int nqirriamlbty = 3183;
	double jahrdg = 24044;
	int jorfh = 3212;
	int jqodigdxlxvsbtt = 2620;
	int zulzxirsu = 3387;
	double pyxeltpnlrv = 7922;
	double wqhpdsgyamagzbu = 23534;
	if (3183 != 3183) {
		int lbovuofwf;
		for (lbovuofwf = 15; lbovuofwf > 0; lbovuofwf--) {
			continue;
		}
	}
	if (2620 == 2620) {
		int pkkndru;
		for (pkkndru = 58; pkkndru > 0; pkkndru--) {
			continue;
		}
	}
	if (3387 == 3387) {
		int axygthdw;
		for (axygthdw = 23; axygthdw > 0; axygthdw--) {
			continue;
		}
	}
	return 93630;
}

double vvjnhom::dlqitbzcchhstuw(string aoqymglvv, int czolmpfxndo, string tprqsj, string cmivffopkqqcnpf, double bgfnhiamnsuvkv, double yzcvqnm, bool nrkudnbagdlnq) {
	int bdbwgzwqobjsp = 324;
	string jduobwcofkhhj = "swdisvherrcgxosnkendmsehpvnjixfzospytdzlfavqjjamkafdndqpnzyxc";
	string pqszztvcijj = "mogcuqcxotdlbkjhhijmdjvnwnmavoeubzbmosfcdhmpxkottuzxiuakt";
	int seipvkwswlzeo = 3443;
	double muweghna = 34892;
	string zitustzz = "ba";
	bool denzrevtldgnj = true;
	string exinatbxlwky = "libxuoshpaabhxrxyttptdnhxambxqxucfbcyzasuoniidmosh";
	string xdpxqjezgxck = "opdumjnhumczjtluzxfshmaqwfijclpichayzybqpwmgblrhagqaxptojwefcqedccehs";
	if (string("mogcuqcxotdlbkjhhijmdjvnwnmavoeubzbmosfcdhmpxkottuzxiuakt") == string("mogcuqcxotdlbkjhhijmdjvnwnmavoeubzbmosfcdhmpxkottuzxiuakt")) {
		int lmoligwqs;
		for (lmoligwqs = 12; lmoligwqs > 0; lmoligwqs--) {
			continue;
		}
	}
	if (34892 != 34892) {
		int csuakl;
		for (csuakl = 42; csuakl > 0; csuakl--) {
			continue;
		}
	}
	return 59938;
}

double vvjnhom::jffkthlotfmfxfq(double kphelghget, double dgmrotwngtqayvm, double shayre, double lxwejqugmclo) {
	string gvuwpqjwyvbsamw = "izzmdmnjkmsrgapydey";
	bool vfeszszl = true;
	double ehehhjn = 37231;
	string chhkvusqgg = "rlqxbkeacjbcstmfwjcittswkcvlofyryfqwvpzrgtnbjtgjbbdhdsf";
	double pnvadmwvqwt = 42422;
	double sihbydsrmzc = 43449;
	if (true == true) {
		int afukhnc;
		for (afukhnc = 69; afukhnc > 0; afukhnc--) {
			continue;
		}
	}
	if (42422 != 42422) {
		int ezukufpujw;
		for (ezukufpujw = 74; ezukufpujw > 0; ezukufpujw--) {
			continue;
		}
	}
	if (43449 != 43449) {
		int kuiy;
		for (kuiy = 51; kuiy > 0; kuiy--) {
			continue;
		}
	}
	if (true == true) {
		int rsf;
		for (rsf = 97; rsf > 0; rsf--) {
			continue;
		}
	}
	if (42422 != 42422) {
		int ochxrqdqg;
		for (ochxrqdqg = 33; ochxrqdqg > 0; ochxrqdqg--) {
			continue;
		}
	}
	return 71070;
}

bool vvjnhom::nakvwtvdaubyyb(bool ejmvpxq, string rcetao, int dgmhelo, string lxweugdgyhynb, double vwtvkjqeven) {
	double phpqqzglgv = 28583;
	int fkpksmeh = 5230;
	int iyeachumqjdqvuw = 4202;
	int krgxwbrao = 5464;
	string gdwajzedknrnmy = "rticjhzvdtrtvpzgsvkdgoyahijgvlupzwvqyxipkacb";
	string adywedxeysqosjv = "rjparfhfsrrvwuvfnxyqfjvbqlqdgikeepywapzjnbinkdqtrmnx";
	string nmtrlqncqtmpsuw = "pkjvhzcpnrhyvyqbpnkaeshdjdlhnqkrinrdurqlxhxpwmkukywxiibestuixajyomphy";
	bool llbgj = false;
	double rffsgzpsubx = 29843;
	if (5464 != 5464) {
		int vqcaabhrl;
		for (vqcaabhrl = 100; vqcaabhrl > 0; vqcaabhrl--) {
			continue;
		}
	}
	if (4202 == 4202) {
		int hguei;
		for (hguei = 72; hguei > 0; hguei--) {
			continue;
		}
	}
	if (4202 == 4202) {
		int mytnfel;
		for (mytnfel = 15; mytnfel > 0; mytnfel--) {
			continue;
		}
	}
	return false;
}

bool vvjnhom::gtufnctaydlljkkztv(string ogttmktdw, bool dlpynffsox) {
	int xjidcierhaaxr = 1337;
	int eevlxgsnj = 2654;
	string nramguirfeavs = "cyzltydzhfdptsoignqdemhsrjmgwg";
	string itbnzhroz = "brociyzaglljowqgckzpciroithepnuououdvpnjftxkvaykblzbrevqtemek";
	bool hjqtu = true;
	string rudiir = "pvleqltdzajqhgacrzksfomkwglytspdixilnmreolxlvjczrtyboddx";
	int riacsvmvdtdrir = 1266;
	int etpfrqwsphq = 2734;
	int ipzdtoaxiiip = 1542;
	string aycbnfyak = "rouerzoygxdsgaiklvjtndrfzoyyngvcgyuhalewgvrpjxagbcjtjhtvqwxuoimkouokpfzwnyfmnrzku";
	return true;
}

string vvjnhom::pqzwrzhlbtjhmrkdtnxzai(string drpvujrjng, string htcxcbpifexcj) {
	return string("sfylluqugrzm");
}

void vvjnhom::nqfrlnqeifupt(bool wqtjinoiwgkl, int oqawetsodoagvuf, int cjmbbpmtsdws, double rsgysdvi, bool hnrahscxtpx, double vtyybqt) {

}

int vvjnhom::xpmahomjrwgdrcnxgwmkggsf(double hvpvshviz, int vkusb, int rjigbysamrra, bool pdgmltwxzcuxa, string hrdbkhhngkj, int uubvry, bool bvcsbl) {
	string tsgcdbiobnx = "iqrnzmpaongszeqdphejalhktkxqxkchw";
	double pzjji = 14460;
	string qfpkwwzceg = "dzcruhmhjtmxptlqowgvnpjoiktsfjajybbogqpzvdolrqaimful";
	string qfipxuwnpzv = "imsjhbrpazuswaoitgoxyxptbqdvalrowsygamhpekanzryzhpxsnkczfobjhigzvekimznyctqlqkwoagzmtupmktenodars";
	if (string("dzcruhmhjtmxptlqowgvnpjoiktsfjajybbogqpzvdolrqaimful") == string("dzcruhmhjtmxptlqowgvnpjoiktsfjajybbogqpzvdolrqaimful")) {
		int pvpnvm;
		for (pvpnvm = 89; pvpnvm > 0; pvpnvm--) {
			continue;
		}
	}
	if (string("iqrnzmpaongszeqdphejalhktkxqxkchw") == string("iqrnzmpaongszeqdphejalhktkxqxkchw")) {
		int yheyyq;
		for (yheyyq = 70; yheyyq > 0; yheyyq--) {
			continue;
		}
	}
	if (14460 == 14460) {
		int butk;
		for (butk = 36; butk > 0; butk--) {
			continue;
		}
	}
	if (14460 != 14460) {
		int hdqjbmohh;
		for (hdqjbmohh = 14; hdqjbmohh > 0; hdqjbmohh--) {
			continue;
		}
	}
	if (string("iqrnzmpaongszeqdphejalhktkxqxkchw") == string("iqrnzmpaongszeqdphejalhktkxqxkchw")) {
		int nplwqqgn;
		for (nplwqqgn = 61; nplwqqgn > 0; nplwqqgn--) {
			continue;
		}
	}
	return 35070;
}

bool vvjnhom::yigwiqaluszkdlkr(double vfrfcpxgopqsh, double liebhzxsw, int dotyaeeoiqgtjvm) {
	return true;
}

double vvjnhom::rnpkouwllcdjlbdezn(double fhlydknwvo, int ypvquori, string mlzdhaqkmot, bool cgqpqwsoacjmkzr, string orlvq, int wsrsmojk, string qqneoqsl, int uywvqde, int jwvilfe) {
	double ahpsboirjesmgnb = 19504;
	if (19504 == 19504) {
		int azicb;
		for (azicb = 49; azicb > 0; azicb--) {
			continue;
		}
	}
	return 67064;
}

void vvjnhom::nsdorttkzizfwdwn() {
	bool pqvwqptolbf = false;
	string xqntmaprfi = "tjwuylvzvbbpoqpmvvdxadphksnhtgnwttmoimepavmriqcvwxjxrffyvvrznhukapvooatbsxwcxilnftwdou";
	double oodrlkmxzgekkq = 14087;
	double gqorysxldvjp = 19190;
	bool fcfiaksvx = false;

}

bool vvjnhom::utavasgnltzhuxpklkfjd(bool fiyxt, int azsgagqvcy, int kcdrxanqmcw, bool zlxagzixduridq, bool tozqftmfunh, int erbbciwyuj, int tovopwa, bool laumd, bool rvgqhdqnvhookva) {
	int qirvohvkz = 1145;
	if (1145 != 1145) {
		int jjdwkozptu;
		for (jjdwkozptu = 60; jjdwkozptu > 0; jjdwkozptu--) {
			continue;
		}
	}
	return true;
}

void vvjnhom::njdzejpyxqcefbulgoi(bool mfbccaeljvp, double opaegxoksi, string gkeojssw, double dzridg, double nolatwbsuokf, string vmitcvjzqikpewz) {
	bool upphqqnrfs = false;
	if (false != false) {
		int aabdg;
		for (aabdg = 78; aabdg > 0; aabdg--) {
			continue;
		}
	}
	if (false != false) {
		int jigxr;
		for (jigxr = 85; jigxr > 0; jigxr--) {
			continue;
		}
	}
	if (false == false) {
		int coqqqhos;
		for (coqqqhos = 43; coqqqhos > 0; coqqqhos--) {
			continue;
		}
	}
	if (false != false) {
		int lbgxdijn;
		for (lbgxdijn = 6; lbgxdijn > 0; lbgxdijn--) {
			continue;
		}
	}

}

void vvjnhom::znsllldevqbhgzlvalcq(int wyfpu, string hijgxqorzqqcs, bool wgvhdjwgfl, bool neehjxkodulhii, double bfrmbfoeqnpynse) {
	int pesltjhcgueyim = 5955;
	string ywmtwd = "oiudzbbrlfspwscexvtezlenomcogyluodeyqirpqwcwntnqrotzdawwx";
	if (string("oiudzbbrlfspwscexvtezlenomcogyluodeyqirpqwcwntnqrotzdawwx") != string("oiudzbbrlfspwscexvtezlenomcogyluodeyqirpqwcwntnqrotzdawwx")) {
		int rivbckm;
		for (rivbckm = 98; rivbckm > 0; rivbckm--) {
			continue;
		}
	}

}

double vvjnhom::pnhneepxknhuhyylcia(bool acnonpuxm, int jrzilqhbg, double igjrsiqnay, string djfnxypltll, int fcaww, int aulgwkkisxfauvf, int utwnobnizgj, double mzsgszea) {
	bool uorznhdtvizs = true;
	string iiqiubjhgnnlnos = "yiyugtqoqjohgxsdsibzvyvquzozkhuhjioygkvsfgcehnzknrrxwqpxxfbvnviradamvwbegizuix";
	int uviit = 6246;
	string fvmcaheuzmvgps = "aekctsotniybbntjkjxgitveurfdngnkwzm";
	string kmdkdr = "jbgqdxzmbdxqcjfexxkbgkehmmnjbdecngiznnjzyacxfhndkjgswltkebqtda";
	int qxfmjkzqhir = 326;
	string aeqrjpitxgbrd = "nmktirwwaiiwwndoinavkradcmeeqhsoobooramzpkhopggajlppkrprxmrozsixysklp";
	string vlsvt = "mmfozjlzfuuocxvhirrg";
	if (string("jbgqdxzmbdxqcjfexxkbgkehmmnjbdecngiznnjzyacxfhndkjgswltkebqtda") == string("jbgqdxzmbdxqcjfexxkbgkehmmnjbdecngiznnjzyacxfhndkjgswltkebqtda")) {
		int vq;
		for (vq = 57; vq > 0; vq--) {
			continue;
		}
	}
	if (string("aekctsotniybbntjkjxgitveurfdngnkwzm") == string("aekctsotniybbntjkjxgitveurfdngnkwzm")) {
		int sihiazpflx;
		for (sihiazpflx = 78; sihiazpflx > 0; sihiazpflx--) {
			continue;
		}
	}
	if (326 != 326) {
		int xvq;
		for (xvq = 8; xvq > 0; xvq--) {
			continue;
		}
	}
	if (string("jbgqdxzmbdxqcjfexxkbgkehmmnjbdecngiznnjzyacxfhndkjgswltkebqtda") == string("jbgqdxzmbdxqcjfexxkbgkehmmnjbdecngiznnjzyacxfhndkjgswltkebqtda")) {
		int dgvkow;
		for (dgvkow = 37; dgvkow > 0; dgvkow--) {
			continue;
		}
	}
	return 40280;
}

string vvjnhom::zuhhcmvehjumfxgwpctwtdf(int qkuhjl, int uehdohxenajykrr) {
	double fjxmj = 19343;
	string shipzrsxgzehl = "tiiudsmrlqguasbmksgdbxyrgnuzmwccaizdnkckqlmfulihdhqunccpyequlelinxyrgfploqbhrrdhczuniae";
	string jgsijwv = "jgzmweehchtwnrmnvznjrloqbbhztujmthhnykccpgyvxomzzoopwc";
	int kakehpcwtqpel = 1812;
	double hdaycdrb = 34335;
	double mahpnss = 35274;
	string mnvfsytpywrk = "pnjglpexfjcpflawhczrfcenmhgeqriynkicfklwycsyolxqnktlaidvxfgj";
	bool tepzkntdj = true;
	double mjfehkg = 22858;
	double nwhuhxzzobtkj = 19289;
	if (34335 == 34335) {
		int izyvldwh;
		for (izyvldwh = 58; izyvldwh > 0; izyvldwh--) {
			continue;
		}
	}
	if (string("pnjglpexfjcpflawhczrfcenmhgeqriynkicfklwycsyolxqnktlaidvxfgj") != string("pnjglpexfjcpflawhczrfcenmhgeqriynkicfklwycsyolxqnktlaidvxfgj")) {
		int jxilvjy;
		for (jxilvjy = 30; jxilvjy > 0; jxilvjy--) {
			continue;
		}
	}
	return string("zkrzigyxvntctdjaipl");
}

vvjnhom::vvjnhom() {
	this->utavasgnltzhuxpklkfjd(true, 548, 1383, false, true, 565, 1025, false, false);
	this->njdzejpyxqcefbulgoi(false, 32690, string("iwlzrcrnxrxyndvwnqflxxfogjjajmxrzxcfnvbtoxjdkdzltovkf"), 26886, 25205, string("wiqjfdjaomaqozsjoxfyymezjcdlhnraoiiwucdguwsixzcfhswggdzdpytblowahzrlddajcpprujfwzgcufamvccoqn"));
	this->znsllldevqbhgzlvalcq(4185, string("fz"), true, false, 21971);
	this->pnhneepxknhuhyylcia(false, 1847, 4621, string("jqzsutzt"), 1720, 9674, 909, 10843);
	this->zuhhcmvehjumfxgwpctwtdf(4018, 5354);
	this->dlqitbzcchhstuw(string("lghtbrwdhfeahhhisqtlgdqkkvrudaxjcuqgtkbnovcngpfrdngjpmxemyx"), 3814, string("peqhbwqetrdciummliltlnvvxxpxmyzkxtfqvitjmagvcovxwchdkezswltutkxqkofaeqlafpsebucxlwenhz"), string("klsyiawugqiyrqgoxuhonkxcndsdynuilncekijpnlafsjaw"), 12975, 1316, false);
	this->jffkthlotfmfxfq(12073, 39215, 21767, 13196);
	this->nakvwtvdaubyyb(false, string("dimhdtdctrkmlydxabfoyzdbfnxtcgbblclgioivonsbhacotkitestobtgmdkrycbgdysthxofiqor"), 1177, string("aeqicgthvhaqvjuqghgylzdvqlkyqjxfudjqbskrgskhkzdlkwe"), 11394);
	this->gtufnctaydlljkkztv(string("kzyqbtgybaumtgeyxax"), false);
	this->pqzwrzhlbtjhmrkdtnxzai(string("rcpsjqmnyldgodfifglznrlliokcpxevbcupvrzyjnxktu"), string("ktfaglbycloiluqoxxuarbpjelsqtvwiuhuuhdpetoqnhjojbszfglsijsfez"));
	this->nqfrlnqeifupt(false, 2842, 4490, 44666, false, 11527);
	this->xpmahomjrwgdrcnxgwmkggsf(29450, 8229, 3219, true, string("dgalctcqlzdbvqvqsporyhbzswemrsnuuncsevsaogvtsbfluxpcejfyaqyucy"), 1519, true);
	this->yigwiqaluszkdlkr(10888, 462, 5194);
	this->rnpkouwllcdjlbdezn(27998, 8599, string("kbgzfyybnncgt"), true, string("fwchweyodbyvxqujxuzptmfdkhdarjychluhlhmcspjhcpfbgsxfhlqgsjemqftaajnkeumdcbwxfkpjtcuuj"), 403, string("vqepeqbgmnhrruvyiuxpslpjfpvccoscvbszxx"), 4132, 1343);
	this->nsdorttkzizfwdwn();
	this->fhhgabotkryttpatp(string("iijweiytsmqqvapdozjpjmwldptndxhhdrqhkiceoezsmkkufpqncpimfydipcodjuprqartfzfkxklislqybqjfxfaf"), string("pgnudjtqnxwhzvwrlcb"), 21312, true, 11186, string("rjqxlningxabrixwlrvfraoxstyxypdketmrinlxthitjeaposwdqbra"), 918);
	this->fjhdmzcdefx();
}


static bool peja;
static bool switchbrak;
static bool safdsfs;



void jolt(CUserCmd *pCmd, bool &bSendPacket)
{
	// (LastLBYUpdateTime + 1 - (GetLatency() * 2) < flServerTime)
	IClientEntity* pLocal = hackManager.pLocal();
	float value = pLocal->GetLowerBodyYaw();
	float flServerTime = (float)(pLocal->GetTickBase()  * Interfaces::Globals->interval_per_tick);
	float moving = abs(pLocal->GetVelocity().Length());
	if (bSendPacket)
	{
		if (moving > 30)
		{
			pCmd->viewangles.y = value + (101 + rand() % 20);
		}
		else
		{
			pCmd->viewangles.y = (value - 90) - rand() % 110;
		}
	}

}





void LowerBodyFake(CUserCmd *pCmd, bool &bSendPacket)
{

	static bool wilupdate;
	static float LastLBYUpdateTime = 0;
	IClientEntity* pLocal = hackManager.pLocal();
	float server_time = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;

	static int ChokedPackets = -1;
	ChokedPackets++;
	if (ChokedPackets < 1)
	{
		

		if (server_time >= LastLBYUpdateTime)
		{
			LastLBYUpdateTime = server_time + 1.009f;
			wilupdate = true;
			pCmd->viewangles.y += hackManager.pLocal()->GetLowerBodyYaw() - RandomInt(80, 150, -90, -160);
		}
		else
		{
			wilupdate = false;
			pCmd->viewangles.y += hackManager.pLocal()->GetLowerBodyYaw() + RandomInt(-160, -90, 90, 170);
		}
		bSendPacket = true;
	}

}
static bool dir = false;
void DoRealAA(CUserCmd* pCmd, IClientEntity* pLocal, bool& bSendPacket)
{


	float memeantilby = Menu::Window.aabuild.moveantilby.GetValue();


	int y2 = 90;
	int spinspeed = 65;


	if (GetAsyncKeyState(Menu::Window.MiscTab.keybasedaaleft.GetKey())) dir = false;
	else if (GetAsyncKeyState(Menu::Window.MiscTab.keybasedaaright.GetKey())) dir = true;
	float server_time = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;

	bool wilupdate;
	static bool switch2;
	Vector oldAngle = pCmd->viewangles;
	float oldForward = pCmd->forwardmove;
	float oldSideMove = pCmd->sidemove;

	if (!Menu::Window.RageBotTab.AntiAimEnable.GetState())
		return;


	float rando = rand() % 1000;



	if (pLocal->GetFlags() && pLocal->GetVelocity().Length() > 2)
	{


		switch (Menu::Window.RageBotTab.movingY.GetIndex())
		{
		case 1:
			//backwards
			pCmd->viewangles.y -= 180;
			break;
		case 2:
			if (switch2)
				pCmd->viewangles.y -= (160 - rand() % 25);
			else
				pCmd->viewangles.y += (160 + rand() % 25);
			switch2 = !switch2;
			break;
		case 3:
		{

			if (rando < 499)
			{
				if (switch2)
					pCmd->viewangles.y = 180;
				else
					pCmd->viewangles.y = 30;
				switch2 = !switch2;
			}
			else if (rando > 500)
			{
				if (switch2)
					pCmd->viewangles.y = 90;
				else
					pCmd->viewangles.y = -90;
				switch2 = !switch2;
			}
			else
				pCmd->viewangles.y = 80;

		}
		break;
		case 4:
		{
			pCmd->viewangles.y = (145 + rand() % 70);
		}
		case 5:
			pCmd->viewangles.y = pLocal->GetLowerBodyYaw() - 179 + rand() % 160;
			break;
		case 6:
		{
			if (NextLBYUpdate())
			{
				pCmd->viewangles.y -= 120;
			}
			else
			{
				pCmd->viewangles.y += 120;
			}
		}
		break;
		case 7:
		{
			if (dir)
			{
				pCmd->viewangles.y += 90;
			}
			else if (!dir)
			{
				pCmd->viewangles.y -= 90;
			}
		}
		break;
		case 8:
		{
			y2 += spinspeed;

			if (y2 >= 179)
				y2 = -179;

			pCmd->viewangles.y = y2;


		}
		break;
		case 9:
		{

			if (pLocal->IsMoving())
			{
				y2 += spinspeed;

				if (y2 >= -120)
					y2 = 120;

				pCmd->viewangles.y = y2;
			}
			else 
			{
				if (switch2)
					pCmd->viewangles.y -= 150;
				else
					pCmd->viewangles.y += 150;
				switch2 = !switch2;
			}

		}
		break;
		}
	}
	else
	{
		switch (Menu::Window.RageBotTab.AntiAimYaw.GetIndex())
		{
		case 1:
			//backwards
			pCmd->viewangles.y -= 180;
			break;
		case 2:
			if (switch2)
				pCmd->viewangles.y += (160 + rand() % 25);
			else
				pCmd->viewangles.y -= (160 - rand() % 25);
			switch2 = !switch2;
			break;
		case 3:
		{

			if (rando < 499)
			{
				if (switch2)
					pCmd->viewangles.y -= 170;
				else
					pCmd->viewangles.y += 30;
				switch2 = !switch2;
			}
			else if (rando > 500)
			{
				if (switch2)
					pCmd->viewangles.y += 90;
				else
					pCmd->viewangles.y -= 90;
				switch2 = !switch2;
			}
			else
				pCmd->viewangles.y -= 80;

		}
		break;
		case 4:
		{
			pCmd->viewangles.y -= (145 + rand() % 70);
		}
		case 5:
			pCmd->viewangles.y -= (pLocal->GetLowerBodyYaw() - 179) + rand() % 160;
			break;
		case 6:
		{
			if (server_time >= LastLBYUpdateTime)
			{
				LastLBYUpdateTime = server_time + 1.090f;
				wilupdate = true;
				pCmd->viewangles.y += hackManager.pLocal()->GetLowerBodyYaw() - RandomInt(80, 150, -90, -160);
			}
			else
			{
				wilupdate = false;
				pCmd->viewangles.y += hackManager.pLocal()->GetLowerBodyYaw() + RandomInt(-160, -90, 90, 170);
			}
			bSendPacket = true;

		}
		break;
		case 7:
		{
			if (dir)
			{
				pCmd->viewangles.y += 90;
			}
			else if (!dir)
			{
				pCmd->viewangles.y -= 90;
			}
		}
		break;
		case 8:
		{
			y2 += spinspeed;

			if (y2 >= 179)
				y2 = -179;

			pCmd->viewangles.y = y2;


		}
		break;
		case 9:
		{

			if (pLocal->IsMoving())
			{
				y2 += spinspeed;

				if (y2 >= -119)
					y2 = 119;

				pCmd->viewangles.y = y2;
			}
			else 
			{
				if (switch2)
					pCmd->viewangles.y += 150;
				else
					pCmd->viewangles.y -= 150;
				switch2 = !switch2;
			}

		}
		break;
		}
	}





}




void DoFakeAA(CUserCmd* pCmd, bool& bSendPacket, IClientEntity* pLocal)
{
	static int ChokedPackets = -1;
	ChokedPackets++;
	static bool switch2;
	Vector oldAngle = pCmd->viewangles;
	float oldForward = pCmd->forwardmove;
	float oldSideMove = pCmd->sidemove;
	IClientEntity* pEntity;
	int flip = (int)floorf(Interfaces::Globals->curtime / 1.1) % 2;
	static bool bFlipYaw;
	static bool wilupdate;



	int y2 = -90;
	int spinspeed = 66;


	float memeantilby = Menu::Window.aabuild.moveantilby.GetValue();



	if (!Menu::Window.RageBotTab.AntiAimEnable.GetState())
		return;


	switch (Menu::Window.RageBotTab.FakeYaw.GetIndex())
	{
	case 0: { /* Muslims suck */ } break;
	case 1:
		//backwards
		pCmd->viewangles.y -= 180.000000;
		break;
	case 2:
	{
		if (switch2)
		{
			pCmd->viewangles.y = -150;
			ChokedPackets = 1;

		}
		else
		{
			pCmd->viewangles.y = 150;
			ChokedPackets = 1;
		}
		switch2 = !switch2;
	}
	break;
	case 3:
	{
		int rando = rand() % 750;
		if (rando < 375)
		{
			if (switch2)
				pCmd->viewangles.y -= (60 - rand() % 20);
			else
				pCmd->viewangles.y -= (120 - rand() % 20);
			switch2 = !switch2;
		}
		else if (rando > 375)
		{
			if (switch2)
				pCmd->viewangles.y += (60 + rand() % 20);
			else
				pCmd->viewangles.y += (120 + rand() % 20);
			switch2 = !switch2;
		}
		else
			pCmd->viewangles.y -= (180 + rand() % 45) - rand() % 20;
	}
	break;
	case 4:
	{

		pCmd->viewangles.y = (pLocal->GetLowerBodyYaw() + 140 + rand() % 123) - rand() % 20;
	}
	break;
	case 5:
	{
		if (NextLBYUpdate)
		pCmd->viewangles.y = (pLocal->GetLowerBodyYaw() - 150) - rand() % 60;
		else
		pCmd->viewangles.y = (pLocal->GetLowerBodyYaw() + 150) + rand() % 60;
	}
		break;
	case 6:
	{
		if (switch2)
		{
			pCmd->viewangles.y = -80.00 + rand() % 150;
		}
		else
			pCmd->viewangles.y = 80.00 - rand() % 150;

	}
	break;
	case 7:
	{
		y2 += spinspeed;

		if (y2 >= 175)
			y2 = -175;

		pCmd->viewangles.y = y2;
	}
	break;
	case 8:
	{
		int rnd = rand() % 70;
		static bool cFlipYaw;
		if (rnd < 20) rnd = 20;
		static bool cFlipYaw2;
		int rndb = rand() % 170;



		y2 += spinspeed;

		if (y2 >= 119)
			y2 = -119;

		if (rndb < 25) rnd = 25;

		if (pLocal->GetHealth() > 45)
		{
			if (switch2)
				pCmd->viewangles.y = hackManager.pLocal()->GetEyeAnglesXY()->y - (60 - rndb);
			else
				pCmd->viewangles.y = hackManager.pLocal()->GetEyeAnglesXY()->y + (140 + rndb);
			switch2 = !switch2;
		}
		else if (pLocal->GetHealth() <= 45 && !IsAbleToShoot)
		{
			pCmd->viewangles.y = hackManager.pLocal()->GetEyeAnglesXY()->y + cFlipYaw2 ? y2 : y2 + 60;

		}
		else if (CanOpenFire)
		{
			if (NextLBYUpdate())
			{
				float transgender = pLocal->GetLowerBodyYaw() - 119.5f;

				static int y2 = transgender;
				int spinBotSpeedFast = 35 + rand() % 10;
				//
				y2 += spinBotSpeedFast;

				if (y2 >= pLocal->GetEyeAnglesXY()->y - 35)
					y2 = pLocal->GetEyeAnglesXY()->y - 90;

				pCmd->viewangles.y = y2;


			}
			else 
			{
				float transgender = pLocal->GetLowerBodyYaw() - 60;
				static int y2 = transgender;
				int spinBotSpeedFast = 45 + rand() % 15;

				y2 += spinBotSpeedFast;

				if (y2 >= pLocal->GetEyeAnglesXY()->y - 55)
					y2 = pLocal->GetEyeAnglesXY()->y - 100;

				pCmd->viewangles.y = y2;
			}
		}
		
		else
		{
			if (switch2)
				pCmd->viewangles.y = hackManager.pLocal()->GetEyeAnglesXY()->y + 160 + rnd;
			else
				pCmd->viewangles.y = hackManager.pLocal()->GetEyeAnglesXY()->y - 130 - rnd;
			switch2 = !switch2;
		}



	}
	break;
	case 9:
		pCmd->viewangles.y + 180.000000; break;
	case 10:
		pCmd->viewangles.y + 0; break;





	}



}


void CRageBot::DoAntiAim(CUserCmd *pCmd, bool &bSendPacket)
{
	IClientEntity* pLocal = hackManager.pLocal();
	static int ChokedPackets = -1;
	ChokedPackets++;
	if ((pCmd->buttons & IN_USE) || pLocal->GetMoveType() == MOVETYPE_LADDER)
		return;

	if (IsAimStepping || pCmd->buttons & IN_ATTACK)
		return;

	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());
	if (pWeapon)
	{
		CSWeaponInfo* pWeaponInfo = pWeapon->GetCSWpnData();

		if (!GameUtils::IsBallisticWeapon(pWeapon))
		{
			if (!CanOpenFire() || pCmd->buttons & IN_ATTACK2)
				return;

		}
	}

	int flip = (int)floorf(Interfaces::Globals->curtime / 1.1) % 2;
	static bool bFlipYaw;
	static bool wilupdate;
	float flInterval = Interfaces::Globals->interval_per_tick;
	float flTickcount = pCmd->tick_count;
	float flTime = flInterval * flTickcount;
	if (std::fmod(flTime, 1) == 0.f)
		bFlipYaw = !bFlipYaw;
	static float LastLBYUpdateTime = 0;



	FakeWalk(pCmd, bSendPacket);

	switch (Menu::Window.RageBotTab.AntiAimPitch.GetIndex())
	{
	case 0:
		break;
	case 1:
		pCmd->viewangles.x = 88.f;
		break;
	case 2:
		pCmd->viewangles.x = -88.f;
		break;
	case 3:
		pCmd->viewangles.x = 1073.000000;
		break;
	case 4:
		pCmd->viewangles.x = -1073.000000;
		break;
	case 5: // stay freaky
	{
		pCmd->viewangles.x = 75.000000 + rand() % 17;


	}

	break;
	case 6:
	{
		float moving = abs(pLocal->GetVelocity().Length() >= 2);
		if (bSendPacket)
		{
			if (abs(pLocal->GetVelocity().Length() < 2))
			{
				pCmd->viewangles.x = 88.f;
			}
			else if (!moving)
			{
				pCmd->viewangles.x = 160.f;
				pCmd->viewangles.z = 4050.f;
			}
			else
			{
				pCmd->viewangles.x = 1060.f;
				pCmd->viewangles.z = -4050.f;
			}
		}
		else
		{
			if (pLocal->IsMoving() && abs(pLocal->GetVelocity().Length() < 2))
			{
				pCmd->viewangles.x = 1080.f;
				pCmd->viewangles.z = 0.f;
			}
			else if (!moving)
			{
				pCmd->viewangles.x = 163.f;
				pCmd->viewangles.z = 320.f;
			}
			else	pCmd->viewangles.x = 1090.f;


		}
	}


	}

	if (Menu::Window.RageBotTab.Active.GetState() && Menu::Window.RageBotTab.AntiAimEnable.GetState())
	{

		static bool respecc = false;
		float antilby = Menu::Window.aabuild.Antilby.GetValue();
		float moveantilby = Menu::Window.aabuild.moveantilby.GetValue();

		if (Menu::Window.aabuild.FakeLagTyp.GetIndex() != 0)
		{

			static int fake = 0;

			bool onground;
			if (pLocal->IsScoped())
				onground = true;
			else
				onground = false;

			if (Menu::Window.aabuild.FakeLagTyp.GetIndex() == 0)
			{
				int tickstochoke = 1;
				//	tickstochoke = std::min<int>(Menu::Window.aabuild.FakeLagChoke.GetValue(), 13);
				if (ChokedPackets >= 0)
				{
					fake = tickstochoke + 1;
				}


				if (fake < tickstochoke)
				{
					bSendPacket = false;

					DoRealAA(pCmd, pLocal, bSendPacket);

					if (!respecc)
					{
						if (fake == 0 && pLocal->GetVelocity().Length2D() <= 1)
						{

							if (NextLBYUpdate())
							{

								pCmd->viewangles.y += antilby;

								pCmd->viewangles.y -= antilby;
								{
									pCmd->viewangles.y += antilby;
								}
							}
						}
						else if (fake == 0 && pLocal->GetVelocity().Length2D() > 1)
						{

							if (NextMovingLBYUpdate())
							{

								pCmd->viewangles.y += moveantilby;

								pCmd->viewangles.y -= moveantilby;
								{
									pCmd->viewangles.y += moveantilby;
								}
							}

						}
					}
					fake++;

				}
				else
				{
					bSendPacket = true;
					DoFakeAA(pCmd, bSendPacket, pLocal);
					fake = 0;
				}
				if (!bSendPacket)
					++ChokedPackets;
				else
					ChokedPackets = 0;

				if (ChokedPackets > 0)
					bSendPacket = true;


			}
			else if (Menu::Window.aabuild.FakeLagTyp.GetIndex() == 1)
			{
				int tickstochoke = 4;
				//	tickstochoke = std::min<int>(Menu::Window.aabuild.FakeLagChoke.GetValue(), 13);
				if (ChokedPackets >= 3)
				{
					fake = tickstochoke + 1;
				}


				if (fake < tickstochoke)
				{
					bSendPacket = false;

					DoRealAA(pCmd, pLocal, bSendPacket);

					if (!respecc)
					{
						if (fake == 0 && pLocal->GetVelocity().Length2D() <= 1)
						{

							if (NextLBYUpdate())
							{

								pCmd->viewangles.y += antilby;

								pCmd->viewangles.y -= antilby;
								{
									pCmd->viewangles.y += antilby;
								}
							}
						}
						else if (fake == 0 && pLocal->GetVelocity().Length2D() > 1)
						{

							if (NextMovingLBYUpdate())
							{

								pCmd->viewangles.y += moveantilby;

								pCmd->viewangles.y -= moveantilby;
								{
									pCmd->viewangles.y += moveantilby;
								}
							}

						}
					}
					fake++;

				}
				else
				{
					bSendPacket = true;
					DoFakeAA(pCmd, bSendPacket, pLocal);
					fake = 0;
				}
				if (!bSendPacket)
					++ChokedPackets;
				else
					ChokedPackets = 0;

				if (ChokedPackets > 3)
					bSendPacket = true;

			}
			else if (Menu::Window.aabuild.FakeLagTyp.GetIndex() == 2)
			{
				int tickstochoke = 9;

				//	tickstochoke = std::min<int>(Menu::Window.aabuild.FakeLagChoke.GetValue(), 13);
				if (ChokedPackets >= 8)
				{
					fake = tickstochoke + 1;
				}


				if (fake < tickstochoke)
				{
					bSendPacket = false;

					DoRealAA(pCmd, pLocal, bSendPacket);

					if (!respecc)
					{
						if (fake == 0 && pLocal->GetVelocity().Length2D() <= 1)
						{

							if (NextLBYUpdate())
							{

								pCmd->viewangles.y += antilby;

								pCmd->viewangles.y -= antilby;
								{
									pCmd->viewangles.y += antilby;
								}
							}
						}
						else if (fake == 0 && pLocal->GetVelocity().Length2D() >= 1)
						{

							if (NextMovingLBYUpdate())
							{

								pCmd->viewangles.y += moveantilby;

								pCmd->viewangles.y -= moveantilby;
								{
									pCmd->viewangles.y += moveantilby;
								}
							}

						}
					}
					fake++;

				}
				else
				{
					bSendPacket = true;
					DoFakeAA(pCmd, bSendPacket, pLocal);
					fake = 0;
				}


				if (!bSendPacket)
					++ChokedPackets;
				else
					ChokedPackets = 0;

				if (ChokedPackets > 8)
					bSendPacket = true;
			}
			else if (Menu::Window.aabuild.FakeLagTyp.GetIndex() == 3)
			{
				int tickstochoke = 12;

				//	tickstochoke = std::min<int>(Menu::Window.aabuild.FakeLagChoke.GetValue(), 13);
				if (ChokedPackets >= 11)
				{
					fake = tickstochoke + 1;
				}


				if (fake < tickstochoke)
				{
					bSendPacket = false;

					DoRealAA(pCmd, pLocal, bSendPacket);
					if (respecc)
					{
						if (fake == 0 && pLocal->GetVelocity().Length2D() <= 0.5)
						{
							if (fake == 0 && pLocal->GetVelocity().Length2D() <= 1)
							{

								if (NextLBYUpdate())
								{

									pCmd->viewangles.y += antilby;

									pCmd->viewangles.y -= antilby;
									{
										pCmd->viewangles.y += antilby;
									}
								}
							}
							else if (fake == 0 && pLocal->GetVelocity().Length2D() > 1)
							{

								if (NextMovingLBYUpdate())
								{

									pCmd->viewangles.y += moveantilby;

									pCmd->viewangles.y -= moveantilby;
									{
										pCmd->viewangles.y += moveantilby;
									}
								}

							}
						}
						fake++;

					}
					else
					{
						bSendPacket = true;
						DoFakeAA(pCmd, bSendPacket, pLocal);
						fake = 0;
					}


					if (!bSendPacket)
						++ChokedPackets;
					else
						ChokedPackets = 0;

					if (ChokedPackets >= 11)
						bSendPacket = true;
				}


			}


			if (flipAA)
			{
				pCmd->viewangles.y - 135;
			}
		}

	}
}

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class gqzcjfp {
public:
	double ovadlhbaw;
	int foaslutgejwjsen;
	gqzcjfp();
	bool jwgcslafburafnzzmkivkgzks();
	int axeahisvzossynpinym(bool tyjhzeeafjxefl);
	bool ehtpnoctzpzlhjweg(int tnyouifjttur, int tzlbmwczzweu, string ntzwvkzmuienm, string qhrnubjqgr, bool tzsvyc, int ajqesgfhmw, double lzphfj);
	double mccjrclcyhkvxpsucymnkn(double udfcrzsh, int wcdfube, bool iovdbbhioxl);
	double fbusfszgsggybbdlz(bool dgutgnciuef, int rosdsyoimomcsp, double itexcrkdzpk, bool xssocqx, double fpohwjwzdsfiwsw, double tiapuhjp, double zutmflsxxoetc, string ouunquxroptwo, double sikvvuljgstftgf, string tqlkmvihqqpyp);
	bool jxdqeyrqxqrbsmbpxjgyfiwu(double kecobrnzkkgoj, string ryppkofwm, string knyzxih, int jtqhfzorysuscx, bool hjwpavbob);

protected:
	bool ejdqp;
	int ypcmxcdramljo;

	int vlppyqabgkjlc(bool yhuwfog, string axqmjxskpsysr, bool gteiz, int wirfrpdb, double dvzvw, bool rvnzrlprxaoo, double aswjt);
	string ebdmmciwthpnykvdddebpl(double ynlkzflperk, bool yqbvo, int ejzoklehogdy, int mrmdt, bool jcfok, bool xrjbuvdvrnv, bool kwirez, double qsdfwchbuv);

private:
	string xkpbaavbxqojo;
	bool mclascq;
	string bsmaf;
	string tlvaidixrkpi;
	double ahrithtnom;

	int cgbycnzzquhfiadcpwd();
	int bvbwboyecqqnm(string vzpxcojoffihj, int atqmcipycbeoe, bool qcealkkt, int fbxfbtqgzxqeedw, bool kulaovqizum, string ragnqvhfmoaxkwu, double xtowfo, string litew, bool rhceyhtunytxj, int ducqzsandtgre);
	int xufnvfskduhjugzqjq(bool ohoupwdehl, int ayirjnwzoefxrva, int oiwgnc, string iggfeahqks, int anrfpdcj, bool rrdmggxvzomsy, string hrermwjxvvif, double leojskvj);
	void jlokjxlkejbaawxhl();
	bool rxpxeodogtmyzbaofxi(double amqexe, int egzitatp, double cjlcpvej);
	int xzwnfccfjfjywi(bool jlpiyk, int cpmmmij, string ejixshxymg, double varjexphbqcmdtd, int xzorudwswgpep, double lrfkqay, bool hopagpajabgdz, double hsistarfemt, string gtfzjtv);
	int zbqoyedqfsnsqck(string ikhod, bool ftpifwygoqedqgg, double xdapmhppje, string pviflbnjonezhz, bool kgctsawrugskmp, bool jgkcrxlpwvkohh);
	int uhxupihpctjnxjrbhb(double xifqylyowzfjn);
	void olmvbbiqfg(int mgmic);
	bool tcmyxcwccz(bool xsgzmigs);

};



int gqzcjfp::cgbycnzzquhfiadcpwd() {
	string ujwcphxvfgnoa = "dtdsuwhomc";
	if (string("dtdsuwhomc") == string("dtdsuwhomc")) {
		int vfh;
		for (vfh = 68; vfh > 0; vfh--) {
			continue;
		}
	}
	if (string("dtdsuwhomc") == string("dtdsuwhomc")) {
		int iyisyyx;
		for (iyisyyx = 36; iyisyyx > 0; iyisyyx--) {
			continue;
		}
	}
	if (string("dtdsuwhomc") != string("dtdsuwhomc")) {
		int pjyxn;
		for (pjyxn = 54; pjyxn > 0; pjyxn--) {
			continue;
		}
	}
	if (string("dtdsuwhomc") == string("dtdsuwhomc")) {
		int etj;
		for (etj = 29; etj > 0; etj--) {
			continue;
		}
	}
	if (string("dtdsuwhomc") != string("dtdsuwhomc")) {
		int auksdbpusd;
		for (auksdbpusd = 57; auksdbpusd > 0; auksdbpusd--) {
			continue;
		}
	}
	return 29080;
}

int gqzcjfp::bvbwboyecqqnm(string vzpxcojoffihj, int atqmcipycbeoe, bool qcealkkt, int fbxfbtqgzxqeedw, bool kulaovqizum, string ragnqvhfmoaxkwu, double xtowfo, string litew, bool rhceyhtunytxj, int ducqzsandtgre) {
	bool ozftwktixhhlqul = true;
	int qpmotxvgrfkzdhr = 562;
	double hmscnysiebkwf = 58919;
	bool iaujwm = true;
	int swrpjactgni = 512;
	string hriwth = "lsiaozxvtfojahuwxfs";
	bool glelwzhlqqytz = false;
	double dvxoaidjhjuf = 21720;
	double qcqjspnta = 35639;
	int voffvsoekngl = 3061;
	if (562 == 562) {
		int kojkbl;
		for (kojkbl = 74; kojkbl > 0; kojkbl--) {
			continue;
		}
	}
	if (35639 != 35639) {
		int nybu;
		for (nybu = 90; nybu > 0; nybu--) {
			continue;
		}
	}
	return 79430;
}

int gqzcjfp::xufnvfskduhjugzqjq(bool ohoupwdehl, int ayirjnwzoefxrva, int oiwgnc, string iggfeahqks, int anrfpdcj, bool rrdmggxvzomsy, string hrermwjxvvif, double leojskvj) {
	string bqekfwfit = "kndinezyrhcklxajxbkkhqyuppzqeijewvyovbitxfsrvjelhdzdwurnnrkgsqizwavkjijfwnscftniyaayxtinyiqru";
	bool avetjhmyrgwvjok = true;
	if (true == true) {
		int wqkeg;
		for (wqkeg = 13; wqkeg > 0; wqkeg--) {
			continue;
		}
	}
	if (true == true) {
		int ykzbksnr;
		for (ykzbksnr = 96; ykzbksnr > 0; ykzbksnr--) {
			continue;
		}
	}
	if (true != true) {
		int rdvnol;
		for (rdvnol = 13; rdvnol > 0; rdvnol--) {
			continue;
		}
	}
	if (string("kndinezyrhcklxajxbkkhqyuppzqeijewvyovbitxfsrvjelhdzdwurnnrkgsqizwavkjijfwnscftniyaayxtinyiqru") == string("kndinezyrhcklxajxbkkhqyuppzqeijewvyovbitxfsrvjelhdzdwurnnrkgsqizwavkjijfwnscftniyaayxtinyiqru")) {
		int epc;
		for (epc = 96; epc > 0; epc--) {
			continue;
		}
	}
	if (string("kndinezyrhcklxajxbkkhqyuppzqeijewvyovbitxfsrvjelhdzdwurnnrkgsqizwavkjijfwnscftniyaayxtinyiqru") == string("kndinezyrhcklxajxbkkhqyuppzqeijewvyovbitxfsrvjelhdzdwurnnrkgsqizwavkjijfwnscftniyaayxtinyiqru")) {
		int pbqx;
		for (pbqx = 76; pbqx > 0; pbqx--) {
			continue;
		}
	}
	return 80095;
}

void gqzcjfp::jlokjxlkejbaawxhl() {
	string bqkxdljjlbzr = "pqiqseqnlaenxuwsmjfqqrlweikifzoswxgateyuekapxxrbmqgcgbzifoibpjwlmsnnruqjzozfzt";
	double iglgg = 15696;
	double xlxvljtv = 34192;
	double wqcyjkl = 76605;
	int vijwrihuqhrgzmz = 7386;
	string yikaubkdgdfoln = "jzpwcfkcjakozofmsxbtyc";
	bool hihtp = true;
	int zesipun = 8029;
	int pkpkcaapumw = 921;
	string xxcbvwcrorptaid = "ftegnlvowglzetxbcorybjjjpecnluopfll";
	if (string("pqiqseqnlaenxuwsmjfqqrlweikifzoswxgateyuekapxxrbmqgcgbzifoibpjwlmsnnruqjzozfzt") == string("pqiqseqnlaenxuwsmjfqqrlweikifzoswxgateyuekapxxrbmqgcgbzifoibpjwlmsnnruqjzozfzt")) {
		int qhl;
		for (qhl = 40; qhl > 0; qhl--) {
			continue;
		}
	}
	if (string("ftegnlvowglzetxbcorybjjjpecnluopfll") == string("ftegnlvowglzetxbcorybjjjpecnluopfll")) {
		int rnuokov;
		for (rnuokov = 2; rnuokov > 0; rnuokov--) {
			continue;
		}
	}
	if (921 != 921) {
		int egb;
		for (egb = 15; egb > 0; egb--) {
			continue;
		}
	}

}

bool gqzcjfp::rxpxeodogtmyzbaofxi(double amqexe, int egzitatp, double cjlcpvej) {
	int popwfllss = 3265;
	bool zpqvlstukoxiwlk = true;
	int rxrkyljh = 41;
	bool vcweodiz = false;
	if (3265 != 3265) {
		int vl;
		for (vl = 98; vl > 0; vl--) {
			continue;
		}
	}
	if (false != false) {
		int ga;
		for (ga = 16; ga > 0; ga--) {
			continue;
		}
	}
	if (true != true) {
		int srbrmszobw;
		for (srbrmszobw = 84; srbrmszobw > 0; srbrmszobw--) {
			continue;
		}
	}
	if (41 == 41) {
		int wlo;
		for (wlo = 86; wlo > 0; wlo--) {
			continue;
		}
	}
	if (false == false) {
		int xtznlfxy;
		for (xtznlfxy = 4; xtznlfxy > 0; xtznlfxy--) {
			continue;
		}
	}
	return true;
}

int gqzcjfp::xzwnfccfjfjywi(bool jlpiyk, int cpmmmij, string ejixshxymg, double varjexphbqcmdtd, int xzorudwswgpep, double lrfkqay, bool hopagpajabgdz, double hsistarfemt, string gtfzjtv) {
	int kwtyeoy = 2530;
	int igiswzdrcsj = 642;
	bool wuqscfqagfbk = true;
	double vxywrn = 41594;
	double fkkpmhjsaymijo = 8101;
	double wtifntxzqjjbmt = 27855;
	string wnghz = "cbxmbxiuajwnpmxaebigimlbmgqidfbtdiaurwkajdizdjblyhitcimevwstsibwlblu";
	double bprttt = 4446;
	bool qwbkuser = false;
	if (true != true) {
		int lrfbwrjlxt;
		for (lrfbwrjlxt = 39; lrfbwrjlxt > 0; lrfbwrjlxt--) {
			continue;
		}
	}
	if (27855 == 27855) {
		int xqxmub;
		for (xqxmub = 11; xqxmub > 0; xqxmub--) {
			continue;
		}
	}
	return 4357;
}

int gqzcjfp::zbqoyedqfsnsqck(string ikhod, bool ftpifwygoqedqgg, double xdapmhppje, string pviflbnjonezhz, bool kgctsawrugskmp, bool jgkcrxlpwvkohh) {
	int jezxfn = 1687;
	string qhdcaenqhz = "veysdytmupkyjjjjshbnxfnpeckqsvrkrrlebeycgbtmikxfzcmxgcbvzpspiwxiabylia";
	double zzqfb = 15000;
	if (15000 == 15000) {
		int yeslhvzzue;
		for (yeslhvzzue = 54; yeslhvzzue > 0; yeslhvzzue--) {
			continue;
		}
	}
	if (1687 == 1687) {
		int zpm;
		for (zpm = 9; zpm > 0; zpm--) {
			continue;
		}
	}
	return 34703;
}

int gqzcjfp::uhxupihpctjnxjrbhb(double xifqylyowzfjn) {
	int llbcxerae = 488;
	double hrkkzugymynimyi = 52021;
	string nfqgvmxso = "zxnotszhfsdq";
	if (488 != 488) {
		int xkyhy;
		for (xkyhy = 88; xkyhy > 0; xkyhy--) {
			continue;
		}
	}
	if (488 != 488) {
		int ni;
		for (ni = 96; ni > 0; ni--) {
			continue;
		}
	}
	if (52021 == 52021) {
		int aqinpdzts;
		for (aqinpdzts = 34; aqinpdzts > 0; aqinpdzts--) {
			continue;
		}
	}
	if (string("zxnotszhfsdq") != string("zxnotszhfsdq")) {
		int gxbmtbz;
		for (gxbmtbz = 24; gxbmtbz > 0; gxbmtbz--) {
			continue;
		}
	}
	return 2915;
}

void gqzcjfp::olmvbbiqfg(int mgmic) {
	double eyknljspv = 3660;
	int tawuuajakorrjky = 7613;
	bool ghbcbycvdosw = false;
	bool pxgse = true;
	string dcfrmvo = "ebfdphigoylwpmrbtxcvzdszewxiafoevnsvxdjpywuwnsvwjtagibppxzvdvntkfppcydtdpyfrzydajfszgscfnfn";
	string rzqkqlq = "ohdttdcfyygatrwgcabhpbcivzfllpvlxbgerigxllxifjzkmyvfgjmmdvzotkafkwtztyligafvcsqffomf";
	double tvipudeq = 21618;
	double gvqqzvckb = 27379;
	int mqqxlgnglvdpyji = 2963;
	bool vnrmwe = false;
	if (21618 == 21618) {
		int kowbje;
		for (kowbje = 89; kowbje > 0; kowbje--) {
			continue;
		}
	}
	if (false == false) {
		int lgu;
		for (lgu = 30; lgu > 0; lgu--) {
			continue;
		}
	}
	if (string("ohdttdcfyygatrwgcabhpbcivzfllpvlxbgerigxllxifjzkmyvfgjmmdvzotkafkwtztyligafvcsqffomf") != string("ohdttdcfyygatrwgcabhpbcivzfllpvlxbgerigxllxifjzkmyvfgjmmdvzotkafkwtztyligafvcsqffomf")) {
		int vygbalh;
		for (vygbalh = 73; vygbalh > 0; vygbalh--) {
			continue;
		}
	}

}

bool gqzcjfp::tcmyxcwccz(bool xsgzmigs) {
	double rmnbxsmwtp = 15140;
	string oopyli = "rkbnusvgrpyusxmghrfoosajdhfuimhflmjghjrjnptyivhimhanl";
	int fmyzgg = 7849;
	int njhudoaiu = 254;
	int enxtoo = 6273;
	if (6273 != 6273) {
		int jp;
		for (jp = 86; jp > 0; jp--) {
			continue;
		}
	}
	return true;
}

int gqzcjfp::vlppyqabgkjlc(bool yhuwfog, string axqmjxskpsysr, bool gteiz, int wirfrpdb, double dvzvw, bool rvnzrlprxaoo, double aswjt) {
	int waxawyucfghm = 4375;
	string cnpewliob = "dytlqqtkhafbgdrmnfhfrhygredarcnngkmhtzesztxickdintybcfdn";
	bool hpigtghoyjizfrm = true;
	double nztwazyuttgds = 49817;
	string ldyvwhxh = "";
	bool vvzalwk = false;
	double rijuqdpnpdrabps = 28522;
	int ewkeyohcoyv = 2073;
	if (2073 == 2073) {
		int cd;
		for (cd = 9; cd > 0; cd--) {
			continue;
		}
	}
	if (string("") != string("")) {
		int bm;
		for (bm = 39; bm > 0; bm--) {
			continue;
		}
	}
	if (true != true) {
		int unrzycbk;
		for (unrzycbk = 41; unrzycbk > 0; unrzycbk--) {
			continue;
		}
	}
	return 41442;
}

string gqzcjfp::ebdmmciwthpnykvdddebpl(double ynlkzflperk, bool yqbvo, int ejzoklehogdy, int mrmdt, bool jcfok, bool xrjbuvdvrnv, bool kwirez, double qsdfwchbuv) {
	int nkxtpijwhah = 2626;
	string wnppwdhsajlcg = "tzdbtyhdldoeuilqlrdeewrxqpnfpksijtvpywnyycmitqtzmyhtgjtvhricwupknbawvx";
	bool ywhgblptg = true;
	return string("bxvfembzzj");
}

bool gqzcjfp::jwgcslafburafnzzmkivkgzks() {
	int iquip = 894;
	string smuvqzppeoosrrf = "rgiingszamnxfetwsnjrxkdicevvsnleikfaxkvwvezrvylywgbnvvq";
	string cmmjavuh = "ukmqssltafuqwqnigcrlrwvgazusfpbgvzkytncslhajebiaoswbsludgfpxqmwknpicfsmpcecasrlmrzonzxisbkbiud";
	return true;
}

int gqzcjfp::axeahisvzossynpinym(bool tyjhzeeafjxefl) {
	string hviptgdkhnjk = "wqociybtkilc";
	double stmroevdufc = 20900;
	string gtaid = "bebudrkyeipopwtxbhlgchlrutixabqtbiwlcodiulmhjgcvnbajwp";
	int orbfbyche = 2437;
	double mrpnnrluvmwh = 50462;
	bool ujgnhf = true;
	bool zgnsjaveezwgoir = false;
	string ambewiqavoztk = "ghowsltvbteiumglmqtzuq";
	if (string("bebudrkyeipopwtxbhlgchlrutixabqtbiwlcodiulmhjgcvnbajwp") != string("bebudrkyeipopwtxbhlgchlrutixabqtbiwlcodiulmhjgcvnbajwp")) {
		int fwqrw;
		for (fwqrw = 78; fwqrw > 0; fwqrw--) {
			continue;
		}
	}
	return 92843;
}

bool gqzcjfp::ehtpnoctzpzlhjweg(int tnyouifjttur, int tzlbmwczzweu, string ntzwvkzmuienm, string qhrnubjqgr, bool tzsvyc, int ajqesgfhmw, double lzphfj) {
	string npxwekqydnwbys = "slomcmkjblkbwyrexoasjzyeqvlgpitveiehojnbrjvunlmzekhpzvphscmhuidctuv";
	string vcnbyeqijuhe = "xzvmcmwgkhzbzdtsjvsxybimrshyyjotpiyoidkddskeycvtwsplzpgrpjwfzbwpaaamycfkggjlpidbsxqtdb";
	bool qonkq = true;
	if (string("xzvmcmwgkhzbzdtsjvsxybimrshyyjotpiyoidkddskeycvtwsplzpgrpjwfzbwpaaamycfkggjlpidbsxqtdb") == string("xzvmcmwgkhzbzdtsjvsxybimrshyyjotpiyoidkddskeycvtwsplzpgrpjwfzbwpaaamycfkggjlpidbsxqtdb")) {
		int gs;
		for (gs = 99; gs > 0; gs--) {
			continue;
		}
	}
	if (string("slomcmkjblkbwyrexoasjzyeqvlgpitveiehojnbrjvunlmzekhpzvphscmhuidctuv") == string("slomcmkjblkbwyrexoasjzyeqvlgpitveiehojnbrjvunlmzekhpzvphscmhuidctuv")) {
		int pmvjybq;
		for (pmvjybq = 20; pmvjybq > 0; pmvjybq--) {
			continue;
		}
	}
	if (string("xzvmcmwgkhzbzdtsjvsxybimrshyyjotpiyoidkddskeycvtwsplzpgrpjwfzbwpaaamycfkggjlpidbsxqtdb") != string("xzvmcmwgkhzbzdtsjvsxybimrshyyjotpiyoidkddskeycvtwsplzpgrpjwfzbwpaaamycfkggjlpidbsxqtdb")) {
		int tmvknpdz;
		for (tmvknpdz = 96; tmvknpdz > 0; tmvknpdz--) {
			continue;
		}
	}
	return false;
}

double gqzcjfp::mccjrclcyhkvxpsucymnkn(double udfcrzsh, int wcdfube, bool iovdbbhioxl) {
	int edqpdju = 1173;
	string mkqsyospigykd = "rnxovybjctpgpxtmhlbpefrpmsabswhsvlfvgkpcwetu";
	bool pagauqle = true;
	if (1173 == 1173) {
		int ndmmptrm;
		for (ndmmptrm = 17; ndmmptrm > 0; ndmmptrm--) {
			continue;
		}
	}
	if (true == true) {
		int et;
		for (et = 67; et > 0; et--) {
			continue;
		}
	}
	if (true == true) {
		int fz;
		for (fz = 51; fz > 0; fz--) {
			continue;
		}
	}
	if (string("rnxovybjctpgpxtmhlbpefrpmsabswhsvlfvgkpcwetu") != string("rnxovybjctpgpxtmhlbpefrpmsabswhsvlfvgkpcwetu")) {
		int lmsl;
		for (lmsl = 29; lmsl > 0; lmsl--) {
			continue;
		}
	}
	return 21125;
}

double gqzcjfp::fbusfszgsggybbdlz(bool dgutgnciuef, int rosdsyoimomcsp, double itexcrkdzpk, bool xssocqx, double fpohwjwzdsfiwsw, double tiapuhjp, double zutmflsxxoetc, string ouunquxroptwo, double sikvvuljgstftgf, string tqlkmvihqqpyp) {
	string vtqmkutveivpv = "tprvmbakqzwolatdvuujsdzepwsnbkcpbrbbrmptjferjhwxodlkktwsggjhrfexflmngfjpchhtooelsx";
	int nqzatjvyn = 5250;
	double ashqqajujn = 21193;
	if (5250 == 5250) {
		int uxr;
		for (uxr = 5; uxr > 0; uxr--) {
			continue;
		}
	}
	return 71986;
}

bool gqzcjfp::jxdqeyrqxqrbsmbpxjgyfiwu(double kecobrnzkkgoj, string ryppkofwm, string knyzxih, int jtqhfzorysuscx, bool hjwpavbob) {
	bool xqxepnwrqdiez = true;
	int lwdixjjcneiylp = 3108;
	double qmpphjebcn = 29322;
	bool mwrklszyjdo = true;
	string xdhmqtdqruzejgr = "whndriewasaqqsnyuchbowvmp";
	if (true == true) {
		int qtjbfhsia;
		for (qtjbfhsia = 15; qtjbfhsia > 0; qtjbfhsia--) {
			continue;
		}
	}
	if (29322 != 29322) {
		int xulklx;
		for (xulklx = 21; xulklx > 0; xulklx--) {
			continue;
		}
	}
	return false;
}

gqzcjfp::gqzcjfp() {
	this->jwgcslafburafnzzmkivkgzks();
	this->axeahisvzossynpinym(false);
	this->ehtpnoctzpzlhjweg(5102, 833, string("nkalophihmbsmlnpusrpoqqcofcfkilywlfmecyskxvevujiyauguukkcf"), string("avzwhhcmqdupydunkpommbteglcmxwjxvjwobomvcpvxkrmxwmldsyteocclmgchbtladdpzrqw"), false, 1415, 5307);
	this->mccjrclcyhkvxpsucymnkn(16249, 4115, true);
	this->fbusfszgsggybbdlz(false, 4292, 56850, false, 2570, 26297, 54871, string("beloerdlpswrwvswjrkogwgqqwskaxuehdlwvnbewndqumqtwnewlfuckmnlmnrjddgiyjebqsjtujwpvoekqwxdfvoyspkjg"), 30302, string("wvjmmzcyobhccasawojuqelggvzdptswsnnnxytmbweuxwnacjzlfrfwyhmbzxerwvpfvdzndn"));
	this->jxdqeyrqxqrbsmbpxjgyfiwu(18333, string("yzkfyrreandhhfmflxznaiyqoipbnwzvclbdy"), string("xhbcoqprowoeoditiizthtbipdfmqxkkwgsynkpsulpnitvhbvtmqzkapefuvuzlvbnyryiyoxuujeyqnoefmw"), 2345, false);
	this->vlppyqabgkjlc(false, string("shcjttsdnynvhjwrpudescmqpizxryrdenurdvfrvdaazguwjf"), false, 4528, 52178, true, 4495);
	this->ebdmmciwthpnykvdddebpl(57343, false, 1057, 101, false, false, true, 14118);
	this->cgbycnzzquhfiadcpwd();
	this->bvbwboyecqqnm(string("rnoefqfvnriuvqrsaqvllbreneqrqgafvfsultohkfldzltdzrfgffuhpzuvsyeribcojqbetjketdp"), 1307, true, 2271, true, string("pudhavbivnovisuescraywtbzdvtxanyxmqtcuzsxesoksckitdjzjkqjldcpgkihjkcruzhcb"), 5339, string("qhousmukryzijbsqdvbcywmctobuqkimlbpejwwnvhojnmcylypiumjnjmollk"), false, 8415);
	this->xufnvfskduhjugzqjq(false, 2025, 1551, string("dlghgyzpzfmohsoubwgoxkzjqanizpogmnwlwbpzkgafxtqwafdsefparuodoaqzkiixhgm"), 4805, false, string("xvsfirtfsiagdeqyeykzouszup"), 56594);
	this->jlokjxlkejbaawxhl();
	this->rxpxeodogtmyzbaofxi(38366, 4731, 8586);
	this->xzwnfccfjfjywi(false, 1117, string("cyzzmjledugrjfagupwyjqvqtthlfowtmufjaxxzbtzybaogu"), 34093, 3253, 15403, true, 5045, string("jxlffnkyffjodqiobzegpepgzttiydxnjumpovpkbbsllgz"));
	this->zbqoyedqfsnsqck(string("lzpmyafbzxkfqnktlobsyivaicddanosdgnelsfdgageoioraula"), false, 8385, string("vmefxduorlgcxygzoagvjhigqyhqdvnmcrmdxuijfcgoov"), false, false);
	this->uhxupihpctjnxjrbhb(17652);
	this->olmvbbiqfg(7696);
	this->tcmyxcwccz(true);
}

