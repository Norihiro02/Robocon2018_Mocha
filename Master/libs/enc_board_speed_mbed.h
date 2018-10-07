/**
        @file   enc_board_speed_mbed.h
        @brief  Mbedエンコーダ基盤から回転数と回転速度を取得するクラス
        @rewriter Watanabe Rui
        @date   2018/8/17
*/

#pragma once
#include <arduino.h>
#include "enc_board_mbed.h"

/*     EncoderBoardSpeedクラス
        エンコーダ基盤から回転数と回転速度を取得するクラス
*/
class EncoderBoardSpeedMbed : public EncoderBoardMbed
{

  const unsigned int ENCODER_RESOLUTION; //エンコーダの分解能

  double rotateSpeed[4]; // 回転速度・返り値

  /**     時刻と角度データ
    */
  struct Data
  {
    long count;            //回転数
    unsigned long time_us; //時刻

    Data(long count = 0, unsigned long time_us = 0) : count(count), time_us(time_us)
    {
    }
  } lastData[8]; //最後に読み取った二回のデータ(0のほうが新しい)

  /*     速度演算
            二つの回転数座標から速度を求める
            @param  now     座標1
            @param  last    座標2
            @return         速度(unit : revolution per second)
    */
  double calcRotateSpeed(const Data &now, const Data &last)
  {
    return (double)(now.count - last.count) / (now.time_us - last.time_us) * 1000000.0 / ENCODER_RESOLUTION; //rpsで計算
  }

  /*
            ある時刻の回転数推定データ
            記憶している二個のデータから推定する、ある時刻の回転数データを得る
            @param  time_us 指定する時刻
            @return         指定された時刻の回転数データ
    */
  Data estimateData1(unsigned long time_us)
  {
    long estCount1 = lastData[1].count + (lastData[0].count - lastData[1].count) * (long)(time_us - lastData[1].time_us) / (long)(lastData[0].time_us - lastData[1].time_us);
    return Data(estCount1, time_us);
  }
  Data estimateData2(unsigned long time_us)
  {
    long estCount2 = lastData[3].count + (lastData[2].count - lastData[3].count) * (long)(time_us - lastData[3].time_us) / (long)(lastData[2].time_us - lastData[3].time_us);
    return Data(estCount2, time_us);
  }
  Data estimateData3(unsigned long time_us)
  {
    long estCount3 = lastData[5].count + (lastData[4].count - lastData[5].count) * (long)(time_us - lastData[5].time_us) / (long)(lastData[4].time_us - lastData[5].time_us);
    return Data(estCount3, time_us);
  }
  Data estimateData4(unsigned long time_us)
  {
    long estCount4 = lastData[7].count + (lastData[6].count - lastData[7].count) * (long)(time_us - lastData[7].time_us) / (long)(lastData[6].time_us - lastData[7].time_us);
    return Data(estCount4, time_us);
  }

public:
  unsigned long INTERVAL_US; //速度の読み取り周期
  /*     コンストラクタ
            @param  address         [in]使用するエンコーダ基盤のI2Cアドレス
            @param  encoderResolution   [in]1回転あたりのステップ数
            @param  interval_ms     [in]速度を計算する微分時間
    */
  EncoderBoardSpeedMbed(uint8_t address, unsigned int encoderResolution, unsigned int interval_ms) : EncoderBoardMbed(address), ENCODER_RESOLUTION(encoderResolution)
  {
    INTERVAL_US = (unsigned long)interval_ms * 1000;
    for (int i = 0; i < 4; i++)
      rotateSpeed[i] = 0;
  }

  /**     データの更新
            I2Cで現在の回転数を読み出し、速度を計算する。
    */
  void update()
  {
    EncoderBoardMbed::update();

    Data nowData1(getCount(1), micros()), nowData2(getCount(2), micros()), nowData3(getCount(3), micros()), nowData4(getCount(4), micros()); //現在のデータを取る

    rotateSpeed[0] = calcRotateSpeed(nowData1, estimateData1(nowData1.time_us - INTERVAL_US));
    rotateSpeed[1] = calcRotateSpeed(nowData2, estimateData2(nowData2.time_us - INTERVAL_US));
    rotateSpeed[2] = calcRotateSpeed(nowData3, estimateData3(nowData3.time_us - INTERVAL_US));
    rotateSpeed[3] = calcRotateSpeed(nowData4, estimateData4(nowData4.time_us - INTERVAL_US));

    if (nowData1.time_us - lastData[0].time_us > INTERVAL_US)
    {
      lastData[1] = lastData[0];
      lastData[0] = nowData1;

      lastData[3] = lastData[2];
      lastData[2] = nowData2;

      lastData[5] = lastData[4];
      lastData[4] = nowData3;

      lastData[7] = lastData[6];
      lastData[6] = nowData4;
    }
  }
  /**     回転速度の取得
            @param  [int]port：ポート番号
            @return     現在の回転速度(revolution per second)
    */
  double getRotateSpeed(int port)
  {
    return rotateSpeed[port - 1];
  }

  void resetCount(int port)
  {
    offsetCount[port - 1] = nowCount[port - 1];
    rotateSpeed[port - 1] = 0;
  }
};
