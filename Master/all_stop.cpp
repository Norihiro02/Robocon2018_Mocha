/*
    @program  モータを全て停止させる
    @date 2018/09/05
    @author Watanabe Rui
*/

#include<arduino.h>
#include"sub.h"

extern int gyroOffset;

void allStop(){
    i2cMotorBoard.reset();

    i2cOmni.reset();
    i2cOmni.setData(5, 127);

    gyroOffset = setGyroOffset();
}
