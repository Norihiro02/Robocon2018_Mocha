#pragma once
#include<Wire.h>

class I2CMasterReader {

  protected:
    uint8_t I2C_ADDR;
    uint8_t *data;
    uint8_t dataSize;

  public:

    I2CMasterReader(int address, int arraySize) : dataSize(arraySize), I2C_ADDR(address) {
      data = new byte[arraySize];
      for (int i = 0; i < arraySize; i++)
        data[i] = 0;
    }

    virtual ~I2CMasterReader() {
      delete[] data;
    }

    void update() {
      Wire.requestFrom(I2C_ADDR, dataSize);
      while (Wire.available()) {
        for (int i = 0; i < dataSize; i++)
          data[i] = Wire.read();
      }
    }
    byte getData(int arrayNum) {
      return data[arrayNum - 1];
    }

    bool getBitData(int arrayNum, int bitNum) {
      return bitRead(data[arrayNum - 1], bitNum - 1);
    }

    void show() {
      for (int i = 0; i < dataSize; i++) {
        Serial.print(data[i]);
        Serial.print("\t");
      }
      Serial.println();
    }
};
