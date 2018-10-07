/*
    @program 7セグLEDを制御するプログラム
    @date 2018/08/30
    @author Niki Ryota
*/

#include <Wire.h>
#include "sub.h"

const int DISPLAY_ADDR = 0x71;

/************************************
   Linkage
************************************/

extern bool SWMODE;
extern double locations[25][2];
extern uint8_t routeMat[6][5];
extern uint8_t route[31];
extern uint8_t *pRoute;

/************************************
   Variable
************************************/
bool isConfirmed; //移動テーブルの位置を確定する
bool isBottleSetted;
bool isPointMode;

uint8_t sendData[2][4] = {
  {0, 3, 3, 3},
  {0, 0, 0, 0},
};

//大中小上下
bool isStanding[5] = {1, 1, 1, 1, 1};
bool bottle[6] = {1, 1, 1, 1, 1, 0};

/************************************/

void setRoute();

// 初期設定
void LED7SetUp()
{
  Wire.beginTransmission(DISPLAY_ADDR);
  Wire.write('v');
  Wire.write(0x7A); // Brightness control command
  Wire.write(100);  // Set brightness level: 0% to 100%
  Wire.write(0x77);
  Wire.write(0b00000001);
  Wire.endTransmission();
}

//ペットボトルが立っているかどうかの情報を表示する
void sendBottleDataToDisplay(bool *bottleData)
{
  Wire.beginTransmission(DISPLAY_ADDR);
  Wire.write(isBottleSetted);

  for (int i = 0; i < 3; i++)
  {
    Wire.write(0x7C + i);
    Wire.write(bottleData[i] | bottleData[i + 3] << 3);
  }
  Wire.endTransmission();
}

//移動テーブルの位置情報を表示する
void sendDataToDisplay()
{
  Wire.beginTransmission(DISPLAY_ADDR);
  Wire.write(0x77);
  Wire.write(0b00000001);
  Wire.endTransmission();
}

//モード切替の際にセグメントをリフレッシュ
void reflesh()
{
  Wire.beginTransmission(DISPLAY_ADDR);
  Wire.write('v');
  Wire.endTransmission();
}

//行を交換
void swap(int col1, int col2)
{
  int tmp;
  for (int i = 0; i < 5; i++)
  {
    tmp = routeMat[col1][i];
    routeMat[col1][i] = routeMat[col2][i];
    routeMat[col2][i] = tmp;
  }
}

//ペットボトルの状況に応じて移動する経路を変更する
void setRoute()
{
  //移動テーブル
  for (int i = 3; i <= 5; i++)
    routeMat[i][0] = isStanding[5 - i];

  //二段テーブル
  //上も下も乗ってない
  if (isStanding[3] && isStanding[4])
  {
    routeMat[0][0] = false;
    routeMat[1][0] = false;
    routeMat[2][0] = true;
  }
  //上が乗ってない
  else if (isStanding[3] && !isStanding[4])
  {
    routeMat[0][0] = false;
    routeMat[1][0] = true;
    routeMat[2][0] = false;
  }
  //下が乗ってない
  else if (!isStanding[3] && isStanding[4])
  {
    routeMat[0][0] = true;
    routeMat[1][0] = false;
    routeMat[2][0] = false;
  }
  //二段テーブルに行かない場合
  else
  {
    routeMat[0][0] = false;
    routeMat[1][0] = false;
    routeMat[2][0] = false;
  }

  //ルートをセット
  int k = 0;
  for (int i = 0; i < 6; i++)
  {
    for (int j = 1; j < 5; j++, k++)
    {
      if (routeMat[i][0])
        route[k] = routeMat[i][j];
      else
        route[k] = 0;
    }
  }

  route[28] = 19; //最後のテーブルが終わったとき壁に戻る
  route[29] = 20; //壁沿いに戻る
  route[30] = 9;  //原点に帰る

  //移動テーブルに行かない場合
  if (!isStanding[0] && !isStanding[1] && !isStanding[2])
  {
    for (int i = 0; i < 31; i++)
    {
      if (route[i] == 17)
        route[i] = 9;
    }
    route[28] = 0;
    route[29] = 0;
    route[30] = 0;
  }
}

//各移動テーブルの関係から座標を計算する
void calcLocations()
{

  //二段テーブルに行く場合
  if (isStanding[3] || isStanding[4])
  {
    //小テーブルに行き，4以上離れている場合
    if (isStanding[2] && sendData[0][3] >= 4)
    {
      //17番の座標を変更
      locations[16][0] = locations[4][0];
      locations[16][1] = locations[6][1];

      //3の座標を変更
      locations[4][1] = locations[6][1];
    }
    else if (isStanding[2] && sendData[0][3] > 1 && sendData[0][3] <= 4) {
      locations[16][0] = locations[4][0];
      locations[16][1] = sendData[0][3] * 300;

      locations[4][1] = sendData[0][3] * 300;
    }
    else if (isStanding[2] && sendData[0][3] <= 1)
    {
      if (i2cSW.getBitData(2, 2))
        locations[16][0] = -2800;
      else
        locations[16][0] = 2800;
    }
  }
  
  //小テーブルの距離が1よりも大きい
  if (sendData[0][3] > 1)
  {
    //小テーブル<よりも中テーブルのほうが遠い
    if (sendData[0][3] <= sendData[0][2])
    {
      locations[24][1] = locations[14][1] - 500;
      locations[2][1] = locations[14][1] - 500;
    }
    //小テーブルのほうが中テーブルより遠い
    else if (sendData[0][3] > sendData[0][2])
    {
      locations[24][1] = sendData[0][2] * 300;
      locations[2][1] = sendData[0][2] * 300;

      if (sendData[0][2] <= 1)
      {
        locations[24][1] = 0;
        locations[2][1] = 0;
      }
    }
  }

  //中テーブルの距離が1よりも大きい
  if (sendData[0][2] > 1)
  {
    //中テーブルよりも大テーブルのほうが遠い
    if (sendData[0][2] <= sendData[0][1])
    {
      locations[22][1] = locations[12][1] - 500;
      locations[0][1] = locations[12][1] - 500;
    }
    //中テーブルのほうが大テーブルよりも遠い
    else if (sendData[0][2] > sendData[0][1])
    {
      locations[22][1] = sendData[0][1] * 300;
      locations[0][1] = sendData[0][1] * 300;

      if (sendData[0][1] <= 1)
      {
        locations[22][1] = 0;
        locations[0][1] = 0;
      }
    }
  }

  if (!isStanding[2]) {
    if (!isStanding[1] && isStanding[0]) {
      locations[0][1] = 0;
      locations[20][1] = 0;
    }
    else if (isStanding[1] && !isStanding[0]) {
      locations[2][1] = 0;
      locations[22][1] = 0;
    }
  }


  //最後の移動テーブルが終わったら壁沿いに帰る
  if (isStanding[0])
  {
    locations[18][0] = locations[0][0];
    locations[19][0] = locations[0][0];
    locations[18][1] = 0;
  }
  else if (isStanding[1])
  {
    locations[18][0] = locations[2][0];
    locations[19][0] = locations[2][0];
    locations[18][1] = 0;
  }
  else if (isStanding[2])
  {
    locations[18][0] = locations[4][0];
    locations[19][0] = locations[4][0];
    locations[18][1] = 0;
  }
  else
    locations[18][0] = 0;
}

void pointMode() {
  route[0] = 7;
  route[1] = 10;
  route[2] = 12;
  route[3] = 10;
  route[4] = 9;
}

//7セグ
void setLED()
{

  if (i2cSW.getBitData(2, 2)) {
    if (locations[16][0] > 0) {
      locations[16][0] *= -1;
      locations[9][0] *= -1;
    }
  }


  //モードの切り替え
  switch (SWMODE)
  {

    /************************************
          Bottle data
        ************************************/
    case 0:
      reflesh();

      //決定
      if (i2cSW.getClickEnter() && !isBottleSetted)
      {
        for (int i = 0; i < 5; i++)
          isStanding[i] = bottle[i];

        //ボトル情報が正しく入力された場合
        for (int i = 0; i < 5; i++)
        {
          if (isStanding[i] != 0)
            isBottleSetted = true;
          else if (bottle[5])
            isBottleSetted = true;
        }
      }

      //リセット
      if (i2cSW.getClickDelete())
      {
        locations[0][1] = 0;
        locations[2][1] = 0;
        locations[4][1] = 0;
        locations[16][1] = -150;

        for (int i = 0; i < 5; i++)
        {
          bottle[i] = 0;
          isStanding[i] = false;
        }

        bottle[5] = false;

        for (int i = 0; i < 31; i++)
          route[i] = 0;

        isBottleSetted = false;
      }

      //ボトルのデータを入力
      if (i2cSW.getClickRUp())
        bottle[0] = !bottle[0];
      if (i2cSW.getClickLUp())
        bottle[2] = !bottle[2];
      if (i2cSW.getClickCUp())
        bottle[1] = !bottle[1];

      if (i2cSW.getClickRDown())
        bottle[3] = !bottle[3];
      if (i2cSW.getClickCDown())
        bottle[4] = !bottle[4];

      if (i2cSW.getClickLDown()) {
        bottle[5] = !bottle[5];
        for (int i = 0; i < 5; i++)
          bottle[i] = false;
      }

      sendBottleDataToDisplay(bottle);

      break;

    /************************************
          Movable table data
        ************************************/
    case 1:
      reflesh();

      //移動テーブル情報を入力
      if (i2cSW.getClickRUp() && sendData[0][1] < 5)
        sendData[0][1]++;
      if (i2cSW.getClickRDown() && sendData[0][1] > 1)
        sendData[0][1]--;

      if (i2cSW.getClickCUp() && sendData[0][2] < 5)
        sendData[0][2]++;
      if (i2cSW.getClickCDown() && sendData[0][2] > 1)
        sendData[0][2]--;

      if (i2cSW.getClickLUp() && sendData[0][3] < 5)
        sendData[0][3]++;
      if (i2cSW.getClickLDown() && sendData[0][3] > 1)
        sendData[0][3]--;

      //決定ボタン
      if (i2cSW.getClickEnter() && !isConfirmed)
      {
        //移動テーブルの距離を計算
        for (int i = 1; i <= 3; i++)
        {
          if (sendData[0][i] > 1)
            locations[10 + (2 * (i - 1))][1] = sendData[0][i] * 500;
          else
            locations[10 + (2 * (i - 1))][1] = 0;
        }

        isConfirmed = true;

        sendDataToDisplay();
      }

      //リセット
      if (i2cSW.getClickDelete())
      {
        for (int i = 1; i <= 3; i++)
          sendData[0][i] = 3;
        isConfirmed = false;

        locations[0][1] = 0;
        locations[2][1] = 0;
        locations[4][1] = 0;
        locations[16][1] = -150;
      }

      //7セグ基板へ送るデータをセット
      sendData[0][0] = isConfirmed;

      for (int i = 0; i < 4; i++)
        i2c7LED.setData(i + 1, sendData[0][i]);

      i2c7LED.update();
      break;
  }

  //赤ゾーン青ゾーン切り替え
  if (i2cSW.getTogleSWChangeDown())
  {
    //位置情報のxの符号を逆転
    for (int i = 0; i < 25; i++)
      locations[i][0] *= -1;
  }

  if (i2cSW.getClickEnter() && isBottleSetted && isConfirmed)
  {
    if (bottle[5]) {
      pointMode();
    }
    else {
      setRoute();
    }
    calcLocations();
  }
}
