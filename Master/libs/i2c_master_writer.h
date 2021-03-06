/*
  @program I2C通信でslaveへデータを送信するクラス
  @date 2018/06/25
  @author Watanabe Rui
*/

#pragma once

#include <arduino.h>
#include <Wire.h>

class I2CMasterWriter
{
  private:

    uint8_t ADDR;
    uint8_t *data;
    uint8_t size;

  public:

    /*
          コンストラクタ
          @param  address     I2Cアドレス
          @param  dataSize  送信するデータ個数
    */
    I2CMasterWriter(int address, int dataSize) : ADDR(address), size(dataSize)
    {

      data = new uint8_t[size];

      for (int i = 0; i < size; i++)
        data[i] = 0;

    }

    //  デストラクタ
    virtual ~I2CMasterWriter()
    {
      delete[] data;
    }

    //送るデータの中身を表示
    void show()
    {
      for (int i = 0; i < size; i++)
      {
        Serial.print(data[i]);
        Serial.print("\t");
      }
      Serial.println("");
    }

    /*
          指定した添字の配列にデータをセット
          @param  arrayNum  配列の添字
          @param  val        [byte]送信する値
    */
    void setData(int arrayNum, byte val)
    {
      data[arrayNum-1] = val;
    }

    byte getData(int arrayNum){
      return data[arrayNum -1];
    }

    /*
        指定したモーターにデータをセット
        @param motor_mun　モーターの番号
        @param data　回転数 -255~255
    */
    void setMotorData(int motor_num, int value) {
      value = constrain(value, -250, 250);
      setBitData(1, motor_num, value > 0 );
      setData(motor_num + 1, abs(value));
    }

    /*
          指定した添字の配列にビットデータをセット
          @param  arrayNum  配列の添字
          @param  bitNum    bit番号
          @param  bit       1 or 0
    */
    void setBitData(int arrayNum, byte bitNum, bool bit)
    {
      bitWrite(data[arrayNum-1], bitNum-1, bit);
    }

    //  データの初期化
    void reset()
    {
      for (int i = 0; i < size; i++)
        data[i] = 0;
    }

    //  Slave側へ送信
    void update()
    {
      Wire.beginTransmission(ADDR);
      Wire.write(data, size);
      Wire.endTransmission();
    }

};