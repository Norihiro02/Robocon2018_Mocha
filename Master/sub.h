#pragma once

/************************************
   Include
************************************/

#include "libs/controller_manager_ds3.h"
#include "libs/i2c_master_writer.h"
#include "libs/i2c_master_reader.h"
#include "libs/sw_board.h"
#include "libs/loop_cycle_controller.h"
#include "libs/pid_controller.h"
#include "libs/enc_board_mbed.h"
#include "libs/enc_board_speed_mbed.h"

/************************************
   Instace
************************************/

extern ControllerManagerDS3 DS3;
extern EncoderBoardSpeedMbed encLaunch;
extern EncoderBoardMbed enc;
extern I2CMasterReader limitSW;
extern I2CMasterWriter i2c7LED, i2cOmni, i2cMotorBoard, i2cDecoration;
extern SwBoard i2cSW;
extern LoopCycleController loopPeriodUs;
extern PidController xomniPid, yomniPid, gyroPid, maxonSpeedPid;

/************************************
   Functions
************************************/

//gyro.cpp
int setGyroOffset();
int getGyro();

//all_update.cpp
void allUpdate();

//led_segment.cpp
void LED7SetUp();
void setLED();
void sendLEDData(bool*);

//motor_board.cpp
void motorBoard();

//omni.cpp
void omni();

//funcs.cpp
void pidClearPower();
void encInit();
double getProPower(double, long, long, int16_t);
void switches();

//all_stop.cpp
void allStop();

//decoration.cpp
void decoration();