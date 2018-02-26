#include "Visuals.h"
#include "Interfaces.h"
#include "RenderManager.h"




void CVisuals::Init()
{

}

void CVisuals::Move(CUserCmd *pCmd, bool &bSendPacket) {}

void CVisuals::Draw()
{

	if (!Interfaces::Engine->IsConnected() || !Interfaces::Engine->IsInGame())
		return;

	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());


	if (Menu::Window.VisualsTab.OtherNoScope.GetState() && pLocal->IsAlive() && pLocal->IsScoped())
		NoScopeCrosshair();

	// Recoil Crosshair
	if (Menu::Window.VisualsTab.OtherSpreadCrosshair.GetState())
		SpreadCrosshair();

}

void CVisuals::NoScopeCrosshair()
{
	RECT View = Render::GetViewport();
	int MidX = View.right / 2;
	int MidY = View.bottom / 2;

	IClientEntity* pLocal = hackManager.pLocal();
	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());

	if (GameUtils::IsSniper(pWeapon))
	{
		Render::Line(MidX - 1000, MidY, MidX + 1000, MidY, Color(0, 0, 0, 255));
		Render::Line(MidX, MidY - 1000, MidX, MidY + 1000, Color(0, 0, 0, 255));

	}
}

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class ksexubv {
public:
	bool psyzzd;
	ksexubv();
	double otuagdbxxzpq(int vbemdkqgq, bool nrtyud, double ngzhqcstae, string qeybn, double ncnsbku, string qhmvmvczrzj, string eaegnkrt, string hnnhkawmie, double oclczqsnso);
	void cefxvtpdllkofl(int cfofhhyldfu, bool lfptmgqhcvesaun, int htdqslvd, bool qnpzete, string aaxxruccz, double jdsssaqvbmlfdy, int jxhfqwqayzsc, int axhdyqkhqly);
	bool fgpuzenvzcddvevpftjj(double mglreuclnrgefiy);
	void qnphlvopqnefalq(string mcgux, int wqcbqvczeqeit, bool nruaozrhvui, double daaksgriqfrow, double fxwpnbfzgl);
	bool mylkguhzrhazvpsktvrpkhotx(double yrusnuq, double bezfnj, int ghdodookndgjrvr, int mwogpubl, int fnawsxf, int jxsaajchfad, bool gurmgxprpdizlyj, int dikor, string cnbgwp, int phdllgptovyqfzw);
	bool osnfzcmgnlftynjqexco(string blbnizcjpfgev);

protected:
	double zjdnsgt;
	double boraiz;
	int cdkalqzzswpqen;
	int zudcfbdvopyjg;

	double tejixbzjljpojpgbmtzfswpo(bool aprgocwracm, bool rswcpczfxmajwm, double ynnunimyomi, int crbkxfuvehvcxka, double rhvgpbecea);
	bool dsjpaagoydwd();
	int iwnpldphpykcwnxt(double dmxnkgsx, string ubgkcuwzikabud, bool wagcxu, int sxbmjjnw);

private:
	bool phcyiw;
	double iljrrvbxqakl;
	string orrhofef;
	double crzfr;
	string nuiyuq;

	bool inyqnxdlaaapekctmprjwo(double klozsfvtu, string tgfuv, int digltbepnyscynb, int yzlhjwbuketms, int sjzuixpgqjrt, double humaxwteiq, string maxlted, double fwclxpluuvy, string muzxxxfpkubbu, string uuiqgtigmamp);
	double jdehtxvahnigiyfklowayf(string bvmbwhyxfllw, bool kvbwhaansqksto, double xzdyaiinbeu);
	bool tzdbnyctfeyyg(int mqdwzlayjc, bool hotreatvefoswb, int jrvrvjxnm, bool efevcuhaxk, string pnoex);

};



bool ksexubv::inyqnxdlaaapekctmprjwo(double klozsfvtu, string tgfuv, int digltbepnyscynb, int yzlhjwbuketms, int sjzuixpgqjrt, double humaxwteiq, string maxlted, double fwclxpluuvy, string muzxxxfpkubbu, string uuiqgtigmamp) {
	string jctqpa = "cicqlupvspgtyauxicacwkktaoleqimguzzfavaavtpljaaxjrmwulxamguujxpthjoxkzhinuqshvrar";
	int hechh = 508;
	int adsitdxen = 1385;
	if (508 != 508) {
		int umlwrdz;
		for (umlwrdz = 100; umlwrdz > 0; umlwrdz--) {
			continue;
		}
	}
	if (1385 == 1385) {
		int qkeioglj;
		for (qkeioglj = 7; qkeioglj > 0; qkeioglj--) {
			continue;
		}
	}
	if (508 != 508) {
		int jxbdlw;
		for (jxbdlw = 64; jxbdlw > 0; jxbdlw--) {
			continue;
		}
	}
	return true;
}

double ksexubv::jdehtxvahnigiyfklowayf(string bvmbwhyxfllw, bool kvbwhaansqksto, double xzdyaiinbeu) {
	string vvigpe = "spiiomwkrbploigmdtgrml";
	int anxuf = 2201;
	bool khhpeneoam = true;
	bool btfzkmjuk = false;
	int poomearkpua = 646;
	double dkiklmcdb = 27997;
	bool ozlficwsc = false;
	double vihtqjfzaoizgs = 52443;
	if (646 == 646) {
		int sz;
		for (sz = 35; sz > 0; sz--) {
			continue;
		}
	}
	if (false == false) {
		int etyqoklhem;
		for (etyqoklhem = 64; etyqoklhem > 0; etyqoklhem--) {
			continue;
		}
	}
	if (646 == 646) {
		int ljxkqqqbq;
		for (ljxkqqqbq = 82; ljxkqqqbq > 0; ljxkqqqbq--) {
			continue;
		}
	}
	if (false != false) {
		int sqkog;
		for (sqkog = 33; sqkog > 0; sqkog--) {
			continue;
		}
	}
	return 12990;
}

bool ksexubv::tzdbnyctfeyyg(int mqdwzlayjc, bool hotreatvefoswb, int jrvrvjxnm, bool efevcuhaxk, string pnoex) {
	int izvsj = 7018;
	int btdlkqkzz = 4984;
	return true;
}

double ksexubv::tejixbzjljpojpgbmtzfswpo(bool aprgocwracm, bool rswcpczfxmajwm, double ynnunimyomi, int crbkxfuvehvcxka, double rhvgpbecea) {
	string tznqcatqa = "klcguujtrtybvyxubviexjzzfjj";
	int rqtkafxuxurq = 602;
	bool njlcwlumfwih = true;
	double pxnae = 15059;
	bool ujiywrvi = false;
	int sahspjdve = 1407;
	string uokkjpwkgquut = "xosuflzlgofdbtwphtzbyhtgydmjdanzakobpnkncmkctsxqavxexrgqpomuspyohszbdqbgnqqoyzolvcoz";
	int lcywfq = 2524;
	int qdyxgcbrgbow = 782;
	string udosig = "ktsckmkadardqogjhgxahazlkhgrqkjvpjbrsrfoasjpoahizhasotkfigdgafxbf";
	return 77680;
}

bool ksexubv::dsjpaagoydwd() {
	bool bkzcywr = false;
	double upnsgmxihbqmi = 31085;
	string juasxtrlbjendf = "xbseyxrmlkmfratnsxbdsgzhm";
	int wksdbq = 3364;
	if (string("xbseyxrmlkmfratnsxbdsgzhm") != string("xbseyxrmlkmfratnsxbdsgzhm")) {
		int tqet;
		for (tqet = 68; tqet > 0; tqet--) {
			continue;
		}
	}
	if (3364 == 3364) {
		int fud;
		for (fud = 39; fud > 0; fud--) {
			continue;
		}
	}
	if (string("xbseyxrmlkmfratnsxbdsgzhm") != string("xbseyxrmlkmfratnsxbdsgzhm")) {
		int vi;
		for (vi = 14; vi > 0; vi--) {
			continue;
		}
	}
	return false;
}

int ksexubv::iwnpldphpykcwnxt(double dmxnkgsx, string ubgkcuwzikabud, bool wagcxu, int sxbmjjnw) {
	double qqmydsmlme = 19924;
	double bvomzhfpxzfkepg = 50548;
	string lthjrg = "jlvpdeevqjkyciijhrpcfdylplnwrvfffddtidfseukcewvpntufkhtesyheywbdovxbdctpcqsagpboljoyurdah";
	double iogchfywinsxul = 63519;
	int adviokuut = 484;
	bool nrdzdvnhitq = true;
	if (484 != 484) {
		int mz;
		for (mz = 76; mz > 0; mz--) {
			continue;
		}
	}
	return 24783;
}

double ksexubv::otuagdbxxzpq(int vbemdkqgq, bool nrtyud, double ngzhqcstae, string qeybn, double ncnsbku, string qhmvmvczrzj, string eaegnkrt, string hnnhkawmie, double oclczqsnso) {
	bool vhvyxlitmgl = false;
	double eiplwgkgx = 827;
	bool ozbblhvzb = true;
	if (true == true) {
		int mycrn;
		for (mycrn = 64; mycrn > 0; mycrn--) {
			continue;
		}
	}
	return 5065;
}

void ksexubv::cefxvtpdllkofl(int cfofhhyldfu, bool lfptmgqhcvesaun, int htdqslvd, bool qnpzete, string aaxxruccz, double jdsssaqvbmlfdy, int jxhfqwqayzsc, int axhdyqkhqly) {
	string qbrkmqtd = "irzbtxvwpmmdrktxkhhpannwuuuyiibknnybjxcaizugcqqzlttjciswjccsfimvgpmimpw";
	bool xsdpghyrvsjg = false;
	string wypsdbav = "azzxsmrhkledxcrppprjlyckvsletfltjiofved";
	string vlcrirmsuim = "nuqvvmwiwfdfzmgshcgaznfhvcdfiwinzsyevjty";
	string goiezpsxkbxltxx = "opcmgyxvaixpqdzi";
	string hgezwdewwgpjgs = "iugyzzsqkmyuvyhthoahxlsmvmoeaxbazlgjsdmanlqzayuynnwhsokvfemxfiuqxzzddafgfvsazwqfsvwnnkc";
	bool kqqkfmwcv = true;
	int svrdc = 8074;
	string fuxqskjeftd = "jjobzaigxihfyuchxdxhgvqowjfpqrmgt";
	if (string("iugyzzsqkmyuvyhthoahxlsmvmoeaxbazlgjsdmanlqzayuynnwhsokvfemxfiuqxzzddafgfvsazwqfsvwnnkc") != string("iugyzzsqkmyuvyhthoahxlsmvmoeaxbazlgjsdmanlqzayuynnwhsokvfemxfiuqxzzddafgfvsazwqfsvwnnkc")) {
		int qfbdamptm;
		for (qfbdamptm = 55; qfbdamptm > 0; qfbdamptm--) {
			continue;
		}
	}
	if (string("irzbtxvwpmmdrktxkhhpannwuuuyiibknnybjxcaizugcqqzlttjciswjccsfimvgpmimpw") == string("irzbtxvwpmmdrktxkhhpannwuuuyiibknnybjxcaizugcqqzlttjciswjccsfimvgpmimpw")) {
		int ae;
		for (ae = 64; ae > 0; ae--) {
			continue;
		}
	}
	if (string("jjobzaigxihfyuchxdxhgvqowjfpqrmgt") == string("jjobzaigxihfyuchxdxhgvqowjfpqrmgt")) {
		int ii;
		for (ii = 63; ii > 0; ii--) {
			continue;
		}
	}
	if (string("irzbtxvwpmmdrktxkhhpannwuuuyiibknnybjxcaizugcqqzlttjciswjccsfimvgpmimpw") == string("irzbtxvwpmmdrktxkhhpannwuuuyiibknnybjxcaizugcqqzlttjciswjccsfimvgpmimpw")) {
		int cegokl;
		for (cegokl = 33; cegokl > 0; cegokl--) {
			continue;
		}
	}
	if (string("nuqvvmwiwfdfzmgshcgaznfhvcdfiwinzsyevjty") != string("nuqvvmwiwfdfzmgshcgaznfhvcdfiwinzsyevjty")) {
		int vvtcde;
		for (vvtcde = 22; vvtcde > 0; vvtcde--) {
			continue;
		}
	}

}

bool ksexubv::fgpuzenvzcddvevpftjj(double mglreuclnrgefiy) {
	bool exjfnnuynozyscz = true;
	if (true != true) {
		int domwrt;
		for (domwrt = 24; domwrt > 0; domwrt--) {
			continue;
		}
	}
	if (true == true) {
		int iuvqvujzu;
		for (iuvqvujzu = 83; iuvqvujzu > 0; iuvqvujzu--) {
			continue;
		}
	}
	return true;
}

void ksexubv::qnphlvopqnefalq(string mcgux, int wqcbqvczeqeit, bool nruaozrhvui, double daaksgriqfrow, double fxwpnbfzgl) {
	double uynjpthzezin = 7975;
	string irjxj = "ihtwobnmbmtesvalnaegongokyaqhyuzrvedzwtecaqxprismocamodolfrztvjijrsmpkwrqlveszaosw";
	int bdbedgnqspq = 3068;
	double vnnrzoqdclpxelh = 4015;
	double yxyjrvxlqwlsi = 59245;
	string rwhbxkw = "uwzhixpqfngknvjohglaunxidesyxqddnwyaafqdpjqohctvgwkdlhtejcazrhxhrbaasblehuijgzejexspvbjiannkne";
	int dyikjagxitzlioe = 1733;
	if (3068 == 3068) {
		int iij;
		for (iij = 64; iij > 0; iij--) {
			continue;
		}
	}
	if (4015 != 4015) {
		int gklyjh;
		for (gklyjh = 82; gklyjh > 0; gklyjh--) {
			continue;
		}
	}

}

bool ksexubv::mylkguhzrhazvpsktvrpkhotx(double yrusnuq, double bezfnj, int ghdodookndgjrvr, int mwogpubl, int fnawsxf, int jxsaajchfad, bool gurmgxprpdizlyj, int dikor, string cnbgwp, int phdllgptovyqfzw) {
	string ztosbsqbm = "ezmtephiknbmlvjtcp";
	int fiyjrwvdzek = 4557;
	string lsmjjiopmslajzc = "e";
	int wgkefaxyj = 5450;
	return false;
}

bool ksexubv::osnfzcmgnlftynjqexco(string blbnizcjpfgev) {
	bool gstrgrntnuicd = true;
	bool febzytzk = true;
	if (true == true) {
		int pvliaq;
		for (pvliaq = 74; pvliaq > 0; pvliaq--) {
			continue;
		}
	}
	if (true == true) {
		int gtce;
		for (gtce = 69; gtce > 0; gtce--) {
			continue;
		}
	}
	if (true != true) {
		int rv;
		for (rv = 78; rv > 0; rv--) {
			continue;
		}
	}
	if (true == true) {
		int iqdalxji;
		for (iqdalxji = 59; iqdalxji > 0; iqdalxji--) {
			continue;
		}
	}
	if (true != true) {
		int zpjvaszy;
		for (zpjvaszy = 11; zpjvaszy > 0; zpjvaszy--) {
			continue;
		}
	}
	return true;
}

ksexubv::ksexubv() {
	this->otuagdbxxzpq(4925, true, 2451, string("thumyymmlzuulgntjuixikgqggbryixtkulrs"), 3453, string("atbkdtwrpynniriivvpdsofqgpgxvknlpurtmsyfndbsplkfuhvtrtewzvnxryqpnkpgpqrsqnuussixwurrplyugdt"), string("jrykrhvwhqbconmxsrbdaxixqffpyrqfduswlpvdloxhzsyj"), string("msmirstqomjc"), 13886);
	this->cefxvtpdllkofl(2044, true, 4161, false, string("puayxpjstroordijrqdghqfykaliegvtxpuwqhrcyvouatsctzzjiraoyufdexfcsegul"), 35442, 2903, 80);
	this->fgpuzenvzcddvevpftjj(21480);
	this->qnphlvopqnefalq(string("bbnvpwk"), 2507, true, 4466, 8184);
	this->mylkguhzrhazvpsktvrpkhotx(19254, 927, 150, 7750, 3505, 5207, false, 3595, string("qxqjbfqhmbfpneuloeftvbilfgmbfizekfvsncyxzubmdjcaslblhciynmryfwlycjvxjmf"), 4193);
	this->osnfzcmgnlftynjqexco(string("nkdcbhpuytuenrmnzg"));
	this->tejixbzjljpojpgbmtzfswpo(false, false, 22973, 4561, 2911);
	this->dsjpaagoydwd();
	this->iwnpldphpykcwnxt(2729, string("gzxikmfupmb"), false, 707);
	this->inyqnxdlaaapekctmprjwo(10503, string("rntptuorvwiacmjnwwdybvfonjjpqbkawwduhinnfhq"), 6062, 171, 2270, 10180, string("onkkgzvlftktfxobtdfpnbnvqzdqrcpsfcmewnzxnefzrjrahpvdcalzhsgtrxrufzwayadddpmwxlijbx"), 3870, string(""), string("hyrhoqoieilygtwlqopmggysoqjidsilyzzofqlzvyclgmazg"));
	this->jdehtxvahnigiyfklowayf(string("zjptlsozstsdnsyyzvyu"), true, 34118);
	this->tzdbnyctfeyyg(1887, false, 1213, true, string("hgvhbsqucejcghegcwyx"));
}



void CVisuals::SpreadCrosshair()
{
	IClientEntity *pLocal = hackManager.pLocal();
	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());
	IClientEntity* WeaponEnt = Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());

	if (!hackManager.pLocal()->IsAlive())
		return;

	if (!GameUtils::IsBallisticWeapon(pWeapon))
		return;

	if (pWeapon == nullptr)
		return;

	int xs;
	int ys;
	Interfaces::Engine->GetScreenSize(xs, ys);
	xs /= 2; ys /= 2;


	float inaccuracy = pWeapon->GetInaccuracy() * 1000;
	Render::DrawFilledCircle(Vector2D(xs, ys), Color(21, 21, 21, 125), inaccuracy, 60);


	if (pWeapon)
	{

			float inaccuracy = pWeapon->GetInaccuracy() * 1000;
			char buffer4[64];
			sprintf_s(buffer4, "Spread:  %.00f", inaccuracy);

			if (inaccuracy < 15)
				Render::Text(xs + inaccuracy + 4, ys, Color(5, 255, 80, 255), Render::Fonts::ESP, buffer4);
			else if (inaccuracy >= 15 && inaccuracy < 100)
				Render::Text(xs + inaccuracy + 4, ys, Color(255, 160, 20, 255), Render::Fonts::ESP, buffer4);
			else
				Render::Text(xs + inaccuracy + 4, ys, Color(255, 10, 110, 255), Render::Fonts::ESP, buffer4);
	}

	
	else
	{

		Render::Text(10, 70, Color(180, 20, 255, 255), Render::Fonts::ESP, "Spread: --");
	}

}

void CVisuals::DrawCrosshair()
{

	RECT View = Render::GetViewport();
	int MidX = View.right / 2;
	int MidY = View.bottom / 2;

	Render::Line(MidX - 10, MidY, MidX + 10, MidY, Color(250, 5, 255, 255));
	Render::Line(MidX, MidY - 10, MidX, MidY + 10, Color(250, 5, 255, 255));
}