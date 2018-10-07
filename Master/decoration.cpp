/*
    @program 装飾用
    @date 2018/09/23
    @author Watanabe Rui
*/

#include<arduino.h>
#include"sub.h"

/************************************
   Linkage
************************************/

extern bool isLaunching, isPlaying;
extern bool isAngleOffsetted, isTurnOffsetted;
extern uint8_t launchState;

/************************************
   Variable
************************************/

static bool old, now, offsetted;

/************************************/

void decoration() {

  //選択ボタン
  if (i2cSW.getClickRUp() || i2cSW.getClickLUp() || i2cSW.getClickRDown() || i2cSW.getClickLDown() || i2cSW.getClickCUp() || i2cSW.getClickCDown())
    i2cDecoration.setBitData(1, 1, true);
  else
    i2cDecoration.setBitData(1, 1, false);

  //デリートボタン
  if (i2cSW.getClickDelete())
    i2cDecoration.setBitData(1, 2, true);
  else
    i2cDecoration.setBitData(1, 2, false);

  //エンターボタン
  i2cDecoration.setBitData(1, 3, false);

  //ゾーン切り替え
  if (i2cSW.getTogleSWChangeUp())
    i2cDecoration.setBitData(1, 4, true);
  else
    i2cDecoration.setBitData(1, 4, false);

  if (i2cSW.getTogleSWChangeDown())
    i2cDecoration.setBitData(1, 5, true);
  else
    i2cDecoration.setBitData(1, 5, false);

  //発射OK状態
  if (launchState == 1)
    i2cDecoration.setBitData(1, 6, true);
  else
    i2cDecoration.setBitData(1, 6, false);

  //オフセットが取れた時
  if (isAngleOffsetted || isTurnOffsetted)
    offsetted = true;

  now = offsetted;

  if (now && old)
    i2cDecoration.setBitData(1, 7, true);
  else
    i2cDecoration.setBitData(1, 7, false);

  old = now;

  i2cDecoration.setBitData(1, 8, isPlaying);
}
