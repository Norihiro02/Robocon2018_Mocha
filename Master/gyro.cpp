/*
    @program  ジャイロセンサの値を読み取るプログラム
    @date   2018/08/29
    @author Watanabe Rui
*/

#include <arduino.h>

/************************************
   Linkage
************************************/

extern int gyroOffset;

/************************************/

//ジャイロセンサからの値を取得
int getGyro()
{
  int yaw = 0;
  static int i = 0;
  if (Serial.available() >= 3)
  {
    if (Serial.read() == 'H')
    {
      yaw = (Serial.read() << 8) | Serial.read();
      yaw -= gyroOffset;
      if (yaw > 180)
        yaw -= 360;
      else if (yaw < -180)
        yaw += 360;
      i = yaw;
      return yaw;
    }
  }
  return i;
}

//初期位置を設定
int setGyroOffset()
{
  int yaw = 0;
  static int i = 0;
  if (Serial.available() >= 3)
  {
    if (Serial.read() == 'H')
    {
      yaw = (Serial.read() << 8) | Serial.read();
      i = yaw;
      return yaw;
    }
  }
  return i;
}
