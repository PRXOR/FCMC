# 设计说明书
### 四国翻棋
### 沈默 2024040129
## 类设计
### Chess结构体
```cpp
struct Chess
{
	int level, cpg, cpx, cpy, cflag;//棋子大小，横竖坐标，被标记为
	bool live;//是否存活
	Chess(int le, int _g, int _x, int _y):level(le),cpg(_g),cpx(_x),cpy(_y),cflag(le == 10 ? 2 : 0),live(true){}//将第一排标记为叹号，军旗翻开
	Chess():level(0),cpg(0),cpx(0),cpy(0),cflag(0),live(0){}
};
```
### Pos结构体
```cpp
struct Pos
{
	int g, x, y;//区块，横坐标，纵坐标
	Pos(int _g, int _x, int _y):g(_g),x(_x),y(_y){}
};
```
### Player类
```cpp
class Player
{
public:
	bool live=true;//初始存活
	int initarray[_CN] = {}, deflevel[_CN] = { 10,20,20,20,21,21,32,32,32,33,33,33,34,34,34,35,35,36,36,37,37,38,38,39,40 };//旗雷炸兵排连营团旅师军司
	int defposx[_CN] = { 6,1,1,1,1,1,2,2,2,3,3,3,3,4,4,4,5,5,5,5,5,6,6,6,6 }, defposy[_CN] = { 4,1,2,3,4,5,1,3,5,1,2,4,5,1,3,5,1,2,3,4,5,1,2,3,5 };//初始位置
	int skiptimes = 0;//跳过次数
	void Init(int ppos);//初始化
	Chess MyChess[_CN] = {};//棋子
	vector<string> DeadChess;//死亡棋子
} P[5];//全局最多有玩家1-4
```
### Record类(用于写入存档)
```cpp
class Record
{
	int cnt_step = 0;//记录步数
public:
	static vector<Pos> Go_Path;//存储走棋路径
	static int Mix(Pos P);//混合坐标
	static Pos UnMix(int mixed);//解混合坐标
	void Record_Initialize();//初始化记录存档
	static void Record_End(int who);//记录结束
	void Move(int lsp, int lsc, int G, int X, int Y);//记录移动
	void Show(int lsp, int lsc);//记录翻开
	void Dead(int lsp, int lsc);//记录棋子死亡
	static void RS(int sound);//记录音效
	int Step();//记录步数，返回步数状态
	void Player_Dead(int who);//记录玩家死亡
};
```
### Pub类(继承Record，包含_2,_4的公共函数)
```cpp
class Pub : public Record
{
public:
	static map< pair<int, int>, vector< pair<int, int> > > NA;//非铁道普攻
	static vector<int> _2ROAD[8], _4ROAD[20];//1-7,1-18,分别为双人/四人铁道
	static string _2LEVELMAP[41], _4LEVELMAP[41];//双人/四人棋子名称
	static pair<int, int> XY[5];//行营
	string P_Name[5] = { "","玩家1","玩家2","玩家3","玩家4" };//玩家名称
	static void PS(int sound);//播放音效
	static void WIN(int who);//胜利
	void Draw_Arrow(int x1, int y1, int x2, int y2, int L);//画箭头
	bool IS_VOID(int g, int x, int y);//判断是否为空
	bool IS_DEAD(int p);//判断玩家是否死亡
	bool NORMAL_GO(Chess A, Chess B);//普通移动
	int N_KILL(Chess A, Chess B);//判断大小，返回谁死亡
	void Go_Super();//超级模式初始化
	static void Game_Initialize();//游戏初始化
	virtual void PRINTNOW() = 0;
	virtual bool ROAD_GO_N(Pos F, Pos T) = 0;
	virtual bool ROAD_GO_B(Pos F, Pos T) = 0;
	virtual bool GOABLE(Chess A, Chess B, bool att) = 0;
	virtual void MC() = 0;
};
```
### _2类(双人模式)
```cpp
class _2 : Pub
{
	int ChessX = 60, ChessY = 27;//棋子的长宽
	int _2startposx[3] = { 0,20,480 }, _2startposy[3] = { 0,424,274 }, _2divx = 115, _2divy = 51;//绘图相关棋盘参数
	int trans[3] = { 0,1,-1 };//转换坐标
	bool ROAD_GO_N(Pos F, Pos T);//常规铁道移动
	bool ROAD_GO_B(Pos F, Pos T);//工兵铁道移动
	bool GOABLE(Chess A, Chess B, bool att);//判断是否可移动
public:
	void PRINTNOW();//打印当前棋盘及附加图像
	void MC();//主函数
} PLAY2, H2;//PLAY2用于游玩，H2用于双人模式的存档
```
### _4类(四人模式)
```cpp
class _4 : Pub
{
	//绘图相关棋盘参数
	int stpx[5] = { 0,442,651,610,400 }, stpy[5] = { 0,472,430,220,262 }, L = 27, D = 42, DD = 30;
	int ppx[5] = { 0,651,651,190,190 }, ppy[5] = { 0,472,10,10,472 };
	COLORREF MyCol[5] = { RGB(100,100,100),RGB(216,108,0),RGB(145,63,165),RGB(112,156,0),RGB(39,104,160) };
	int transxx[5] = { 0,0,1,0,-1 }, transxy[5] = { 0,1,0,-1,0 }, transyx[5] = { 0,1,0,-1,0 }, transyy[5] = { 0,0,-1,0,1 };//转换坐标
	bool TEAM_WIN();//判断队伍胜利
	bool ROAD_GO_N(Pos F, Pos T);//常规铁道移动
	bool ROAD_GO_B(Pos F, Pos T);//工兵铁道移动
	bool GOABLE(Chess A, Chess B, bool att);//判断是否可移动
public:
	void GO_NEXT();//轮换下一步
	void PRINTNOW();//打印当前棋盘及附加图像
	void MC();//主函数
} PLAY4, H4;//PLAY4用于游玩，H4用于四人模式的存档
```