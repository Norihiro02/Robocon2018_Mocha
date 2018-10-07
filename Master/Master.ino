/*
    @program メインスケッチ
    @author Niki Ryota,Watanabe Rui
*/

#include <Wire.h>
#include "sub.h"

/************************************
   I2C Address
************************************/

const uint8_t DISPLAY_ADDR     = 0x71;    //7セグ
const uint8_t ENC_OMNI         =    2;    //地面読み取りエンコーダ
const uint8_t ENC_LAUNCH       =   30;    //発射台まわりのエンコーダ
const uint8_t LIMITSW_ADDR     =   11;    //リミットスイッチ基板
const uint8_t DS3_ADDR         =    9;    //コントローラ
const uint8_t OMNIBOARD_ADDR   =   10;    //足回り
const uint8_t MOTORBOARD_ADDR  =   12;    //12Vモータ，マクソン
const uint8_t SWBOARD_ADDR     =   13;    //7セグ基板
const uint8_t DECORATION_ADDR  =    7;    //装飾

/************************************
   Instance
************************************/

ControllerManagerDS3 DS3(DS3_ADDR);
LoopCycleController loopPeriodUs(15000);

I2CMasterWriter i2c7LED(DISPLAY_ADDR, 4), i2cOmni(OMNIBOARD_ADDR, 5), i2cMotorBoard(MOTORBOARD_ADDR, 5), i2cDecoration(DECORATION_ADDR, 1);
I2CMasterReader limitSW(LIMITSW_ADDR, 2);
SwBoard i2cSW(SWBOARD_ADDR);

EncoderBoardSpeedMbed encLaunch(ENC_LAUNCH, 1000, 80); //(ADDR,解像度,微分時間);
EncoderBoardMbed enc(ENC_OMNI);

PidController yomniPid(0.1567, 0, 0.055, loopPeriodUs.getCycleUs(), 300.0);
PidController xomniPid(0.1567, 0, 0.055, loopPeriodUs.getCycleUs(), 300.0);

PidController gyroPid(3.5, 0, 0.087, loopPeriodUs.getCycleUs(), 50.0);
PidController maxonSpeedPid(19, 10, 0.8, loopPeriodUs.getCycleUs(), 1000.0);   //マクソン回転速度PIDオブジェクト

// kp / ki / kd / ループ周期 / 積分量の最大値

/************************************
   Variable
************************************/

int gyroOffset;

// limit switch
bool SWFR, SWFL, SWBR, SWBL, SWANGLE, SWREVOLVER, SWUP, SWDOWN, SWSTART, SWMODE;

/************************************
   setup
************************************/

void setup()
{
  Wire.begin();
  Wire.setClock(400000UL); // use 400 kHz I2C
  Serial.begin(115200);

  encInit();       //エンコーダ初期化
  pidClearPower(); //PIDパラメータの初期化

  LED7SetUp(); //7セグ基板初期化

  i2cOmni.reset();
  i2cOmni.setData(5, 127);
}

/************************************
   loop
************************************/

void loop()
{
  switches();
  decoration();
  setLED();
  motorBoard();
  omni();

  allUpdate();
}
