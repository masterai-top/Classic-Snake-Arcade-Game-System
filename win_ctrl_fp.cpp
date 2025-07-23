#include <time.h>
#include <stdlib.h>
#include "win_ctrl_fp.h"


enum
{
	FPCTRL_INITSIGN,			//初始标志
	FPCTRL_BASECOIN,			//控制净分
	FPCTRL_ST,				//游戏状态
	FPCTRL_MAXGUN,			//玩家一段时间内的平均炮倍
	FPCTRL_BETOUTTAIL,		//抽水余数

	FPCTRL_REALBASECOIN,		//真实输赢
	FPCTRL_AIMCOIN,			//杀送分目标
	FPCTRL_NOWBASECOIN,		//当前控制净分
	FPCTRL_UNFITGUN_NUM,		//发炮次数
	FPCTRL_NORMALTIMES,		//正常状态发射次数

	FPCTRL_AIMTIMES,			//目标发射次数
	FPCTRL_FDCOIN,			//没有打到鱼的子弹，废弹
};



//static int JXSET_BETOUTRATE = 20;				//抽水率，千分
//static int JXSET_AIMTIMES_UNIT = 200;			//控制分数走向的基础手数
//static int JXSET_COID_LIMIT[6] = {
//	4000, 800,							//超难档
//	2000, 900,							//困难档
//	-3000, 1150							//容易档
//};
//static int JXSET_NORMALTIMES_CHOICE[8] = {	//普通状态下的变档手数选项 
//	120, 300, 100, 150,
//	50, 100, 150, 200
//};


static int set_time_rand_sign = 0;
static int set_rand_numrange_sign = 0;

static int GetRanNum(void)
{
	int tempnum = 0;
	int times = 0;

	if (set_time_rand_sign == 0)
	{
		int i;
		srand((int)time(0));
		set_time_rand_sign = 1;

		for (i = 0; i < 100; i++)
		{
			if (rand() >= 100000)
			{
				set_rand_numrange_sign = 1;
				break;
			}
		}
	}

	tempnum = rand();

	if (set_rand_numrange_sign > 0)
	{
		tempnum = tempnum % 30000;
	}
	else
	{
		times = 0;
		while (tempnum >= 30000)
		{
			times++;
			tempnum = rand();

			if (times >= 5)
			{
				tempnum = tempnum % 30000;
				break;
			}
		}

	}

	return tempnum;
}
static int GetLongRanNum(void)
{
	int ran1 = GetRanNum();
	int ran2 = GetRanNum();

	int ran = (ran1 % 10000) * 10000 + ran2 % 10000;

	return ran;

}

static int JXSetWinflg(long long player_info[], int now_gun,int setinfo[])
{
	int i;
	int ran_choice;
	long long tempnum;
	int winflg = 0;
	int *JXSET_NORMALTIMES_CHOICE = &setinfo[ID_FPSET_NORMALTIMES_CHOICE];
	int *JXSET_COID_LIMIT = &setinfo[ID_FPSET_COID_LIMIT];

	if (player_info[FPCTRL_INITSIGN] != 0xcfde)
	{
		for (i = 0; i < 20; i++)
		{
			player_info[i] = 0;
		}
		player_info[FPCTRL_INITSIGN] = 0xcfde;
		player_info[FPCTRL_MAXGUN] = 2000;
		player_info[FPCTRL_NORMALTIMES] = 68;
	}

	if (now_gun > player_info[FPCTRL_MAXGUN])
	{
		player_info[FPCTRL_MAXGUN] = (player_info[FPCTRL_MAXGUN] * 6 + now_gun) / 7;
	}
	else if (now_gun < player_info[FPCTRL_MAXGUN])
	{
		player_info[FPCTRL_MAXGUN] = (player_info[FPCTRL_MAXGUN] * 66 + now_gun) / 67;
	}

	if (player_info[FPCTRL_ST] == 0)
	{
		player_info[FPCTRL_NORMALTIMES]--;

		if (player_info[FPCTRL_BASECOIN] >= player_info[FPCTRL_MAXGUN] * JXSET_COID_LIMIT[0])
		{
			ran_choice = GetRanNum() % 8 + 1;
			player_info[FPCTRL_ST] = 3;
			tempnum = (JXSET_COID_LIMIT[0] - JXSET_COID_LIMIT[4])*player_info[FPCTRL_MAXGUN] / 8;
			player_info[FPCTRL_AIMCOIN] = 0 - tempnum*ran_choice;
			player_info[FPCTRL_AIMTIMES] = setinfo[ID_FPSET_AIMTIMES_UNIT] * ran_choice;
			player_info[FPCTRL_NOWBASECOIN] = 0;
			player_info[FPCTRL_UNFITGUN_NUM] = 0;
			winflg = 3;
		}
		else if (player_info[FPCTRL_BASECOIN] >= player_info[FPCTRL_MAXGUN] * JXSET_COID_LIMIT[2])
		{
			ran_choice = GetRanNum() % 8 + 1;
			player_info[FPCTRL_ST] = 2;
			tempnum = (JXSET_COID_LIMIT[2] - JXSET_COID_LIMIT[4])*player_info[FPCTRL_MAXGUN] / 8;
			player_info[FPCTRL_AIMCOIN] = 0 - tempnum*ran_choice;
			player_info[FPCTRL_AIMTIMES] = setinfo[ID_FPSET_AIMTIMES_UNIT] * ran_choice;
			player_info[FPCTRL_NOWBASECOIN] = 0;
			player_info[FPCTRL_UNFITGUN_NUM] = 0;
			winflg = 2;
		}
		else if (player_info[FPCTRL_BASECOIN] <= player_info[FPCTRL_MAXGUN] * JXSET_COID_LIMIT[4])
		{
			ran_choice = GetRanNum() % 8 + 1;
			player_info[FPCTRL_ST] = 1;
			tempnum = (JXSET_COID_LIMIT[2] - JXSET_COID_LIMIT[4])*player_info[FPCTRL_MAXGUN] / 8;
			player_info[FPCTRL_AIMCOIN] = tempnum*ran_choice;
			player_info[FPCTRL_AIMTIMES] = setinfo[ID_FPSET_AIMTIMES_UNIT] * ran_choice;
			player_info[FPCTRL_UNFITGUN_NUM] = 0;
			player_info[FPCTRL_NOWBASECOIN] = 0;
			winflg = 1;
		}
		else
		{
			if (player_info[FPCTRL_NORMALTIMES] <= 0)
			{
				if (GetRanNum() % 100 < 55)
				{
					ran_choice = GetRanNum() % 8 + 1;
					player_info[FPCTRL_ST] = 2;
					tempnum = (JXSET_COID_LIMIT[2] - JXSET_COID_LIMIT[4])*player_info[FPCTRL_MAXGUN] / 8;
					player_info[FPCTRL_AIMCOIN] = 0 - tempnum*ran_choice;
					player_info[FPCTRL_AIMTIMES] = setinfo[ID_FPSET_AIMTIMES_UNIT] * ran_choice;
					player_info[FPCTRL_NOWBASECOIN] = 0;
					player_info[FPCTRL_UNFITGUN_NUM] = 0;
					winflg = 2;
				}
				else
				{
					ran_choice = GetRanNum() % 8 + 1;
					player_info[FPCTRL_ST] = 1;
					tempnum = (JXSET_COID_LIMIT[2] - JXSET_COID_LIMIT[4])*player_info[FPCTRL_MAXGUN] / 8;
					player_info[FPCTRL_AIMCOIN] = tempnum*ran_choice;
					player_info[FPCTRL_AIMTIMES] = setinfo[ID_FPSET_AIMTIMES_UNIT] * ran_choice;
					player_info[FPCTRL_UNFITGUN_NUM] = 0;
					player_info[FPCTRL_NOWBASECOIN] = 0;
					winflg = 1;
				}
			}
			else
				winflg = 0;
		}
	}
	else if (player_info[FPCTRL_ST] == 1)
	{
		player_info[FPCTRL_UNFITGUN_NUM]++;
		player_info[FPCTRL_AIMTIMES]--;


		if (player_info[FPCTRL_NOWBASECOIN] >= player_info[FPCTRL_AIMCOIN] || player_info[FPCTRL_BASECOIN] >= player_info[FPCTRL_MAXGUN] * JXSET_COID_LIMIT[2])
		{
			winflg = 0;
			player_info[FPCTRL_ST] = 0;
			player_info[FPCTRL_NORMALTIMES] = JXSET_NORMALTIMES_CHOICE[GetRanNum() % 8];
		}
		else if (player_info[FPCTRL_UNFITGUN_NUM] >= 3000 && player_info[FPCTRL_NOWBASECOIN] <= 0)
		{
			player_info[FPCTRL_ST] = 4;
			winflg = 1;
		}
		else if (player_info[FPCTRL_UNFITGUN_NUM] >= player_info[FPCTRL_AIMTIMES] && player_info[FPCTRL_NOWBASECOIN] <= player_info[FPCTRL_AIMCOIN]/3)
		{
			player_info[FPCTRL_ST] = 4;
			winflg = 1;
		}
		else if (player_info[FPCTRL_AIMTIMES] <= 0)
		{
			player_info[FPCTRL_ST] = 4;
			winflg = 1;
		}
		else
		{
			winflg = 1;
		}
	}
	else if (player_info[FPCTRL_ST] == 4)
	{
		if (player_info[FPCTRL_NOWBASECOIN] >= player_info[FPCTRL_AIMCOIN] || player_info[FPCTRL_BASECOIN] >= player_info[FPCTRL_MAXGUN] * JXSET_COID_LIMIT[2])
		{
			winflg = 0;
			player_info[FPCTRL_ST] = 0;
			player_info[FPCTRL_NORMALTIMES] = JXSET_NORMALTIMES_CHOICE[GetRanNum() % 8];
		}
		else
			winflg = 1;
	}
	else if (player_info[FPCTRL_ST] == 2)
	{
		if (player_info[FPCTRL_NOWBASECOIN] <= player_info[FPCTRL_AIMCOIN] || player_info[FPCTRL_BASECOIN] <= player_info[FPCTRL_MAXGUN] * JXSET_COID_LIMIT[4])
		{
			winflg = 0;
			player_info[FPCTRL_ST] = 0;
			player_info[FPCTRL_NORMALTIMES] = JXSET_NORMALTIMES_CHOICE[GetRanNum() % 8];
		}
		else if (player_info[FPCTRL_BASECOIN] >= player_info[FPCTRL_MAXGUN] * JXSET_COID_LIMIT[0])
		{
			ran_choice = GetRanNum() % 8 + 1;
			player_info[FPCTRL_ST] = 3;
			tempnum = (JXSET_COID_LIMIT[0] - JXSET_COID_LIMIT[4])*player_info[FPCTRL_MAXGUN] / 8;
			player_info[FPCTRL_AIMCOIN] = 0 - tempnum*ran_choice;
			player_info[FPCTRL_AIMTIMES] = setinfo[ID_FPSET_AIMTIMES_UNIT] * ran_choice;
			player_info[FPCTRL_NOWBASECOIN] = 0;
			player_info[FPCTRL_UNFITGUN_NUM] = 0;
			winflg = 3;
		}
		else
			winflg = 2;
	}
	else
	{
		if (player_info[FPCTRL_NOWBASECOIN] <= player_info[FPCTRL_AIMCOIN] || player_info[FPCTRL_BASECOIN] <= player_info[FPCTRL_MAXGUN] * JXSET_COID_LIMIT[4])
		{
			winflg = 0;
			player_info[FPCTRL_ST] = 0;
			player_info[FPCTRL_NORMALTIMES] = JXSET_NORMALTIMES_CHOICE[GetRanNum() % 8];
		}
		else
			winflg = 3;
	}

	return winflg;
}


//通过权重和出现控制，获得随机结果
static int GetWeightRusult(int quanzhong[], int resultsign[], int num)
{
	int tempall = 0;
	int abc = 10000;
	int i;
	int tempresult[16];
	int tempran;
	int sign = 0;

	for (i = 0; i < num; i++)
	{
		if (resultsign[i] > 0)
		{
			sign = 1;
			break;
		}
	}

	if (sign == 0)
	{
		for (i = 0; i < num; i++)
			resultsign[i] = 1;
	}

	for (i = 0; i < num; i++)
	{
		if (resultsign[i] > 0)
		{
			tempresult[i] = quanzhong[i];
		}
		else
			tempresult[i] = 0;

		tempall += tempresult[i];

	}

	for (i = 1; i < num; i++)
	{
		tempresult[i] = tempresult[i - 1] + tempresult[i];
	}

	if (tempall == 0)
	{
		tempall = tempall;
	}

	tempran = GetRanNum() % tempall;

	for (i = 0; i < num; i++)
	{
		if (tempran < tempresult[i] && resultsign[i] && quanzhong[i] > 0)
		{
			return i;
		}
	}

	return 0;
}


int FPGetBigbombfishFourNum(int superbombfishtype)
{
	int j;
	int onenum = 0;
	int nownum;
	int numrate3000[] = { 130, 280, 150, 250, 120, 70, 30, 20, 10, 10 }; //3000
	int numrate2500[] = { 160, 310, 170, 150, 100, 50, 20, 5, 0, 0 };    //2500
	int numrate2000[] = { 200, 420, 290, 80, 40, 20, 10, 0, 0, 0 };    //2000
	int numrate1500[] = { 200, 500, 110, 30, 0, 0, 0, 0, 0, 0 };    //1500
	int numrate1000[] = { 500, 330, 30, 0, 0, 0, 0, 0, 0, 0 };    //1000
	int *numrate;
	int numrate2[] = { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 };
	int numsign[10];

	for (j = 0; j < 10; j++)
	{
		numsign[j] = 1;
	}
	if (superbombfishtype == 5)
	{
		numrate = numrate3000;
	}
	else if (superbombfishtype == 3)
	{
		numrate = numrate2000;
	}
	else if (superbombfishtype == 4)
	{
		numrate = numrate2500;
	}
	else if (superbombfishtype == 2)
		numrate = numrate1500;
	else
		numrate = numrate1000;

	nownum = GetWeightRusult(numrate, numsign, 10);
	onenum += nownum;
	numsign[nownum] = 0;

	if (onenum == 0)
	{
		//int tempnumsign[] = { 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 };
		int tempnumsign[] = { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
		if (superbombfishtype < 2)
		{
			tempnumsign[5] = 1;
			tempnumsign[6] = 1;
		}

		nownum = GetWeightRusult(numrate2, tempnumsign, 10);
		onenum += nownum;
		numsign[nownum] = 0;
		for (j = 0; j < 2; j++)
		{
			onenum = onenum * 10;
			nownum = GetWeightRusult(numrate2, numsign, 10);
			onenum += nownum;
			numsign[nownum] = 0;
		}
	}
	else
	{
		for (j = 0; j < 3; j++)
		{
			onenum = onenum * 10;
			nownum = GetWeightRusult(numrate2, numsign, 10);
			onenum += nownum;
			numsign[nownum] = 0;
		}
	}


	return onenum;
}


int FPCheckInput(int now_gun, int power, int fish_score, int superbombfishtype)
{
	if (now_gun<0 || now_gun >100000)
	{
		return 1;
	}

	if (power<0 || power > 10000)
	{
		return 1;
	}

	if (fish_score<0 || fish_score > 20000)
	{
		return 1;
	}

	if (superbombfishtype < 0 || superbombfishtype > 10)
	{
		return 1;
	}
	return 0;
}

int FPCheckFishDead(long long player_info[], int now_gun,int power, int fish_score, int superbombfishtype,int setinfo[])
{
	int winflg = JXSetWinflg(player_info, now_gun,setinfo);
	int *JXSET_COID_LIMIT = &setinfo[ID_FPSET_COID_LIMIT];
	int win_rate[] = {
		1000 - setinfo[ID_FPSET_BETOUTRATE],
		JXSET_COID_LIMIT[5],
		JXSET_COID_LIMIT[3],
		JXSET_COID_LIMIT[1],
	};
	int ran = GetLongRanNum() % 1000000;
	int win_sign = 0;
	int firstrate;
	int usingpower = power;

	if (FPCheckInput(now_gun,power,fish_score,superbombfishtype) > 0)
	{
		return -1;
	}

	//winflg = 0;

	firstrate = win_rate[winflg];


	if (player_info[FPCTRL_ST] == 4)
	{
		usingpower = usingpower * 3 / 2;
	}

	if (player_info[FPCTRL_FDCOIN] >= now_gun && usingpower < fish_score)
	{
		long long tnum = player_info[FPCTRL_FDCOIN] / now_gun;
		long long usingfpnum = 0;

		if (tnum + usingpower < fish_score)
		{
			usingpower = usingpower + (int)tnum;
			usingfpnum = tnum * now_gun;
		}
		else
		{
			usingfpnum = (fish_score - usingpower) * now_gun;
			usingpower = fish_score;
		}


		player_info[FPCTRL_FDCOIN] -= usingfpnum;
		player_info[FPCTRL_BASECOIN] -= usingfpnum;
		player_info[FPCTRL_REALBASECOIN] -= usingfpnum;
	}

	if (ran < usingpower*(win_rate[winflg] * 1000) / fish_score)
	{
		win_sign = 1;
	}


	player_info[FPCTRL_BASECOIN] -= now_gun*power;
	player_info[FPCTRL_NOWBASECOIN] -= now_gun*power;
	player_info[FPCTRL_REALBASECOIN] -= now_gun*power;




	if (win_sign > 0)
	{
		player_info[FPCTRL_BASECOIN] += fish_score * now_gun;
		player_info[FPCTRL_REALBASECOIN] += fish_score * now_gun;
		player_info[FPCTRL_BETOUTTAIL] += fish_score * now_gun;
		player_info[FPCTRL_NOWBASECOIN] += fish_score * now_gun;

		player_info[FPCTRL_BASECOIN] += player_info[FPCTRL_BETOUTTAIL] / 1000 * setinfo[ID_FPSET_BETOUTRATE];
		player_info[FPCTRL_BETOUTTAIL] = player_info[FPCTRL_BETOUTTAIL] % 1000;
	}

	if (win_sign>0 && superbombfishtype > 0)
	{
		return FPGetBigbombfishFourNum(superbombfishtype);
	}

	return win_sign;
}

//对玩家没有作用到鱼身上的子弹进行处理
void FPSetBetin(long long player_info[], int betin_coin)
{
	if (player_info[FPCTRL_INITSIGN] != 0xcfde)
	{
		int i;
		for (i = 0; i < 20; i++)
		{
			player_info[i] = 0;
		}
		player_info[FPCTRL_INITSIGN] = 0xcfde;
		player_info[FPCTRL_MAXGUN] = 3000;
		player_info[FPCTRL_NORMALTIMES] = 1688;
	}
	player_info[FPCTRL_FDCOIN] += betin_coin;
}