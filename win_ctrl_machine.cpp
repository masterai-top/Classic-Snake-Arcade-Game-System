#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sys/time.h>
#include "win_ctrl_machine.h"
using namespace std;


enum
{
	JXCTRL_INITSIGN,			//初始标志
	JXCTRL_BASECOIN,			//控制净分
	JXCTRL_ST,				//游戏状态
	JXCTRL_MAXGUN,			//玩家一段时间内的平均炮倍
	JXCTRL_BETOUTTAIL,		//抽水余数

	JXCTRL_REALBASECOIN,		//真实输赢
	JXCTRL_AIMCOIN,			//杀送分目标
	JXCTRL_NOWBASECOIN,		//当前控制净分
	JXCTRL_UNFITGUN_NUM,		//发炮次数
	JXCTRL_BASEGUN,			//基础炮倍

	JXCTRL_NOWIN,				//玩家投入
	JXCTRL_NOWOUT,			//玩家获得
	JXCTRL_AIMIN,				//目标投入
	JXCTRL_AIMOUT,			//目标获得
	JXCTRL_NORMALTIMES,		//正常状态发射次数

	JXCTRL_AIMTIMES,			//目标发射次数
	JXCTRL_FDCOIN,			//没有打到鱼的子弹
	JXCTRL_DEAD_RATE,       //鱼击杀概率
	JXCTRL_RATE_RANGE,      //概率区间
	JXCTRL_RAND_NUM,        //本次随的数
};


static int JXSET_BETOUTRATE = 20;				//抽水率，千分
static int JXSET_SONG_BASE = 4;				//送分比例
static int JXSET_SHA_BASE = 6;					//杀分比例
static int JXSET_SHA_RATE = 960;				//杀分回报率
static int JXSET_SONG_NUM = 400;				//送分段基础倍率
static int JXSET_AIMTIMES_UNIT = 8000;			//控制分数走向的基础手数
static int JXSET_COID_LIMIT[6] = {
	2000, 920,							//超难档
	1500, 950,							//困难档
	-2000, 1050							//容易档
};
static int JXSET_NORMALTIMES_CHOICE[8] = {	//普通状态下的变档手数选项 
	3000, 4000, 5000, 6000,
	1000, 2000, 3000, 4000
};
static int jx_set_sign = 0;

static int set_time_rand_sign = 0;
static int set_rand_numrange_sign = 0;

static int GetRanNum(void)
{
	/*
	struct timeval tpstart;
	gettimeofday(&tpstart,NULL);
	srand(tpstart.tv_usec); 
	int nRandNum = rand()%30000;
	return nRandNum;*/
	
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

			if (times >= 10)
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
static void JXGameSetCtrlAim(int nowgun, long long playerinfo[])
{
	int songbeinum = JXSET_SONG_NUM;
	int shabeinum = 0;
	int shasong_sign = 0;
	int temprate[] = {
		100, 50, 60, 90, 150, 120, 130, 100
	};

	if (playerinfo[JXCTRL_AIMIN] > playerinfo[JXCTRL_AIMOUT])
	{
		shasong_sign = 1;
	}
	else
	{
		shasong_sign = 0;
	}

	if (playerinfo[JXCTRL_ST] >= 2 && playerinfo[JXCTRL_NOWBASECOIN] >= 0)
	{
		shasong_sign = 0;
	}

	if (playerinfo[JXCTRL_NOWIN] >= playerinfo[JXCTRL_AIMIN] && playerinfo[JXCTRL_NOWOUT] >= playerinfo[JXCTRL_AIMOUT])
	{
		playerinfo[JXCTRL_NOWIN] = playerinfo[JXCTRL_NOWIN] - playerinfo[JXCTRL_AIMIN];
		playerinfo[JXCTRL_NOWOUT] = playerinfo[JXCTRL_NOWOUT] - playerinfo[JXCTRL_AIMOUT];
	}

	playerinfo[JXCTRL_BASEGUN] = nowgun;

	if (shasong_sign == 0)
	{
		shabeinum = (1000 * (JXSET_SONG_BASE + 1)*JXSET_SONG_NUM - JXSET_SHA_RATE * JXSET_SONG_BASE *JXSET_SONG_NUM) / (JXSET_SHA_RATE*JXSET_SHA_BASE - 1000 * (JXSET_SHA_BASE - 1));
		shabeinum = shabeinum * temprate[GetRanNum() % 8] / 100;
		playerinfo[JXCTRL_AIMIN] = shabeinum * nowgun * JXSET_SHA_BASE;
		playerinfo[JXCTRL_AIMOUT] = shabeinum * nowgun * (JXSET_SHA_BASE - 1);
	}
	else
	{
		songbeinum = songbeinum * temprate[GetRanNum() % 8] / 100;
		playerinfo[JXCTRL_AIMIN] = songbeinum * nowgun * JXSET_SONG_BASE;
		playerinfo[JXCTRL_AIMOUT] = songbeinum * nowgun * (JXSET_SONG_BASE + 1);
	}
}

void JXSetCtrlValue(int set_array[])
{
	int i;

	JXSET_BETOUTRATE = set_array[ID_JXSET_BETOUTRATE];
	JXSET_SONG_BASE = set_array[ID_JXSET_SONG_BASE];
	JXSET_SHA_BASE = set_array[ID_JXSET_SHA_BASE];
	JXSET_SHA_RATE = set_array[ID_JXSET_SHA_RATE];
	JXSET_SONG_NUM = set_array[ID_JXSET_SONG_NUM];

	JXSET_AIMTIMES_UNIT = set_array[ID_JXSET_AIMTIMES_UNIT];

	for (i = 0; i < 6;i++)
	{
		JXSET_COID_LIMIT[i] = set_array[ID_JXSET_COID_LIMIT+i];
	}

	for (i = 0; i < 8; i++)
	{
		JXSET_NORMALTIMES_CHOICE[i] = set_array[ID_JXSET_NORMALTIMES_CHOICE + i];
	}

	jx_set_sign = 36;
}

static int JXSetWinflg(long long player_info[], int now_gun,int bullet_type)
{
	int i;
	int ran_choice;
	long long tempnum;
	int winflg = 0;

	if (player_info[JXCTRL_INITSIGN] != 0xcfde)
	{
		for (i = 0; i < 20; i++)
		{
			player_info[i] = 0;
		}
		player_info[JXCTRL_INITSIGN] = 0xcfde;
		player_info[JXCTRL_MAXGUN] = 3000;
		player_info[JXCTRL_NORMALTIMES] = 1688;
	}

	if (now_gun > player_info[JXCTRL_MAXGUN])
	{
		player_info[JXCTRL_MAXGUN] = (player_info[JXCTRL_MAXGUN] * 9 + now_gun) / 10;
	}
	else if (now_gun < player_info[JXCTRL_MAXGUN])
	{
		player_info[JXCTRL_MAXGUN] = (player_info[JXCTRL_MAXGUN] * 99 + now_gun) / 100;
	}

	if (player_info[JXCTRL_ST] == 0)
	{
		player_info[JXCTRL_NORMALTIMES]--;
		if (bullet_type == 1)
		{
			player_info[JXCTRL_NORMALTIMES]--;
		}
		else if (bullet_type == 2)
		{
			player_info[JXCTRL_NORMALTIMES] -= 3;
		}

		if (player_info[JXCTRL_BASECOIN] >= player_info[JXCTRL_MAXGUN] * JXSET_COID_LIMIT[0])
		{
			ran_choice = GetRanNum() % 8 + 1;
			player_info[JXCTRL_ST] = 3;
			tempnum = (JXSET_COID_LIMIT[0] - JXSET_COID_LIMIT[4])*player_info[JXCTRL_MAXGUN] / 8;
			player_info[JXCTRL_AIMCOIN] = 0 - tempnum*ran_choice;
			player_info[JXCTRL_AIMTIMES] = JXSET_AIMTIMES_UNIT * ran_choice;
			player_info[JXCTRL_NOWBASECOIN] = 0;
			player_info[JXCTRL_UNFITGUN_NUM] = 0;
			player_info[JXCTRL_NOWIN] = 0;
			player_info[JXCTRL_NOWOUT] = 0;
			player_info[JXCTRL_AIMOUT] = player_info[JXCTRL_AIMIN] + 10;
			JXGameSetCtrlAim(now_gun, player_info);
			winflg = 3;
		}
		else if (player_info[JXCTRL_BASECOIN] >= player_info[JXCTRL_MAXGUN] * JXSET_COID_LIMIT[2])
		{
			ran_choice = GetRanNum() % 8 + 1;
			player_info[JXCTRL_ST] = 2;
			tempnum = (JXSET_COID_LIMIT[2] - JXSET_COID_LIMIT[4])*player_info[JXCTRL_MAXGUN] / 8;
			player_info[JXCTRL_AIMCOIN] = 0 - tempnum*ran_choice;
			player_info[JXCTRL_AIMTIMES] = JXSET_AIMTIMES_UNIT * ran_choice;
			player_info[JXCTRL_NOWBASECOIN] = 0;
			player_info[JXCTRL_UNFITGUN_NUM] = 0;
			player_info[JXCTRL_NOWIN] = 0;
			player_info[JXCTRL_NOWOUT] = 0;
			player_info[JXCTRL_AIMOUT] = player_info[JXCTRL_AIMIN] + 10;
			JXGameSetCtrlAim(now_gun, player_info);
			winflg = 2;
		}
		else if (player_info[JXCTRL_BASECOIN] <= player_info[JXCTRL_MAXGUN] * JXSET_COID_LIMIT[4])
		{
			ran_choice = GetRanNum() % 8 + 1;
			player_info[JXCTRL_ST] = 1;
			tempnum = (JXSET_COID_LIMIT[2] - JXSET_COID_LIMIT[4])*player_info[JXCTRL_MAXGUN] / 8;
			player_info[JXCTRL_AIMCOIN] = tempnum*ran_choice;
			player_info[JXCTRL_AIMTIMES] = JXSET_AIMTIMES_UNIT * ran_choice;
			player_info[JXCTRL_UNFITGUN_NUM] = 0;
			player_info[JXCTRL_NOWBASECOIN] = 0;
			winflg = 1;
		}
		else
		{
			if (player_info[JXCTRL_NORMALTIMES] <= 0)
			{
				if (GetRanNum() % 100 < 55)
				{
					ran_choice = GetRanNum() % 8 + 1;
					player_info[JXCTRL_ST] = 2;
					tempnum = (JXSET_COID_LIMIT[2] - JXSET_COID_LIMIT[4])*player_info[JXCTRL_MAXGUN] / 8;
					player_info[JXCTRL_AIMCOIN] = 0 - tempnum*ran_choice;
					player_info[JXCTRL_AIMTIMES] = JXSET_AIMTIMES_UNIT * ran_choice;
					player_info[JXCTRL_NOWBASECOIN] = 0;
					player_info[JXCTRL_UNFITGUN_NUM] = 0;
					player_info[JXCTRL_NOWIN] = 0;
					player_info[JXCTRL_NOWOUT] = 0;
					player_info[JXCTRL_AIMOUT] = player_info[JXCTRL_AIMIN] + 10;
					JXGameSetCtrlAim(now_gun, player_info);
					winflg = 2;
				}
				else
				{
					ran_choice = GetRanNum() % 8 + 1;
					player_info[JXCTRL_ST] = 1;
					tempnum = (JXSET_COID_LIMIT[2] - JXSET_COID_LIMIT[4])*player_info[JXCTRL_MAXGUN] / 8;
					player_info[JXCTRL_AIMCOIN] = tempnum*ran_choice;
					player_info[JXCTRL_AIMTIMES] = JXSET_AIMTIMES_UNIT * ran_choice;
					player_info[JXCTRL_UNFITGUN_NUM] = 0;
					player_info[JXCTRL_NOWBASECOIN] = 0;
					winflg = 1;
				}
			}
			else
				winflg = 0;
		}
	}
	else if (player_info[JXCTRL_ST] == 1)
	{
		player_info[JXCTRL_UNFITGUN_NUM]++;
		player_info[JXCTRL_AIMTIMES]--;
		if (bullet_type == 1)
		{
			player_info[JXCTRL_UNFITGUN_NUM]++;
			player_info[JXCTRL_AIMTIMES]--;
		}
		else if (bullet_type == 2)
		{
			player_info[JXCTRL_UNFITGUN_NUM] += 3;
			player_info[JXCTRL_AIMTIMES] -= 3;
		}

		if (player_info[JXCTRL_NOWBASECOIN] >= player_info[JXCTRL_AIMCOIN] || player_info[JXCTRL_BASECOIN] >= player_info[JXCTRL_MAXGUN] * JXSET_COID_LIMIT[2])
		{
			winflg = 0;
			player_info[JXCTRL_ST] = 0;
			player_info[JXCTRL_NORMALTIMES] = JXSET_NORMALTIMES_CHOICE[GetRanNum() % 8];
		}
		else if (player_info[JXCTRL_UNFITGUN_NUM] >= 3000 && player_info[JXCTRL_NOWBASECOIN] <= 0)
		{
			player_info[JXCTRL_ST] = 4;
			winflg = 1;
		}
		else if (player_info[JXCTRL_UNFITGUN_NUM] >= player_info[JXCTRL_AIMTIMES] && player_info[JXCTRL_NOWBASECOIN] <= player_info[JXCTRL_AIMCOIN]/3)
		{
			player_info[JXCTRL_ST] = 4;
			winflg = 1;
		}
		else if (player_info[JXCTRL_AIMTIMES] <= 0)
		{
			player_info[JXCTRL_ST] = 4;
			winflg = 1;
		}
		else if (player_info[JXCTRL_BASECOIN] <= 2*player_info[JXCTRL_MAXGUN] * JXSET_COID_LIMIT[4])
		{
			player_info[JXCTRL_ST] = 4;
			winflg = 1;
		}
		else
		{
			winflg = 1;
		}
	}
	
	else if (player_info[JXCTRL_ST] == 4)
	{
		if (player_info[JXCTRL_NOWBASECOIN] >= player_info[JXCTRL_AIMCOIN] || player_info[JXCTRL_BASECOIN] >= player_info[JXCTRL_MAXGUN] * JXSET_COID_LIMIT[2])
		{
			winflg = 0;
			player_info[JXCTRL_ST] = 0;
			player_info[JXCTRL_NORMALTIMES] = JXSET_NORMALTIMES_CHOICE[GetRanNum() % 8];
		}
		else
			winflg = 1;
	}
	else if (player_info[JXCTRL_ST] == 2)
	{
		if (player_info[JXCTRL_NOWBASECOIN] <= player_info[JXCTRL_AIMCOIN] || player_info[JXCTRL_BASECOIN] <= player_info[JXCTRL_MAXGUN] * JXSET_COID_LIMIT[4])
		{
			winflg = 0;
			player_info[JXCTRL_ST] = 0;
			player_info[JXCTRL_NORMALTIMES] = JXSET_NORMALTIMES_CHOICE[GetRanNum() % 8];
		}
		else if (player_info[JXCTRL_BASECOIN] >= player_info[JXCTRL_MAXGUN] * JXSET_COID_LIMIT[0])
		{
			player_info[JXCTRL_ST] = 3;
			winflg = 3;
		}
		else
			winflg = 2;
	}
	else
	{
		if (player_info[JXCTRL_NOWBASECOIN] <= player_info[JXCTRL_AIMCOIN] || player_info[JXCTRL_BASECOIN] <= player_info[JXCTRL_MAXGUN] * JXSET_COID_LIMIT[4])
		{
			winflg = 0;
			player_info[JXCTRL_ST] = 0;
			player_info[JXCTRL_NORMALTIMES] = JXSET_NORMALTIMES_CHOICE[GetRanNum() % 8];
		}
		else
			winflg = 3;
	}

	return winflg;
}


int JXRealCheckDead(long long playerinfo[], int nowgun, int fishscore, int bullettype)
{
	int deadsign = 0;
	int tempout = nowgun * fishscore;
	int rannum = GetRanNum() % 10000;
	int bulletpower = 1;
	int nDeadRate = 0;

	if (bullettype == 1)
	{
		bulletpower = 2;
	}
	else if (bullettype == 2)
	{
		bulletpower = 4;
	}

	if (playerinfo[JXCTRL_AIMIN] > 12000 * nowgun || playerinfo[JXCTRL_AIMOUT] > 12000 * nowgun  || playerinfo[JXCTRL_AIMIN] <= 0 || playerinfo[JXCTRL_AIMOUT] <= 0 || playerinfo[JXCTRL_AIMIN] >= playerinfo[JXCTRL_AIMOUT] * 2 || playerinfo[JXCTRL_AIMIN] * 2 <= playerinfo[JXCTRL_AIMOUT])
	{
		playerinfo[JXCTRL_NOWIN] = 0;
		playerinfo[JXCTRL_NOWOUT] = 0;
		playerinfo[JXCTRL_NOWBASECOIN] = 0;
		playerinfo[JXCTRL_UNFITGUN_NUM] = 0;
		JXGameSetCtrlAim(nowgun, playerinfo);
	}

	if (nowgun < playerinfo[JXCTRL_BASEGUN] / 3)
	{
		playerinfo[JXCTRL_UNFITGUN_NUM]++;

		if (playerinfo[JXCTRL_UNFITGUN_NUM] >= 1000)
		{
			playerinfo[JXCTRL_NOWIN] = 0;
			playerinfo[JXCTRL_NOWOUT] = 0;
			JXGameSetCtrlAim(nowgun, playerinfo);
		}

		if (playerinfo[JXCTRL_ST] == 0)
		{
			nDeadRate = (10000 - JXSET_BETOUTRATE * 10) * bulletpower / fishscore;
			if (rannum < nDeadRate)
				deadsign = 1;
		}
		else if (playerinfo[JXCTRL_ST] == 1)
		{
			nDeadRate = JXSET_COID_LIMIT[5] * 10 * bulletpower / fishscore;
			if (rannum < nDeadRate)
				deadsign = 1;
		}
		else
		{
			nDeadRate = JXSET_COID_LIMIT[3] * 10 * bulletpower / fishscore;
			if (rannum < nDeadRate)
				deadsign = 1;
		}
		playerinfo[JXCTRL_DEAD_RATE] = nDeadRate;
		return deadsign;
	}
	else
	{
		playerinfo[JXCTRL_UNFITGUN_NUM] = 0;
	}

	if (nowgun >= playerinfo[JXCTRL_BASEGUN] * 3)
	{
		playerinfo[JXCTRL_AIMOUT] *= nowgun / playerinfo[JXCTRL_BASEGUN];
		playerinfo[JXCTRL_AIMIN] *= nowgun / playerinfo[JXCTRL_BASEGUN];
		playerinfo[JXCTRL_BASEGUN] = nowgun;
	}

	if (playerinfo[JXCTRL_NOWIN] >= playerinfo[JXCTRL_AIMIN] && playerinfo[JXCTRL_NOWOUT] >= playerinfo[JXCTRL_AIMOUT])
	{
		JXGameSetCtrlAim(nowgun, playerinfo);
	}


	if (playerinfo[JXCTRL_NOWIN] < playerinfo[JXCTRL_AIMIN] * 2 / 3)
	{
		if (playerinfo[JXCTRL_NOWOUT] + tempout < playerinfo[JXCTRL_AIMOUT] && (playerinfo[JXCTRL_AIMIN] - playerinfo[JXCTRL_NOWIN]) * 10 / (playerinfo[JXCTRL_AIMOUT] - playerinfo[JXCTRL_NOWOUT] - tempout) < 19)
		{
			if (playerinfo[JXCTRL_NOWIN] < 88 * nowgun)
			{
				if (playerinfo[JXCTRL_ST] == 1)
				{
					nDeadRate = 10500 * bulletpower / fishscore;
					if (rannum < nDeadRate)
						deadsign = 1;
				}
				else
				{
					nDeadRate = 9500 * bulletpower / fishscore;
					if (rannum < nDeadRate)
						deadsign = 1;
				}

			}
			else
			{
				if (playerinfo[JXCTRL_AIMOUT] > playerinfo[JXCTRL_AIMIN])
				{
					nDeadRate = 10600 * bulletpower / fishscore;
					if (rannum < nDeadRate)
						deadsign = 1;
				}
				else
				{
					nDeadRate = 9300 * bulletpower / fishscore;
					if (rannum < nDeadRate)
						deadsign = 1;
				}

			}
		}
	}
	else if (playerinfo[JXCTRL_NOWIN] < playerinfo[JXCTRL_AIMIN])
	{
		if (playerinfo[JXCTRL_NOWIN] + 20 * nowgun >= playerinfo[JXCTRL_AIMIN])
		{
			if (playerinfo[JXCTRL_NOWOUT] + playerinfo[JXCTRL_AIMIN] + tempout <= playerinfo[JXCTRL_NOWIN] + playerinfo[JXCTRL_AIMOUT] + 8 * nowgun)
			{

				if (fishscore > 248)
				{
					nDeadRate = (100000 + 20000 * (bulletpower - 1)) / fishscore;
					if (rannum < nDeadRate)
						deadsign = 1;
				}
				else if (fishscore > 138)
				{
					nDeadRate = (50000 + 20000 * (bulletpower - 1)) / fishscore;
					if (rannum < nDeadRate)
						deadsign = 1;
				}
				else if (fishscore > 48)
				{
					nDeadRate = (30000 + 10000 * (bulletpower - 1)) / fishscore;
					if (rannum < nDeadRate)
						deadsign = 1;
				}
				else
				{
					nDeadRate = 11000 * bulletpower / fishscore;
					if (rannum < nDeadRate)
						deadsign = 1;
				}

			}
		}
		else
		{
			if (playerinfo[JXCTRL_NOWOUT] + tempout < playerinfo[JXCTRL_NOWIN] * (20 * playerinfo[JXCTRL_AIMOUT] / playerinfo[JXCTRL_AIMIN]) / 20)
			{
				if (fishscore > 248)
				{
					nDeadRate = (80000 + 20000 * (bulletpower - 1)) / fishscore;
					if (rannum < nDeadRate)
						deadsign = 1;
				}
				else if (fishscore > 138)
				{
					nDeadRate = (50000 + 20000 * (bulletpower - 1)) / fishscore;
					if (rannum < nDeadRate)
						deadsign = 1;
				}
				else if (fishscore > 48)
				{
					nDeadRate = (30000 + 20000 * (bulletpower - 1)) / fishscore;
					if (rannum < nDeadRate)
						deadsign = 1;
				}
				else if (fishscore > 10)
				{
					nDeadRate = (15000 + 10000 * (bulletpower - 1)) / fishscore;
					if (rannum < nDeadRate)
						deadsign = 1;
				}
				else
				{
					nDeadRate = 10000 * bulletpower / fishscore;
					if (rannum < nDeadRate)
						deadsign = 1;
				}
			}
			else if (playerinfo[JXCTRL_NOWOUT] + tempout + 100 * nowgun < playerinfo[JXCTRL_NOWIN] * (20 * playerinfo[JXCTRL_AIMOUT] / playerinfo[JXCTRL_AIMIN]) / 20)
			{
				if (fishscore > 248)
				{
					nDeadRate = (100000 + 20000 * (bulletpower - 1)) / fishscore;
					if (rannum < nDeadRate)
						deadsign = 1;
				}
				else if (fishscore > 138)
				{
					nDeadRate = (50000 + 20000 * (bulletpower - 1)) / fishscore;
					if (rannum < nDeadRate)
						deadsign = 1;
				}
				else if (fishscore > 48)
				{
					nDeadRate = (30000 + 20000 * (bulletpower - 1)) / fishscore;
					if (rannum < nDeadRate)
						deadsign = 1;
				}
				else if (fishscore > 10)
				{
					nDeadRate = (15000 + 10000 * (bulletpower - 1)) / fishscore;
					if (rannum < nDeadRate)
						deadsign = 1;
				}
				else
				{
					nDeadRate = 10000 * bulletpower / fishscore;
					if (rannum < nDeadRate)
						deadsign = 1;
				}
			}
			else if (playerinfo[JXCTRL_NOWOUT] + tempout < playerinfo[JXCTRL_AIMOUT] && (playerinfo[JXCTRL_AIMIN] - playerinfo[JXCTRL_NOWIN]) * 10 / (playerinfo[JXCTRL_AIMOUT] - playerinfo[JXCTRL_NOWOUT] - tempout) < 25)
			{
				nDeadRate = 10000 * bulletpower / fishscore;
				if (rannum < nDeadRate)
					deadsign = 1;
			}
		}
	}


	else
	{

		if (playerinfo[JXCTRL_NOWOUT] + tempout + playerinfo[JXCTRL_AIMIN] <= playerinfo[JXCTRL_AIMOUT] + playerinfo[JXCTRL_NOWIN])
		{
			if (fishscore > 248)
			{
				nDeadRate = (100000 + 20000 * (bulletpower-1)) / fishscore;
				if (rannum < nDeadRate)
					deadsign = 1;
			}
			else if (fishscore > 138)
			{
				nDeadRate = (50000 + 20000 * (bulletpower - 1)) / fishscore;
				if (rannum < nDeadRate)
					deadsign = 1;
			}
			else if (fishscore > 48)
			{
				nDeadRate = (30000 + 20000 * (bulletpower - 1)) / fishscore;
				if (rannum < nDeadRate)
					deadsign = 1;
			}
			else if (fishscore > 10)
			{
				nDeadRate = (15000 + 20000 * (bulletpower - 1)) / fishscore;
				if (rannum < nDeadRate)
					deadsign = 1;
			}
			else
			{
				nDeadRate = 11000 * bulletpower / fishscore;
				if (rannum < nDeadRate)
					deadsign = 1;
			}
		}
	}
	playerinfo[JXCTRL_DEAD_RATE] = nDeadRate;
	return deadsign;
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


int JXGetBigbombfishFourNum(int superbombfishtype)
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


int JXCheckFishDead(long long player_info[], int now_gun, int fish_score, int bullet_type, int superbombfishtype)
{
	int winflg = JXSetWinflg(player_info, now_gun,bullet_type);
	int win_rate[] = {
		1000 - JXSET_BETOUTRATE,
		JXSET_COID_LIMIT[5],
		JXSET_COID_LIMIT[3],
		JXSET_COID_LIMIT[1],
	};
	int ran = GetLongRanNum() % 1000000;
	int win_sign = 0;
	int nDeadRate = 0;
	player_info[JXCTRL_RAND_NUM] = ran;

	if (jx_set_sign != 36)
	{
		return -1;
	}

	if (fish_score <= 0 || fish_score > 20000)
	{
		return -1;
	}

	if (superbombfishtype > 0)
	{
		if (player_info[JXCTRL_ST] == 4)
		{
			win_rate[winflg] += 1000;
		}
		if (1 == bullet_type)
		{
			win_rate[winflg] *= 2;
		}
		if (2 == bullet_type)
		{
			win_rate[winflg] *= 4;
		}
		if (player_info[JXCTRL_FDCOIN] >= now_gun)
		{
			player_info[JXCTRL_FDCOIN] -= now_gun;
			player_info[JXCTRL_BASECOIN] -= now_gun;
			player_info[JXCTRL_REALBASECOIN] -= now_gun;
			win_rate[winflg] += 1000;
		}
		
		nDeadRate = (win_rate[winflg] * 1000) / fish_score;
		player_info[JXCTRL_DEAD_RATE] = nDeadRate;
		player_info[JXCTRL_RATE_RANGE] = win_rate[winflg];
		/*
		FILE *pFile = fopen("../logs/ctrl.log", "a+");
		if (pFile)
		{
			fprintf(pFile, "win flag:%d, win rate:%d, rand:%d, fish_score:%d, rand num:%d state:%d \n", winflg, win_rate[winflg], ran, fish_score, player_info[JXCTRL_DEAD_RATE], player_info[JXCTRL_ST]);	
			fclose(pFile);		
		}*/

		if (ran < nDeadRate)
		{
			win_sign = 1;
		}
		player_info[JXCTRL_BASECOIN] -= now_gun;
		player_info[JXCTRL_NOWBASECOIN] -= now_gun;
		player_info[JXCTRL_REALBASECOIN] -= now_gun;

		if (1 == bullet_type)
		{
			player_info[JXCTRL_BASECOIN] -= now_gun;
			player_info[JXCTRL_NOWBASECOIN] -= now_gun;
			player_info[JXCTRL_REALBASECOIN] -= now_gun;
		}
		if (2 == bullet_type)
		{
			player_info[JXCTRL_BASECOIN] -= 3 * now_gun;
			player_info[JXCTRL_NOWBASECOIN] -= 3 * now_gun;
			player_info[JXCTRL_REALBASECOIN] -= 3 * now_gun;
		}


		if (win_sign > 0)
		{
			player_info[JXCTRL_BASECOIN] += fish_score * now_gun;
			player_info[JXCTRL_REALBASECOIN] += fish_score * now_gun;
			player_info[JXCTRL_BETOUTTAIL] += fish_score * now_gun;
			player_info[JXCTRL_NOWBASECOIN] += fish_score * now_gun;

			player_info[JXCTRL_BASECOIN] += player_info[JXCTRL_BETOUTTAIL] / 1000 * JXSET_BETOUTRATE;
			player_info[JXCTRL_BETOUTTAIL] = player_info[JXCTRL_BETOUTTAIL] % 1000;
		}


		if (win_sign > 0 && superbombfishtype > 0)
		{
			return JXGetBigbombfishFourNum(superbombfishtype);
		}

		return win_sign;
	}
	
	if (fish_score >= 50 && player_info[JXCTRL_ST] == 4)
	{
		win_rate[winflg] += 500;
		if (fish_score > 248)
		{
			win_rate[winflg] += 700;
		}
	}
	if (1 == bullet_type)
	{
		win_rate[winflg] *= 2;
	}
	if (2 == bullet_type)
	{
		win_rate[winflg] *= 4;
	}

	if (player_info[JXCTRL_ST] == 2 || player_info[JXCTRL_ST] == 3)
	{
		win_sign = JXRealCheckDead(player_info, now_gun, fish_score, bullet_type);
	}
	else
	{
		if (player_info[JXCTRL_FDCOIN] >= now_gun)
		{
			player_info[JXCTRL_FDCOIN] -= now_gun;
			player_info[JXCTRL_BASECOIN] -= now_gun;
			player_info[JXCTRL_REALBASECOIN] -= now_gun;
			win_rate[winflg] += 1000;
		}
		nDeadRate = (win_rate[winflg] * 1000) / fish_score;
		player_info[JXCTRL_DEAD_RATE] = nDeadRate;
		player_info[JXCTRL_RATE_RANGE] = win_rate[winflg];
		if (ran < nDeadRate)
		{
			win_sign = 1;
		}
	}

	player_info[JXCTRL_BASECOIN] -= now_gun;
	player_info[JXCTRL_NOWIN] += now_gun;
	player_info[JXCTRL_NOWBASECOIN] -= now_gun;
	player_info[JXCTRL_REALBASECOIN] -= now_gun;

	if (1 == bullet_type)
	{
		player_info[JXCTRL_BASECOIN] -= now_gun;
		player_info[JXCTRL_NOWIN] += now_gun;
		player_info[JXCTRL_NOWBASECOIN] -= now_gun;
		player_info[JXCTRL_REALBASECOIN] -= now_gun;
	}
	if (2 == bullet_type)
	{
		player_info[JXCTRL_BASECOIN] -= 3 * now_gun;
		player_info[JXCTRL_NOWIN] += 3 * now_gun;
		player_info[JXCTRL_NOWBASECOIN] -= 3 * now_gun;
		player_info[JXCTRL_REALBASECOIN] -= 3 * now_gun;
	}


	if (win_sign > 0)
	{
		player_info[JXCTRL_NOWOUT] += fish_score * now_gun;
		player_info[JXCTRL_BASECOIN] += fish_score * now_gun;
		player_info[JXCTRL_REALBASECOIN] += fish_score * now_gun;
		player_info[JXCTRL_BETOUTTAIL] += fish_score * now_gun;
		player_info[JXCTRL_NOWBASECOIN] += fish_score * now_gun;

		player_info[JXCTRL_BASECOIN] += player_info[JXCTRL_BETOUTTAIL] / 1000 * JXSET_BETOUTRATE;
		player_info[JXCTRL_BETOUTTAIL] = player_info[JXCTRL_BETOUTTAIL] % 1000;
	}

	if (win_sign>0 && superbombfishtype > 0)
	{
		return JXGetBigbombfishFourNum(superbombfishtype);
	}

	return win_sign;
}

//对玩家没有作用到鱼身上的子弹进行处理
void JXSetBetin(long long player_info[], int betin_coin)
{
	if (player_info[JXCTRL_INITSIGN] != 0xcfde)
	{
		int i;
		for (i = 0; i < 20; i++)
		{
			player_info[i] = 0;
		}
		player_info[JXCTRL_INITSIGN] = 0xcfde;
		player_info[JXCTRL_MAXGUN] = 3000;
		player_info[JXCTRL_NORMALTIMES] = 1688;
	}
	player_info[JXCTRL_FDCOIN] += betin_coin;
}