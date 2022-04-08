#include "Toshiba.h"
using namespace Air;

Toshiba::Toshiba()
    : mAirStatus{new air_status_t()}, _acTmr{new MySimpleTimer()},
      _tempTmr{new MySimpleTimer()}, _statusTmr{new MySimpleTimer()},
      _readSerialTmr{new MySimpleTimer()}, mDht_tmp{new float[MAX_LOG_DATA]},
      mDht_hum{new float[MAX_LOG_DATA]}, mBmp_tmp{new float[MAX_LOG_DATA]},
      mBmp_pre{new float[MAX_LOG_DATA]}, mAC_sensor{new float[MAX_LOG_DATA]},
      mAC_outdoor_te{new float[MAX_LOG_DATA]},
      mTimestamps{new unsigned long[MAX_LOG_DATA]} {}

void Toshiba::initialize() { init_air_serial(mAirStatus); }

void Toshiba::setPower(Air::Power val) {
  if (val == Power::Off) {
    air_set_power_off(mAirStatus);
    return;
  }
  air_set_power_on(mAirStatus);
}
void Toshiba::setTemp(uint8_t value) { air_set_temp(mAirStatus, value); }
void Toshiba::setTemp(Air::Temperature val) {
  switch (val) {
  case Temperature::Decrease:
    if (mAirStatus->target_temp < 19)
      return;
    air_set_temp(mAirStatus, mAirStatus->target_temp - 1);
    break;

  case Temperature::Increase:
    if (mAirStatus->target_temp > 29)
      return;
    air_set_temp(mAirStatus, mAirStatus->target_temp + 1);
    break;
  }
}
void Toshiba::setMode(Air::Mode val) {
  switch (val) {
  case Mode::Cool:
    air_set_mode(mAirStatus, MODE_COOL);
    break;

  case Mode::Dry:
    air_set_mode(mAirStatus, MODE_DRY);
    break;

  case Mode::Fan:
    air_set_mode(mAirStatus, MODE_FAN);
    break;

  case Mode::Heat:
    air_set_mode(mAirStatus, MODE_HEAT);
    break;
  }
}
void Toshiba::setFan(Air::Fan val) {
  switch (val) {
  case Fan::Low:
    air_set_fan(mAirStatus, FAN_LOW);
    break;

  case Fan::Medium:
    air_set_fan(mAirStatus, FAN_MEDIUM);
    break;
  case Fan::High:
    air_set_fan(mAirStatus, FAN_HIGH);
    break;
  case Fan::Auto:
    air_set_fan(mAirStatus, FAN_AUTO);
    break;
  }
}
void Toshiba::setSave(bool val) {
  if (val)
    return air_set_save_on(mAirStatus);
  air_set_save_off(mAirStatus);
}
void Toshiba::setHWtimer(Air::HWTimer val, uint16_t time) {
  switch (val) {
  case HWTimer::Cancel:
    air_set_timer(mAirStatus, TIMER_HW_CANCEL, time);
    break;

  case HWTimer::Off:
    air_set_timer(mAirStatus, TIMER_HW_OFF, time);
    break;

  case HWTimer::RepeatOff:
    air_set_timer(mAirStatus, TIMER_HW_REPEAT_OFF, time);
    break;

  case HWTimer::On:
    air_set_timer(mAirStatus, TIMER_HW_ON, time);
    break;
  }
}
void Toshiba::setTempInterval(uint16_t seconds) {
  timerInitialize(_tempTmr, kTimerUnit, seconds);
}
float *Toshiba::getDht_t_ds() { return mDht_tmp; }
float *Toshiba::getDht_h_ds() { return mDht_hum; }
float *Toshiba::getBmp_t_ds() { return mBmp_tmp; }
float *Toshiba::getBmp_p_ds() { return mBmp_pre; }
unsigned long *Toshiba::getTimestamps() { return mTimestamps; }
dht_data_t Toshiba::gDht_current() {
  return Sensors::read_dht();
}
bmp_data_t Toshiba::gBmp_current() {
  return Sensors::read_bmp();
}
int Toshiba::getTempIdx() { return mTemp_idx; }

void Toshiba::timerInitialize(MySimpleTimer *timer, uint16_t unit,
                              uint16_t interval) {
  timer->setUnit(unit);
  timer->setInterval(interval);
  timer->repeat();
  timer->start();
}
