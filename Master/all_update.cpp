/*
    @program アップデート関数
    @date 2018/08/30
    @author Niki Ryota
*/

#include "sub.h"

//全ての情報を更新
void allUpdate()
{
  enc.update();
  encLaunch.update();

  DS3.update();

  limitSW.update();

  i2cSW.update();
  i2cOmni.update();
  i2cMotorBoard.update();
  i2cDecoration.update();

  setGyroOffset();

  loopPeriodUs.update();
}
