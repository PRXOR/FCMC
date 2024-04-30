#include <iostream>
#include <stdio.h>
#include <map>
#include <queue>
#include <string>
#include <vector>
#include <random>
#include <filesystem>
#include <graphics.h>
#include <easyx.h>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <mmsystem.h>
#include <dsound.h>
#pragma comment(lib, "WINMM.LIB")
#pragma comment(lib, "MSIMG32.LIB")
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )

using namespace std;
const int _CN = 25;
int np, tp, Tp, MAX_STEPS, MAX_JUMPS, REST_STEPS, STEPS_STATUS = 0;
bool supermode, historymode, showmode, AutoUpdate, DO_REC, PLAY_BGM, PLAY_SOUND;
FILE* stream;

struct Chess
{
	int level, cpg, cpx, cpy, cflag;//棋子大小，横竖坐标，被标记为
	bool live;//是否存活
	Chess(int le, int _g, int _x, int _y):level(le),cpg(_g),cpx(_x),cpy(_y),cflag(le == 10 ? 2 : 0),live(true){}//将第一排标记为叹号，军旗翻开
	Chess():level(0),cpg(0),cpx(0),cpy(0),cflag(0),live(0){}
};
struct Pos
{
	int g, x, y;
	Pos(int _g, int _x, int _y):g(_g),x(_x),y(_y){}
};
class Player
{
private:
public:
	bool live=true;
	int initarray[_CN] = {}, deflevel[_CN] = { 10, 20, 20, 20, 21, 21, 32, 32, 32, 33, 33, 33, 34, 34, 34, 35, 35, 36, 36, 37, 37, 38, 38, 39, 40 };//旗雷炸兵---
	int defposx[_CN] = {6,1,1,1,1,1,2,2,2,3,3,3,3,4,4,4,5,5,5,5,5,6,6,6,6}, defposy[_CN] = {4,1,2,3,4,5,1,3,5,1,2,4,5,1,3,5,1,2,3,4,5,1,2,3,5};
	int skiptimes = 0;
	void Init(int ppos);
	Chess MyChess[_CN] = {};
	vector<string> DeadChess;
} P[5];
void putnewbk(IMAGE* dstimg, int x, int y, IMAGE* srcimg) //新版png
{
	HDC dstDC = GetImageHDC(dstimg);
	HDC srcDC = GetImageHDC(srcimg);
	int w = srcimg->getwidth();
	int h = srcimg->getheight();
	BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
	AlphaBlend(dstDC, x, y, w, h, srcDC, 0, 0, w, h, bf);
}
IMAGE bk0, bk2, bk4;
void button(int x, int y, int w, int h, const char* text,COLORREF col,int size=20)
{

	setlinecolor(WHITE);//设置框边颜色
	setbkmode(TRANSPARENT);//设置字体背景透明
	setfillcolor(col);//设置填充颜色
	fillroundrect(x, y, x + w, y + h, 10, 10);//画一个按钮框
	char text_[50] = { 0 };
	strcpy_s(text_, text);
	settextcolor(BLACK);
//	settextstyle(size, 0, "黑体");
	LOGFONT f;
	gettextstyle(&f);
	f.lfHeight = size;
	_tcscpy_s(f.lfFaceName, "黑体");
	f.lfQuality = ANTIALIASED_QUALITY;
	settextstyle(&f);
	int tx = x + (w - textwidth(text_)) / 2;
	int ty = y + (h - textheight(text_)) / 2;
	outtextxy(tx, ty, text_);
}
class Record
{
	int cnt_step = 0;
public:
	static vector<Pos> Go_Path;
	int Mix(Pos P);
	Pos UnMix(int mixed);
	void Record_Initialize();
	void Record_End(int who);
	void Move(int lsp,int lsc,int G,int X,int Y);
	void Show(int lsp, int lsc);
	void Dead(int lsp, int lsc);
	void RS(int sound);
	int Step();
	void Player_Dead(int who);
};
class Pub : public Record
{
public:
	static map< pair<int, int>, vector< pair<int, int> > > NA;//非铁道普攻
	static vector<int> _2ROAD[8], _4ROAD[20];//1-7,1-18
	static string _2LEVELMAP[41], _4LEVELMAP[41];
	pair<int, int> XY[5] = { make_pair(2,2),make_pair(2,4),make_pair(3,3),make_pair(4,2),make_pair(4,4) };//行营
	string P_Name[5] = { "","玩家1","玩家2","玩家3","玩家4" };
	bool IS_MSG(ExMessage msg, int spx, int spy, int lx, int ly) { return (spx <= msg.x && msg.x <= spx + lx && spy <= msg.y && msg.y <= spy + ly); }
	void PS(int sound);
	void WIN(int who);
	void Draw_Arrow(int x1, int y1, int x2, int y2, int L);
	bool IS_VOID(int g, int x, int y);
	bool IS_DEAD(int p);
	bool NORMAL_GO(Chess A, Chess B);
	int N_KILL(Chess A, Chess B);
	void Go_Super();
	static void Game_Initialize();
} FH;
vector<Pos> Record::Go_Path = {};
map< pair<int, int>, vector< pair<int, int> > > Pub::NA = {};
vector<int> Pub::_2ROAD[8] = {}, Pub::_4ROAD[20] = {};
string Pub::_2LEVELMAP[41] = {};
string Pub::_4LEVELMAP[41] = {};
class _2 : public Pub
{
	int ChessX = 60, ChessY = 27;//棋子的长宽
	int _2startposx[3] = { 0,20,480 }, _2startposy[3] = { 0,424,274 }, _2divx = 115, _2divy = 51;
	int trans[3] = { 0,1,-1 };
	bool ROAD_GO_N(Pos F,Pos T),ROAD_GO_B(Pos F,Pos T);
	bool GOABLE(Chess A, Chess B, bool att);
public:
	void PRINTNOW();
	void MC();
} PLAY2, H2;
class _4 : public Pub
{
	int stpx[5] = { 0,442,651,610,400 }, stpy[5] = { 0,472,430,220,262 }, L = 27, D = 42, DD = 30;
	int ppx[5] = { 0,651,651,190,190 }, ppy[5] = { 0,472,10,10,472 };
	COLORREF MyCol[5] = { RGB(100,100,100),RGB(216,108,0),RGB(145,63,165),RGB(112,156,0),RGB(39,104,160) };
	int transxx[5] = { 0,0,1,0,-1 }, transxy[5] = { 0,1,0,-1,0 }, transyx[5] = { 0,1,0,-1,0 }, transyy[5] = { 0,0,-1,0,1 };
	bool TEAM_WIN();
	bool ROAD_GO_N(Pos F, Pos T), ROAD_GO_B(Pos F, Pos T);
	bool GOABLE(Chess A, Chess B, bool att);
public:
	void GO_NEXT();
	void PRINTNOW();
	void MC();
} PLAY4, H4;
void Setting();
void HISTORY();
void PRINT_main();

int main()
{
	initgraph(1080, 720, NULL);
//	initgraph(1080, 720, EX_SHOWCONSOLE);
	Pub::Game_Initialize();
	putimage(0, 0, &bk0);
	button(240, 240, 600, 240, "欢迎来到四国军棋 :)",RGB(491, 491, 100), 50);
	Sleep(500);
	cleardevice();
	int mode = 0, _dx = 170, _sx = 40, _sy = 120, _x = 150, _y = 60;
	ExMessage msg;
	PRINT_main();
	while (1)
	{
		if (peekmessage(&msg, EM_MOUSE) && msg.message == WM_LBUTTONDOWN)
		{
			for (mode = 1; mode <= 6 && !(_sx + _dx * (mode - 1) <= msg.x && msg.x <= _sx + _dx * (mode - 1) + _x && _sy <= msg.y && msg.y <= _sy + _y); mode++) {}
			if (_sx + 900 <= msg.x && msg.x <= _x + 1000 && 40 <= msg.y && msg.y <= 100) mode = 0;
			switch (mode)
			{
			case 0:
				button(_sx + 900, 40, 100, 60, "设置", RGB(100, 100, 100), 30);
				Sleep(200);
				Setting();
				PRINT_main();
				break;
			case 1:
				button(_sx, _sy, _x, _y, "全暗双人", RGB(100, 100, 100), 25);
				supermode = 0, tp = 2, Tp = 2, historymode = 0, showmode = 0;
				Sleep(200);
				PLAY2.MC();
				PRINT_main();
				break;
			case 2:
				button(_sx + _dx, _sy, _x, _y, "随机双人", RGB(100, 100, 100), 25);
				supermode = 1, tp = 2, Tp = 2, historymode = 0, showmode = 0;
				Sleep(200);
				PLAY2.MC();
				PRINT_main();
				break;
			case 3:
				button(_sx + _dx * 2, _sy, _x, _y, "全暗四人", RGB(100, 100, 100), 25);
				supermode = 0, tp = 4, Tp = 4, historymode = 0, showmode = 0;
				Sleep(200);
				PLAY4.MC();
				PRINT_main();
				break;
			case 4:
				button(_sx + _dx * 3, _sy, _x, _y, "随机四人", RGB(100, 100, 100), 25);
				supermode = 1, tp = 4, Tp = 4, historymode = 0, showmode = 0;
				Sleep(200);
				PLAY4.MC();
				PRINT_main();
				break;
			case 5:
				button(_sx + _dx * 4, _sy, _x, _y, "加载复盘", RGB(100, 100, 100), 25);
				historymode = 1;
				Sleep(200);
				HISTORY();
				PRINT_main();
				break;
			case 6:
				button(_sx + _dx * 5, _sy, _x, _y, "退出游戏", RGB(100, 100, 100), 25);
				Sleep(200);
				return 0;
			}
		}
	}
	return 0;
}
void Pub::Game_Initialize()
{
	srand(uint32_t(time(NULL)));
	DO_REC = 1, AutoUpdate = 0, MAX_STEPS = 400, MAX_JUMPS = 5, REST_STEPS = 40;
	freopen_s(&stream, "settings.config", "r", stdin);
	string key;
	while (cin >> key)
	{
		if (key == "DO_REC") cin >> DO_REC;
		if (key == "AutoUpdate") cin >> AutoUpdate;
		if (key == "MAX_STEPS") cin >> MAX_STEPS;
		if (key == "MAX_JUMPS") cin >> MAX_JUMPS;
		if (key == "REST_STEPS") cin >> REST_STEPS;
		if (key == "PLAY_BGM") cin >> PLAY_BGM;
		if (key == "PLAY_SOUND") cin >> PLAY_SOUND;
		if (key == "END") break;
	}
	freopen_s(&stream, "CON", "r", stdin);
	if (DO_REC) CreateDirectory("Records", NULL);
	if (PLAY_BGM) PlaySound("Resources/BGM.wav", NULL, SND_ASYNC | SND_LOOP);
	loadimage(&bk0, _T("Resources/background0.png"), 1080, 720);
	loadimage(&bk2, _T("Resources/background3.jpg"), 560, 720);
	loadimage(&bk4, _T("Resources/background6.png"), 720, 720);
	_2LEVELMAP[10] = "军旗", _2LEVELMAP[20] = "地雷", _2LEVELMAP[21] = "炸弹", _2LEVELMAP[32] = "工兵", _2LEVELMAP[33] = "排长", _2LEVELMAP[34] = "连长", _2LEVELMAP[35] = "营长", _2LEVELMAP[36] = "团长", _2LEVELMAP[37] = "旅长", _2LEVELMAP[38] = "师长", _2LEVELMAP[39] = "军长", _2LEVELMAP[40] = "司令";
	_4LEVELMAP[10] = "旗", _4LEVELMAP[20] = "雷", _4LEVELMAP[21] = "炸", _4LEVELMAP[32] = "兵", _4LEVELMAP[33] = "排", _4LEVELMAP[34] = "连", _4LEVELMAP[35] = "营", _4LEVELMAP[36] = "团", _4LEVELMAP[37] = "旅", _4LEVELMAP[38] = "师", _4LEVELMAP[39] = "军", _4LEVELMAP[40] = "司";
	NA[make_pair(2, 2)].push_back(make_pair(1, 1)), NA[make_pair(2, 2)].push_back(make_pair(1, 2)), NA[make_pair(2, 2)].push_back(make_pair(1, 3)), NA[make_pair(2, 2)].push_back(make_pair(2, 1)), NA[make_pair(2, 2)].push_back(make_pair(2, 3)), NA[make_pair(2, 2)].push_back(make_pair(3, 1)), NA[make_pair(2, 2)].push_back(make_pair(3, 2)), NA[make_pair(2, 2)].push_back(make_pair(3, 3));
	NA[make_pair(2, 3)].push_back(make_pair(1, 3)), NA[make_pair(2, 3)].push_back(make_pair(2, 2)), NA[make_pair(2, 3)].push_back(make_pair(2, 4)), NA[make_pair(2, 3)].push_back(make_pair(3, 3));
	NA[make_pair(2, 4)].push_back(make_pair(1, 3)), NA[make_pair(2, 4)].push_back(make_pair(1, 4)), NA[make_pair(2, 4)].push_back(make_pair(1, 5)), NA[make_pair(2, 4)].push_back(make_pair(2, 3)), NA[make_pair(2, 4)].push_back(make_pair(2, 5)), NA[make_pair(2, 4)].push_back(make_pair(3, 3)), NA[make_pair(2, 4)].push_back(make_pair(3, 4)), NA[make_pair(2, 4)].push_back(make_pair(3, 5));
	NA[make_pair(3, 2)].push_back(make_pair(2, 2)), NA[make_pair(3, 2)].push_back(make_pair(3, 1)), NA[make_pair(3, 2)].push_back(make_pair(3, 3)), NA[make_pair(3, 2)].push_back(make_pair(4, 2));
	NA[make_pair(3, 3)].push_back(make_pair(2, 2)), NA[make_pair(3, 3)].push_back(make_pair(2, 3)), NA[make_pair(3, 3)].push_back(make_pair(2, 4)), NA[make_pair(3, 3)].push_back(make_pair(3, 2)), NA[make_pair(3, 3)].push_back(make_pair(3, 4)), NA[make_pair(3, 3)].push_back(make_pair(4, 2)), NA[make_pair(3, 3)].push_back(make_pair(4, 3)), NA[make_pair(3, 3)].push_back(make_pair(4, 4));
	NA[make_pair(3, 4)].push_back(make_pair(2, 4)), NA[make_pair(3, 4)].push_back(make_pair(3, 3)), NA[make_pair(3, 4)].push_back(make_pair(3, 5)), NA[make_pair(3, 4)].push_back(make_pair(4, 4));
	NA[make_pair(4, 2)].push_back(make_pair(3, 1)), NA[make_pair(4, 2)].push_back(make_pair(3, 2)), NA[make_pair(4, 2)].push_back(make_pair(3, 3)), NA[make_pair(4, 2)].push_back(make_pair(4, 1)), NA[make_pair(4, 2)].push_back(make_pair(4, 3)), NA[make_pair(4, 2)].push_back(make_pair(5, 1)), NA[make_pair(4, 2)].push_back(make_pair(5, 2)), NA[make_pair(4, 2)].push_back(make_pair(5, 3));
	NA[make_pair(4, 3)].push_back(make_pair(3, 3)), NA[make_pair(4, 3)].push_back(make_pair(4, 2)), NA[make_pair(4, 3)].push_back(make_pair(4, 4)), NA[make_pair(4, 3)].push_back(make_pair(5, 3));
	NA[make_pair(4, 4)].push_back(make_pair(3, 3)), NA[make_pair(4, 4)].push_back(make_pair(3, 4)), NA[make_pair(4, 4)].push_back(make_pair(3, 5)), NA[make_pair(4, 4)].push_back(make_pair(4, 3)), NA[make_pair(4, 4)].push_back(make_pair(4, 5)), NA[make_pair(4, 4)].push_back(make_pair(5, 3)), NA[make_pair(4, 4)].push_back(make_pair(5, 4)), NA[make_pair(4, 4)].push_back(make_pair(5, 5));
	NA[make_pair(6, 1)].push_back(make_pair(5, 1)), NA[make_pair(6, 1)].push_back(make_pair(6, 2));
	NA[make_pair(6, 2)].push_back(make_pair(5, 2)), NA[make_pair(6, 2)].push_back(make_pair(6, 1)), NA[make_pair(6, 2)].push_back(make_pair(6, 3));
	NA[make_pair(6, 3)].push_back(make_pair(5, 3)), NA[make_pair(6, 3)].push_back(make_pair(6, 2)), NA[make_pair(6, 3)].push_back(make_pair(6, 4));
	NA[make_pair(6, 5)].push_back(make_pair(5, 5)), NA[make_pair(6, 5)].push_back(make_pair(6, 4));
	NA[make_pair(1, 1)].push_back(make_pair(2, 2));
	NA[make_pair(1, 2)].push_back(make_pair(2, 2));
	NA[make_pair(1, 3)].push_back(make_pair(2, 2)), NA[make_pair(1, 3)].push_back(make_pair(2, 3)), NA[make_pair(1, 3)].push_back(make_pair(2, 4));
	NA[make_pair(1, 4)].push_back(make_pair(2, 4));
	NA[make_pair(1, 5)].push_back(make_pair(2, 4));
	NA[make_pair(2, 1)].push_back(make_pair(2, 2));
	NA[make_pair(2, 5)].push_back(make_pair(2, 4));
	NA[make_pair(3, 1)].push_back(make_pair(2, 2)), NA[make_pair(3, 1)].push_back(make_pair(3, 2)), NA[make_pair(3, 1)].push_back(make_pair(4, 2));
	NA[make_pair(3, 5)].push_back(make_pair(2, 4)), NA[make_pair(3, 5)].push_back(make_pair(3, 4)), NA[make_pair(3, 5)].push_back(make_pair(4, 4));
	NA[make_pair(4, 1)].push_back(make_pair(4, 2));
	NA[make_pair(4, 5)].push_back(make_pair(4, 4));
	NA[make_pair(5, 1)].push_back(make_pair(4, 2)), NA[make_pair(5, 1)].push_back(make_pair(6, 1));
	NA[make_pair(5, 2)].push_back(make_pair(4, 2)), NA[make_pair(5, 2)].push_back(make_pair(6, 2));
	NA[make_pair(5, 3)].push_back(make_pair(4, 2)), NA[make_pair(5, 3)].push_back(make_pair(4, 3)), NA[make_pair(5, 3)].push_back(make_pair(4, 4)), NA[make_pair(5, 3)].push_back(make_pair(6, 3));
	NA[make_pair(5, 4)].push_back(make_pair(4, 4)), NA[make_pair(5, 4)].push_back(make_pair(6, 4));
	NA[make_pair(5, 5)].push_back(make_pair(4, 4)), NA[make_pair(5, 5)].push_back(make_pair(6, 5));
	_2ROAD[1].push_back(151), _2ROAD[1].push_back(141), _2ROAD[1].push_back(131), _2ROAD[1].push_back(121), _2ROAD[1].push_back(111), _2ROAD[1].push_back(215), _2ROAD[1].push_back(225), _2ROAD[1].push_back(235), _2ROAD[1].push_back(245), _2ROAD[1].push_back(255);
	_2ROAD[2].push_back(251), _2ROAD[2].push_back(241), _2ROAD[2].push_back(231), _2ROAD[2].push_back(221), _2ROAD[2].push_back(211), _2ROAD[2].push_back(115), _2ROAD[2].push_back(125), _2ROAD[2].push_back(135), _2ROAD[2].push_back(145), _2ROAD[2].push_back(155);
	_2ROAD[3].push_back(151), _2ROAD[3].push_back(152), _2ROAD[3].push_back(153), _2ROAD[3].push_back(154), _2ROAD[3].push_back(155);
	_2ROAD[4].push_back(251), _2ROAD[4].push_back(252), _2ROAD[4].push_back(253), _2ROAD[4].push_back(254), _2ROAD[4].push_back(255);
	_2ROAD[5].push_back(111), _2ROAD[5].push_back(112), _2ROAD[5].push_back(113), _2ROAD[5].push_back(114), _2ROAD[5].push_back(115);
	_2ROAD[6].push_back(211), _2ROAD[6].push_back(212), _2ROAD[6].push_back(213), _2ROAD[6].push_back(214), _2ROAD[6].push_back(215);
	_2ROAD[7].push_back(113), _2ROAD[7].push_back(213);
	_4ROAD[1].push_back(151), _4ROAD[1].push_back(152), _4ROAD[1].push_back(153), _4ROAD[1].push_back(154), _4ROAD[1].push_back(155);
	_4ROAD[2].push_back(251), _4ROAD[2].push_back(252), _4ROAD[2].push_back(253), _4ROAD[2].push_back(254), _4ROAD[2].push_back(255);
	_4ROAD[3].push_back(351), _4ROAD[3].push_back(352), _4ROAD[3].push_back(353), _4ROAD[3].push_back(354), _4ROAD[3].push_back(355);
	_4ROAD[4].push_back(451), _4ROAD[4].push_back(452), _4ROAD[4].push_back(453), _4ROAD[4].push_back(454), _4ROAD[4].push_back(455);
	_4ROAD[5].push_back(111), _4ROAD[5].push_back(112), _4ROAD[5].push_back(113), _4ROAD[5].push_back(114), _4ROAD[5].push_back(115);
	_4ROAD[6].push_back(211), _4ROAD[6].push_back(212), _4ROAD[6].push_back(213), _4ROAD[6].push_back(214), _4ROAD[6].push_back(215);
	_4ROAD[7].push_back(311), _4ROAD[7].push_back(312), _4ROAD[7].push_back(313), _4ROAD[7].push_back(314), _4ROAD[7].push_back(315);
	_4ROAD[8].push_back(411), _4ROAD[8].push_back(412), _4ROAD[8].push_back(413), _4ROAD[8].push_back(414), _4ROAD[8].push_back(415);
	_4ROAD[9].push_back(455), _4ROAD[9].push_back(445), _4ROAD[9].push_back(435), _4ROAD[9].push_back(425), _4ROAD[9].push_back(415), _4ROAD[9].push_back(60), _4ROAD[9].push_back(70), _4ROAD[9].push_back(80), _4ROAD[9].push_back(211), _4ROAD[9].push_back(221), _4ROAD[9].push_back(231), _4ROAD[9].push_back(241), _4ROAD[9].push_back(251);
	_4ROAD[10].push_back(155), _4ROAD[10].push_back(145), _4ROAD[10].push_back(135), _4ROAD[10].push_back(125), _4ROAD[10].push_back(115), _4ROAD[10].push_back(80), _4ROAD[10].push_back(50), _4ROAD[10].push_back(20), _4ROAD[10].push_back(311), _4ROAD[10].push_back(321), _4ROAD[10].push_back(331), _4ROAD[10].push_back(341), _4ROAD[10].push_back(351);
	_4ROAD[11].push_back(255), _4ROAD[11].push_back(245), _4ROAD[11].push_back(235), _4ROAD[11].push_back(225), _4ROAD[11].push_back(215), _4ROAD[11].push_back(20), _4ROAD[11].push_back(10), _4ROAD[11].push_back(0), _4ROAD[11].push_back(411), _4ROAD[11].push_back(421), _4ROAD[11].push_back(431), _4ROAD[11].push_back(441), _4ROAD[11].push_back(451);
	_4ROAD[12].push_back(355), _4ROAD[12].push_back(345), _4ROAD[12].push_back(335), _4ROAD[12].push_back(325), _4ROAD[12].push_back(315), _4ROAD[12].push_back(0), _4ROAD[12].push_back(30), _4ROAD[12].push_back(60), _4ROAD[12].push_back(111), _4ROAD[12].push_back(121), _4ROAD[12].push_back(131), _4ROAD[12].push_back(141), _4ROAD[12].push_back(151);
	_4ROAD[13].push_back(155), _4ROAD[13].push_back(145), _4ROAD[13].push_back(135), _4ROAD[13].push_back(125), _4ROAD[13].push_back(115), _4ROAD[13].push_back(211), _4ROAD[13].push_back(221), _4ROAD[13].push_back(231), _4ROAD[13].push_back(241), _4ROAD[13].push_back(251);
	_4ROAD[14].push_back(255), _4ROAD[14].push_back(245), _4ROAD[14].push_back(235), _4ROAD[14].push_back(225), _4ROAD[14].push_back(215), _4ROAD[14].push_back(311), _4ROAD[14].push_back(321), _4ROAD[14].push_back(331), _4ROAD[14].push_back(341), _4ROAD[14].push_back(351);
	_4ROAD[15].push_back(355), _4ROAD[15].push_back(345), _4ROAD[15].push_back(335), _4ROAD[15].push_back(325), _4ROAD[15].push_back(315), _4ROAD[15].push_back(411), _4ROAD[15].push_back(421), _4ROAD[15].push_back(431), _4ROAD[15].push_back(441), _4ROAD[15].push_back(451);
	_4ROAD[16].push_back(455), _4ROAD[16].push_back(445), _4ROAD[16].push_back(435), _4ROAD[16].push_back(425), _4ROAD[16].push_back(415), _4ROAD[16].push_back(111), _4ROAD[16].push_back(121), _4ROAD[16].push_back(131), _4ROAD[16].push_back(141), _4ROAD[16].push_back(151);
	_4ROAD[17].push_back(413), _4ROAD[17].push_back(30), _4ROAD[17].push_back(40), _4ROAD[17].push_back(50), _4ROAD[17].push_back(213);
	_4ROAD[18].push_back(113), _4ROAD[18].push_back(70), _4ROAD[18].push_back(40), _4ROAD[18].push_back(10), _4ROAD[18].push_back(313);
}
void Player::Init(int ppos)
{
	live = 1, skiptimes = 0;
	DeadChess.clear();
	for (int i = 0; i < _CN; i++) initarray[i] = i;
	random_device rd;
	mt19937 gg(rd());
	shuffle(initarray + 1, initarray + _CN, gg);
	for (int i = 0; i < _CN; i++) MyChess[i] = Chess(deflevel[initarray[i]], ppos, defposx[i], defposy[i]);
}
int Record::Mix(Pos P)
{
	return P.g * 100 + P.x * 10 + P.y;
}
Pos Record::UnMix(int mixed)
{
	return Pos(mixed / 100, (mixed / 10) % 10, mixed % 10);
}
inline void Pub::Draw_Arrow(int x1, int y1, int x2, int y2, int L)
{
	int mx = (x1 + x2) / 2, my = (y1 + y2) / 2;
	double nx = (double(x2) - double(x1)) / (sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1))), ny = (double(y2) - double(y1)) / (sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)));
	POINT pa[6] = { { int(mx + L * nx),int(my + L * ny) },{int(mx - L * ny),int(my + L * nx)},{int(mx - L * (nx + ny)),int(my + L * (nx - ny))},{int(mx),int(my)},{int(mx + L * (ny - nx)),int(my - L * (ny + nx))},{int(mx + L * ny),int(my - L * nx)} };
	fillpolygon(pa, 6);
}
bool Pub::IS_VOID(int g, int x, int y)
{
	for (int i = 1; i <= Tp; i++)
	{
		for (int j = 0; j < _CN; j++)
		{
			Chess tmp = P[i].MyChess[j];
			if (tmp.live == 0) continue;
			if (tmp.cpg == g && tmp.cpx == x && tmp.cpy == y) return false;
		}
	}
	return true;
}
bool Pub::IS_DEAD(int p)
{
	for (int i = 0; i < _CN; i++)
		if (P[p].MyChess[i].live != 0 && P[p].MyChess[i].level != 10 && !(P[p].MyChess[i].level == 20 && P[p].MyChess[i].cflag == 2) && !(P[p].MyChess[i].cpx == 6 && P[p].MyChess[i].cpy == 4)) return false;
	return true;
}
bool Pub::NORMAL_GO(Chess A, Chess B)
{
	if (A.cpg == 0 || B.cpg == 0) return false;
	pair<int, int> pa = make_pair(A.cpx, A.cpy), pb = make_pair(B.cpx, B.cpy);
	if (A.cpg == B.cpg && NA.count(pa) > 0)
	{
		for (vector<pair<int, int> >::iterator i = NA[pa].begin(); i != NA[pa].end(); i++)
		{
			if (*i == pb)
			{
				Go_Path.clear();
				Go_Path.push_back(Pos(A.cpg, A.cpx, A.cpy));
				Go_Path.push_back(Pos(B.cpg, B.cpx, B.cpy));
				return true;
			}
		}
			
	}
	return false;
}
void Pub::PS(int sound)
{
	if (historymode == 0 && sound != 5) RS(sound);
	if (!PLAY_SOUND) return;
	switch (sound)
	{
	case 0:
		PlaySound("Resources/start.wav", NULL, SND_ASYNC);
		break;
	case 1:
		PlaySound("Resources/move.wav", NULL, SND_ASYNC);
		break;
	case 2:
		PlaySound("Resources/kendiao.wav", NULL, SND_ASYNC);
		break;
	case 3:
		PlaySound("Resources/zhuangsi.wav", NULL, SND_ASYNC);
		break;
	case 4:
		PlaySound("Resources/doumei.wav", NULL, SND_ASYNC);
		break;
	case 5:
		PlaySound("Resources/end.wav", NULL, SND_ASYNC);
		break;
	case 6:
		PlaySound("Resources/dead.wav", NULL, SND_ASYNC);
		break;
	}
}
void Pub::WIN(int who)
{
	if (historymode == 0) Record_End(who);
	string Winner[5] = { "对局结束！","玩家1获胜！","玩家2获胜！","玩家1 3获胜！","玩家2 4获胜！" };
	char tc[20];
	strcpy_s(tc, Winner[who].c_str());
	if(who==1||who==2) button(600, 310, 300, 100, tc, RGB(255, 255, 0), 30);
	else button(390, 310, 300, 100, tc, RGB(255, 255, 0), 30);
	PS(5);
	Sleep(2000);
}
int Pub::N_KILL(Chess c1, Chess c2)
{
	int ans = 0;//0拔旗,1啃掉,2撞死,3都没
	if (c2.level == 10) return 0;
	else if (c1.level == 21 || c2.level == 21) return 3;
	else if (c2.level == 20)
	{
		if (c1.level == 20) return 3;//对雷
		else if (c1.level == 32) return 1;//啃雷
		else return 2;//撞雷
	}
	else if (c1.level == 20)
	{
		if (c2.level == 32) return 2;
		else return 1;
	}
	else
	{
		if (c1.level > c2.level) return 1;
		else if (c1.level < c2.level) return 2;
		else return 3;
	}
}
void Pub::Go_Super()
{
	int N = rand() % 16 + 4;
	printf("N %d\nShow\n", N);
	for (int i = 1; i <= Tp; i++)
	{
		int re = N, rj = 0;
		while(re)
		{
			rj = rand() % 25;
			if (P[i].MyChess[rj].cflag == 0) P[i].MyChess[rj].cflag = 2, re--, printf("%d ", rj);
		}
		printf("\n");
	}
}
void _2::PRINTNOW()
{
	COLORREF MyCol[3] = { RGB(100,100,100),RGB(100,100,255),RGB(255,100,100) };
	BeginBatchDraw();
	cleardevice();
	putimage(0, 0, &bk0);
	putimage(0, 0, &bk2);
	int lrx, lry, rx, ry;
	setcolor(RGB(80,20,20));
	setfillcolor(RED);
	for (int i = 0; i < Go_Path.size(); i++)
	{
		Pos p = Go_Path[i];
		rx = _2startposx[p.g] + _2divx * (p.y - 1) * trans[p.g]  + 30, ry = _2startposy[p.g] + _2divy * (p.x - 1) * trans[p.g] + 13;
		fillcircle(rx, ry, 12);
		setcolor(RGB(80, 80, 20));
		setfillcolor(RGB(200, 200, 100));
		if (i) Draw_Arrow(lrx, lry, rx, ry, 10);
		if (i == Go_Path.size() - 2)
		{
			setcolor(RGB(20,80,80));
			setfillcolor(RGB(100,200,200));
		}
		lrx = rx, lry = ry;
	}
	for (int i = 1; i <= 2; i++)
	{
		for (int j = 0; j < _CN; j++)
		{
			Chess tmp = P[i].MyChess[j];
			if (P[i].MyChess[j].live == 0) continue;
			char tname[20];
			strcpy_s(tname, ((tmp.cflag == 2 || showmode) ? _2LEVELMAP[tmp.level].c_str() : ""));
			button(_2startposx[tmp.cpg] + _2divx * (tmp.cpy - 1) * trans[tmp.cpg], _2startposy[tmp.cpg] + _2divy * (tmp.cpx - 1) * trans[tmp.cpg], ChessX, ChessY, tname, MyCol[i]);//注意cpxcpy与_2系列横纵坐标定义相反
		}
	}
	if (np == 1)
	{
		button(600, 450, 200, 200, "玩家1(行动)", MyCol[1], 30), button(600, 65, 200, 200, "玩家2", MyCol[2], 30);
		if (historymode == 0) button(710, 580, 60, 50, "跳过", MyCol[0]), button(630, 580, 60, 50, "投降", MyCol[0]);
	}
	else
	{
		button(600, 450, 200, 200, "玩家1", MyCol[1], 30), button(600, 65, 200, 200, "玩家2(行动)", MyCol[2], 30);
		if (historymode == 0) button(630, 195, 60, 50, "投降", MyCol[0]), button(710, 195, 60, 50, "跳过", MyCol[0]);
	}
	int cnt = 0, d = 40;
	for (vector<string>::iterator i = P[1].DeadChess.begin(); i != P[1].DeadChess.end(); i++)
	{
		char tmp[20];
		strcpy_s(tmp, (*i).c_str());
		button(850 + d * (cnt % 5), 450 + d * (cnt / 5), d, d, tmp, MyCol[1], 25);
		cnt++;
	}
	cnt = 0;
	for (vector<string>::iterator i = P[2].DeadChess.begin(); i != P[2].DeadChess.end(); i++)
	{
		char tmp[20];
		strcpy_s(tmp, (*i).c_str());
		button(850 + d * (cnt % 5), 65 + d * (cnt / 5), d, d, tmp, MyCol[2], 25);
		cnt++;
	}
	if (STEPS_STATUS == 1 || STEPS_STATUS == 2) button(600, 15, 450, 40, "请注意，相持时间已经接近系统限制", RGB(200, 0, 0));
	EndBatchDraw();
}
bool _2::ROAD_GO_N(Pos F, Pos T)
{
	int f = Mix(F), t = Mix(T);
	for (int r = 1; r <= 7; r++)
	{
		int pick = 0, pi = 0;
		for (int i = 0; i < _2ROAD[r].size(); i++)
		{
			if (_2ROAD[r][i] == f || _2ROAD[r][i] == t)
			{
				if (pick)
				{
					Go_Path.clear();
					if (_2ROAD[r][i] == t)
					{
						for (int ii = pi; ii <= i; ii++) Go_Path.push_back(UnMix(_2ROAD[r][ii]));
					}
					else
					{
						for (int ii = i; ii >= pi; ii--) Go_Path.push_back(UnMix(_2ROAD[r][ii]));
					}
					return true;
				}
				pick = 1, pi = i;
				continue;
			}
			if (pick && !IS_VOID(_2ROAD[r][i] / 100, (_2ROAD[r][i] / 10) % 10, _2ROAD[r][i] % 10)) break;
		}
	}
	return false;
}
bool _2::ROAD_GO_B(Pos F, Pos T)
{
	queue<int> q;
	bool vis[501];
	int pre[501];
	while (!q.empty()) q.pop();
	memset(vis, 0, sizeof(vis));
	memset(pre, 0x3f, sizeof(pre));
	q.push(Mix(F));
	vis[Mix(F)] = 1;
	int mn = Mix(F), next = Mix(F);
	while (!q.empty())
	{
		mn = q.front();
		q.pop();
		if (mn == Mix(T))
		{
			Go_Path.clear();
			while (mn != 0x3f3f3f3f)
			{
				Go_Path.push_back(UnMix(mn));
				mn = pre[mn];
			}
			reverse(Go_Path.begin(), Go_Path.end());
			return true;
		}
		if (!IS_VOID(mn / 100, (mn / 10) % 10, mn % 10) && (mn != Mix(F))) continue;
		for (int i = 1; i <= 7; i++)
		{
			for (uint64_t j = 0; j < _2ROAD[i].size(); j++)
			{
				if (mn == _2ROAD[i][j])
				{
					if (j > 0 && vis[_2ROAD[i][j - 1]] == 0) vis[_2ROAD[i][j - 1]] = true, q.push(_2ROAD[i][j - 1]), pre[_2ROAD[i][j - 1]] = mn;
					if (j < _2ROAD[i].size() - 1 && vis[_2ROAD[i][j + 1]] == 0) vis[_2ROAD[i][j + 1]] = true, q.push(_2ROAD[i][j + 1]), pre[_2ROAD[i][j + 1]] = mn;
				}
			}
		}
	}
	return false;
}
bool _2::GOABLE(Chess A, Chess B, bool att)//att=1为攻击，=0为走动
{
	for (int i = 0; att == 1 && i < 5; i++) if (make_pair(B.cpx, B.cpy) == XY[i]) return false;
	if (NORMAL_GO(A, B)) return true;
	if (A.level == 32 && A.cflag == 2) return ROAD_GO_B(Pos(A.cpg, A.cpx, A.cpy), Pos(B.cpg, B.cpx, B.cpy));
	else return ROAD_GO_N(Pos(A.cpg, A.cpx, A.cpy), Pos(B.cpg, B.cpx, B.cpy));
}
void _2::MC()
{
	P[1].Init(1), P[2].Init(2);
	Record_Initialize();
	if (supermode) Go_Super();
	Go_Path.clear();
	ExMessage msg;
	int sc = -1;
	np = 1;
	STEPS_STATUS = 0;
	PRINTNOW();
	PS(0);
	bool did = 0;
	while (1)
	{
		if (peekmessage(&msg, EM_MOUSE) && msg.message == WM_LBUTTONDOWN)
		{
			did = 0;
			if (np == 1)
			{
				if (IS_MSG(msg, 630, 580, 60, 50))
				{
					WIN(2);
					return;
				}
				if (IS_MSG(msg, 710, 580, 60, 50))
				{
					if (P[np].skiptimes == MAX_JUMPS) continue;
					P[np].skiptimes++;
					np = 3 - np;
					STEPS_STATUS = Step();
					PRINTNOW();
					PS(1);
					if (STEPS_STATUS == 2)
					{
						WIN(0);
						return;
					}
					continue;
				}
			}
			else
			{
				if (IS_MSG(msg, 630, 195, 60, 50))
				{
					WIN(1);
					return;
				}
				if (IS_MSG(msg, 710, 195, 60, 50))
				{
					if (P[np].skiptimes == MAX_JUMPS) continue;
					P[np].skiptimes++;
					np = 3 - np;
					STEPS_STATUS = Step();
					PRINTNOW();
					PS(1);
					if (STEPS_STATUS == 2)
					{
						WIN(0);
						return;
					}
					continue;
				}
			}
			for (int j = 0; j < _CN; j++)
			{
				Chess tmp = P[np].MyChess[j];
				if (tmp.live == 0) continue;
				if (IS_MSG(msg, _2startposx[tmp.cpg] + _2divx * (tmp.cpy - 1) * trans[tmp.cpg], _2startposy[tmp.cpg] + _2divy * (tmp.cpx - 1) * trans[tmp.cpg], ChessX, ChessY))
				{
					PRINTNOW();
					did = 1;
					if (tmp.level == 10 || (tmp.level == 20 && tmp.cflag == 2)) continue;
					if (tmp.cflag == 0 && sc == j)
					{
						P[np].MyChess[j].cflag = 2;
						STEPS_STATUS = Step();
						Show(np, j);
						did = 1;
						sc = -1;
						np = 3 - np;
						PRINTNOW();
						PS(1);
						if (STEPS_STATUS == 2)
						{
							WIN(0);
							return;
						}
						break;
					}
					sc = j;
					char tname[20];
					strcpy_s(tname, (tmp.cflag == 2 ? _2LEVELMAP[tmp.level].c_str() : ""));
					button(_2startposx[tmp.cpg] + _2divx * (tmp.cpy - 1) * trans[tmp.cpg], _2startposy[tmp.cpg] + _2divy * (tmp.cpx - 1) * trans[tmp.cpg], ChessX, ChessY, tname, RGB(200, 200, 200));
					break;
				}
			}
			if (did == 1 || sc == -1) continue;
			for (int j = 0; j < _CN; j++)
			{
				Chess tmp = P[3 - np].MyChess[j];
				if (tmp.live == 0) continue;
				if (IS_MSG(msg, _2startposx[tmp.cpg] + _2divx * (tmp.cpy - 1) * trans[tmp.cpg], _2startposy[tmp.cpg] + _2divy * (tmp.cpx - 1) * trans[tmp.cpg], ChessX, ChessY))
				{
					int nextsd = -1;
					if (GOABLE(P[np].MyChess[sc], P[3 - np].MyChess[j], 1))
					{
						STEPS_STATUS = Step();
						did = 1;
						Chess c1 = P[np].MyChess[sc], c2 = P[3 - np].MyChess[j];
						P[np].MyChess[sc].cpg = c2.cpg, P[np].MyChess[sc].cpx = c2.cpx, P[np].MyChess[sc].cpy = c2.cpy;
						Move(np, sc, c2.cpg, c2.cpx, c2.cpy);
						bool d1 = 0, d2 = 0;
						P[np].MyChess[sc].cflag = 2, P[3 - np].MyChess[j].cflag = 2;
						Show(np, sc), Show(3 - np, j);
						switch (N_KILL(c1, c2))
						{
						case 0:
							for (int i = 0; i < _CN; i++) P[3 - np].MyChess[i].live = 0;
							Player_Dead(3 - np);
							if (c1.level == 21) P[np].MyChess[sc].live = 0, Dead(np, sc);//炸旗
							PRINTNOW();
							WIN(np);
							return;
						case 1:
							nextsd = 2;
							P[3 - np].MyChess[j].live = 0, P[3 - np].DeadChess.push_back(_4LEVELMAP[P[3 - np].MyChess[j].level]), Dead(3 - np, j);
							break;
						case 2:
							nextsd = 3;
							P[np].MyChess[sc].live = 0, P[np].DeadChess.push_back(_4LEVELMAP[P[np].MyChess[sc].level]), Dead(np, sc);
							break;
						case 3:
							nextsd = 4;
							P[np].MyChess[sc].live = 0, P[np].DeadChess.push_back(_4LEVELMAP[P[np].MyChess[sc].level]), Dead(np, sc);
							P[3 - np].MyChess[j].live = 0, P[3 - np].DeadChess.push_back(_4LEVELMAP[P[3 - np].MyChess[j].level]), Dead(3 - np, j);
							break;
						}
						np = 3 - np;
					}
					sc = -1;
					PRINTNOW();
					if (nextsd != -1) PS(nextsd);
					if (IS_DEAD(np))
					{
						WIN(3 - np);
						return;
					}
					if (IS_DEAD(3 - np))
					{
						WIN(np);
						return;
					}
					if (STEPS_STATUS == 2)
					{
						WIN(0);
						return;
					}
					break;
				}
			}
			if (did == 1 || sc == -1) continue;
			Chess tmp = P[np].MyChess[sc];
			for (int g = 1; g <= 2 && !did; g++)
			{
				for (int X = 1; X <= 6 && !did; X++)
				{
					for (int Y = 1; Y <= 5 && !did; Y++)
					{
						int sx = _2startposx[g] + _2divx * trans[g] * (Y - 1), sy = _2startposy[g] + _2divy * trans[g] * (X - 1);
						if (IS_MSG(msg,sx,sy,ChessX,ChessY) && GOABLE(P[np].MyChess[sc], Chess(0, g, X, Y), 0))
						{
							STEPS_STATUS = Step();
							P[np].MyChess[sc].cpg = g, P[np].MyChess[sc].cpx = X, P[np].MyChess[sc].cpy = Y;
							Move(np, sc, g, X, Y);
							np = 3 - np;
							did = 1;
							sc = -1;
							PRINTNOW();
							PS(1);
							if (STEPS_STATUS == 2)
							{
								WIN(0);
								return;
							}
						}
					}
				}
			}
		}
	}
}
void _4::GO_NEXT()
{
	do np = np % 4 + 1;
	while (P[np].live == 0);
}
void _4::PRINTNOW()
{
	BeginBatchDraw();
	cleardevice();
	putimage(0, 0, &bk0);
//	putimage(180, 0, &bk4);
	putnewbk(NULL, 180, 0, &bk4);
	setcolor(RGB(80, 20, 20));
	setfillcolor(RED);
	int lrx, lry, rx, ry;
	for (int i = 0; i < Go_Path.size(); i++)
	{
		Pos p = Go_Path[i];
		rx = (p.g ? stpx[p.g] + D * ((p.x - 1) * transxx[p.g] + (p.y - 1) * transyx[p.g]) : 442 + 2 * D * (p.x % 3)) + 13, ry = (p.g ? stpy[p.g] + D * ((p.x - 1) * transxy[p.g] + (p.y - 1) * transyy[p.g]) : 262 + 2 * D * (p.x / 3)) + 13;
		fillcircle(rx, ry, 8);
		setcolor(RGB(80, 80, 20));
		setfillcolor(RGB(200, 200, 100));
		if (i) Draw_Arrow(lrx, lry, rx, ry, 7);
		if (i == Go_Path.size() - 2)
		{
			setcolor(RGB(20, 80, 80));
			setfillcolor(RGB(100, 200, 200));
		}
		lrx = rx, lry = ry;
	}
	for (int i = 1; i <= 4; i++)
	{
		if (P[i].live == 0) continue;
		for (int j = 0; j < _CN; j++)
		{
			Chess tmp = P[i].MyChess[j];
			if (P[i].MyChess[j].live == 0) continue;
			char tname[20];
			strcpy_s(tname, ((tmp.cflag == 2 || showmode) ? _4LEVELMAP[tmp.level].c_str() : ""));
			if (tmp.cpg == 0) button(442+2*D*(tmp.cpx%3), 262+2*D*(tmp.cpx/3), L, L, tname, MyCol[i], 20);
			else button(stpx[tmp.cpg] + D * ((tmp.cpx-1)*transxx[tmp.cpg]+(tmp.cpy - 1) * transyx[tmp.cpg]), stpy[tmp.cpg] + D * ((tmp.cpx - 1) * transxy[tmp.cpg]+(tmp.cpy-1)*transyy[tmp.cpg]), L, L, tname, MyCol[i],20);
		}
	}
	for (int i = 1; i <= 4; i++)
	{
		char tname[20];
		string ts = P_Name[i];
		if (P[i].live == 0) ts += "(卒)";
		if (np == i && P[i].live != 0)
		{
			ts += "(行动)";
			if (historymode == 0) button(ppx[i] + 150, ppy[i], 40, 40, "投", MyCol[0], 25), button(ppx[i] + 193, ppy[i], 40, 40, "跳", MyCol[0], 25);
		}
		strcpy_s(tname, ts.c_str());
		button(ppx[i], ppy[i], 147, 40, tname, MyCol[i], 25);
		for (int cnt = 0; cnt != P[i].DeadChess.size(); cnt++)
		{
			strcpy_s(tname, P[i].DeadChess[cnt].c_str());
			button(ppx[i] + DD * (cnt % 5), ppy[i] + 50 + DD * (cnt / 5), L, L, tname, MyCol[i]);
		}
	}
	if (STEPS_STATUS == 1 || STEPS_STATUS == 2) button(600, 15, 450, 40, "请注意，相持时间已经接近系统限制", RGB(200, 0, 0));
	EndBatchDraw();
}
bool _4::TEAM_WIN()
{
	return ((P[1].live && P[3].live) || (P[2].live && P[4].live));
}
bool _4::ROAD_GO_N(Pos F, Pos T)
{
	int f = Mix(F), t = Mix(T);
	for (int r = 1; r <= 18; r++)
	{
		int pick = 0, pi = 0;
		for (int i = 0; i < _4ROAD[r].size(); i++)
		{
			if (_4ROAD[r][i] == f || _4ROAD[r][i] == t)
			{
				if (pick)
				{
					Go_Path.clear();
					if (_4ROAD[r][i] == t)
					{
						for (int ii = pi; ii <= i; ii++) Go_Path.push_back(UnMix(_4ROAD[r][ii]));
					}
					else
					{
						for (int ii = i; ii >= pi; ii--) Go_Path.push_back(UnMix(_4ROAD[r][ii]));
					}
					return true;
				}
				pick = 1;
				pi = i;
				continue;
			}
			if (pick && !IS_VOID(_4ROAD[r][i] / 100, (_4ROAD[r][i] / 10) % 10, _4ROAD[r][i] % 10)) break;
		}
	}
	return false;
}
bool _4::ROAD_GO_B(Pos F, Pos T)
{
	queue<int> q;
	bool vis[501];
	int pre[501];
	while (!q.empty()) q.pop();
	memset(vis, 0, sizeof(vis));
	memset(pre, 0x3f, sizeof(pre));
	q.push(Mix(F));
	vis[Mix(F)] = 1;
	int mn = Mix(F), next = Mix(F);
	while (!q.empty())
	{
		mn = q.front();
		q.pop();
		if (mn == Mix(T))
		{
			Go_Path.clear();
			while (mn != 0x3f3f3f3f)
			{
				Go_Path.push_back(UnMix(mn));
				mn = pre[mn];
			}
			reverse(Go_Path.begin(), Go_Path.end());
			return true;
		}
		if (!IS_VOID(mn / 100, (mn / 10) % 10, mn % 10) && (mn != Mix(F))) continue;
		for (int i = 1; i <= 18; i++)
		{
			for (uint64_t j = 0; j < _4ROAD[i].size(); j++)
			{
				if (mn == _4ROAD[i][j])
				{
					if (j > 0 && vis[_4ROAD[i][j - 1]] == 0) vis[_4ROAD[i][j - 1]] = true, q.push(_4ROAD[i][j - 1]), pre[_4ROAD[i][j - 1]] = mn;
					if (j < _4ROAD[i].size() - 1 && vis[_4ROAD[i][j + 1]] == 0) vis[_4ROAD[i][j + 1]] = true, q.push(_4ROAD[i][j + 1]), pre[_4ROAD[i][j + 1]] = mn;
				}
			}
		}
	}
	return false;
}
bool _4::GOABLE(Chess A, Chess B, bool att)
{
	for (int i = 0; att == 1 && i < 5; i++) if (make_pair(B.cpx, B.cpy) == XY[i]) return false;
	if (NORMAL_GO(A, B)) return true;
	if (A.level == 32 && A.cflag == 2) return ROAD_GO_B(Pos(A.cpg, A.cpx, A.cpy), Pos(B.cpg, B.cpx, B.cpy));
	else return ROAD_GO_N(Pos(A.cpg, A.cpx, A.cpy), Pos(B.cpg, B.cpx, B.cpy));
}
void _4::MC()
{
	for (int i = 1; i <= 4; i++) P[i].Init(i);
	Record_Initialize();
	if (supermode) Go_Super();
	Go_Path.clear();
	ExMessage msg;
	np = 1;
	STEPS_STATUS = 0;
	PRINTNOW();
	PS(0);
	bool did = 0;
	int sp = 0, sc = -1, lsp = 0, lsc = -1;
	while (1)
	{
		if (peekmessage(&msg, EM_MOUSE) && msg.message == WM_LBUTTONDOWN)
		{
			did = 0;
			if (IS_MSG(msg, ppx[np] + 150, ppy[np], 40, 40))//投降
			{
				STEPS_STATUS = Step();
				Player_Dead(np);
				P[np].live = 0;
				for (int i = 0; i < _CN; i++) P[np].MyChess[i].live = 0;
				sp = 0, sc = -1, lsp = 0, lsc = -1;
				GO_NEXT();
				tp--;
				if (tp == 1 || (tp == 2 && TEAM_WIN()))
				{
					PRINTNOW();
					WIN(((np == 1 || np == 3) ? 3 : 4));
					return;
				}
				PRINTNOW();
				PS(6);
				if (STEPS_STATUS == 2)
				{
					WIN(0);
					return;
				}
				continue;
			}
			if (IS_MSG(msg, ppx[np] + 193, ppy[np], 40, 40))//跳过
			{
				if (P[np].skiptimes == MAX_JUMPS) continue;
				STEPS_STATUS = Step();
				P[np].skiptimes++, GO_NEXT();
				sp = 0, sc = -1, lsp = 0, lsc = -1;
				PRINTNOW();
				PS(1);
				if (STEPS_STATUS == 2)
				{
					WIN(0);
					return;
				}
				continue;
			}
			for (int i = 1; i <= 4 && did == 0; i++)
			{
				if (P[i].live == 0) continue;
				for (int j = 0; j < _CN; j++)
				{
					Chess tmp = P[i].MyChess[j];
					if (tmp.live == 0) continue;
					if ((tmp.cpg != 0 && IS_MSG(msg, stpx[tmp.cpg] + D * ((tmp.cpx - 1) * transxx[tmp.cpg] + (tmp.cpy - 1) * transyx[tmp.cpg]), stpy[tmp.cpg] + D * ((tmp.cpx - 1) * transxy[tmp.cpg] + (tmp.cpy - 1) * transyy[tmp.cpg]), L, L)) || (tmp.cpg == 0 && IS_MSG(msg, 442 + 2 * D * (tmp.cpx % 3), 262 + 2 * D * (tmp.cpx / 3), L, L)))
					{
						if (i == np)
						{
							if (tmp.level == 10 || (tmp.level == 20 && tmp.cflag == 2))
							{
								sp = 0, sc = -1, lsp = 0, lsc = -1;
								PRINTNOW();
								break;
							}
							char tname[20];
							strcpy_s(tname, (tmp.cflag == 2 ? _2LEVELMAP[tmp.level].c_str() : ""));
							if (tmp.cpg == 0) button(442 + 2 * D * (tmp.cpx % 3), 262 + 2 * D * (tmp.cpx / 3), L, L, tname, MyCol[0], 20);
							else button(stpx[tmp.cpg] + D * ((tmp.cpx - 1) * transxx[tmp.cpg] + (tmp.cpy - 1) * transyx[tmp.cpg]), stpy[tmp.cpg] + D * ((tmp.cpx - 1) * transxy[tmp.cpg] + (tmp.cpy - 1) * transyy[tmp.cpg]), L, L, tname, MyCol[0], 20);
						}
						sp = i;
						sc = j;
						did = 1;
						break;
					}
				}
			}
			if (sp == np)
			{
				if (sc == lsc)
				{
					if (P[np].MyChess[lsc].cflag == 0)
					{
						STEPS_STATUS = Step();
						Show(np, lsc);
						P[np].MyChess[lsc].cflag = 2;
						lsp = 0, lsc = -1, sp = 0, sc = -1;
						GO_NEXT();
						PRINTNOW();
						PS(1);
						if (STEPS_STATUS == 2)
						{
							WIN(0);
							return;
						}
						continue;
					}
				}
				PRINTNOW();
				Chess tmp = P[sp].MyChess[sc];
				char tname[20];
				strcpy_s(tname, (tmp.cflag == 2 ? _4LEVELMAP[tmp.level].c_str() : ""));
				if (tmp.cpg == 0) button(442 + 2 * D * (tmp.cpx % 3), 262 + 2 * D * (tmp.cpx / 3), L, L, tname, MyCol[0], 20);
				else button(stpx[tmp.cpg] + D * ((tmp.cpx - 1) * transxx[tmp.cpg] + (tmp.cpy - 1) * transyx[tmp.cpg]), stpy[tmp.cpg] + D * ((tmp.cpx - 1) * transxy[tmp.cpg] + (tmp.cpy - 1) * transyy[tmp.cpg]), L, L, tname, MyCol[0], 20);
				lsp = sp, lsc = sc;
				sp = 0, sc = -1;
			}
			else if (sp != 0)
			{
				if (lsp == 0||abs(lsp - sp) == 2)
				{
					sp = 0, sc = -1, lsp = 0, lsc = -1;
					PRINTNOW();
					continue;
				}
				Chess A = P[lsp].MyChess[lsc], B = P[sp].MyChess[sc];
				int nextsd = -1, jump = 0;
				if (GOABLE(A, B, 1))
				{
					did = 1;
					STEPS_STATUS = Step();
					Chess c1 = P[lsp].MyChess[lsc], c2 = P[sp].MyChess[sc];
					P[lsp].MyChess[lsc].cpg = c2.cpg, P[lsp].MyChess[lsc].cpx = c2.cpx, P[lsp].MyChess[lsc].cpy = c2.cpy;
					Move(lsp, lsc, c2.cpg, c2.cpx, c2.cpy);
					P[lsp].MyChess[lsc].cflag = 2, P[sp].MyChess[sc].cflag = 2;
					Show(lsp, lsc), Show(sp, sc);
					switch (N_KILL(c1, c2))
					{
					case 0:
						if (c1.level == 21) P[lsp].MyChess[lsc].live = 0, Dead(lsp, lsc);//炸旗
						for (int i = 0; i < _CN; i++) P[sp].MyChess[i].live = 0;
						Player_Dead(sp);
						P[sp].live = 0;
						tp--;
						GO_NEXT();
						PRINTNOW();
						if (tp == 1 || (tp == 2 && TEAM_WIN()))
						{
							WIN(((np == 1 || np == 3) ? 3 : 4));
							return;
						}
						sp = 0, sc = -1, lsp = 0, lsc = -1;
						PS(6);//叮
						jump = 1;
						break;
					case 1:
						nextsd = 2;
						P[sp].MyChess[sc].live = 0, P[sp].DeadChess.push_back(_4LEVELMAP[P[sp].MyChess[sc].level]), Dead(sp, sc);
						break;
					case 2:
						nextsd = 3;
						P[lsp].MyChess[lsc].live = 0, P[lsp].DeadChess.push_back(_4LEVELMAP[P[lsp].MyChess[lsc].level]), Dead(lsp, lsc);
						break;
					case 3:
						nextsd = 4;
						P[sp].MyChess[sc].live = 0, P[sp].DeadChess.push_back(_4LEVELMAP[P[sp].MyChess[sc].level]), Dead(sp, sc);
						P[lsp].MyChess[lsc].live = 0, P[lsp].DeadChess.push_back(_4LEVELMAP[P[lsp].MyChess[lsc].level]), Dead(lsp, lsc);
						break;
					}
					if (jump) continue;
					if (IS_DEAD(sp))
					{
						P[sp].live = 0;
						Player_Dead(sp);
						for (int i = 0; i < _CN; i++) P[sp].MyChess[i].live = 0;
						tp--;
						if (tp == 1 || (tp == 2 && TEAM_WIN()))
						{
							GO_NEXT();
							PRINTNOW();
							WIN(((np == 1 || np == 3) ? 3 : 4));
							return;
						}
					}
					if (IS_DEAD(lsp))
					{
						P[lsp].live = 0;
						Player_Dead(lsp);
						for (int i = 0; i < _CN; i++) P[lsp].MyChess[i].live = 0;
						tp--;
						if (tp == 1||(tp == 2 && TEAM_WIN()))
						{
							GO_NEXT();
							PRINTNOW();
							WIN(((np == 1 || np == 3) ? 3 : 4));
							return;
						}
					}
					GO_NEXT();
				}
				sp = 0, sc = -1, lsp = 0, lsc = -1;
				PRINTNOW();
				if (nextsd != -1) PS(nextsd);
				if (STEPS_STATUS == 2)
				{
					WIN(0);
					return;
				}
			}
			else
			{
				if (lsp == 0) continue;
				int G = 0, X = 0, Y = 0, did = 0;
				for (int j = 0; j < 9; j++)
				{
					if (IS_MSG(msg, 442 + 2 * D * (j % 3), 262 + 2 * D * (j / 3), L, L))
					{
						G = 0, X = j;
						did = 1;
						break;
					}
				}
				for (int i = 1; i <= 4 && did == 0; i++)
				{
					for (int j = 1; j <= 6 && did == 0; j++)
					{
						for (int k = 1; k <= 5; k++)
						{
							if (IS_MSG(msg, stpx[i] + D * ((j - 1) * transxx[i] + (k - 1) * transyx[i]), stpy[i] + D * ((j - 1) * transxy[i] + (k - 1) * transyy[i]), L, L))
							{
								G = i, X = j, Y = k;
								did = 1;
								break;
							}
						}
					}
				}
				if (did == 0) continue;
				if (GOABLE(P[lsp].MyChess[lsc], Chess(0, G, X, Y), 0))
				{
					STEPS_STATUS = Step();
					P[lsp].MyChess[lsc].cpg = G, P[lsp].MyChess[lsc].cpx = X, P[lsp].MyChess[lsc].cpy = Y;
					Move(lsp, lsc, G, X, Y);
					sp = 0, sc = -1, lsp = 0, lsc = -1;
					GO_NEXT();
					PRINTNOW();
					PS(1);
					if (STEPS_STATUS == 2)
					{
						WIN(0);
						return;
					}
					continue;
				}
			}
		}
	}
}
void Record::Record_Initialize()
{
	if (!DO_REC) return;
	cnt_step = 0;
	string NAME = "Records/Rec-", TS = "";
	if (Tp == 2) NAME += "2-";
	else NAME += "4-";
	time_t timep;
	time(&timep);
	char tmp[64];
	struct tm nowTime;
	localtime_s(&nowTime, &timep);
	strftime(tmp, sizeof(tmp), "%Y-%m-%d-%H-%M-%S", &nowTime);
	NAME += string(tmp);
	NAME += ".rec";
	char file_name[100];
	strcpy_s(file_name, NAME.c_str());
	freopen_s(&stream, file_name, "w", stdout);
	printf("REC %d %d\nplayers\n", Tp, supermode);
	for (int i = 1; i <= Tp; i++)
	{
		for (int j = 0; j < _CN; j++) printf("%d ", P[i].MyChess[j].level);
		printf("\n");
	}
}
void Record::Record_End(int who)
{
	printf("win %d\n", who);
	freopen_s(&stream, "CON", "w", stdout);
}
void Record::Move(int lsp, int lsc, int G, int X, int Y)
{
	printf("move %d %d %d %d %d\n", lsp, lsc, G, X, Y);
	printf("path %llu\n", Go_Path.size());
	for (vector<Pos>::iterator i = Go_Path.begin(); i != Go_Path.end(); i++) printf("%d ", Mix(*i));
	printf("\n");
}
void Record::Show(int lsp, int lsc)
{
	printf("show %d %d\n", lsp, lsc);
}
void Record::Dead(int lsp, int lsc)
{
	printf("dead %d %d\n", lsp, lsc);
}
void Record::RS(int sound)
{
	printf("sound %d\n", sound);
}
int Record::Step()
{
	cnt_step++;
	if (cnt_step > MAX_STEPS) return 2;
	if (cnt_step > MAX_STEPS - REST_STEPS) return 1;
	printf("step %d\n", ++cnt_step);
	return 0;
}
void Record::Player_Dead(int who)
{
	printf("pd %d\n", who);
}
void HISTORY()
{
	string path = "Records";
	vector<string> Recs;
	Recs.clear();
	int N_R = 0, PAGE = 1, EP = 12;
	//------------------------------------------------------------抄来的-Begin
	//这个结构体能区分文件和目录
	struct stat sb;
	//循环直到文件夹中的所有项都结束
	for (const auto& entry : filesystem::directory_iterator(path)) {
		//在下面的行中将路径转换为const char *
		filesystem::path outfilename = entry.path();
		string outfilename_str = outfilename.string();
		const char* path = outfilename_str.c_str();
		//检测路径是否指向非目录；如果是，则显示路径
		if (stat(path, &sb) == 0 && !(sb.st_mode & S_IFDIR)) Recs.push_back(outfilename_str), N_R++;
	}
	//------------------------------------------------------------抄来的-End
	cleardevice();
	if (N_R == 0)
	{
		button(100, 200, 880, 420, "没有存档", RGB(255, 255, 0), 40);
		Sleep(2000);
		return;
	}
	int total_page = (N_R - 1) / EP + 1;
	bool ft = 1, did = 0;
	ExMessage msg;
	char tname[100];
	while (1)
	{
		did = 0;
		if (peekmessage(&msg, EM_MOUSE) && msg.message == WM_LBUTTONDOWN)
		{
			did = 1;
			if (PAGE != 1 && FH.IS_MSG(msg, 30, 30, 80, 50))
			{
				PAGE--;
				continue;
			}
			if (PAGE != total_page && FH.IS_MSG(msg, 970, 30, 80, 50))
			{
				PAGE++;
				continue;
			}
			if (FH.IS_MSG(msg, 30, 650, 60, 40)) return;
			for (int i = (PAGE - 1) * EP; i < min(PAGE * EP, N_R); i++)
			{
				if (FH.IS_MSG(msg, 120, 100 + 50 * (i - (PAGE - 1) * EP), 840, 45))
				{
					strcpy_s(tname, Recs[i].c_str());
					button(120, 100 + 50 * (i - (PAGE - 1) * EP), 840, 45, tname, RGB(100, 100, 100));
					Sleep(300);
					freopen_s(&stream, tname, "r", stdin);
					cleardevice();
					string key;
					cin >> key;
					if (key != "REC")
					{
						button(40, 40, 1000, 50, "ERROR!NOT A RECORD FILE!", RGB(255, 255, 0), 30);
						freopen_s(&stream, "CON", "r", stdin);
						Sleep(1000);
						break;
					}
					cin >> Tp >> supermode;
					tp = Tp, np = 0, showmode = 0;
					FH.Go_Path.clear();
					int ns = 0, op = 0, go_back = 0, lsp, lsc, g, x, y;
					cin >> key;
					for (int j = 1; j <= Tp; j++)
					{
						P[j].Init(j);
						for (int k = 0; k < _CN; k++) cin >> P[j].MyChess[k].level;
					}
					if (supermode)
					{
						cin >> key;
						if (key != "N")
						{
							button(40, 40, 1000, 50, "ERROR!AN OLD SUPER RECORD!", RGB(255, 255, 0), 30);
							freopen_s(&stream, "CON", "r", stdin);
							Sleep(1000);
							break;
						}
						int N;
						cin >> N;
						cin >> key;
						for (int i = 1; i <= Tp; i++)
						{
							for (int j = 0; j < N; j++)
							{
								int rj;
								cin >> rj;
								P[i].MyChess[rj].cflag = 2;
							}
						}
					}
					if (Tp == 4)
					{
						while (key != "win" && go_back == 0 && cin >> key)
						{
							if (key == "sound")
							{
								cin >> op;
								FH.PS(op);
								H4.PRINTNOW();
								BeginBatchDraw();
								button(30, 30, 60, 30, "复盘", RGB(255, 0, 0), 25);
								button(100, 30, 80, 30, "下一步", RGB(0, 0, 255), 25);
								button(30, 70, 60, 30, (showmode == 0 ? "正常" : "全明"), RGB(255, 255, 0), 25);
								string title = to_string(ns) + "步";
								strcpy_s(tname, title.c_str());
								button(100, 70, 80, 30, tname, RGB(0, 255, 0), 25);
								button(30, 650, 60, 40, "返回", RGB(100, 100, 100), 25);
								EndBatchDraw();
								while (1)
								{
									
									if (peekmessage(&msg, EM_MOUSE) && msg.message == WM_LBUTTONDOWN )
									{
										
										if (FH.IS_MSG(msg, 100, 30, 80, 30)) break;
										if (FH.IS_MSG(msg, 30, 650, 60, 40))
										{
											go_back = 1;
											break;
										}
										if (FH.IS_MSG(msg, 30, 70, 60, 30))
										{
											showmode = 1 - showmode;
											H4.PRINTNOW();
											BeginBatchDraw();
											button(30, 30, 60, 30, "复盘", RGB(255, 0, 0), 25);
											button(100, 30, 80, 30, "下一步", RGB(0, 0, 255), 25);
											button(30, 70, 60, 30, (showmode == 0 ? "正常" : "全明"), RGB(255, 255, 0), 25);
											string title = to_string(ns) + "步";
											strcpy_s(tname, title.c_str());
											button(100, 70, 80, 30, tname, RGB(0, 255, 0), 25);
											button(30, 650, 60, 40, "返回", RGB(100, 100, 100), 25);
											EndBatchDraw();
										}
									}
								}
							}
							else if (key == "step")
							{
								cin >> ns;
								H4.GO_NEXT();
							}
							else if (key == "pd")
							{
								cin >> op;
								P[op].live = 0;
								for (int k = 0; k < _CN; k++) P[op].MyChess[k].live = 0;
							}
							else if (key == "move")
							{
								cin >> lsp >> lsc >> g >> x >> y;
								P[lsp].MyChess[lsc].cpg = g, P[lsp].MyChess[lsc].cpx = x, P[lsp].MyChess[lsc].cpy = y;
								cin >> key;
								cin >> op;
								FH.Go_Path.clear();
								for (int j = 0; j < op; j++)
								{
									int mixed;
									cin >> mixed;
									FH.Go_Path.push_back(FH.UnMix(mixed));
								}
							}
							else if (key == "show")
							{
								cin >> lsp >> lsc;
								P[lsp].MyChess[lsc].cflag = 2;
							}
							else if (key == "dead")
							{
								cin >> lsp >> lsc;
								P[lsp].MyChess[lsc].live = 0;
								P[lsp].DeadChess.push_back(FH._4LEVELMAP[P[lsp].MyChess[lsc].level]);
							}
						}
						if (go_back) continue;
						if (key != "win") op = 0;
						else cin >> op;
						H4.PRINTNOW();
						FH.WIN(op);
						
					}
					else 
					{
						while (key != "win" && go_back == 0 && cin >> key)
						{
							if (key == "sound")
							{
								cin >> op;
								FH.PS(op);
								H2.PRINTNOW();
								BeginBatchDraw();
								button(900, 300, 60, 30, "复盘", RGB(255, 0, 0), 25);
								button(970, 300, 80, 30, "下一步", RGB(0, 0, 255), 25);
								button(900, 340, 60, 30, (showmode == 0 ? "正常" : "全明"), RGB(255, 255, 0), 25);
								string title = to_string(ns) + "步";
								strcpy_s(tname, title.c_str());
								button(970, 340, 80, 30, tname, RGB(0, 255, 0), 25);
								button(990, 660, 60, 40, "返回", RGB(100, 100, 100), 25);
								EndBatchDraw();
								while (1)
								{
									if (peekmessage(&msg, EM_MOUSE) && msg.message == WM_LBUTTONDOWN)
									{
										if (FH.IS_MSG(msg, 970, 300, 80, 30)) break;
										if (FH.IS_MSG(msg, 990, 660, 60, 40))
										{
											go_back = 1;
											break;
										}
										if (FH.IS_MSG(msg, 900, 340, 60, 30))
										{
											showmode = 1 - showmode;
											H2.PRINTNOW();
											BeginBatchDraw();
											button(900, 300, 60, 30, "复盘", RGB(255, 0, 0), 25);
											button(970, 300, 80, 30, "下一步", RGB(0, 0, 255), 25);
											button(900, 340, 60, 30, (showmode == 0 ? "正常" : "全明"), RGB(255, 255, 0), 25);
											string title = to_string(ns) + "步";
											strcpy_s(tname, title.c_str());
											button(970, 340, 80, 30, tname, RGB(0, 255, 0), 25);
											button(990, 660, 60, 40, "返回", RGB(100, 100, 100), 25);
											EndBatchDraw();
										}
									}
								}
							}
							else if (key == "step")
							{
								cin >> ns;
								np = 3 - np;
							}
							else if (key == "move")
							{
								cin >> lsp >> lsc >> g >> x >> y;
								P[lsp].MyChess[lsc].cpg = g, P[lsp].MyChess[lsc].cpx = x, P[lsp].MyChess[lsc].cpy = y;
								cin >> key;
								cin >> op;
								FH.Go_Path.clear();
								for (int j = 0; j < op; j++)
								{
									int mixed;
									cin >> mixed;
									FH.Go_Path.push_back(FH.UnMix(mixed));
								}
							}
							else if (key == "show")
							{
								cin >> lsp >> lsc;
								P[lsp].MyChess[lsc].cflag = 2;
							}
							else if (key == "dead")
							{
								cin >> lsp >> lsc;
								P[lsp].MyChess[lsc].live = 0;
								P[lsp].DeadChess.push_back(FH._4LEVELMAP[P[lsp].MyChess[lsc].level]);
							}
						}
						if (go_back) continue;
						if (key != "win") op = 0;
						else cin >> op;
						H2.PRINTNOW();
						FH.WIN(op);
					}
					freopen_s(&stream, "CON", "r", stdin);
				}
			}
		}
		if (ft || did)
		{
			ft = 0, did = 0;
			BeginBatchDraw();
			cleardevice();
			putimage(0, 0, &bk0);
			if (PAGE != 1) button(30, 30, 80, 50, "上一页", RGB(100, 100, 100), 25);
			if (PAGE != total_page) button(970, 30, 80, 50, "下一页", RGB(100, 100, 100), 25);
			button(30, 650, 60, 40, "返回", RGB(100, 100, 100), 25);
			string tmp = "复盘列表:";
			tmp += to_string(PAGE);
			tmp += "/";
			tmp += to_string(total_page);
			strcpy_s(tname, tmp.c_str());
			button(120, 30, 840, 50, tname, RGB(255, 0, 0), 25);
			for (int i = (PAGE - 1) * EP; i < min(PAGE * EP, N_R); i++)
			{
				strcpy_s(tname, Recs[i].c_str());
				button(120, 100 + 50 * (i - (PAGE - 1) * EP), 840, 45, tname, RGB(0, 255, 0));
			}
			EndBatchDraw();
		}
	}
}
void Setting()
{
	ExMessage msg;
	IMAGE on, off, select;
	loadimage(&on, "Resources/on.png", 120, 60);
	loadimage(&off, "Resources/off.png", 120, 60);
	loadimage(&select, "Resources/select.png", 200, 60);
	Pub FS;
	bool ft = 1, did = 0;
	int vx = 700, vy = 90,div = 80;
	bool* op[4] = { &DO_REC, &AutoUpdate, &PLAY_BGM, &PLAY_SOUND };
	int* num[3] = { &MAX_STEPS, &MAX_JUMPS, &REST_STEPS };
	while (1)
	{
		if (peekmessage(&msg, EM_MOUSE) && msg.message == WM_LBUTTONDOWN)
		{
			did = 1;
			if (FS.IS_MSG(msg, 30, 650, 60, 40))
			{
				freopen_s(&stream, "settings.config", "w", stdout);
				string sts[7] = { "DO_REC","AutoUpdate","PLAY_BGM","PLAY_SOUND","MAX_STEPS","MAX_JUMPS","REST_STEPS" };
				for (int i = 0; i < 7; i++)
				{
					printf("%s %d\n", sts[i].c_str(), (int)((i < 4) ? *op[i] : *num[i - 4]));
				}
				printf("END 0");
				freopen_s(&stream, "CON", "w", stdout);
				return;
			}
			for (int i = 0; i < 4; i++)
			{
				if ((msg.x - vx - 30) * (msg.x - vx - 30) + (msg.y - vy - 30 - div * i) * (msg.y - vy - 30 - div * i) <= 900 || (msg.x - vx - 90) * (msg.x - vx - 90) + (msg.y - vy - 30 - div * i) * (msg.y - vy - 30 - div * i) <= 900 || (vx + 30 <= msg.x && msg.x <= vx + 90 && vy + div * i <= msg.y && msg.y <= vy + div * i + 60))
				{
					*op[i] = !(*op[i]);
					if (i == 2)
					{
						if (*op[i] == 1) PlaySound("Resources/BGM.wav", NULL, SND_ASYNC | SND_LOOP);
						else PlaySound(NULL, NULL, NULL);
					}
				}
			}
			for (int i = 4; i < 7; i++)
			{
				if (FS.IS_MSG(msg, vx - 40, vy + div * i, 50, 60)) (*num[i - 4])--;
				if (FS.IS_MSG(msg, vx + 110, vy + div * i, 50, 60)) (*num[i - 4])++;
				if ((*num[i - 4]) < 0) (*num[i - 4]) = 0;
				if (REST_STEPS >= MAX_STEPS + 20)
				{
					if (i == 4) (*num[i - 4])++;
					else (*num[i - 4])--;
				}
			}
		}
		if (ft || did)
		{
			ft = 0, did = 0;
			BeginBatchDraw();
			cleardevice();
			putimage(0, 0, &bk0);
			button(500, 20, 100, 50, "设置", RGB(100, 100, 200), 35);
			button(30, 650, 60, 40, "返回", RGB(100, 100, 100), 25);
			string str[7] = { "自动保存对局","自动更新软件","播放背景音乐","播放落子音效","最大对局步数","最大跳过次数","警告剩余步数" };
			for (int i = 0; i < 7; i++)
			{
				char tname[20];
				strcpy_s(tname, str[i].c_str());
				button(200, vy + div * i, 300, 60, tname, RGB(200, 100, 0), 25);
				if (i < 4)
				{
					if (*op[i]) putnewbk(NULL, vx, vy + div * i, &on);
					else putnewbk(NULL, vx, vy + div * i, &off);
				}
				else
				{
					putnewbk(NULL, vx - 40, vy + div * i, &select);
					settextstyle(30, 0, "黑体");
					outtextxy(vx + 60 - textwidth(to_string(*num[i-4]).c_str()) / 2, vy + 30 + div * i - textheight(to_string(*num[i-4]).c_str()) / 2, to_string(*num[i-4]).c_str());
				}
			}
			EndBatchDraw();
		}
	}
}
void PRINT_main()
{
	int _dx = 170, _sx = 40, _sy = 120, _x = 150, _y = 60;
	BeginBatchDraw();
	cleardevice();
	putimage(0, 0, &bk0);
	button(_sx + 110, 40, 780, 60, "请选择游玩模式:", RGB(200, 20, 20), 30);
	button(_sx + 900, 40, 100, 60, "设置", RGB(100, 100, 200), 30);
	button(_sx, 40, 100, 60, "关于", RGB(100, 100, 200), 30);
	button(_sx, _sy, _x, _y, "全暗双人", RGB(20, 200, 20), 25);
	button(_sx + _dx, _sy, _x, _y, "随机双人", RGB(20, 200, 20), 25);
	button(_sx + _dx * 2, _sy, _x, _y, "全暗四人", RGB(20, 200, 20), 25);
	button(_sx + _dx * 3, _sy, _x, _y, "随机四人", RGB(20, 200, 20), 25);
	button(_sx + _dx * 4, _sy, _x, _y, "加载复盘", RGB(20, 200, 20), 25);
	button(_sx + _dx * 5, _sy, _x, _y, "退出游戏", RGB(20, 200, 20), 25);
	LOGFONT f;
	gettextstyle(&f);
	f.lfHeight = 18;
	_tcscpy_s(f.lfFaceName, "黑体");
	f.lfQuality = ANTIALIASED_QUALITY;
	settextstyle(&f);
	outtextxy(30, 670, "version 0.5.2");
	outtextxy(670, 670, "Copyright 2024 PRXOR. All rights reserved.");
	EndBatchDraw();
}