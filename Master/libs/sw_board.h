#pragma once
#include "i2c_master_reader.h"

class SwBoard : public I2CMasterReader {
    
  private:
    bool oldData[10] = {};
    bool newData[10] = {};
    bool clickData[10] = {};
    bool toggleData[8] = {};
    uint8_t countUpData[8] = {};

  public:
    SwBoard(int address) : I2CMasterReader(address, 2) {}

    void update() {
      I2CMasterReader::update();
      for (int i = 0; i < 8; i++) {
        newData[i] = getBitData(1, i + 1);
        if (oldData[i] == 0 && newData[i] == 1)
          clickData[i] = true;
        else
          clickData[i] = false;
        oldData[i] = newData[i];
      }
      for (int i = 8; i < 10; i++) {
        newData[i] = getBitData(2, i - 7);
        if (oldData[i] != newData[i])
          clickData[i] = true;
        else
          clickData[i] = false;
        oldData[i] = newData[i];
      }
      for (int i = 0; i < 8; i++)
        if (clickData[i])   toggleData[i] ^= 1;
      for (int i = 0; i < 8; i++)
        if (clickData[i])       countUpData[i]++;
    }

    void reset() {
      for (int i = 0; i < 8; i++) {
        clickData[i] = 0;
        toggleData[i] = 0;
        countUpData[i] = 0;
      }
    }

    bool getTogleSWChangeUp() {
      return clickData[8];
    }
    bool getTogleSWChangeDown() {
      return clickData[9];
    }
    // Click //
    bool getClickDelete() {
      return clickData[0];
    }
    bool getClickEnter() {
      return clickData[1];
    }
    bool getClickRUp() {
      return clickData[2];
    }
    bool getClickRDown() {
      return clickData[3];
    }
    bool getClickCUp() {
      return clickData[4];
    }
    bool getClickCDown() {
      return clickData[5];
    }
    bool getClickLUp() {
      return clickData[6];
    }
    bool getClickLDown() {
      return clickData[7];
    }
    // Toggle //
    bool getToggleDelete() {
      return toggleData[0];
    }
    bool getToggleEnter() {
      return toggleData[1];
    }
    bool getToggleRUp() {
      return toggleData[2];
    }
    bool getToggleRDown() {
      return toggleData[3];
    }
    bool getToggleCUp() {
      return toggleData[4];
    }
    bool getToggleCDown() {
      return toggleData[5];
    }
    bool getToggleLUp() {
      return toggleData[6];
    }
    bool getToggleLDown() {
      return toggleData[7];
    }
    //  CountUp //
    int getCountUpDelete() {
      return countUpData[0];
    }
    int getCountUpEnter() {
      return countUpData[1];
    }
    int getCountUpRUp() {
      return countUpData[2];
    }
    int getCountUpRDown() {
      return countUpData[3];
    }
    int getCountUpCUp() {
      return countUpData[4];
    }
    int getCountUpCDown() {
      return countUpData[5];
    }
    int getCountUpLUp() {
      return countUpData[6];
    }
    int getCountUpLDown() {
      return countUpData[7];
    }
};
