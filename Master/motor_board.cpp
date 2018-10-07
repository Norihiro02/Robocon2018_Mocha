/*
    @program 射出関係のモータ制御を行うプログラム
    @date 2018/08/20
    @author Niki Ryota
*/

#include "sub.h"

/************************************
   Linkage
************************************/

extern bool isPlaying;
extern bool SWANGLE, SWREVOLVER, SWUP, SWDOWN;
extern uint8_t *pRoute;
extern int gyroOffset;


/************************************
   Variable
************************************/

bool isAngleOffsetted, isTurnOffsetted; //角度とリボルバーのオフセット
bool isPush, isPull;                    //送り出し
bool isAngleSetted;                     //目標の角度になっているか
bool isPushUp;
bool isRevolverSetted;

long spinCounter;     //目標の回転数になったら増加するカウンター
uint8_t launchState = 0; //射出機構の状態 [0:待機/1:射出/2:射出完了]
uint8_t maxonSpeed = 0;  //maxonの回転速度[rps]
uint8_t refSpeed[5] = {50, 48, 46, 46, 59};                  //マクソン回転速度
long refAngle[5] = { -66000, -72000, -72000, -75000, -72000}; //  移動大：移動中：移動小：二段下：二段上

long shotAngle = 0; //射出の角度
long turnValue = 0; //次のマガジンまでの回転数

/************************************/

//12Vモタド，Maxon
void motorBoard()
{

  // リボルバーと角度のオフセットが取れた場合
  if (isTurnOffsetted && isAngleOffsetted)
  {

    //パラメータの設定
    switch (*pRoute)
    {
      //二段下
      case 7:
        maxonSpeed = refSpeed[3];
        shotAngle = refAngle[3];
        break;
      //二段上
      case 10:
        maxonSpeed = refSpeed[4];
        shotAngle = refAngle[4];
        break;
      //移動小
      case 5:
        maxonSpeed = refSpeed[2];
        shotAngle = refAngle[2];
        break;
      //移動中
      case 3:
        maxonSpeed = refSpeed[1];
        shotAngle = refAngle[1];
        break;
      //移動大
      case 1:
        maxonSpeed = refSpeed[0];
        shotAngle = refAngle[0];
        break;
      //スタートゾーンに戻る
      case 19:
        maxonSpeed = 0;
        shotAngle = 0;
        break;
    }

    switch (launchState)
    {

      // 待機
      case 0:

        if (!isPlaying)
          maxonSpeed = 0;

        i2cMotorBoard.setMotorData(4, 0); //巻き上げモータ停止

        if (SWDOWN) {
          i2cMotorBoard.setMotorData(4, 0);
        }
        else
          i2cMotorBoard.setMotorData(4, 240);

        break;

      // 射出
      case 1:

        // 押し出し限界
        if (SWUP)
        {
          isPushUp = true;
          launchState = 2;

          break;
        }
        //目標値との誤差が2rps未満になったらカウンターを増やす
        else if (abs(maxonSpeed - encLaunch.getRotateSpeed(1)) < 2)
          spinCounter++;
        else if (abs(maxonSpeed - encLaunch.getRotateSpeed(1)) >= 2)
          spinCounter = 0;

        //カウンターが100以上溜まり，角度調節が完了したらペットボトルを送る
        if (spinCounter >= 4 && isAngleSetted && isRevolverSetted && *pRoute != 10)
          i2cMotorBoard.setMotorData(4, -240);
        else if (spinCounter >= 80 && isAngleSetted && isRevolverSetted)
          i2cMotorBoard.setMotorData(4, -240);

        break;

      // 射出完了，マガジンを進める
      case 2:

        // 送りが元の位置に戻った場合
        if (SWDOWN)
        {
          turnValue -= 683; //目標のマガジンの位置に設定
          spinCounter = 0;
          launchState = 0;  //待機状態に移行
          break;
        }
        i2cMotorBoard.setMotorData(4, 240);
        break;
    }

    // 角度調整PID
    i2cMotorBoard.setMotorData(2, getProPower(0.35, encLaunch.getCount(3), shotAngle, 255));

    // 目標値と現在の角度の差が1000未満になったらフラグを立てる
    isAngleSetted = abs(encLaunch.getCount(3) - shotAngle) < 700 ? true : false;

    // リボルバーのモータのPID
    i2cMotorBoard.setMotorData(3, getProPower(1.30, encLaunch.getCount(4), turnValue, 100));

    if (abs(encLaunch.getCount(4) - turnValue) < 40)
      isRevolverSetted = true;
    else
      isRevolverSetted = false;


    //ローラー回転速度PID
    maxonSpeedPid.update(encLaunch.getRotateSpeed(1), maxonSpeed);
    i2cMotorBoard.setMotorData(1, (int)maxonSpeedPid.getPower());

  }
  //オフセットが取れていない状態
  else
  {

    gyroOffset = setGyroOffset();

    //マクソン停止
    i2cMotorBoard.setMotorData(1, 0);

    //角度のオフセットをとる
    if (!isAngleOffsetted)
    {
      //オフセットが取れた場合
      if (SWANGLE)
      {
        //モータを停止し，オフセットを取得
        i2cMotorBoard.setMotorData(2, 0);
        encLaunch.resetCount(3);
        shotAngle = 0;
        isAngleOffsetted = true;
      }
      else
      {
        //オフセットが取れるまでモータを回し，リミットスイッチが感知するまでフラグを降ろす
        i2cMotorBoard.setMotorData(2, 150);
        isAngleOffsetted = false;
      }
    }

    //リボルバーのオフセットをとる
    if (!isTurnOffsetted)
    {
      //オフセットが取れた場合
      if (SWREVOLVER)
      {
        // モータを停止し，オフセットを取得
        i2cMotorBoard.setMotorData(3, 0);
        encLaunch.resetCount(4);
        turnValue = -683;
        isTurnOffsetted = true;
      }
      else
      {
        //オフセットが取れるまでモータを回し，リミットスイッチが感知するまでフラグを降ろす
        i2cMotorBoard.setMotorData(3, -75);
        isTurnOffsetted = false;
      }
    }
  }
}
