#include <time.h>
#include <stdlib.h>
#include "wctrlbrniu.h"
enum
{	//WC  WIN CTRL
	NIUCTRL_INITSIGN,			//初始化标志
	//NIUCTRL_PLAYTIMES,		//游戏次数
	NIUCTRL_BETOUTTAIL,
	NIUCTRL_BASECOIN,		   //净分
	NIUCTRL_MAINRANGE,
	NIUCTRL_NOWRANGE,

	NIUCTRL_BASECOIN2,

	NIUCTRL_ST,
	NIUCTRL_NOWBASECOIN,
	NIUCTRL_EVMAXBET,

	NIUCTRL_BASECOIN1,

	NIUCTRL_BONUSBASE,
	NIUCTRL_BONUSGET,
};


static int NIUCHOICENUM = 5;
static int NIUGAMEWINRATE = 0;
static int REDBLACKBONUSRATE = 0;
static int RBMAXGAMEWAVEUP = 100000000;
static int RBMAXGAMEWAVEDOWN = 300000000;

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
			if (rand() >= 1000000)
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

			if (times >= 100)
			{
				tempnum = tempnum % 30000;
				break;
			}
		}

	}

	return tempnum;
}

void NiuSetGameinfo(int setarray[])
{
	NIUGAMEWINRATE = setarray[BRNIU_SET_BETOUTRATE];
	RBMAXGAMEWAVEUP = setarray[BRNIU_SET_GAMEWAVEUP];
	RBMAXGAMEWAVEDOWN = setarray[BRNIU_SET_GAMEWAVEDOWN];
}

static int CheckBomb(int card[])
{
	int temp[13] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	int i;

	for (i = 0; i < 5;i++)
	{
		temp[(card[i] % 16) - 1]++;
	}

	for (i = 0; i < 13;i++)
	{
		if (temp[i] > 3)
		{
			return 1;
		}
	}

	return 0;
}

static int CheckJinNiu(int card[])
{
	int i;

	for (i = 0; i < 5;i++)
	{
		if (card[i]%16<11)
		{
			return 0;
		}
	}

	return 1;
}

static int CheckWuXiaoNiu(int card[])
{
	if (card[0] % 16 + card[1] % 16 + card[2] % 16 + card[3] % 16 + card[4] % 16 < 10)
		return 1;

	return 0;
}

//函数返回值，0～10，无牛到牛牛；11，炸弹；12，金牛；13，五小牛
static int CheckNiu(int card[])
{
	int array[50] = {
		0, 1, 2, 3, 4,
		0, 1, 3, 2, 4,
		0, 1, 4, 3, 2,
		0, 2, 3, 1, 4,
		0, 2, 4, 1, 3,

		0, 3, 4, 1, 2,
		1, 2, 3, 0, 4,
		1, 2, 4, 0, 3,
		1, 3, 4, 0, 2,
		2, 3, 4, 0, 1,
	};
	int i;
	int num1, num2, num3,num4,num5;
	int cardnum1, cardnum2, cardnum3,cardnum4,cardnum5;


	if (CheckWuXiaoNiu(card))
		return 13;
	if (CheckJinNiu(card))
		return 12;
	if (CheckBomb(card))
		return 11;

	for (i = 0; i < 10;i++)
	{
		num1 = array[5 * i];
		num2 = array[5 * i + 1];
		num3 = array[5 * i + 2];
		num4 = array[5 * i + 3];
		num5 = array[5 * i + 4];
		cardnum1 = card[num1] % 16;
		cardnum2 = card[num2] % 16;
		cardnum3 = card[num3] % 16;
		if (cardnum1 > 10)
			cardnum1 = 10;
		if (cardnum2 > 10)
			cardnum2 = 10;
		if (cardnum3 > 10)
			cardnum3 = 10;
		if ((cardnum1 + cardnum2 + cardnum3 ) % 10 == 0)
		{
			cardnum4 = card[num4] % 16;
			cardnum5 = card[num5] % 16;
			if (cardnum4 > 10)
				cardnum4 = 10;
			if (cardnum5 > 10)
				cardnum5 = 10;
			if ((cardnum4 + cardnum5) % 10 == 0)
			{
				return 10;
			}

			return (cardnum4 + cardnum5) % 10;
		}
	}

	return 0;
}

static int NiuGetMaxcard(int card[])
{
	int i;
	int cardresult = 0;
	int tempcard;

	for (i = 0; i < 5;i++)
	{
		tempcard = ((card[i] % 16) * 16) + card[i] / 16;
		if (tempcard > cardresult)
		{
			cardresult = tempcard;
		}
	}

	return cardresult;
}

static int NiuGetrate(int cardtype)
{
	if (cardtype < 7)
	{
		return 1;
	}
	else if (cardtype < 10)
	{
		return 2;
	}
	else if (cardtype == 10)
	{
		return 3;
	}
	else if (cardtype == 11 || cardtype == 12 || cardtype == 13)
	{
		return 5;
	}
	else
		return 1;
}

//闲家角度的输赢
static void NiuGetPaybackrate(int card[], int paybackrate[],int gameresult[])
{
	int cardtype[5];
	int i;

	for (i = 0; i < 5; i++)
	{
		cardtype[i] = CheckNiu(&card[i * 5]);
		gameresult[i] = cardtype[i];
	}



	for (i = 0; i < 4; i++)
	{
		if (cardtype[i+1] < cardtype[0])
		{
			paybackrate[i] = 0 - NiuGetrate(cardtype[0]);
		}
		else if (cardtype[i + 1] == cardtype[0])
		{
			int max1, max2;
			max1 = NiuGetMaxcard(&card[5 * (i + 1)]);
			max2 = NiuGetMaxcard(card);

			if (max2 > max1)
				paybackrate[i] = 0 - NiuGetrate(cardtype[0]);
			else
				paybackrate[i] = NiuGetrate(cardtype[i + 1]);
		}
		else
		{
			paybackrate[i] = NiuGetrate(cardtype[i+1]);
		}
	}
}

//玩家输赢
static long long NiuPlayerWinlose(long long bet[], int card[])
{
	int paybackrate[4];
	long long winresult = 0;
	long long winresult2 = 0;
	int i;
	int gameresult[5];

	NiuGetPaybackrate(card, paybackrate,gameresult);

	for (i = 0; i < 4;i++)
	{
		winresult += bet[i] * paybackrate[i];
		winresult2 += (bet[i] + bet[4 + i]) * paybackrate[i];
	}

	if (winresult2 + bet[8] < 0)
	{
		winresult2 = bet[8];
	}
	else if (winresult2 > bet[8])
	{
		winresult2 = 0 - bet[8];
	}
	else
		winresult2 = 0 - winresult2;

	return winresult2 + winresult;
	//if (winresult + winresult2 < 0)
	//{
	//	return -1;
	//}
	//else if (winresult + winresult2 > 0)
	//{
	//	return 1;
	//}
	//else
	//{
	//	return 0;
	//}
}

long long NiuPlayerWinloseNum(long long bet[], int paybackrate[])
{
	long long winresult = 0;
	long long winresult2 = 0;
	int i;

	for (i = 0; i < 4; i++)
	{
		winresult += bet[i] * paybackrate[i];
		winresult2 += (bet[i] + bet[4 + i]) * paybackrate[i];
	}

	if (winresult2 + bet[8] < 0)
	{
		winresult2 = bet[8];
	}
	else if (winresult2 > bet[8])
	{
		winresult2 = 0 - bet[8];
	}
	else
		winresult2 = 0 - winresult2;

	return winresult2 + winresult;
}

static void NiuDisOrderPlayercard(int card[])
{
	int i,j;
	int tempcard;
	int rand;

	for (i = 0; i < 5;i++)
	{
		rand = GetRanNum() % 5;
		if (rand != i)
		{
			for (j = 0; j < 5;j++)
			{
				tempcard = card[5 * i + j];
				card[5 * i + j] = card[5 * rand + j];
				card[5 * rand + j] = tempcard;
			}
		}
	}
}

static long long NiuGetBasecoin(long long basecoin1, long long basecoin2, long long basecoin3)
{
	if (basecoin1 == basecoin2 || basecoin1 == basecoin3)
	{
		return basecoin1;
	}

	if (basecoin2 == basecoin3)
	{
		return basecoin3;
	}

	return 0;
}

static void DisOrderTool(int arr[], int num)
{
	int i;
	int temp;
	int ran;

	for (i = 0; i < num; i++)
	{
		ran = GetRanNum() % num;
		temp = arr[i];
		arr[i] = arr[ran];
		arr[ran] = temp;
	}
}


static int IsBonuscardtype(int cardresult[])
{
	int i;
	int tempsign = 0;
	for (i = 0; i < 5; i++)
	{
		if (CheckWuXiaoNiu(&cardresult[5 * i]))
		{
			tempsign++;
		}
		else if (CheckJinNiu(&cardresult[5 * i]))
		{
			tempsign++;
		}
		else if (CheckBomb(&cardresult[5 * i]))
		{
			tempsign++;
		}
	}

	return tempsign;
}

static void CheckBonusCardtype(int cardresult[], long long bet[], long long playerinfo[])
{
	int card[] = {
		0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,	//方块A，方块2，...，方块K
		0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,  //梅花A，梅花2，...，梅花K
		0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d,  //红桃A，红桃2，...，红桃K
		0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d,  //黑桃A，黑桃2，...，黑桃K
	};
	int i;
	int tempsign = 0;
	long long maxbet = 0;
	long long allbet = 0;
	int num = 0;

	tempsign = IsBonuscardtype(cardresult);

	while (tempsign > 1 && num < 11)
	{
		num++;
		DisOrderTool(card, 52);
		for (i = 0; i < 25; i++)
		{
			cardresult[i] = card[i];
		}
		tempsign = IsBonuscardtype(cardresult);
	}
}
int BRNiuGameResult(long long bet[], long long playerinfo[], int cardresult[], int gameresult[],int paybackrate[])
{
	long long maxbet = 0;
	const int gamerange[8] = { 50, 20, 30, 100, 80, 40, 50, 60 };
	//const int subgamerange[8] = {240,80,160,480,320,160,240,180};
	int i;
	int winflg = 0;
	int temprand = 0;
	//int *playerinfo = redbluectrlinfo;
	int card[] = {
		0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
		0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
		0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d,
		0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d,

		//方块A...
		//梅花A...
		//红桃A...
		//黑桃A...
	};

	//计算平均最大押分
	for (i = 0; i < 4;i++)
	{
		if (bet[i] > maxbet)
		{
			maxbet = bet[i];
		}
	}




	//初始化
	if (playerinfo[NIUCTRL_INITSIGN] != 0xcfcf)
	{
		playerinfo[NIUCTRL_INITSIGN] = 0xcfcf;

		playerinfo[NIUCTRL_BASECOIN] = 0;
		playerinfo[NIUCTRL_BASECOIN2] = 0;
		playerinfo[NIUCTRL_BASECOIN1] = 0;
		playerinfo[NIUCTRL_ST] = 0;
		playerinfo[NIUCTRL_MAINRANGE] = gamerange[GetRanNum() % 8] / 4;
		playerinfo[NIUCTRL_NOWRANGE] = 20 + 10 * (1 + GetRanNum() % 4);

		playerinfo[NIUCTRL_NOWBASECOIN] = 0;
		playerinfo[NIUCTRL_BETOUTTAIL] = 0;
		playerinfo[NIUCTRL_EVMAXBET] = maxbet;
		playerinfo[NIUCTRL_BONUSBASE] = (4 + GetRanNum() % 4) * 10000000;
		playerinfo[NIUCTRL_BONUSGET] = 0;
	}

	playerinfo[NIUCTRL_BASECOIN] = NiuGetBasecoin(playerinfo[NIUCTRL_BASECOIN], playerinfo[NIUCTRL_BASECOIN1], playerinfo[NIUCTRL_BASECOIN2]);

	if (maxbet > 0)
		playerinfo[NIUCTRL_EVMAXBET] = (playerinfo[NIUCTRL_EVMAXBET] * 9 + maxbet) / 10;

	if (playerinfo[NIUCTRL_EVMAXBET] < 500000)
	{
		playerinfo[NIUCTRL_EVMAXBET] = 500000;
	}


	if (playerinfo[NIUCTRL_BONUSBASE] > 1000000000 && playerinfo[NIUCTRL_BONUSGET] > 1000000000)
	{
		playerinfo[NIUCTRL_BONUSBASE] -= 100000000;
		playerinfo[NIUCTRL_BONUSGET] -= 100000000;
	}



	if (playerinfo[NIUCTRL_ST] == 0)
	{
		//去送分
		if (playerinfo[NIUCTRL_BASECOIN] + RBMAXGAMEWAVEDOWN < 0 || playerinfo[NIUCTRL_BASECOIN] + playerinfo[NIUCTRL_MAINRANGE] * playerinfo[NIUCTRL_EVMAXBET] < 0)
		{
			playerinfo[NIUCTRL_ST] = 1;
			playerinfo[NIUCTRL_NOWBASECOIN] = 0;
			playerinfo[NIUCTRL_NOWRANGE] = playerinfo[NIUCTRL_MAINRANGE] / 3 * (1 + GetRanNum() % 4);
			winflg = 1;

		}
		//去杀分
		else if (playerinfo[NIUCTRL_BASECOIN] > RBMAXGAMEWAVEUP || playerinfo[NIUCTRL_BASECOIN] > playerinfo[NIUCTRL_MAINRANGE] * playerinfo[NIUCTRL_EVMAXBET] / 2)
		{
			playerinfo[NIUCTRL_ST] = 2;
			playerinfo[NIUCTRL_NOWBASECOIN] = 0;
			playerinfo[NIUCTRL_NOWRANGE] = playerinfo[NIUCTRL_MAINRANGE] / 3 * (1 + GetRanNum() % 6);
			winflg = 2;
		}
		else
		{
			temprand = GetRanNum();
			if (temprand % 800 < 10)
			{
				if (temprand % 800 < 5)
				{
					playerinfo[NIUCTRL_ST] = 2;
					playerinfo[NIUCTRL_NOWBASECOIN] = 0;
					playerinfo[NIUCTRL_NOWRANGE] = playerinfo[NIUCTRL_MAINRANGE] / 6 * (1 + GetRanNum() % 6);
					winflg = 2;
				}
				else
				{
					playerinfo[NIUCTRL_ST] = 1;
					playerinfo[NIUCTRL_NOWBASECOIN] = 0;
					playerinfo[NIUCTRL_NOWRANGE] = playerinfo[NIUCTRL_MAINRANGE] / 6 * (1 + GetRanNum() % 4);
					winflg = 1;
				}
			}
			else
			{
				winflg = 0;
			}
		}
	}
	else if (playerinfo[NIUCTRL_ST] == 1)
	{
		if (playerinfo[NIUCTRL_BASECOIN] > RBMAXGAMEWAVEUP || (playerinfo[NIUCTRL_NOWBASECOIN] > playerinfo[NIUCTRL_NOWRANGE] * playerinfo[NIUCTRL_EVMAXBET]) || (playerinfo[NIUCTRL_BASECOIN] > playerinfo[NIUCTRL_MAINRANGE] * playerinfo[NIUCTRL_EVMAXBET] / 2) || (playerinfo[NIUCTRL_BASECOIN] > 0 && GetRanNum() % 36 == 1))
		{
			playerinfo[NIUCTRL_ST] = 0;
			playerinfo[NIUCTRL_NOWBASECOIN] = 0;
			playerinfo[NIUCTRL_MAINRANGE] = gamerange[GetRanNum() % 8];
			winflg = 0;
		}
		else
		{
			winflg = 1;
		}
	}
	else
	{
		if (playerinfo[NIUCTRL_BASECOIN] + RBMAXGAMEWAVEDOWN < 0 || (playerinfo[NIUCTRL_NOWBASECOIN] + playerinfo[NIUCTRL_NOWRANGE] * playerinfo[NIUCTRL_EVMAXBET] < 0) || (playerinfo[NIUCTRL_BASECOIN] + playerinfo[NIUCTRL_MAINRANGE] * playerinfo[NIUCTRL_EVMAXBET] * 2 / 3 < 0))
		{
			playerinfo[NIUCTRL_ST] = 0;
			playerinfo[NIUCTRL_NOWBASECOIN] = 0;
			playerinfo[NIUCTRL_MAINRANGE] = gamerange[GetRanNum() % 8];
			winflg = 0;
		}
		else
		{
			winflg = 2;
		}
	}

	DisOrderTool(card, 52);
	for (i = 0; i < 25; i++)
	{
		cardresult[i] = card[i];
	}

	//CheckBonusCardtype(cardresult, bet, playerinfo);

	//SetPlayerCard(playerinfo, bet, cardresult);
	NiuGetPaybackrate(cardresult, paybackrate, gameresult);

	//NiuSetinout(playerinfo, bet, cardresult,gameresult,paybackrate);

	playerinfo[NIUCTRL_BASECOIN2] = playerinfo[NIUCTRL_BASECOIN];
	playerinfo[NIUCTRL_BASECOIN1] = playerinfo[NIUCTRL_BASECOIN];
	return 1;
}

void BRResetNiuGameResult(long long playerinfo[], int cardresult[], int gameresult[], int paybackrate[],long long gamewin, bool reset)
{
	if (reset)
	{
		if (playerinfo[NIUCTRL_ST] == 1)
		{
			if (gamewin < 0)
			{
				if (playerinfo[NIUCTRL_BASECOIN] + RBMAXGAMEWAVEDOWN <= 0)
				{
					if (GetRanNum() % 2 == 1)
						NiuDisOrderPlayercard(cardresult);
				}
				else
				{
					if (GetRanNum() % 4 == 1)
						NiuDisOrderPlayercard(cardresult);
				}
			}
		}
		else if (playerinfo[NIUCTRL_ST] == 2)
		{
			if (gamewin > 0)
			{
				if (playerinfo[NIUCTRL_BASECOIN] >= RBMAXGAMEWAVEUP)
				{
					if (GetRanNum() % 2 == 1)
						NiuDisOrderPlayercard(cardresult);
				}
				else
				{
					if (GetRanNum() % 4 == 1)
						NiuDisOrderPlayercard(cardresult);
				}
			}
		}
	}
	NiuGetPaybackrate(cardresult, paybackrate, gameresult);
}

void BRBetOut(long long playerinfo[], long long gamewin, long long playerwin)
{
	playerinfo[NIUCTRL_BASECOIN] = NiuGetBasecoin(playerinfo[NIUCTRL_BASECOIN], playerinfo[NIUCTRL_BASECOIN1], playerinfo[NIUCTRL_BASECOIN2]);
	playerinfo[NIUCTRL_BASECOIN] += gamewin;
	playerinfo[NIUCTRL_NOWBASECOIN] += gamewin;

	playerinfo[NIUCTRL_BASECOIN] += (playerwin + playerinfo[NIUCTRL_BETOUTTAIL]) / 1000 * NIUGAMEWINRATE;
	playerinfo[NIUCTRL_BETOUTTAIL] = (playerwin + playerinfo[NIUCTRL_BETOUTTAIL]) % 1000;

	playerinfo[NIUCTRL_BASECOIN2] = playerinfo[NIUCTRL_BASECOIN];
	playerinfo[NIUCTRL_BASECOIN1] = playerinfo[NIUCTRL_BASECOIN];
}

int BRNiuGameResultTest(long long bet[], long long playerinfo[], int cardresult[], int gameresult[], int paybackrate[],int setcardtype[])
{
	int card[] = {
		0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
		0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
		0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d,
		0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d,

		//方块A...
		//梅花A...
		//红桃A...
		//黑桃A...
	};
	int i;
	//long long info[20];
	int carddemo[70]=
	{
		0x01, 0x17, 0x3a, 0x08, 0x26,   //无牛
		0x01, 0x17, 0x3a, 0x08, 0x25,   //牛一
		0x02, 0x17, 0x3a, 0x08, 0x25,   //牛二
		0x03, 0x17, 0x3a, 0x08, 0x25,   //牛三
		0x04, 0x17, 0x3a, 0x08, 0x25,   //牛四
		0x05, 0x17, 0x3a, 0x08, 0x25,   //牛五
		0x06, 0x17, 0x3a, 0x08, 0x25,   //牛六
		0x07, 0x17, 0x3a, 0x08, 0x25,   //牛七
		0x18, 0x17, 0x3a, 0x08, 0x25,   //牛八
		0x09, 0x17, 0x3a, 0x08, 0x25,   //牛九
		0x0a, 0x17, 0x3a, 0x08, 0x25,   //牛牛

		0x26, 0x16, 0x06, 0x36, 0x37,	 //炸弹
		0x2b, 0x1c, 0x0d, 0x3c, 0x3d,	 //金牛
		0x11, 0x12, 0x01, 0x31, 0x33,	 //五小牛
	};

	DisOrderTool(card, 52);
	for (i = 0; i < 25; i++)
	{
		cardresult[i] = card[i];
	}
	//cardresult[0] = 0x11;
	//cardresult[1] = 0x01;
	//cardresult[2] = 0x21;
	//cardresult[3] = 0x22;
	//cardresult[4] = 0x23;
	for (i = 0; i < 5;i++)
	{
		if (setcardtype[i]>=0)
		{
			int j;
			for (j = 0; j < 5;j++)
			{
				cardresult[5 * i + j] = carddemo[5 * setcardtype[i] + j];
			}
			
		}
	}

	NiuGetPaybackrate(cardresult, paybackrate, gameresult);

	return 1;
}