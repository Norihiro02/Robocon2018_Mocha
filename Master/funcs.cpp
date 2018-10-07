/*
    @program 関数まとめ
    @date 2018/09/04
    @author Watanabe Rui
*/

#include <arduino.h>
#include "sub.h"

/************************************
   Linkage
************************************/

extern bool SWFR, SWFL, SWBR, SWBL, SWANGLE, SWREVOLVER, SWUP, SWDOWN, SWSTART, SWMODE;
extern uint8_t routeMat[6][5];

/************************************/

//全PIDの初期化
void pidClearPower()
{
  xomniPid.clearPower();
  yomniPid.clearPower();
  gyroPid.clearPower();
  maxonSpeedPid.clearPower();
}

//全エンコーダ回転数の初期化
void encInit()
{
  enc.init();
  encLaunch.init();
}

//リミットスイッチ
void switches()
{
  SWFR = limitSW.getBitData(2, 1);
  SWFL = limitSW.getBitData(1, 4);
  SWBR = limitSW.getBitData(1, 1);
  SWBL = limitSW.getBitData(1, 3);

  SWANGLE = limitSW.getBitData(2, 4);
  SWREVOLVER = limitSW.getBitData(1, 2);
  SWUP = limitSW.getBitData(2, 7);
  SWDOWN = limitSW.getBitData(1, 7);
  SWSTART = limitSW.getBitData(2, 3);

  SWMODE = i2cSW.getBitData(2, 1);
}

/*
  P制御
  @param kp 比例係数
  @param controlValue 制御値
  @param targetValue 目標値
  @param maxPower 最大値

  @return PWM値(-255~255)
*/
double getProPower(double kp, long controlValue, long targetValue, int16_t maxPower)
{
  double proPower = kp * (targetValue - controlValue);

  proPower = constrain(proPower, -maxPower, maxPower);

  return proPower;
}
