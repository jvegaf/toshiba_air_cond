#pragma once
#include <Arduino.h>
#include "MySimpleTimer.h"
#include "air_enums.h"
#include "sensors.hpp"
#include "toshiba_serial.hpp"
#include "config.h"


class Toshiba {

public:
  Toshiba();
  void initialize();
  void setPower(Air::Power val);
  void setTemp(uint8_t value);
  void setTemp(Air::Temperature val);
  void setMode(Air::Mode val);
  void setFan(Air::Fan val);
  void setSave(bool val);
  void setHWtimer(Air::HWTimer val, uint16_t time);
  void setTempInterval(uint16_t seconds);
  float* getDht_t_ds();
  float* getDht_h_ds();
  float* getBmp_t_ds();
  float* getBmp_p_ds();
  unsigned long* getTimestamps();
  dht_data_t gDht_current();
  bmp_data_t gBmp_current();
  int getTempIdx();

private:
  air_status_t* mAirStatus; 
  MySimpleTimer *_acTmr { nullptr };
  MySimpleTimer *_tempTmr { nullptr };
  MySimpleTimer *_statusTmr { nullptr };
  MySimpleTimer *_readSerialTmr { nullptr };
  float *mDht_tmp { nullptr };
  float *mDht_hum { nullptr };
  float *mBmp_tmp { nullptr };
  float *mBmp_pre { nullptr };
  float *mAC_sensor { nullptr };
  float *mAC_outdoor_te { nullptr };
  unsigned long *mTimestamps { nullptr };
  int mTemp_idx { 0 };
  dht_data_t* dht_current { nullptr };
  bmp_data_t* bmp_current { nullptr };

  void timerInitialize(MySimpleTimer* timer, uint16_t unit, uint16_t interval);
};
