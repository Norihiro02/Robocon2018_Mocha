/*
    @program 足回りプログラム
    @date   2018/08/29
    @author Watanabe Rui
*/

#include <arduino.h>
#include "sub.h"

const double PULSE_PER_MM = 27.16244362;

/************************************
   Linkage
************************************/

extern bool SWFR, SWFL, SWBR, SWBL, SWSTART;
extern bool isTurnOffsetted, isAngleOffsetted;
extern bool isStanding[5], bottle[6], isBottleSetted;
extern uint8_t launchState; //射出の状態
extern int gyroOffset;
extern bool isPushUp;

/************************************
   Variable
************************************/

bool isLaunching; //射出中かどうか
bool isPlaying, old, now; //スタートスイッチ

double wheelPower[2]; //PIDのx成分，y成分

static double nowX, nowYL, nowYR; //エンコーダ情報

uint8_t route[31] = {}; //移動ルート
uint8_t *pRoute = route; //routeのポインタ

uint8_t routeMat[6][5] = {
  0, 7,  8, 17,  0,  //下が乗ってない
  0, 7, 10, 17,  0,  //上が乗ってない
  0, 7,  8, 10, 17,  //上下乗ってない
  0, 5, 15,  6,  25,  //移動小
  0, 3, 13,  4,  23,  //移動中
  0, 1, 11,  2,  21   //移動大
};

//位置情報(x,y)[mm]
double locations[25][2] =
{
  5800,    0,  //移動大
  5800,    0,
  4880,    0,  //移動中
  4880,    0,
  3850,    0,  //移動小
  3850,    0,
  1820, 1450,  //二段
  1820,    0,
  -200,    0,
  1820,    0,  //10
  5800,    0,
  1820, 1400,
  4880,    0,
  1500, 1450,
  3850,    0,
  1900, 1450,
  2800,    -120,
  0,       0,
  0,       0,
  0,       0,  //20
  5800,    0,
  0,       0,
  4880,    0,
  0,       0,
  3850,    0
};

/************************************/

/*
    目標値によってpid出力値と角度を割り出す
    @param  x：現在値 x [mm]
    @param  y：現在値 y [mm]
    @param  maxPower：最大PWM値
    @param  power[]：x成分とy成分のPID出力値を格納する配列
*/

void calcWheelPower(double x, double y, int maxPower, double power[])
{
  static int counter = 0; //カウンター
  static double targetX, targetY;

  if (*pRoute == 12) {
    if (i2cSW.getBitData(2, 2))
      locations[9][0] = locations[6][0] - 240;
    else
      locations[9][0] = locations[6][0] + 200;
  }


  //目標値を設定
  if (*pRoute != 0)
  {
    targetX = locations[*pRoute - 1][0];
    targetY = locations[*pRoute - 1][1];
  }
  else {
    maxPower = 0;
    pRoute++;
  }

  if (*pRoute == 12)
    counter++;
  //移動
  else if ((abs(targetX - x) < 200 && abs(targetY - y) < 200) && *pRoute % 2 == 1)
    counter++;
  //土台に突撃した場合
  else if (*pRoute % 2 == 1 && (SWFR || SWFL))
    counter++;
  //テーブルの方へ進んでいる場合
  else if ((*pRoute % 2 == 0 && *pRoute <= 10) && (SWFR || SWFL) && !isLaunching)
  {
    launchState = 1;
    isLaunching = true;
  }
  else if ((*pRoute != 8 && *pRoute != 10) && isPushUp && isLaunching)
    counter++;
  else if (launchState == 0 && isLaunching)
    counter++;
  else
    counter = 0;

  //全て終わったら
  if (pRoute == &route[31])
  {
    launchState = 0;
    isPlaying = false;

    locations[0][1] = 0;
    locations[2][1] = 0;
    locations[4][1] = 0;
    locations[9][0] = 1820;
    locations[16][0] = 2800;
    locations[16][1] = -150;

    counter = 0;

    for (int i = 0; i < 5; i++)
    {
      bottle[i] = 0;
      isStanding[i] = false;
    }

    isBottleSetted = false;

    pRoute = &route[0];

    allStop();
  }

  // カウンターを回す
  if (*pRoute % 2 == 0 && *pRoute <= 10) {
    if (counter > 1)
    {
      isPushUp = false;
      pRoute++; //ポインタを一つ進める
      isLaunching = false;
      counter = 0;
    }
  }
  else if (*pRoute % 2 == 1)
  {
    if (counter > 13)
    {
      pRoute++; //ポインタを一つ進める
      isLaunching = false;
      counter = 0;
    }
  }
  else if (*pRoute == 12) {
    if (counter > 40)
    {
      pRoute++; //ポインタを一つ進める
      isLaunching = false;
      counter = 0;
    }
  }

  //壁へ戻る
  if (*pRoute == 21 || *pRoute == 23 || *pRoute == 25 || *pRoute == 19 || *pRoute == 17 || *pRoute == 20)
  {
    maxPower = 75;
    if (SWBL || SWBR)
      pRoute++;
  }

  if (*pRoute == 9)
    maxPower = 160;

  if (*pRoute == 9 && !isStanding[0] && !isStanding[1] && !isStanding[2])
    maxPower = 60;


  /************************************
                PID
  ************************************/
  xomniPid.update(targetX - x, 0, maxPower);
  yomniPid.update(targetY - y, 0, maxPower);

  power[0] = -xomniPid.getPower();
  power[1] = -yomniPid.getPower();

  /************************************/

  //壁へ前進する
  if (*pRoute != 0 && *pRoute % 2 == 0 && *pRoute <= 10)
    power[1] = 34;

  //最後の移動テーブルを終えた後，壁に一回ぶつけてYのオフセットを取り直す
  if (*pRoute == 20)
    power[1] = -40;
}

// 足回りモータ駆動用
void omni()
{
  // エンコーダのパルスをmmに変換
  nowX = (double)(-enc.getCount(1)) / PULSE_PER_MM;
  nowYL = (double)(enc.getCount(2)) / PULSE_PER_MM;
  nowYR = (double)(-enc.getCount(3)) / PULSE_PER_MM;

  //スタートスイッチ
  old = SWSTART;
  if (now == true && old == false) {
    isPlaying = true;
    pRoute = route;
  }
  now = old;

  //後ろ側のリミットスイッチが押されたときyを0にリセット
  if (SWBR && SWBL)
  {
    enc.resetCount(2);
    enc.resetCount(3);

    gyroOffset = setGyroOffset();
  }

  //前側のリミットスイッチが押されたときジャイロをリセット
  if (SWFR && SWFL)
    gyroOffset = setGyroOffset();

  if (i2cSW.getClickEnter()) {
    enc.resetCount(1);
    enc.resetCount(2);
    enc.resetCount(3);
    gyroOffset = setGyroOffset();
  }

  //初期化
  if (i2cSW.getClickDelete() || DS3.getTriangleClick())
  {
    pRoute = route;
    isTurnOffsetted = false;
    isPlaying = false;

    gyroOffset = setGyroOffset();
  }

  gyroPid.update(getGyro(), 0);

  //自動モード
  if (isPlaying)
  {
    calcWheelPower(nowX, nowYR, 130, wheelPower);

    i2cOmni.setData(1, (int)wheelPower[0] >> 8 & 0x00ff);
    i2cOmni.setData(2, (int)wheelPower[0] & 0x00ff);
    i2cOmni.setData(3, (int)wheelPower[1] >> 8 & 0x00ff);
    i2cOmni.setData(4, (int)wheelPower[1] & 0x00ff);

    i2cOmni.setData(5, map(gyroPid.getPower(), -255, 255, 0, 255));
  }
  else
  {
    i2cOmni.setData(1, 0);
    i2cOmni.setData(2, 0);
    i2cOmni.setData(3, 0);
    i2cOmni.setData(4, 0);
    i2cOmni.setData(5, 127);
  }
}
