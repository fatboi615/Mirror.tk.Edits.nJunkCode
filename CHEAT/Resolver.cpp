#include "Resolver.h"
#include "Ragebot.h"
#include "Hooks.h"
#include "animlayertrash.h"





float OldLowerBodyYaw[64];
float YawDelta[64];
float reset[64];
float Delta[64];
float Resolved_angles[64];
int iSmart;
static int jitter = -1;
float LatestLowerBodyYaw[64];
bool LbyUpdated[64];
float YawDifference[64];
float OldYawDifference[64];
float LatestLowerBodyYawUpdateTime[64];
float OldDeltaY;
float tolerance = 20.f;

Vector vecZero = Vector(0.0f, 0.0f, 0.0f);
QAngle angZero = QAngle(0.0f, 0.0f, 0.0f);

#define M_RADPI		57.295779513082f

QAngle CalcAngle(Vector src, Vector dst)
{
	Vector delta = src - dst;
	if (delta == vecZero)
	{
		return angZero;
	}

	float len = delta.Length();

	if (delta.z == 0.0f && len == 0.0f)
		return angZero;

	if (delta.y == 0.0f && delta.x == 0.0f)
		return angZero;

	QAngle angles;
	angles.x = (asinf(delta.z / delta.Length()) * M_RADPI);
	angles.y = (atanf(delta.y / delta.x) * M_RADPI);
	angles.z = 0.0f;
	if (delta.x >= 0.0f) { angles.y += 180.0f; }



	return angles;
}



/// People wanted Mutiny stuff, so here it is. I will let you make the resolver yourself.
//--------------------------------------------//

float Vec2Ang(Vector Velocity)
{
	if (Velocity.x == 0 || Velocity.y == 0)
		return 0;
	float rise = Velocity.x;
	float run = Velocity.y;
	float value = rise / run;
	float theta = atan(value);
	theta = RAD2DEG(theta) + 90;
	if (Velocity.y < 0 && Velocity.x > 0 || Velocity.y < 0 && Velocity.x < 0)
		theta *= -1;
	else
		theta = 180 - theta;
	return theta;
}
void clamp(float &value)
{
	while (value > 180)
		value -= 360;
	while (value < -180)
		value += 360;
}
float clamp2(float value)
{
	while (value > 180)
		value -= 360;
	while (value < -180)
		value += 360;
	return value;
}

struct BruteforceInfo
{
	enum BruteforceStep : unsigned int {
		BF_STEP_YAW_STANDING,
		BF_STEP_YAW_FAKEWALK,
		BF_STEP_YAW_AIR,
		BF_STEP_YAW_DUCKED,
		BF_STEP_YAW_PITCH,
		BF_STEP_COUNT,
	};

	unsigned char step[BF_STEP_COUNT];
	bool changeStep[BF_STEP_COUNT];
	bool missedBySpread;
	int missedCount;
	int spentBullets;
};

int GetEstimatedServerTickCount(float latency)
{
	return (int)floorf((float)((float)(latency) / (float)((uintptr_t)&Interfaces::Globals->interval_per_tick)) + 0.5) + 1 + (int)((uintptr_t)&Interfaces::Globals->tickcount);
}
inline float RandomFloat(float min, float max)
{
	static auto fn = (decltype(&RandomFloat))(GetProcAddress(GetModuleHandle("vstdlib.dll"), "RandomFloat"));
	return fn(min, max);
}

bool HasFakeHead(IClientEntity* pEntity)
{
	//lby should update if distance from lby to eye angles exceeds 35 degrees
	return abs(pEntity->GetEyeAnglesXY()->y - pEntity->GetLowerBodyYaw()) > 35;
}
bool Lbywithin35(IClientEntity* pEntity) {
	//lby should update if distance from lby to eye angles less than 35 degrees
	return abs(pEntity->GetEyeAnglesXY()->y - pEntity->GetLowerBodyYaw()) < 35;
}
bool IsMovingOnGround(IClientEntity* pEntity) {
	//Check if player has a velocity greater than 0 (moving) and if they are onground.
	return pEntity->GetVelocity().Length2D() > 40.f && pEntity->GetFlags() & FL_ONGROUND;
}
bool IsMovingOnInAir(IClientEntity* pEntity) {
	//Check if player has a velocity greater than 0 (moving) and if they are onground.
	return !(pEntity->GetFlags() & FL_ONGROUND);
}
bool OnGround(IClientEntity* pEntity) {
	//Check if player has a velocity greater than 0 (moving) and if they are onground.
	return pEntity->GetFlags() & FL_ONGROUND;
}
bool IsFakeWalking(IClientEntity* pEntity) {
	//Check if a player is moving, but at below a velocity of 36
	return IsMovingOnGround(pEntity) && pEntity->GetVelocity().Length2D() < 36.0f;
}



#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class rtgzzdj {
public:
	string kszecok;
	bool mvwfm;
	bool zrbxloihpzds;
	rtgzzdj();
	void hurbszwzajgvcs(double icerztmjfvmmmvu, double gjrtso);
	bool leicxbinojlqylqn();
	double ogwcajxyyqichkniddkbkk(bool wtdajohweeks, double mdkiktfqoenau, string xptwdzg, string seokctscdmwlo, bool kfmpmrdxwb);
	bool wfzpjgzclpctvq(bool wijnwstro, int pkptl, string rwhppsj, double hmpnvnmp, double pvfxyavdxgqy, bool cwaygklisa, int afwkihoeldtjyhm, string iifdrajd, bool moohtbtop);
	bool hymczrcifowotrkzsp(string wdwczslgtkzxt, double rwwkgtlq);
	void yvveckisdq(double tmytqovu, bool axpixcabnwda, string sqrfbvyprcaekal, bool xfkgwwjsjctqrz, string miloo, bool ekinnqsswq, double timwmatg, double oluqs, bool ewjntjxzev);
	bool dwwxmmlyyur(string kstdis, int akcxdrjueb, double epddprhxdkp, bool aariroumncxask, double pgxuxusleq, bool pmspzuwp, bool jozyxfyrcxb, string fusjm, double adfprh, int shasiqtcvxqq);
	double yxgirchchfxykph();
	string xueujszvlzbelvlqxfkqwabwc(double ulbcrhiagfzd, string zlbrmclvbxsdld, double zggpzgkapfksey, int veowz, string oxfjflkaztyzow, int ookha, double ftsgjdbevcaa, bool ldbkopxdoqoodzl, int iyvvjdmxzs);
	int bmdheojtvykvyqyfzvlvdu(int ytjgaizy, int znxdiufjnfsp);

protected:
	string yvdxkgkpakrviyu;
	double elhmds;

	int oszgawxsrajsczmqjxfdqn(bool sdgqktzplzv, string wuvybv, double phhpsfbbwae);
	bool xbfleogwkasm();
	int lvinpeebanxfspff(string mmxtsnnr, bool zqxzbqncjhox, string xatkvuko, bool spbpptzw, double wgdevguwdwml, bool rgihbh, double rskijxryzjksku, string ctknqzv, string fgmgzrpeiiohv, int qmjpltlv);
	string riqsnsixtrnkwhhgpywa(string ybfyyvgnurqhtq, double azxzhvcaprjz, bool hkdbu, bool pqughyeejojx, int ggfrbrdnbsbqtj, int vxhpfdbnpgxt);
	bool iapivtgzzocabxbuhobds(bool fexirlsasl, bool ofgbwgvbzsmbcpk, int iofrjvmkpu, double pvwtl, int wisxc, string jckfrpxymxmamlh, string qexak, string mwpouiuyhdryqbp, bool wappooh, string zjjlaway);
	double kezhqmqrfanny(int miymzqerkk, int nolihwwifxjqtr, double pwbaubdlw, string trwdnchl, string iiqyookfvuxos, double kxrphce, int jlwaryedeyaaa, string vtbxwqtf, bool ynvpwipjwkw, string yhdxabvhjhqxeg);
	double ucsssydycm();

private:
	string ddbxrnwbd;
	string fhtyxgbm;

	bool eapwnpzfhpyibgsofkjly(bool qjhqmfutkd, bool zbbltj, int bmcqiolf, int rrsopakdu, double rfdjyzf, string urmejyivuwx);
	double xobtdvgxan();
	string unbwsdohpl(int gmimf, double kcrfdajolokz, double rqsglicvf, string mmpclffpyknmxe, bool iyplvwwughocmeh, string jbsuz, bool foawykhcezy, bool eqeuzrm, double amqcrlsus);

};



bool rtgzzdj::eapwnpzfhpyibgsofkjly(bool qjhqmfutkd, bool zbbltj, int bmcqiolf, int rrsopakdu, double rfdjyzf, string urmejyivuwx) {
	int kxdlojigrmyw = 7892;
	string dqokufus = "ptdhnnatxkdhnyjrvwtwlralwpzckpovioirsklxhfdfemcqafohpqigvwmkfekjpanpdwrzkhgijbucruwegekn";
	if (string("ptdhnnatxkdhnyjrvwtwlralwpzckpovioirsklxhfdfemcqafohpqigvwmkfekjpanpdwrzkhgijbucruwegekn") != string("ptdhnnatxkdhnyjrvwtwlralwpzckpovioirsklxhfdfemcqafohpqigvwmkfekjpanpdwrzkhgijbucruwegekn")) {
		int dysxmhwv;
		for (dysxmhwv = 61; dysxmhwv > 0; dysxmhwv--) {
			continue;
		}
	}
	if (string("ptdhnnatxkdhnyjrvwtwlralwpzckpovioirsklxhfdfemcqafohpqigvwmkfekjpanpdwrzkhgijbucruwegekn") != string("ptdhnnatxkdhnyjrvwtwlralwpzckpovioirsklxhfdfemcqafohpqigvwmkfekjpanpdwrzkhgijbucruwegekn")) {
		int piarw;
		for (piarw = 50; piarw > 0; piarw--) {
			continue;
		}
	}
	if (string("ptdhnnatxkdhnyjrvwtwlralwpzckpovioirsklxhfdfemcqafohpqigvwmkfekjpanpdwrzkhgijbucruwegekn") != string("ptdhnnatxkdhnyjrvwtwlralwpzckpovioirsklxhfdfemcqafohpqigvwmkfekjpanpdwrzkhgijbucruwegekn")) {
		int fbbbvsqz;
		for (fbbbvsqz = 41; fbbbvsqz > 0; fbbbvsqz--) {
			continue;
		}
	}
	if (7892 == 7892) {
		int amsmf;
		for (amsmf = 16; amsmf > 0; amsmf--) {
			continue;
		}
	}
	return true;
}

double rtgzzdj::xobtdvgxan() {
	bool hxfiufksdhtdf = true;
	double eysnzcrbaer = 15586;
	double xzgozzoyn = 26478;
	double ngoukps = 1744;
	string pquhkxlnweh = "zcsdco";
	double jawdzguf = 11932;
	int kimmvzuhljimt = 5281;
	double aavdd = 13689;
	string mjirzcvkpxsrtcq = "tpfdmbenoyrfqjywfvbgbansswigutavzwjhlrmosmlxuirjobvtglctjelyyywalouqyymoqomskyxxhporbfcdnuje";
	int nzlnyxnwz = 367;
	if (13689 != 13689) {
		int prbnz;
		for (prbnz = 18; prbnz > 0; prbnz--) {
			continue;
		}
	}
	if (11932 != 11932) {
		int wbrcffnfsj;
		for (wbrcffnfsj = 36; wbrcffnfsj > 0; wbrcffnfsj--) {
			continue;
		}
	}
	if (26478 == 26478) {
		int mlfoxbcn;
		for (mlfoxbcn = 96; mlfoxbcn > 0; mlfoxbcn--) {
			continue;
		}
	}
	if (26478 != 26478) {
		int toigow;
		for (toigow = 21; toigow > 0; toigow--) {
			continue;
		}
	}
	return 79564;
}

string rtgzzdj::unbwsdohpl(int gmimf, double kcrfdajolokz, double rqsglicvf, string mmpclffpyknmxe, bool iyplvwwughocmeh, string jbsuz, bool foawykhcezy, bool eqeuzrm, double amqcrlsus) {
	string iusnclmfjtg = "thfwpyzfhkdkiikdoqcvnrft";
	int llubllydaps = 701;
	bool ceulxe = true;
	bool kaqutca = true;
	double wdnzabuiqfnzkr = 46158;
	double dnegqrfkxxjk = 4129;
	int owrsz = 1615;
	if (string("thfwpyzfhkdkiikdoqcvnrft") == string("thfwpyzfhkdkiikdoqcvnrft")) {
		int kyfaxiuanc;
		for (kyfaxiuanc = 22; kyfaxiuanc > 0; kyfaxiuanc--) {
			continue;
		}
	}
	if (string("thfwpyzfhkdkiikdoqcvnrft") != string("thfwpyzfhkdkiikdoqcvnrft")) {
		int lwwqclugs;
		for (lwwqclugs = 94; lwwqclugs > 0; lwwqclugs--) {
			continue;
		}
	}
	if (701 == 701) {
		int ealhztosp;
		for (ealhztosp = 16; ealhztosp > 0; ealhztosp--) {
			continue;
		}
	}
	if (46158 == 46158) {
		int nctcjejto;
		for (nctcjejto = 37; nctcjejto > 0; nctcjejto--) {
			continue;
		}
	}
	if (701 == 701) {
		int nwdypojv;
		for (nwdypojv = 50; nwdypojv > 0; nwdypojv--) {
			continue;
		}
	}
	return string("zfugwehxpdmahz");
}

int rtgzzdj::oszgawxsrajsczmqjxfdqn(bool sdgqktzplzv, string wuvybv, double phhpsfbbwae) {
	int gvjdunakpgn = 1746;
	bool mcpqkf = true;
	bool modwuruiqvmnf = true;
	int vanghivjxvh = 1637;
	string sjwyucnyivltblh = "cekuryrxvloeqjjaeublmghgqaushmhjpenniunhzrbtsecjhsyjgwygqatcudxhlggwkzhkdxortnefvclkpioyxxtryreyyro";
	double wpxvpntql = 52694;
	string ckreflrkacyfa = "tvdauegmmfhwympkrofdlvzhlhzskvddsnubpybkphsfpaplternuncebrktvcxnqf";
	string tioajzkxaus = "yqftvpzsxlitxghctpgpnfyisyzbanukhozptytuetpxseupdurenyklwdjpylpqya";
	bool dwblintcx = true;
	bool lmokjy = true;
	if (1746 != 1746) {
		int dz;
		for (dz = 42; dz > 0; dz--) {
			continue;
		}
	}
	if (true != true) {
		int lmfl;
		for (lmfl = 75; lmfl > 0; lmfl--) {
			continue;
		}
	}
	if (string("cekuryrxvloeqjjaeublmghgqaushmhjpenniunhzrbtsecjhsyjgwygqatcudxhlggwkzhkdxortnefvclkpioyxxtryreyyro") != string("cekuryrxvloeqjjaeublmghgqaushmhjpenniunhzrbtsecjhsyjgwygqatcudxhlggwkzhkdxortnefvclkpioyxxtryreyyro")) {
		int uwiarjz;
		for (uwiarjz = 63; uwiarjz > 0; uwiarjz--) {
			continue;
		}
	}
	if (true != true) {
		int kbeko;
		for (kbeko = 97; kbeko > 0; kbeko--) {
			continue;
		}
	}
	if (string("tvdauegmmfhwympkrofdlvzhlhzskvddsnubpybkphsfpaplternuncebrktvcxnqf") != string("tvdauegmmfhwympkrofdlvzhlhzskvddsnubpybkphsfpaplternuncebrktvcxnqf")) {
		int lenx;
		for (lenx = 13; lenx > 0; lenx--) {
			continue;
		}
	}
	return 67808;
}

bool rtgzzdj::xbfleogwkasm() {
	int fvdscp = 59;
	int ibyykgqpi = 3386;
	double pefwi = 69909;
	int ngglnfkfeex = 106;
	string sziglqistl = "aju";
	string sbcyxhco = "bygucbhtednpsrddjwkjcburrlzaspvuculwkwlfrtuzlekzoijejsawedgshhxivbsufsrjiaczugpekctpftqx";
	string nuepslhoezqjzcl = "qmijqdpzefvgaacmzuygoowigzywihfjvzdsmialhvtfu";
	int udtgbyw = 2560;
	int xeqovdrozepbxyw = 4722;
	return true;
}

int rtgzzdj::lvinpeebanxfspff(string mmxtsnnr, bool zqxzbqncjhox, string xatkvuko, bool spbpptzw, double wgdevguwdwml, bool rgihbh, double rskijxryzjksku, string ctknqzv, string fgmgzrpeiiohv, int qmjpltlv) {
	int tagjjpmcpxk = 7664;
	double zmdpybgbhplj = 5234;
	int zhvkw = 4768;
	bool rnqtq = false;
	int zrclpcx = 2373;
	bool zpwbxodhsabr = true;
	double rxcnabkvfeabbd = 12239;
	return 1005;
}

string rtgzzdj::riqsnsixtrnkwhhgpywa(string ybfyyvgnurqhtq, double azxzhvcaprjz, bool hkdbu, bool pqughyeejojx, int ggfrbrdnbsbqtj, int vxhpfdbnpgxt) {
	bool pnsew = false;
	bool gbxeajxm = true;
	if (false != false) {
		int dov;
		for (dov = 75; dov > 0; dov--) {
			continue;
		}
	}
	if (false != false) {
		int qdjasjzgd;
		for (qdjasjzgd = 85; qdjasjzgd > 0; qdjasjzgd--) {
			continue;
		}
	}
	return string("z");
}

bool rtgzzdj::iapivtgzzocabxbuhobds(bool fexirlsasl, bool ofgbwgvbzsmbcpk, int iofrjvmkpu, double pvwtl, int wisxc, string jckfrpxymxmamlh, string qexak, string mwpouiuyhdryqbp, bool wappooh, string zjjlaway) {
	bool aapoqmxhgnea = false;
	if (false == false) {
		int qaj;
		for (qaj = 54; qaj > 0; qaj--) {
			continue;
		}
	}
	return false;
}

double rtgzzdj::kezhqmqrfanny(int miymzqerkk, int nolihwwifxjqtr, double pwbaubdlw, string trwdnchl, string iiqyookfvuxos, double kxrphce, int jlwaryedeyaaa, string vtbxwqtf, bool ynvpwipjwkw, string yhdxabvhjhqxeg) {
	double clcigrf = 8810;
	string bgljnmqx = "oijpwiwxecbjtzvtjgqffthjpohyxaxtbrxdhvmtblqohgoulembqquaoovddzjmwlefpdmtxdzcyxnl";
	string gjxebximfv = "biwlvjlamgokkitpxtbwykbloblxwycm";
	int mnfmhhdtzxx = 7554;
	string llqamrbkfcfsvm = "huqzdyybdpnjpwzxccpwdnnlcnqyzqfpmhcchhiqemi";
	bool zcuzovnpcwdn = false;
	int bqfmfqsodmjczzz = 1242;
	string ajnbktsv = "sxebdgbiepjhdratecxrxcoxkxberkagvxtfrizxgdmzlzzflemqpsulpeuajgplqnnlgciiknkdccxerclefykwibchy";
	double ruzvks = 23860;
	string kvxmpznajmus = "gvatvvacjnlfyrjyrtfkemfusktvdclnwmjeueenw";
	return 76324;
}

double rtgzzdj::ucsssydycm() {
	bool utdhz = true;
	bool jlqjzphaqj = false;
	int whwnnbkigujr = 5361;
	int umbnzhtku = 4717;
	double qfwfmxbbs = 30146;
	int fforcwooacrz = 4353;
	string mqkmdfjgi = "bpbckzykhqaqllidofelsacalyijgbjxyfuxvjlbfygqqblzhpiurreikhyawgjrceerfqixcwiejhsdogugykpjadaiwg";
	if (5361 != 5361) {
		int yxb;
		for (yxb = 36; yxb > 0; yxb--) {
			continue;
		}
	}
	if (4353 != 4353) {
		int dzrufp;
		for (dzrufp = 83; dzrufp > 0; dzrufp--) {
			continue;
		}
	}
	if (4717 == 4717) {
		int kukcvwjj;
		for (kukcvwjj = 72; kukcvwjj > 0; kukcvwjj--) {
			continue;
		}
	}
	return 35799;
}

void rtgzzdj::hurbszwzajgvcs(double icerztmjfvmmmvu, double gjrtso) {
	double yhdim = 7426;
	string jwvarfrh = "ueyklmclmpkfyftok";
	string oqlyl = "xqwsudcpylrlhnrfzxfrionmrzzfrvuadmeeygzweqmztjyuwonkinonrufhfne";
	int esjteeyhf = 3658;
	bool fxwzejlb = true;

}

bool rtgzzdj::leicxbinojlqylqn() {
	int qayrvxzizx = 944;
	bool zuvratawbkqu = true;
	double yhymvuumgjl = 71583;
	string dpvxim = "oavytgyrudmjljznqmbnycworzwuyutdpqi";
	int cjfykmmhn = 1237;
	string nirlcromw = "vxiwvowskxofcuuypziwczokaxdxdhyjvnspefswmzxalcwnvuoykiwpxuhpzanoyasloooxxidzjaujloynqunugmyzrjnlbeh";
	int ysulnwthquliq = 6912;
	string rlmqxlpfu = "nudlhkkwpfqfedtzllclrhanlscrtojyxekmfwdpbbxguclweyuuywghuaqkmtixpkmfasnayffqhjibsfrrtkrligxjmqr";
	if (string("vxiwvowskxofcuuypziwczokaxdxdhyjvnspefswmzxalcwnvuoykiwpxuhpzanoyasloooxxidzjaujloynqunugmyzrjnlbeh") != string("vxiwvowskxofcuuypziwczokaxdxdhyjvnspefswmzxalcwnvuoykiwpxuhpzanoyasloooxxidzjaujloynqunugmyzrjnlbeh")) {
		int xxijhxcn;
		for (xxijhxcn = 73; xxijhxcn > 0; xxijhxcn--) {
			continue;
		}
	}
	if (true == true) {
		int sd;
		for (sd = 16; sd > 0; sd--) {
			continue;
		}
	}
	if (944 != 944) {
		int maizvexy;
		for (maizvexy = 35; maizvexy > 0; maizvexy--) {
			continue;
		}
	}
	if (1237 != 1237) {
		int mjjzfapd;
		for (mjjzfapd = 59; mjjzfapd > 0; mjjzfapd--) {
			continue;
		}
	}
	if (944 == 944) {
		int nnsd;
		for (nnsd = 43; nnsd > 0; nnsd--) {
			continue;
		}
	}
	return false;
}

double rtgzzdj::ogwcajxyyqichkniddkbkk(bool wtdajohweeks, double mdkiktfqoenau, string xptwdzg, string seokctscdmwlo, bool kfmpmrdxwb) {
	string calxmmfdaxor = "rpaqyjkyqaedfrfzkltnpzbnljyxbfqkuidhfjhbddidvggycgxmqqfqnbxicsuehdpyctounhbbsdsfkcaoyoqchkqkopwh";
	double rhmepnoystjqtfn = 17546;
	bool daangx = true;
	int jmlwkaahdwd = 2917;
	string qdrkxmabjsjbztb = "nfresnzydlxdiyesxjkpqgadrqhsugrlxaiucwtpdtnwmwcvuujpwsplihwbvinjdaboxfjmynqlryhzy";
	string gbuknbg = "oqwotzimzblufudkmnlcbxecexbgnjnzlfkiolesqrphdhxpdhnshjmyqvsizjsnsljsqonoofk";
	double shchtyzubjpmkbs = 89873;
	if (string("nfresnzydlxdiyesxjkpqgadrqhsugrlxaiucwtpdtnwmwcvuujpwsplihwbvinjdaboxfjmynqlryhzy") != string("nfresnzydlxdiyesxjkpqgadrqhsugrlxaiucwtpdtnwmwcvuujpwsplihwbvinjdaboxfjmynqlryhzy")) {
		int jafm;
		for (jafm = 4; jafm > 0; jafm--) {
			continue;
		}
	}
	if (17546 != 17546) {
		int swfvvmcl;
		for (swfvvmcl = 45; swfvvmcl > 0; swfvvmcl--) {
			continue;
		}
	}
	if (string("nfresnzydlxdiyesxjkpqgadrqhsugrlxaiucwtpdtnwmwcvuujpwsplihwbvinjdaboxfjmynqlryhzy") != string("nfresnzydlxdiyesxjkpqgadrqhsugrlxaiucwtpdtnwmwcvuujpwsplihwbvinjdaboxfjmynqlryhzy")) {
		int kxq;
		for (kxq = 36; kxq > 0; kxq--) {
			continue;
		}
	}
	if (string("nfresnzydlxdiyesxjkpqgadrqhsugrlxaiucwtpdtnwmwcvuujpwsplihwbvinjdaboxfjmynqlryhzy") == string("nfresnzydlxdiyesxjkpqgadrqhsugrlxaiucwtpdtnwmwcvuujpwsplihwbvinjdaboxfjmynqlryhzy")) {
		int ayzqsq;
		for (ayzqsq = 38; ayzqsq > 0; ayzqsq--) {
			continue;
		}
	}
	if (17546 == 17546) {
		int rgevixhw;
		for (rgevixhw = 68; rgevixhw > 0; rgevixhw--) {
			continue;
		}
	}
	return 74973;
}

bool rtgzzdj::wfzpjgzclpctvq(bool wijnwstro, int pkptl, string rwhppsj, double hmpnvnmp, double pvfxyavdxgqy, bool cwaygklisa, int afwkihoeldtjyhm, string iifdrajd, bool moohtbtop) {
	bool opxkq = true;
	if (true != true) {
		int sioczt;
		for (sioczt = 71; sioczt > 0; sioczt--) {
			continue;
		}
	}
	if (true == true) {
		int dsrh;
		for (dsrh = 82; dsrh > 0; dsrh--) {
			continue;
		}
	}
	if (true != true) {
		int sxhqbvogyc;
		for (sxhqbvogyc = 61; sxhqbvogyc > 0; sxhqbvogyc--) {
			continue;
		}
	}
	return false;
}

bool rtgzzdj::hymczrcifowotrkzsp(string wdwczslgtkzxt, double rwwkgtlq) {
	return true;
}

void rtgzzdj::yvveckisdq(double tmytqovu, bool axpixcabnwda, string sqrfbvyprcaekal, bool xfkgwwjsjctqrz, string miloo, bool ekinnqsswq, double timwmatg, double oluqs, bool ewjntjxzev) {
	bool rzpvsvxllin = true;
	double flyog = 44112;
	bool rdahmdhlkmdcefl = true;
	double bxjzzskdlwxp = 6417;
	bool sxbbekrkvz = false;
	bool fzhzbkgfl = false;
	double ermchzxy = 21356;
	double gpykdjagr = 17766;
	int mumppddozgf = 550;
	string dgqsifpodybx = "reslaetvlgehhslqrgiapqpwlraynguewiajnjypkknruloqxfhkmzsbeaycrqxxwjgibpbyorbg";
	if (false == false) {
		int sqqwzs;
		for (sqqwzs = 61; sqqwzs > 0; sqqwzs--) {
			continue;
		}
	}
	if (21356 == 21356) {
		int ryrqrv;
		for (ryrqrv = 33; ryrqrv > 0; ryrqrv--) {
			continue;
		}
	}

}

bool rtgzzdj::dwwxmmlyyur(string kstdis, int akcxdrjueb, double epddprhxdkp, bool aariroumncxask, double pgxuxusleq, bool pmspzuwp, bool jozyxfyrcxb, string fusjm, double adfprh, int shasiqtcvxqq) {
	double qrkbqwy = 19680;
	string hfzwbuwytpjztyb = "ciplkvqokqqiqovjeywgt";
	double cvseqyucxymchiz = 4810;
	double konvnhrrzgl = 45372;
	bool funlaym = false;
	double kasrg = 15139;
	bool nnvdo = true;
	int ioeljfju = 3245;
	bool bbuirtwy = true;
	bool uabhgcahtd = true;
	if (true != true) {
		int pvxvm;
		for (pvxvm = 98; pvxvm > 0; pvxvm--) {
			continue;
		}
	}
	return false;
}

double rtgzzdj::yxgirchchfxykph() {
	double znwskfequa = 12659;
	string jsljylabcwxhlbl = "tgnywjmkmrewdyfknwdxbwlgbnlvxerphcczjnrqonimwliqhtxwwvicojnshrdcunluaaj";
	bool latcw = true;
	double cnmwbvhnidvbpl = 32450;
	string jcugtpoymtvtuzk = "xclpaeneqvbmvfyzmezh";
	bool wnkvb = false;
	bool upcewcvvddcjvh = true;
	string nsnstcsbvo = "kyphwjwsnpxkjwdtnpregvbpatuvzkdycsyawajgrgaqzhdkptzfxujxkpsexirktbwmrnhsiqikq";
	if (string("tgnywjmkmrewdyfknwdxbwlgbnlvxerphcczjnrqonimwliqhtxwwvicojnshrdcunluaaj") == string("tgnywjmkmrewdyfknwdxbwlgbnlvxerphcczjnrqonimwliqhtxwwvicojnshrdcunluaaj")) {
		int mqnhw;
		for (mqnhw = 35; mqnhw > 0; mqnhw--) {
			continue;
		}
	}
	return 23718;
}

string rtgzzdj::xueujszvlzbelvlqxfkqwabwc(double ulbcrhiagfzd, string zlbrmclvbxsdld, double zggpzgkapfksey, int veowz, string oxfjflkaztyzow, int ookha, double ftsgjdbevcaa, bool ldbkopxdoqoodzl, int iyvvjdmxzs) {
	string pgqtrqsnv = "finkghkupxdcypfsjscslcgkwxlhzgkpgpmjwfvfrmvkqatekgbdvafczgjaepnvzmcvrxxhmcazowvdwjbkzqohaccllzxxzyod";
	bool ekotohizyw = true;
	bool jjtchccrt = true;
	double bgmyglkaa = 10425;
	int qlnns = 591;
	int yolbcyejdpqxua = 2388;
	string qnnrktxxubhxqc = "nfgdmprpvrsbju";
	bool ffekrfbacvn = true;
	double hzspbbxowjopcbf = 20490;
	int qzfqliofqpml = 816;
	if (true != true) {
		int eqdceb;
		for (eqdceb = 14; eqdceb > 0; eqdceb--) {
			continue;
		}
	}
	if (string("finkghkupxdcypfsjscslcgkwxlhzgkpgpmjwfvfrmvkqatekgbdvafczgjaepnvzmcvrxxhmcazowvdwjbkzqohaccllzxxzyod") == string("finkghkupxdcypfsjscslcgkwxlhzgkpgpmjwfvfrmvkqatekgbdvafczgjaepnvzmcvrxxhmcazowvdwjbkzqohaccllzxxzyod")) {
		int iqqlyxaex;
		for (iqqlyxaex = 72; iqqlyxaex > 0; iqqlyxaex--) {
			continue;
		}
	}
	if (string("finkghkupxdcypfsjscslcgkwxlhzgkpgpmjwfvfrmvkqatekgbdvafczgjaepnvzmcvrxxhmcazowvdwjbkzqohaccllzxxzyod") != string("finkghkupxdcypfsjscslcgkwxlhzgkpgpmjwfvfrmvkqatekgbdvafczgjaepnvzmcvrxxhmcazowvdwjbkzqohaccllzxxzyod")) {
		int kkgw;
		for (kkgw = 17; kkgw > 0; kkgw--) {
			continue;
		}
	}
	if (true == true) {
		int vb;
		for (vb = 62; vb > 0; vb--) {
			continue;
		}
	}
	return string("dggjfldjsnswjrr");
}

int rtgzzdj::bmdheojtvykvyqyfzvlvdu(int ytjgaizy, int znxdiufjnfsp) {
	string frqghdnytzkhsla = "pmfbyyxpqnbqhxkdcrrdopaczioiarulqxfxwifyxlggrqelfhtmucvxpfpstcozisxvnorebamwufpmsuheiihyvivs";
	int igeebqecwrju = 1063;
	string bpzisnz = "hjznkcuayxewkrznejobex";
	bool agwtgwsjribb = true;
	double ogftnwsnewwov = 82583;
	int lkioqjkznzupzf = 2190;
	if (82583 != 82583) {
		int tpnhfkbm;
		for (tpnhfkbm = 71; tpnhfkbm > 0; tpnhfkbm--) {
			continue;
		}
	}
	if (true != true) {
		int dpwm;
		for (dpwm = 48; dpwm > 0; dpwm--) {
			continue;
		}
	}
	if (string("hjznkcuayxewkrznejobex") == string("hjznkcuayxewkrznejobex")) {
		int rhipprjwg;
		for (rhipprjwg = 83; rhipprjwg > 0; rhipprjwg--) {
			continue;
		}
	}
	return 99068;
}

rtgzzdj::rtgzzdj() {
	this->hurbszwzajgvcs(52715, 41484);
	this->leicxbinojlqylqn();
	this->ogwcajxyyqichkniddkbkk(true, 66733, string("gmqdhqdjhtxhtdhdrwdscomeujgpejgqbwvqnydbjescaghldssyfljrbputkhllkikm"), string("nefjwakjaarqfqtahskdzesokvvoilvsbyce"), true);
	this->wfzpjgzclpctvq(false, 3044, string("xaqxexkqzvoqbsuhytvwbohf"), 57367, 37549, false, 4197, string("nukchcyrbnirosjaxwycoktcinzmhftffygctuhcjrngpkrjyffkwgeouglunlwnashnqppeuzngwawfxaozh"), true);
	this->hymczrcifowotrkzsp(string("nlrtrrvlwopbsxeufkkjumfojywywwemtqmyiwxdxcqusswmuzysnxvapdnivqnaot"), 37799);
	this->yvveckisdq(15593, true, string("sqkqwamogbtagnt"), false, string("dcuwyfugjxovlhuorxnbqirwdmlybbtuvrryozxz"), true, 24966, 61259, true);
	this->dwwxmmlyyur(string("nfrbswrjaifhjxnmtht"), 4929, 23621, false, 17209, false, false, string("euvivldiugoklizwdrsshmvunkngbrtwzj"), 43782, 105);
	this->yxgirchchfxykph();
	this->xueujszvlzbelvlqxfkqwabwc(13626, string("rythwwktqzeohjicueml"), 14482, 2795, string("t"), 1645, 51004, true, 2215);
	this->bmdheojtvykvyqyfzvlvdu(766, 1426);
	this->oszgawxsrajsczmqjxfdqn(false, string("ovrvkjswiytnapplmmlhzehjlxcqebjborolnnbwamymhenxbcqkmasytpuevceqqtdrnvvemkjmgktospiocljzwlclvhbhmqz"), 22636);
	this->xbfleogwkasm();
	this->lvinpeebanxfspff(string("ombgjnvladtrikgpniqbamfmvhnxdm"), true, string("adiezzycufooqff"), true, 70428, false, 54682, string("jfg"), string("nrvusolrjrbvnidizdtdtwloclqawpsfqvkhqgejuytfrhbdrxrytqkdxqskcodohufnvaug"), 3454);
	this->riqsnsixtrnkwhhgpywa(string("ybhemuogznjyjmxdqtvhmsyqflptswpomxaemhprfcceyjayrfzrwsyuze"), 1952, true, true, 3598, 509);
	this->iapivtgzzocabxbuhobds(true, true, 3659, 17680, 1190, string("wojocqwlhrevvoopghoapscatrsyfjumqdytnsyydcpbygahsavusnbdlvzyhwqptylcktoplejaysehxswhqanbdqcrsdow"), string("fivxkyohoikl"), string("wjjxq"), true, string("dmgemdqoaklerqdziauzzjvznwdmunajivgjxtsci"));
	this->kezhqmqrfanny(3613, 1636, 27084, string("barqv"), string("evtpdifgzvisj"), 8601, 2000, string("qcpgdbmesgjo"), false, string("flklgyhycszeqcwkltntqfegmkvpsqioqiqlaofupprbpunqrqihqwrkmo"));
	this->ucsssydycm();
	this->eapwnpzfhpyibgsofkjly(false, false, 49, 311, 95134, string("lgkqdkkwdjywj"));
	this->xobtdvgxan();
	this->unbwsdohpl(247, 27451, 5156, string("vbli"), true, string("tyreaaeaikkzccuhpwnbjwvrpcjgpoyxtthxvysihpeznogmljovudjvpggxeelqdxgrjmefazq"), true, false, 69245);
}


#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class puqyjhc {
public:
	string rxqqhyjmww;
	bool atvfabxesrdhnk;
	double hknrnbevt;
	double oxmcvrjjikc;
	int wlskkhmine;
	puqyjhc();
	double anepzpntsp(int ekbabozlaz, string jsuxtdajptt, double sfexjyymlpm, double swpqtaktlwzhwjw, double xkwnfuwysrrokxq, string roxfmrh, string ojzox, double imylkfkjxhfw, bool dspgsiaxzz);
	string wkqksmybetiennwlbizwaoeb();
	string qsdfaqqrnkkhlbslywwa(string tynddgwtotnukf, string rdwudifn, double tfdfr, int fgsqgvapphkdbhk, string jxxsqgnbml, double knhsw, string rnacegkurutqw, string ehlgvgslrbu, int frgsspjmtmystde, bool vytenzntm);
	double nmcwtuxemvphftyuetazyfu(double rtqzchgvebswy, bool iegwujfr, bool ialgei, bool mweohsvoatqzhws, int rplnllzrqiufay, string ypehdmhjr, string ebiqwkuzhun, bool emgwpcdnnckrc, bool swdmktzgz);
	void hzalsjguqjg(string ylhcubflxkkvuj, int bqdjjpltmkl, double wecfmszatoufxke);
	bool wyqegsxqzwvkr();
	int daahxhxvhqwnrvhazuwdgnb(bool bmmnrixj, string hbycm, string ukoxtg, double clvkeofpyjeybwv, bool rdluyln, string eqabvxpmp, double kdvmkyimjjnux);
	double kxrfwbhlmegcynfzevk(int wsclfrmekmjgfq, int gtdblrlepq, double dpqrjs, string ggiimdfnm, double ovfmwrqg, int czmfct, int skrsmvgqz);
	int vgrbltoocbplovbqhntza(double ogffbey, string cpnzowyqnbjco, int cobaqabta, bool tyzzjsnxpcmbd, double dxiwzirznj);
	bool rhajxbawbqasjnezxv(double xvkfqsnytxtr, bool zjwqdzgc, bool whaldulzhb, string mshzxiucgxg, double fjhwtete, string mmkrjpdq, double tvzbyhygyt, string arvxr, string sclsxbdyouscyun);

protected:
	double oeekv;

	void xpcsofyvrfxoxikxlepmbwh(string vuwlirctza, int ztueucvmfszy, int xzihecjzcj, int ochsidzmz, int ronrvnslxy);
	string uhnfdooftyvck(double ymqmgoaqdnlca, bool qnrajbwrehugxh, double qcrdubiiart, string kmduggrg, string pebzrnx, int btuxztnkkschbnu);
	void wxlrwheesazl(bool kxsxb, double zhnxnkfn, double ktxuswzaqkmpnfu, double tdadlqrjfdv);
	bool rxowkmuuaqfljcjbhktd(bool xpvjeggfsfzfxq, int mcibejh);
	int kajyoegwsosfeprayyjefutz();
	int dnbgtepynikdkl(bool fibwbo, bool kjbavaoml, int whuppeykeet, int xrskrefbyyrw, int efctau, int bltno, double kleydvxh, string ompwdn);
	int fecpxjzsts(string kccarbtdqbnabu, double cvebalhn, int hwhoyzj, double jhuzdk, double sitbgpxh, int xdlkzlsbnbhz, double pkafiuxobox, double axoznpqath, double xipmb, bool osgtt);
	string wjmqnraovwqn(bool esjbykzum, string oqwriaimyzwhxp, double kkevqcdzodg, bool dtsknwed);
	void bxqmgqnylgfda();
	double sxwekrhohhaqqroqqke(string qduukz, string cjmnjcu, double nlime, bool vmrtfbkovgshf);

private:
	bool eycqtt;

	double ocldffyncvdyrags(int ulcycs, string ytbvhujxaekc);
	int evixtzkdyp(string cqhswxubzmdn, bool wqddtvtbd, int ammix, bool dwirpkycnwb, string nhatlcbkvt, double azbdf, bool buvjddophhtod, bool zivnjixwuy, double jogbvcaxnnb);
	string allrmhdklvq(double tstekdpgwjqbag);
	int djladnlwgw(string dxgjtck, bool ztwjmua, bool ragcrxexol);
	bool luziyycxzdicnqussasw(double trxhtclmeb, bool xckazpraeyxejo, int mhpssufzl, string xzmfeghorlr, int pagaskt, int tsbpdaidw, bool nxuuaggi, int xvfdzsmkxsxsqtb);
	void kciyujitironawuwjiufiuxv(string pduzrvzwz, string qeqlce, int ipjnltikr, int hfvtafs);

};



double puqyjhc::ocldffyncvdyrags(int ulcycs, string ytbvhujxaekc) {
	int pgxhttlpuvg = 2359;
	double mgfurydd = 27718;
	bool taouflfinwzjc = false;
	double yrteetaxgwzhtk = 71807;
	int kaidlkfr = 2290;
	double iiuquhslh = 13396;
	double zrnlbankvt = 9135;
	double nedpncnpalcu = 11914;
	if (2359 != 2359) {
		int pwwsqgkdo;
		for (pwwsqgkdo = 94; pwwsqgkdo > 0; pwwsqgkdo--) {
			continue;
		}
	}
	return 74519;
}

int puqyjhc::evixtzkdyp(string cqhswxubzmdn, bool wqddtvtbd, int ammix, bool dwirpkycnwb, string nhatlcbkvt, double azbdf, bool buvjddophhtod, bool zivnjixwuy, double jogbvcaxnnb) {
	return 12939;
}

string puqyjhc::allrmhdklvq(double tstekdpgwjqbag) {
	string cidgss = "cfbyxkjflmtiafukl";
	int idyejnbydqadiod = 1701;
	int ajafrgffat = 1312;
	int ekcxipp = 1532;
	double gkkroyz = 13041;
	string zvgdg = "ixktxsyhtpbmrhyoutomoktnjaxbafysgupbvrjvvgqlv";
	bool xpusxl = true;
	int tqxsywqeavtxx = 1916;
	double ojfdpau = 85117;
	if (85117 == 85117) {
		int pua;
		for (pua = 36; pua > 0; pua--) {
			continue;
		}
	}
	if (string("ixktxsyhtpbmrhyoutomoktnjaxbafysgupbvrjvvgqlv") == string("ixktxsyhtpbmrhyoutomoktnjaxbafysgupbvrjvvgqlv")) {
		int go;
		for (go = 32; go > 0; go--) {
			continue;
		}
	}
	if (true != true) {
		int vtmsktmp;
		for (vtmsktmp = 11; vtmsktmp > 0; vtmsktmp--) {
			continue;
		}
	}
	if (1916 == 1916) {
		int hygsqsh;
		for (hygsqsh = 32; hygsqsh > 0; hygsqsh--) {
			continue;
		}
	}
	if (1532 != 1532) {
		int xrzv;
		for (xrzv = 8; xrzv > 0; xrzv--) {
			continue;
		}
	}
	return string("hpgzxqdnrwzrzbgqzlf");
}

int puqyjhc::djladnlwgw(string dxgjtck, bool ztwjmua, bool ragcrxexol) {
	return 21451;
}

bool puqyjhc::luziyycxzdicnqussasw(double trxhtclmeb, bool xckazpraeyxejo, int mhpssufzl, string xzmfeghorlr, int pagaskt, int tsbpdaidw, bool nxuuaggi, int xvfdzsmkxsxsqtb) {
	bool obucm = true;
	string jpiofzmfsui = "vmqzpqueoprhaqiwaoaocqyluekegfzubrfo";
	double hhkcstknrkqvfi = 27861;
	double ygeeaenoon = 13459;
	string tlhusf = "xosepukcrntpnvfttwodwtezzrutryxecctntrcfwjlipwxvpwexbryomiklcohscoznnxvykipxommxx";
	int njskgmhlwn = 2906;
	bool jcfsz = true;
	if (true == true) {
		int gvrxbmhc;
		for (gvrxbmhc = 0; gvrxbmhc > 0; gvrxbmhc--) {
			continue;
		}
	}
	return false;
}

void puqyjhc::kciyujitironawuwjiufiuxv(string pduzrvzwz, string qeqlce, int ipjnltikr, int hfvtafs) {
	int wpsoxei = 1728;
	double olfhsriyfssko = 13676;
	if (1728 != 1728) {
		int pv;
		for (pv = 20; pv > 0; pv--) {
			continue;
		}
	}
	if (13676 != 13676) {
		int bjvzmqizq;
		for (bjvzmqizq = 70; bjvzmqizq > 0; bjvzmqizq--) {
			continue;
		}
	}
	if (1728 != 1728) {
		int qgthedh;
		for (qgthedh = 57; qgthedh > 0; qgthedh--) {
			continue;
		}
	}
	if (1728 == 1728) {
		int zietvx;
		for (zietvx = 70; zietvx > 0; zietvx--) {
			continue;
		}
	}
	if (13676 != 13676) {
		int phaeipqamb;
		for (phaeipqamb = 65; phaeipqamb > 0; phaeipqamb--) {
			continue;
		}
	}

}

void puqyjhc::xpcsofyvrfxoxikxlepmbwh(string vuwlirctza, int ztueucvmfszy, int xzihecjzcj, int ochsidzmz, int ronrvnslxy) {

}

string puqyjhc::uhnfdooftyvck(double ymqmgoaqdnlca, bool qnrajbwrehugxh, double qcrdubiiart, string kmduggrg, string pebzrnx, int btuxztnkkschbnu) {
	int rchnjhdrrrafli = 4491;
	string mpsvdquwhi = "osnluazkqcyjmabwasmq";
	int mpcpjn = 135;
	double kierwvwzuo = 53967;
	bool ufjswqbrue = false;
	string oiqgcysnobt = "jhwnihbfpbymnuvvhjkejwlsrewljtcfqplcbxorfqeuogeddomydkghfvtiwlvhsmxeqlwixdl";
	if (string("jhwnihbfpbymnuvvhjkejwlsrewljtcfqplcbxorfqeuogeddomydkghfvtiwlvhsmxeqlwixdl") != string("jhwnihbfpbymnuvvhjkejwlsrewljtcfqplcbxorfqeuogeddomydkghfvtiwlvhsmxeqlwixdl")) {
		int hndlo;
		for (hndlo = 46; hndlo > 0; hndlo--) {
			continue;
		}
	}
	return string("sxubvpiywpzbdlkxwgs");
}

void puqyjhc::wxlrwheesazl(bool kxsxb, double zhnxnkfn, double ktxuswzaqkmpnfu, double tdadlqrjfdv) {
	double gevrdihjgfv = 9159;
	string avqqi = "kgwloaojbssgvktxwhgcasfjvf";
	string oynixfahwjf = "cnftapwaptrumfoqkerqhgzpqrhffrdcaiajbfqpijawnlmqghzzsokapav";
	int ixutrh = 136;
	int hnichmiah = 15;
	if (string("kgwloaojbssgvktxwhgcasfjvf") == string("kgwloaojbssgvktxwhgcasfjvf")) {
		int jucqcyic;
		for (jucqcyic = 44; jucqcyic > 0; jucqcyic--) {
			continue;
		}
	}

}

bool puqyjhc::rxowkmuuaqfljcjbhktd(bool xpvjeggfsfzfxq, int mcibejh) {
	string fubgpsmrvmbw = "dloorpptcfdglktwpiknlcmnbiyzeyvtsqjbfqsyrwqrpefeikljnym";
	if (string("dloorpptcfdglktwpiknlcmnbiyzeyvtsqjbfqsyrwqrpefeikljnym") == string("dloorpptcfdglktwpiknlcmnbiyzeyvtsqjbfqsyrwqrpefeikljnym")) {
		int xh;
		for (xh = 11; xh > 0; xh--) {
			continue;
		}
	}
	if (string("dloorpptcfdglktwpiknlcmnbiyzeyvtsqjbfqsyrwqrpefeikljnym") == string("dloorpptcfdglktwpiknlcmnbiyzeyvtsqjbfqsyrwqrpefeikljnym")) {
		int pnfil;
		for (pnfil = 86; pnfil > 0; pnfil--) {
			continue;
		}
	}
	return true;
}

int puqyjhc::kajyoegwsosfeprayyjefutz() {
	string yhvjcuyuef = "ydneaqfhehyzzczaiwprbmmelflhwkmcbbccgmhiatmsfuahkpomw";
	double mssyhfgwurdogca = 47462;
	bool frfwhbyez = false;
	bool svasdci = false;
	double fdsnrnkoo = 52236;
	int qtesgrzck = 1866;
	bool hvgac = false;
	bool tlbucfflijpo = true;
	if (false != false) {
		int oersrihfl;
		for (oersrihfl = 56; oersrihfl > 0; oersrihfl--) {
			continue;
		}
	}
	if (1866 != 1866) {
		int gtv;
		for (gtv = 88; gtv > 0; gtv--) {
			continue;
		}
	}
	if (47462 == 47462) {
		int lucklb;
		for (lucklb = 3; lucklb > 0; lucklb--) {
			continue;
		}
	}
	return 21850;
}

int puqyjhc::dnbgtepynikdkl(bool fibwbo, bool kjbavaoml, int whuppeykeet, int xrskrefbyyrw, int efctau, int bltno, double kleydvxh, string ompwdn) {
	string frpgkzwgffkdzp = "tecfxcundeaoj";
	int crnvaglkv = 5324;
	if (string("tecfxcundeaoj") == string("tecfxcundeaoj")) {
		int bug;
		for (bug = 30; bug > 0; bug--) {
			continue;
		}
	}
	if (5324 == 5324) {
		int dzlgia;
		for (dzlgia = 39; dzlgia > 0; dzlgia--) {
			continue;
		}
	}
	if (string("tecfxcundeaoj") == string("tecfxcundeaoj")) {
		int jynlxmqrv;
		for (jynlxmqrv = 46; jynlxmqrv > 0; jynlxmqrv--) {
			continue;
		}
	}
	return 96102;
}

int puqyjhc::fecpxjzsts(string kccarbtdqbnabu, double cvebalhn, int hwhoyzj, double jhuzdk, double sitbgpxh, int xdlkzlsbnbhz, double pkafiuxobox, double axoznpqath, double xipmb, bool osgtt) {
	double tnxzbvigoufowv = 44067;
	bool abmvd = false;
	double ykdpe = 86683;
	double scnqddlwhmmsvsd = 23218;
	int wjbnejykjkrqwyv = 79;
	int hvmvlhzhr = 1368;
	bool tcjog = false;
	int cdetdjbxwrg = 2067;
	return 95487;
}

string puqyjhc::wjmqnraovwqn(bool esjbykzum, string oqwriaimyzwhxp, double kkevqcdzodg, bool dtsknwed) {
	int ewksztbila = 312;
	string kfxiwieclldwgy = "bfvqvhzaimnjcodofjivqtzrpybdaecslpgecfq";
	bool yarjtm = false;
	if (false == false) {
		int qkwnmivrou;
		for (qkwnmivrou = 87; qkwnmivrou > 0; qkwnmivrou--) {
			continue;
		}
	}
	if (string("bfvqvhzaimnjcodofjivqtzrpybdaecslpgecfq") == string("bfvqvhzaimnjcodofjivqtzrpybdaecslpgecfq")) {
		int pfnmjx;
		for (pfnmjx = 30; pfnmjx > 0; pfnmjx--) {
			continue;
		}
	}
	if (string("bfvqvhzaimnjcodofjivqtzrpybdaecslpgecfq") == string("bfvqvhzaimnjcodofjivqtzrpybdaecslpgecfq")) {
		int zygnenr;
		for (zygnenr = 28; zygnenr > 0; zygnenr--) {
			continue;
		}
	}
	if (312 != 312) {
		int mhoytwdmog;
		for (mhoytwdmog = 5; mhoytwdmog > 0; mhoytwdmog--) {
			continue;
		}
	}
	if (string("bfvqvhzaimnjcodofjivqtzrpybdaecslpgecfq") != string("bfvqvhzaimnjcodofjivqtzrpybdaecslpgecfq")) {
		int ivwty;
		for (ivwty = 82; ivwty > 0; ivwty--) {
			continue;
		}
	}
	return string("kirfiitl");
}

void puqyjhc::bxqmgqnylgfda() {

}

double puqyjhc::sxwekrhohhaqqroqqke(string qduukz, string cjmnjcu, double nlime, bool vmrtfbkovgshf) {
	bool hodsrivozivumes = true;
	bool adaeulpq = false;
	bool xnccw = false;
	double mrtmeqp = 22070;
	if (true == true) {
		int qea;
		for (qea = 36; qea > 0; qea--) {
			continue;
		}
	}
	return 14433;
}

double puqyjhc::anepzpntsp(int ekbabozlaz, string jsuxtdajptt, double sfexjyymlpm, double swpqtaktlwzhwjw, double xkwnfuwysrrokxq, string roxfmrh, string ojzox, double imylkfkjxhfw, bool dspgsiaxzz) {
	return 5611;
}

string puqyjhc::wkqksmybetiennwlbizwaoeb() {
	int xpqbinjpizdh = 2730;
	double wwhjqosop = 62821;
	double gbkccdrayxzknp = 7330;
	double iwpnpeyrygxyblm = 14366;
	int hkjzgxpctbb = 313;
	string qtjffpt = "xuabbmgovocbbgnejpaphrtrhrukjnecelntkgkxqqpkzvmscyizbqjhisgi";
	bool rzmixhcsj = true;
	bool ptxwov = true;
	if (14366 == 14366) {
		int vigyeg;
		for (vigyeg = 47; vigyeg > 0; vigyeg--) {
			continue;
		}
	}
	return string("thoazayqievg");
}

string puqyjhc::qsdfaqqrnkkhlbslywwa(string tynddgwtotnukf, string rdwudifn, double tfdfr, int fgsqgvapphkdbhk, string jxxsqgnbml, double knhsw, string rnacegkurutqw, string ehlgvgslrbu, int frgsspjmtmystde, bool vytenzntm) {
	int mitic = 3616;
	int jiiyvkrddjrxxs = 1302;
	bool zzgpxlnlskesmtp = false;
	int bvfjyiyo = 3316;
	if (1302 != 1302) {
		int ou;
		for (ou = 16; ou > 0; ou--) {
			continue;
		}
	}
	if (3316 != 3316) {
		int bni;
		for (bni = 37; bni > 0; bni--) {
			continue;
		}
	}
	if (1302 == 1302) {
		int pddgya;
		for (pddgya = 38; pddgya > 0; pddgya--) {
			continue;
		}
	}
	return string("sibxigzlvroifjqnu");
}

double puqyjhc::nmcwtuxemvphftyuetazyfu(double rtqzchgvebswy, bool iegwujfr, bool ialgei, bool mweohsvoatqzhws, int rplnllzrqiufay, string ypehdmhjr, string ebiqwkuzhun, bool emgwpcdnnckrc, bool swdmktzgz) {
	string rrohpmibk = "ngkzzhwpyph";
	if (string("ngkzzhwpyph") == string("ngkzzhwpyph")) {
		int jqzoqaa;
		for (jqzoqaa = 45; jqzoqaa > 0; jqzoqaa--) {
			continue;
		}
	}
	if (string("ngkzzhwpyph") == string("ngkzzhwpyph")) {
		int twuy;
		for (twuy = 68; twuy > 0; twuy--) {
			continue;
		}
	}
	if (string("ngkzzhwpyph") == string("ngkzzhwpyph")) {
		int om;
		for (om = 11; om > 0; om--) {
			continue;
		}
	}
	return 50891;
}

void puqyjhc::hzalsjguqjg(string ylhcubflxkkvuj, int bqdjjpltmkl, double wecfmszatoufxke) {
	double rzryzrlzrlxv = 58637;
	bool fdxhukuksozk = true;
	string vuozryvtofxqca = "mfmaumwzsjevnnppkylhuresnknzfzucbkacqdnyylviukpjnmnaxafrglqyumzqdzudittkuxyga";
	int klxmmzngigpxpr = 2994;
	double zfsemkwbi = 69228;
	if (string("mfmaumwzsjevnnppkylhuresnknzfzucbkacqdnyylviukpjnmnaxafrglqyumzqdzudittkuxyga") != string("mfmaumwzsjevnnppkylhuresnknzfzucbkacqdnyylviukpjnmnaxafrglqyumzqdzudittkuxyga")) {
		int fsbh;
		for (fsbh = 18; fsbh > 0; fsbh--) {
			continue;
		}
	}
	if (true != true) {
		int ovxbwsw;
		for (ovxbwsw = 66; ovxbwsw > 0; ovxbwsw--) {
			continue;
		}
	}

}

bool puqyjhc::wyqegsxqzwvkr() {
	int lqjgjvgfu = 1187;
	bool hzjznf = true;
	double nyivatiu = 11518;
	if (true == true) {
		int prxibvoshx;
		for (prxibvoshx = 95; prxibvoshx > 0; prxibvoshx--) {
			continue;
		}
	}
	if (1187 == 1187) {
		int bsqjqqzhwu;
		for (bsqjqqzhwu = 62; bsqjqqzhwu > 0; bsqjqqzhwu--) {
			continue;
		}
	}
	return true;
}

int puqyjhc::daahxhxvhqwnrvhazuwdgnb(bool bmmnrixj, string hbycm, string ukoxtg, double clvkeofpyjeybwv, bool rdluyln, string eqabvxpmp, double kdvmkyimjjnux) {
	bool ipqsdbyhujgkob = false;
	double nifvvu = 5870;
	int hhsdhpkztvn = 4649;
	int juohlps = 325;
	bool wvzxeqqwscujs = false;
	int migrlu = 4092;
	int papwslgwaz = 703;
	if (703 != 703) {
		int jcfaruzmsr;
		for (jcfaruzmsr = 61; jcfaruzmsr > 0; jcfaruzmsr--) {
			continue;
		}
	}
	if (4092 != 4092) {
		int obfltkvicp;
		for (obfltkvicp = 29; obfltkvicp > 0; obfltkvicp--) {
			continue;
		}
	}
	if (4092 != 4092) {
		int miowreb;
		for (miowreb = 90; miowreb > 0; miowreb--) {
			continue;
		}
	}
	return 82558;
}

double puqyjhc::kxrfwbhlmegcynfzevk(int wsclfrmekmjgfq, int gtdblrlepq, double dpqrjs, string ggiimdfnm, double ovfmwrqg, int czmfct, int skrsmvgqz) {
	bool fnnjstu = false;
	bool cwyhcirppzmgl = false;
	double zjjaqetilq = 37233;
	bool rcawldz = false;
	if (false != false) {
		int smccd;
		for (smccd = 78; smccd > 0; smccd--) {
			continue;
		}
	}
	return 12002;
}

int puqyjhc::vgrbltoocbplovbqhntza(double ogffbey, string cpnzowyqnbjco, int cobaqabta, bool tyzzjsnxpcmbd, double dxiwzirznj) {
	string gqigkfkikaz = "nmftapfmvskaa";
	string auusxrgg = "pwdakildzaakpgtbkbvka";
	string hsgxxhnja = "anarnfxzfarbkuzzountmulwvbdldmicuephdhnwehrbuwibzdbyaqgif";
	int gqafslpqtf = 5929;
	int ganafjtw = 5998;
	int shnetg = 813;
	bool pfukhqoxr = true;
	string oecbyx = "egggzvpkzhofmfghrclnctdzkezeefozkgfoldpskxbwonygwrzzychbnmwmambtnvgvfoagofwmsbwgjwhvnfrncxuvog";
	return 67831;
}

bool puqyjhc::rhajxbawbqasjnezxv(double xvkfqsnytxtr, bool zjwqdzgc, bool whaldulzhb, string mshzxiucgxg, double fjhwtete, string mmkrjpdq, double tvzbyhygyt, string arvxr, string sclsxbdyouscyun) {
	int ophxdekqzrzkjc = 559;
	if (559 == 559) {
		int sqn;
		for (sqn = 2; sqn > 0; sqn--) {
			continue;
		}
	}
	if (559 != 559) {
		int wgrdo;
		for (wgrdo = 26; wgrdo > 0; wgrdo--) {
			continue;
		}
	}
	if (559 != 559) {
		int lpl;
		for (lpl = 58; lpl > 0; lpl--) {
			continue;
		}
	}
	return false;
}

puqyjhc::puqyjhc() {
	this->anepzpntsp(394, string("nrazuyyppvrhyzqodcynjyohugzjoflcumtjumkgdhl"), 5046, 51467, 23855, string("nliuujhuotumgawvxmmiredonugojsnphdbxdwegxesxhmbkegctdyjvvodtiuwjmoymmbx"), string("sycjibdfavauanasqkrrpvolyapgpoxrwpaxwdsofevrdlvstihahkp"), 7177, false);
	this->wkqksmybetiennwlbizwaoeb();
	this->qsdfaqqrnkkhlbslywwa(string("qrouqdbgyzxfdpzymgkvsyhcrpvqhocpsy"), string("gvxvqwyzymmduidhhqbbkeixbrlmffypnfoecrzomkmdqzjtcampqzflzwgocegjhgxeqhzjzayyayxiaazse"), 13875, 5155, string("vtymcdghjtuzfxjnwmmtprjstll"), 21007, string("a"), string("vglixjxtfnkdrjdbbltlvqmtnogcpqlisgfhzxvqpkebgfafoqgkxjjksknbr"), 453, true);
	this->nmcwtuxemvphftyuetazyfu(13023, true, false, false, 2960, string("yfqbvjlaqxybslavvnjxarzkqfhpmvkdonnjqn"), string("uoctovfpxftriwrbmbthkbeojghdvfljugektizcnvbutiqllsegppudsxqosrhnfdliawmbxxdqimjptuezfcgflmekkboprhyx"), false, true);
	this->hzalsjguqjg(string(""), 5357, 44135);
	this->wyqegsxqzwvkr();
	this->daahxhxvhqwnrvhazuwdgnb(true, string("ddpfgdecurcbkngzmjxdlakorlaoznvxklcbjkzpkpobrkyreyxjrqrfyydwaratoqhzamildbfnusnclynjmzyyynr"), string("lnojcfpddzbzzlcswc"), 51676, true, string("tg"), 3494);
	this->kxrfwbhlmegcynfzevk(2013, 645, 47104, string("gnctpxgfomnbiwevmuzuheajqqhdzetlqobbjvfwrezhjr"), 33006, 4085, 4968);
	this->vgrbltoocbplovbqhntza(30969, string(""), 4311, true, 6810);
	this->rhajxbawbqasjnezxv(73167, true, false, string("hwqlvkbrcabevjuojwbigzrdbumtilklychlklbulrkegsyjtzldzgditsm"), 22504, string("iputoayahj"), 8371, string("nbovipdwaedufrmtscyinomxbdwmcdltksfxbapwtkvhplgitayyhklydjp"), string("rotuxmbemcqobdeyhbncdmfypzoenyqczkje"));
	this->xpcsofyvrfxoxikxlepmbwh(string("phzkrhmnabgnwaqacxhlp"), 4621, 4676, 3342, 5550);
	this->uhnfdooftyvck(6056, true, 17790, string("rncctgdieksofm"), string("ipddrdozzafeggxoqjbavdultitfxhmqdpheagrpfipotzbgssblodfpbstxqzhsdvfmkaonvkzpzicd"), 2161);
	this->wxlrwheesazl(false, 33534, 79979, 7957);
	this->rxowkmuuaqfljcjbhktd(false, 7708);
	this->kajyoegwsosfeprayyjefutz();
	this->dnbgtepynikdkl(true, true, 5272, 943, 1342, 4602, 25092, string("mamtuexpbqrjrznesjqzzgrqojvynnkprehttmv"));
	this->fecpxjzsts(string("btzvjbazkfqndhgctkaopqqrwdusmdzmmdlxgxnynvc"), 34493, 2858, 59953, 9212, 351, 14675, 57758, 286, true);
	this->wjmqnraovwqn(true, string("sxmiukbafyfgntddy"), 8223, true);
	this->bxqmgqnylgfda();
	this->sxwekrhohhaqqroqqke(string("pladltynwkmftyzazfbqmaenbzbumeyufqcnydpkmicpyfjnix"), string("byglzcmsuplzkvjngzpxkloyegqdm"), 37933, false);
	this->ocldffyncvdyrags(2164, string("vzdgqyjpxspkjevttyxnanqvrgcfkeyuzwigetpxkoryxevfiqmefwxjojuuxxlptyiemgydev"));
	this->evixtzkdyp(string("jfvctogwkapiwflxwtcft"), false, 5604, false, string("egctpjqvpfntlkfafffieoufcywxnikloyxoccmbfiiwqapcfydajscrdvrtxwvywyytndqjqjiumqtvqjjdtmtwoksth"), 8235, true, false, 76969);
	this->allrmhdklvq(54767);
	this->djladnlwgw(string("lpvwxjirltthsehfnmnwjwefklsdiayrcqj"), false, false);
	this->luziyycxzdicnqussasw(46941, false, 1007, string("asvbmqjphfwlglyjgqxkcigvyitnamegcudqrzgothcrqrglf"), 4439, 1934, false, 2128);
	this->kciyujitironawuwjiufiuxv(string("vuswnhxmylfpjuynzqtehgplsxcakcnqjkritqzodzkvijcf"), string("ffhmjrykdbzyuzqnboefzzzpraietewzohdkozvueghfynkixolgzwnkldlgyzrrduttsrgbkznihtovomhcdzdiquodknw"), 2617, 445);
}




//--------------------------------------------//

static bool memerino = false;
void ResoSetup::Resolve(IClientEntity* pEntity, int CurrentTarget) // int + currenttarget = bad
{
	IClientEntity* pLocal = hackManager.pLocal();


	float angletolerance;
	angletolerance = pEntity->GetEyeAnglesXY()->y + 180.0;
	float consider;
	float v24;
	double v20;



	if (Interfaces::Engine->IsConnected() & pLocal->IsPlayer())
	{

#define RandomInt(nMin, nMax) (rand() % (nMax - nMin + 1) + nMin);
		std::string aa_info[64];


		//------bool------//

		std::vector<int> HitBoxesToScan;




		bool MeetsLBYReq;
		bool maybefakehead = 0;

		//------bool------//

		//------float------//
		float org_yaw;
		float server_time = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;

		//------float------//


		//------Statics------//
		static Vector Skip[65];
		static float StaticHeadAngle = 0;

		static bool GetEyeAngles[65]; //Resolve: Frame EyeAngle
		static bool GetLowerBodyYawTarget[65]; //Resolve: LowerBody
		static bool isLBYPredictited[65];
		static bool switch2;

		static float OldLowerBodyYaws[65];
		static float OldYawDeltas[65];
		static float oldTimer[65];

	

	





		ResoSetup::NewANgles[pEntity->GetIndex()] = *pEntity->GetEyeAnglesXY();
		ResoSetup::newlby[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
		ResoSetup::newsimtime = pEntity->GetSimulationTime();
		ResoSetup::newdelta[pEntity->GetIndex()] = pEntity->GetEyeAnglesXY()->y;
		ResoSetup::newlbydelta[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
		ResoSetup::finaldelta[pEntity->GetIndex()] = ResoSetup::newdelta[pEntity->GetIndex()] - ResoSetup::storeddelta[pEntity->GetIndex()];
		ResoSetup::finallbydelta[pEntity->GetIndex()] = ResoSetup::newlbydelta[pEntity->GetIndex()] - ResoSetup::storedlbydelta[pEntity->GetIndex()];
		if (newlby == storedlby)
			ResoSetup::lbyupdated = false;
		else
			ResoSetup::lbyupdated = true;
		StoreThings(pEntity);

		IClientEntity* player = (IClientEntity*)pLocal;
		IClientEntity* pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
		INetChannelInfo *nci = Interfaces::Engine->GetNetChannelInfo();
		Vector* eyeAngles = player->GetEyeAnglesXY();

		if (pEntity->GetFlags() & FL_ONGROUND)
			MeetsLBYReq = true;
		else
			MeetsLBYReq = false;

		StoreFGE(pEntity);

		float OldLowerBodyYaw;




		static float LatestLowerBodyYawUpdateTime[55];
		static float LatestLowerBodyYawUpdateTime1[55];
		static float brokenlby[55];

		static float time_at_update[65];
		float kevin[64];
		static bool bLowerBodyIsUpdated = false;

		if (pEntity->GetLowerBodyYaw() != kevin[pEntity->GetIndex()])
			bLowerBodyIsUpdated = true;
		else
			bLowerBodyIsUpdated = false;

	



		bool IsMoving;

		if (pEntity->GetVelocity().Length2D() >= 1)
			IsMoving = true;
		else
			IsMoving = false;

		bool tolerantLBY;
		if (!MeetsLBYReq)
			if (pEntity->GetLowerBodyYawTarget() + 35)
				tolerantLBY = true;
			else
				tolerantLBY = false;

		bool IsBreathing;
		{
			bool idk = 0;
			{
				if (pEntity->GetEyePosition().y)
					idk = true;
				else
					idk = false;
			}

			server_time >= idk;
			if (server_time >= idk + 0.705f)
			{
				IsBreathing = true;
			}
			else
				IsBreathing = false;

		}

		float breakcomp = pEntity->GetEyeAnglesXY()->y - (pEntity->GetLowerBodyYaw() - 35);

		


	




		/*
		static float LastLBYUpdateTime = 0;
		IClientEntity* pLocal = hackManager.pLocal();
		float server_time = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;
		if (server_time >= LastLBYUpdateTime)
		{
		LastLBYUpdateTime = server_time + 1.125f;

		*/


	

		


			bool sidejitter2;
			bool sidejitter1;

			bool bLowerBodyUpdated = false;
			static bool isMoving;
			float PlayerIsMoving = abs(pEntity->GetVelocity().Length());
			if (PlayerIsMoving > 0.1) isMoving = true;
			else if (PlayerIsMoving <= 0.1) isMoving = false;
			bool FakeWalking;
			bool shouldbrute;
			float oldlowerbodyyaw;
			bool IsUsingFakeAngles = false;

			bool IsFast;

			if (pEntity->GetFlags() & FL_ONGROUND & pEntity->GetVelocity().Length2D() >= 170.5)
				IsFast = true;
			else
				IsFast = false;


			int flip = (int)floorf(Interfaces::Globals->curtime / 0.34) % 3;
			static bool bFlipYaw;
			float flInterval = Interfaces::Globals->interval_per_tick;

			if (ResoSetup::storedlby[pEntity->GetIndex()] > pEntity->GetLowerBodyYaw() + 119)
				lbyupdated = true;



			if (Menu::Window.RageBotTab.AimbotResolver.GetIndex() == 0)
			{

			}
			else if (Menu::Window.RageBotTab.AimbotResolver.GetIndex() == 1)
			{
			
				if (Globals::missedshots >= Menu::Window.RageBotTab.forceback.GetValue())
				{
					switch (Globals::missedshots % 5)
					{
					case 0:	pEntity->GetLowerBodyYaw() + 80; break;
					case 1: pEntity->GetLowerBodyYaw() + 150; break;
					case 2: pEntity->GetLowerBodyYaw() - 150; break;
					case 3: pEntity->GetLowerBodyYaw() - 80; break;
					case 4: pEntity->GetLowerBodyYaw() - (90 - rand() % 180); break;
					}
				}

				// when someone is moving, their lby is always updating. So getting lby, or applying OldLowerBody is enough.
				if (pEntity->IsMoving())
				{
					switch (Globals::missedshots % 4)
					{
					case 0:
						pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw(); break;
					case 1:
						pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 25; break;
					case 2:
						pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 25; break;
					case 3:
						pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw(); break;
					}

				}

				else  // very basic stuff. Nowadays everyone uses opposite AAs or close to opposite AAs, soooooooo
				{
					if (pEntity->GetEyeAnglesXY()->x <= 25) // this MIGHT fuck me over, but in the case you meet a legit AA, this should do K
						pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw();
					else
					{
						switch (Globals::Shots % 10)
						{
						case 0: pEntity->GetEyeAnglesXY()->y -= 180; break;
						case 1: pEntity->GetEyeAnglesXY()->y -= 160; break;
						case 2: pEntity->GetEyeAnglesXY()->y += 160; break;
						case 3: pEntity->GetEyeAnglesXY()->y -= 180; break;
						case 4: pEntity->GetEyeAnglesXY()->y -= 130; break;
						case 5: pEntity->GetEyeAnglesXY()->y += 130; break;
						case 6: pEntity->GetEyeAnglesXY()->y = (pEntity->GetLowerBodyYaw() - 45); break;
						case 7: pEntity->GetEyeAnglesXY()->y = (pEntity->GetLowerBodyYaw() + 45); break;
						case 8: pEntity->GetEyeAnglesXY()->y = (pEntity->GetLowerBodyYaw() - 120); break;
						case 9: pEntity->GetEyeAnglesXY()->y = (pEntity->GetLowerBodyYaw() + 120); break;
						}
					}
					/* cases 6-9 (MASSIVE meme) should effectively make even the use of this resolver in HvH
					actually not too bad. The above is just usual ayyware memes, but i guess if you are shooting more than
					1 person at once, or are dumping fakes, this will inevetably compensate for LBY breaks. Maybe case 6
					is a bit too late for such a thing, but hey, not a bad thing to build off of. */
				}
	
			}
			else if (Menu::Window.RageBotTab.AimbotResolver.GetIndex() == 2)
			{

				if (Globals::missedshots >= Menu::Window.RageBotTab.forceback.GetValue())
				{
					switch (Globals::missedshots % 5)
					{
					case 0:	pEntity->GetLowerBodyYaw() + 80; break;
					case 1: pEntity->GetLowerBodyYaw() + 150; break;
					case 2: pEntity->GetLowerBodyYaw() - 150; break;
					case 3: pEntity->GetLowerBodyYaw() - 80; break;
					case 4: pEntity->GetLowerBodyYaw() - (90 - rand() % 180); break;
					}
				}

				if (Globals::missedshots > 3 && Globals::missedshots < 21)
				{
					if (MeetsLBYReq && lbyupdated)
					{
						pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw();
					}
					else if (!MeetsLBYReq && lbyupdated)
					{
						switch (Globals::Shots % 6)
						{
						case 1:
							pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 15;
							break;
						case 2:
							pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 40;
							break;
						case 3:
							pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 15;
							break;
						case 4:
							pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 40;
							break;
						case 5:
							pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 160;
							break;
						case 6:
							pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 160;
							break;
						}
					}
					else
						pEntity->GetEyeAnglesXY()->y = (rand() % 180 - rand() % 35);
				}

				else if (Globals::missedshots >= 2 && Globals::missedshots <= 3)
				{
					if (MeetsLBYReq && lbyupdated)
					{
						pEntity->GetEyeAnglesXY()->y = ResoSetup::finallbydelta[pEntity->GetIndex()];
					}
					else
						pEntity->GetEyeAnglesXY()->y = ResoSetup::finaldelta[pEntity->GetIndex()];
				}
				else
				{
					if (MeetsLBYReq && lbyupdated)
					{
						bool timer = 0;
						if (timer)
							pEntity->GetEyeAnglesXY()->y = ResoSetup::finallbydelta[pEntity->GetIndex()] + rand() % 35;
						else
							pEntity->GetEyeAnglesXY()->y = ResoSetup::finallbydelta[pEntity->GetIndex()] - rand() % 35;
						timer = !timer;
					}
					else
					{
						bool timer = 0;
						if (timer)
							pEntity->GetEyeAnglesXY()->y = ResoSetup::finaldelta[pEntity->GetIndex()] + rand() % 35;
						else
							pEntity->GetEyeAnglesXY()->y = ResoSetup::finaldelta[pEntity->GetIndex()] - rand() % 35;
						timer = !timer;
					}
				}
			
			}
			else if (Menu::Window.RageBotTab.AimbotResolver.GetIndex() == 3)
			{

			

				for (int i = 0; i < Interfaces::Engine->GetMaxClients(); ++i)
				{
					IClientEntity* pLocal = hackManager.pLocal();
					IClientEntity *player = (IClientEntity*)Interfaces::EntList->GetClientEntity(i);

					if (!player || player->IsDormant() || player->IsAlive() || (DWORD)player == (DWORD)pLocal)
						continue;

					if (!player)
						continue;

					if (pLocal)
						continue;

					if (pLocal && player)
					{

						Vector* eyeAngles = player->GetEyeAnglesXY();

						if (eyeAngles->x > 90.0 || eyeAngles->x < 60.0) eyeAngles->x = -88.9f;
						else if (eyeAngles->x > 60.0 && eyeAngles->x < 20.0) eyeAngles->x = 50.f;
						else if (eyeAngles->x > 20.0 && eyeAngles->x < -30.0) eyeAngles->x = 0.f;
						else if (eyeAngles->x < -30.0 && eyeAngles->x < 60.0) eyeAngles->x = 55.f;
						else eyeAngles->x = 88.9f;




					}
					else
					{
						eyeAngles->x = 88.9;
						pEntity->GetEyeAnglesXY()->y = 90;
					}
				}

				if (Globals::missedshots >= Menu::Window.RageBotTab.forceback.GetValue())
				{
					switch (Globals::missedshots % 5)
					{
					case 1:	pEntity->GetLowerBodyYaw() + 80; break;
					case 2: pEntity->GetLowerBodyYaw() + 150; break;
					case 3: pEntity->GetLowerBodyYaw() - 150; break;
					case 4: pEntity->GetLowerBodyYaw() - 80; break;
					case 5: pEntity->GetLowerBodyYaw() - (90 - rand() % 180); break;
					}
				}

				if (LatestLowerBodyYawUpdateTime[pEntity->GetIndex()] < pEntity->GetSimulationTime() || bLowerBodyIsUpdated)
				{
					pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw();
					LatestLowerBodyYawUpdateTime[pEntity->GetIndex()] = pEntity->GetSimulationTime() + 1.1;
				}
				else
				{
					if (IsMovingOnGround(pEntity))
					{
						if (IsFakeWalking(pEntity))
						{
							HitBoxesToScan.clear();
							HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
							HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
							HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
							HitBoxesToScan.push_back((int)CSGOHitboxID::LowerChest);
							HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
							HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
							HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
						}
						else
						{
							switch (Globals::missedshots % 4)
							{
							case 0:
								pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw(); break;
							case 1:
							{
								if (memerino)
									pEntity->GetEyeAnglesXY()->y = (pEntity->GetLowerBodyYaw() - 15); 
								else pEntity->GetEyeAnglesXY()->y = (pEntity->GetLowerBodyYaw() + 15);

								memerino = !memerino;
							}
							break;

							case 2:
							{
								if (memerino)
									pEntity->GetEyeAnglesXY()->y = (pEntity->GetLowerBodyYaw() - 30);
								else pEntity->GetEyeAnglesXY()->y = (pEntity->GetLowerBodyYaw() + 30);

								memerino = !memerino;
							}
							break;
							case 3:
								pEntity->GetEyeAnglesXY()->y = OldLowerBodyYaw; break;
							}
						
						}
					}
					else if (IsMovingOnInAir(pEntity))
					{
						HitBoxesToScan.clear();
						HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
						HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
						HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
						HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
						HitBoxesToScan.push_back((int)CSGOHitboxID::LowerChest);
						HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
						HitBoxesToScan.push_back((int)CSGOHitboxID::Head);

						switch (Globals::missedshots % 7)
						{
						case 0: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw(); break;
						case 1: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 25; break;
						case 2: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 25; break;
						case 3: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 135; break;
						case 4: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 135; break;
						case 5: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 165; break;
						case 6: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 165; break;
						}
					}
					else if (!IsMoving && MeetsLBYReq)
					{
						if (Lbywithin35(pEntity))
						{
							if (HasFakeHead(pEntity))
							{
								(pEntity->GetEyeAnglesXY()->y = *pEntity->GetLowerBodyYawTarget()) - 170;

								switch (Globals::missedshots % 7)
								{
								case 0: pEntity->GetEyeAnglesXY()->y = (pEntity->GetEyeAnglesXY()->y - *pEntity->GetLowerBodyYawTarget() - 160); break;
								case 1: pEntity->GetEyeAnglesXY()->y = pEntity->GetEyeAnglesXY()->y - pEntity->GetLowerBodyYaw() + 160; break;
								case 2: pEntity->GetEyeAnglesXY()->y = pEntity->GetEyeAnglesXY()->y + pEntity->GetLowerBodyYaw() + 130; break;
								case 3: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - (OldLowerBodyYaw + 90); break;
								case 4: pEntity->GetEyeAnglesXY()->y = (pEntity->GetEyeAnglesXY()->y + pEntity->GetLowerBodyYaw()) - 120; break;
								case 5: pEntity->GetEyeAnglesXY()->y = (pEntity->GetEyeAnglesXY()->y + pEntity->GetLowerBodyYaw()) + 120; break;
								case 6: pEntity->GetEyeAnglesXY()->y = (pEntity->GetEyeAnglesXY()->y - pEntity->GetLowerBodyYaw()) - (150 - rand() % 30); break;
								}
							}
							else
							{
								switch (Globals::missedshots % 7)
								{
								case 0: pEntity->GetEyeAnglesXY()->y = pEntity->GetEyeAnglesXY()->y - (*pEntity->GetLowerBodyYawTarget() - 180); break;
								case 1: pEntity->GetEyeAnglesXY()->y = pEntity->GetEyeAnglesXY()->y - pEntity->GetLowerBodyYaw(); break;
								case 2: pEntity->GetEyeAnglesXY()->y = pEntity->GetEyeAnglesXY()->y + pEntity->GetLowerBodyYaw(); break;
								case 3: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - OldLowerBodyYaw; break;
								case 4: pEntity->GetEyeAnglesXY()->y = (pEntity->GetEyeAnglesXY()->y + pEntity->GetLowerBodyYaw()) - 120; break;
								case 5: pEntity->GetEyeAnglesXY()->y = (pEntity->GetEyeAnglesXY()->y + pEntity->GetLowerBodyYaw()) + 120; break;
								case 6: pEntity->GetEyeAnglesXY()->y = (pEntity->GetEyeAnglesXY()->y - pEntity->GetLowerBodyYaw()) - (150 - rand() % 30); break;
								}
							}
						}

					}
					else if (Globals::missedshots >= (Menu::Window.RageBotTab.forceback.GetValue()))
					{
						switch (Globals::Shots % 10)
						{
						case 0: pEntity->GetEyeAnglesXY()->y = (*pEntity->GetLowerBodyYawTarget() - 180); break;
						case 1: pEntity->GetEyeAnglesXY()->y = (*pEntity->GetLowerBodyYawTarget() - 150); break;
						case 2: pEntity->GetEyeAnglesXY()->y = (*pEntity->GetLowerBodyYawTarget() + 150); break;
						case 3: pEntity->GetEyeAnglesXY()->y = (*pEntity->GetLowerBodyYawTarget() + 110); break;
						case 4: pEntity->GetEyeAnglesXY()->y = (*pEntity->GetLowerBodyYawTarget() + 75); break;
						case 5: pEntity->GetEyeAnglesXY()->y = (*pEntity->GetLowerBodyYawTarget() - 110); break;
						case 6: pEntity->GetEyeAnglesXY()->y = (*pEntity->GetLowerBodyYawTarget() - 75); break;
						case 7: pEntity->GetEyeAnglesXY()->y = (*pEntity->GetLowerBodyYawTarget() - 40); break;
						case 8: pEntity->GetEyeAnglesXY()->y = (*pEntity->GetLowerBodyYawTarget() + 40); break;
						case 9: pEntity->GetEyeAnglesXY()->y = *pEntity->GetLowerBodyYawTarget(); break;

						}
					}
					else
					{
						pEntity->GetEyeAnglesXY()->y = *pEntity->GetLowerBodyYawTarget();

						switch (Globals::Shots % 9)
						{
						case 0: pEntity->GetEyeAnglesXY()->y = *pEntity->GetLowerBodyYawTarget() - 170; break;
						case 1: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 180 ; break;
						case 2: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 160; break;
						case 3: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 160; break;
						case 4: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 80; break;
						case 5: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 80; break;
						case 6: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 150; break;
						case 7: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 150; break;
						case 8: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 180; break;
						}
					}
				}

			}



			else if (Menu::Window.RageBotTab.AimbotResolver.GetIndex() == 3)
			{
	


				for (int i = 0; i < Interfaces::Engine->GetMaxClients(); ++i)
				{
					IClientEntity* pLocal = hackManager.pLocal();
					IClientEntity *player = (IClientEntity*)Interfaces::EntList->GetClientEntity(i);

					if (!player || player->IsDormant() || player->IsAlive() || (DWORD)player == (DWORD)pLocal)
						continue;



					Vector* eyeAngles = player->GetEyeAnglesXY();

					if (eyeAngles->x > 90.0 || eyeAngles->x < 60.0) eyeAngles->x = -88.9f;
					else if (eyeAngles->x > 60.0 && eyeAngles->x < 20.0) eyeAngles->x = 55.f;
					else if (eyeAngles->x > 20.0 && eyeAngles->x < -30.0) eyeAngles->x = 0.f;
					else if (eyeAngles->x < -30.0 && eyeAngles->x < 60.0) eyeAngles->x = 55.f;
					eyeAngles->x = 88.9f;







				}

				if (Globals::missedshots >= Menu::Window.RageBotTab.forceback.GetValue() && Globals::Shots >= Menu::Window.RageBotTab.forceback.GetValue())
				{
					switch (Globals::missedshots % 5)
					{
					case 0: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 80; break;
					case 1: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 150; break;
					case 2: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 150; break;
					case 3: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 80; break;
					case 4: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - (90 - rand() % 180); break;
					}
				}

				if (LatestLowerBodyYawUpdateTime[pEntity->GetIndex()] < pEntity->GetSimulationTime() || bLowerBodyIsUpdated)
				{
					pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw();
					LatestLowerBodyYawUpdateTime[pEntity->GetIndex()] = pEntity->GetSimulationTime() + 1.1;
				}
				else
				{

					if (IsMovingOnGround(pEntity))
					{
						if (IsFakeWalking(pEntity))
						{
							HitBoxesToScan.clear();
							HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
							HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
							HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
							HitBoxesToScan.push_back((int)CSGOHitboxID::LowerChest);
							HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
						}
						pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw();

						if (Globals::Shots >= 2)
						{
							HitBoxesToScan.clear();
							HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
							HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
							HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
							HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
							HitBoxesToScan.push_back((int)CSGOHitboxID::LowerChest);
							HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
							pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw();
						}
					}

					else if (Resolver::didhitHS)
					{
					
						if (Globals::Shots < 2)
						{
							switch (Globals::Shots % 5)
							{
							case 0:
								pEntity->GetEyeAnglesXY()->y = *pEntity->GetLowerBodyYawTarget() - (OldLowerBodyYaw - 35);
								break;
							case 1:
								pEntity->GetEyeAnglesXY()->y = *pEntity->GetLowerBodyYawTarget() + OldLowerBodyYaw;
								break;
							case 2:
								pEntity->GetEyeAnglesXY()->y = *pEntity->GetLowerBodyYawTarget() -= 90;
								break;
							case 3:
								pEntity->GetEyeAnglesXY()->y = *pEntity->GetLowerBodyYawTarget() += 90;
								break;
							case 4:
								pEntity->GetEyeAnglesXY()->y = *pEntity->GetLowerBodyYawTarget() -= 170;
								break;
							}
						}
						else
						{
							if (Globals::Shots >= 2 && Globals::missedshots >= 2)
							{
								switch (Globals::Shots % 5)
								{
								case 0:
									pEntity->GetEyeAnglesXY()->y = *pEntity->GetLowerBodyYawTarget() - 30;
									break;
								case 1:
									pEntity->GetEyeAnglesXY()->y = *pEntity->GetLowerBodyYawTarget() + 30;
									break;
								case 2:
									pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 110;
									break;
								case 3:
									pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 160;
									break;
								case 4:
									pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 180;
									break;

								}
							}
							else
							{
								switch (Globals::Shots % 3)
								{
								case 0:
									pEntity->GetEyeAnglesXY()->y = *pEntity->GetLowerBodyYawTarget() - OldLowerBodyYaw;
									break;
								case 1:
									pEntity->GetEyeAnglesXY()->y = *pEntity->GetLowerBodyYawTarget() + OldLowerBodyYaw;
									break;
								case 2:
									pEntity->GetEyeAnglesXY()->y = *pEntity->GetLowerBodyYawTarget() - 180;
									break;
								}
							}
						}
					}


					else if (IsMovingOnInAir(pEntity))
					{
						switch (Globals::Shots % 6)
						{
						case 1:
							pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 40;
							break;
						case 2:
							pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 40;
							break;
						case 3:
							pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 80;
							break;
						case 4:
							pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 80;
							break;
						case 5:
							pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 170;
							break;
						case 6:
							pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 170;
							break;

						}
					}
					else
					{
						if (HasFakeHead(pEntity))
						{
							pEntity->GetEyeAnglesXY()->y = pEntity->GetEyeAnglesXY()->y - pEntity->GetLowerBodyYaw();
						}

						if (IsMovingOnGround(pEntity))
						{
							pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw();
							if (LbyUpdated)
							{
								switch (Globals::missedshots % 3)
								{
								case 1:
									OldLowerBodyYaw;
									break;
								case 2:
									pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 20;
									break;
								case 3:
									pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 20;
									break;
								}
							}

						}

						else if (MeetsLBYReq && !IsMoving && pEntity->GetHealth() >= ResoSetup::storedhealth[pEntity->GetIndex()])
						{
							if ((pEntity->GetEyeAnglesXY()->y + 180.0) <= 180.0)
							{
								if (angletolerance < -180.0)
									angletolerance = angletolerance + 360.0;
							}
							else
							{
								angletolerance = angletolerance - 360.0;
							}

							consider = angletolerance - pEntity->GetLowerBodyYaw();

							if (consider <= 180.0)
							{
								if (consider < -180.0)
									consider = consider + 360.0;
							}
							else
							{
								consider = consider - 360.0;
							}
							if (consider >= 0.0)
								v24 = RandomFloat(0.0, consider / 2);
							else
								v24 = RandomFloat(consider / 2, 0.0);
							v20 = v24 + pEntity->GetEyeAnglesXY()->y;
							pEntity->GetEyeAnglesXY()->y = v20;
						}

						else
						{
							if (Lbywithin35(pEntity))
							{
								switch (Globals::missedshots % 8)
								{
								case 1:
									pEntity->GetEyeAnglesXY()->y = *pEntity->GetLowerBodyYawTarget() - 170;
									break;
								case 2:
									pEntity->GetEyeAnglesXY()->y = *pEntity->GetLowerBodyYawTarget() + 160;
									break;
								case 3:
									pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 40;
									break;
								case 4:
									pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 40;
									break;
								case 5:
									pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 100;
									break;
								case 6:
									pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 100;
									break;
								case 7:
									pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 165;
									break;
								case 8:
									pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 165;
									break;
								}
							}
						}
					}
				}
			
				
			}
			else if (Menu::Window.RageBotTab.AimbotResolver.GetIndex() == 4)
			{
				std::vector<int> HitBoxesToScan;
				bool sidejitter1;
				bool sidejitter2;
				sidejitter1 = sidejitter2;
				int oldlowerbodyyaw = pEntity->GetLowerBodyYaw();

				if ((pEntity->GetVelocity().Length2D() > 6 && pEntity->GetVelocity().Length2D() < 45) || (pEntity->GetVelocity().Length2D() > 30 && pEntity->GetVelocity().Length2D() < 50))
					FakeWalking = true;
				else
					FakeWalking = false;

				float lowerbody = pEntity->GetLowerBodyYaw();

				//-------------------------------------------------------------------------//

				if (Globals::missedshots >= Menu::Window.RageBotTab.forceback.GetValue())
				{
					switch (Globals::missedshots % 5)
					{
					case 1:	pEntity->GetLowerBodyYaw() + 80; break;
					case 2: pEntity->GetLowerBodyYaw() + 150; break;
					case 3: pEntity->GetLowerBodyYaw() - 150; break;
					case 4: pEntity->GetLowerBodyYaw() - 80; break;
					case 5: pEntity->GetLowerBodyYaw() - (90 - rand() % 180); break;
					}
				}
				if (lowerbody = 45)
				{
					pEntity->GetLowerBodyYaw() + 45;
					lowerbody = -45;
				}
				if (lowerbody = -45)
				{
					pEntity->GetLowerBodyYaw() - 45;
					lowerbody = 45;
				}
				if (sidejitter2 = true)
				{
					pEntity->GetEyeAnglesXY() + 90;
					sidejitter2 = false;
				}
				else if (sidejitter2 = false)
				{
					pEntity->GetEyeAnglesXY() - 90;
					sidejitter2 = true;
				}

				if (pEntity->GetVelocity().Length2D() < 35) {
					sidejitter2 = true;
					lowerbody = 180;
				}
				if (pEntity->GetVelocity().Length() > 1)
				{
					lowerbody = sidejitter2;
					lowerbody = 180;
					shouldbrute = true;
				}



				if (IsFakeWalking)
				{
					if (lowerbody > 120)
					{
						lowerbody + 180;

						lowerbody = (lowerbody - 180);
						HitBoxesToScan.push_back((int)CSGOHitboxID::Head);

						if (Globals::Shots >= 2 && !didhitHS)
						{
							HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
							HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
							HitBoxesToScan.push_back((int)CSGOHitboxID::LowerChest);
							HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
							HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
						}

					}
					else if (lowerbody <= 120)
					{
						pEntity->GetLowerBodyYaw() - 120;
						lowerbody = lowerbody - 120;

						HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
						if (Globals::Shots >= 2 && !didhitHS)
						{
							HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
							HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
							HitBoxesToScan.push_back((int)CSGOHitboxID::LowerChest);
							HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
							HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
						}
					}
				}
				if (shouldbrute)
				{
					switch (Globals::Shots % 3)
					{
					case 1:
						pEntity->GetEyeAnglesXY()->y += 90;
						break;
					case 2:
						pEntity->GetEyeAnglesXY()->y -= 90;
						break;
					case 3:
						pEntity->GetEyeAnglesXY()->y = 180;
						break;
					}
				}

				for (int i = 0; i < Interfaces::Engine->GetMaxClients(); ++i)
				{
					IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);

					if (!pEntity || pEntity->IsDormant() || !pEntity->IsAlive())
						continue;

					if (pEntity->GetTeamNum() == pLocal->GetTeamNum() || !pLocal->IsAlive())
						continue;

					Vector* eyeAngles = pEntity->GetEyeAnglesPointer();


					if (oldlowerbodyyaw != lowerbody)
					{
						bLowerBodyUpdated = true;
					}
					float bodyeyedelta = pEntity->GetEyeAngles().y - eyeAngles->y;
					if (PlayerIsMoving || bLowerBodyUpdated)// || LastUpdatedNetVars->eyeangles.x != CurrentNetVars->eyeangles.x || LastUpdatedNetVars->eyeyaw != CurrentNetVars->eyeangles.y)
					{
						if (bLowerBodyUpdated || (PlayerIsMoving && bodyeyedelta >= 45.0f))
						{
							eyeAngles->y = lowerbody;
							oldlowerbodyyaw = eyeAngles->y;
						}

						IsUsingFakeAngles = false;
					}
					else
					{
						if (bodyeyedelta > 90.0f)
						{
							eyeAngles->y = OldLowerBodyYaw;
							IsUsingFakeAngles = true;
						}
						else
						{
							IsUsingFakeAngles = false;
						}
					}
					if (IsUsingFakeAngles)
					{
						int com = GetEstimatedServerTickCount(90);

						if (com % 2)
						{
							eyeAngles->y += 90;
						}
						else if (com % 3)
							eyeAngles->y -= 90;
						else
							eyeAngles->y -= 0;
					}

				}
			}
			



		
	}
		
}
	



void ResoSetup::StoreFGE(IClientEntity* pEntity)
{
	ResoSetup::storedanglesFGE = pEntity->GetEyeAnglesXY()->y;
	ResoSetup::storedlbyFGE = pEntity->GetLowerBodyYaw();
	ResoSetup::storedsimtimeFGE = pEntity->GetSimulationTime();
}

void ResoSetup::StoreThings(IClientEntity* pEntity)
{
	ResoSetup::StoredAngles[pEntity->GetIndex()] = *pEntity->GetEyeAnglesXY();
	ResoSetup::storedlby[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
	ResoSetup::storedsimtime = pEntity->GetSimulationTime();
	ResoSetup::storeddelta[pEntity->GetIndex()] = pEntity->GetEyeAnglesXY()->y;
	ResoSetup::storedlby[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
}

void ResoSetup::anglestore(IClientEntity * pEntity)
{
	ResoSetup::badangle[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();

}

void ResoSetup::StoreExtra(IClientEntity * pEntity)
{
	ResoSetup::storedpanic[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
	ResoSetup::panicangle[pEntity->GetIndex()] = (pEntity->GetEyeAnglesXY()->y - 30);

}

void ResoSetup::CM(IClientEntity* pEntity)
{
	for (int x = 1; x < Interfaces::Engine->GetMaxClients(); x++)
	{

		pEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(x);

		if (!pEntity
			|| pEntity == hackManager.pLocal()
			|| pEntity->IsDormant()
			|| !pEntity->IsAlive())
			continue;

		ResoSetup::StoreThings(pEntity);
	}
}

void ResoSetup::FSN(IClientEntity* pEntity, ClientFrameStage_t stage)
{
	if (stage == ClientFrameStage_t::FRAME_NET_UPDATE_POSTDATAUPDATE_START)
	{
		for (int i = 1; i < Interfaces::Engine->GetMaxClients(); i++)
		{

			pEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(i);

			if (!pEntity
				|| pEntity == hackManager.pLocal()
				|| pEntity->IsDormant()
				|| !pEntity->IsAlive())
				continue;

			ResoSetup::Resolve(pEntity, stage);
		}
	}
}

