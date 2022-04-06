/*
GNU GENERAL PUBLIC LICENSE

Version 2, June 1991

Copyright (C) 1989, 1991 Free Software Foundation, Inc.  
51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

Everyone is permitted to copy and distribute verbatim copies
of this license document, but changing it is not allowed.

*/


typedef struct {
  byte data[1];//MAX_RX_BUFFER];
  int idx_r = 0;
  int idx_w = 0;
  //idx_r is always BEFORE idx_w
  //idx_w
} rb_t;

typedef struct {
  uint8_t save;
  uint8_t heat;
  uint8_t cold;
  uint8_t target_temp;
  float sensor_temp;
  uint8_t fan;
  char fan_str[5];
  uint8_t mode;
  char mode_str[5];
  uint8_t preheat;
  uint8_t power;
  byte last_cmd[MAX_CMD_BUFFER];
  String buffer_cmd;
  String buffer_rx;
  byte rx_data[MAX_RX_BUFFER];  //serial rx data
  //byte rx2_data[MAX_RX_BUFFER];  //serial rx data
  byte rx_data_count = 0;
  //byte tx_data[MAX_CMD_BUFFER]; //serial tx data
  int curr_w_idx = 0;
  int curr_r_idx = 0;
  uint8_t timer_mode_req;
  uint8_t timer_time_req;
  bool timer_enabled;
  int decode_errors = 0;

  unsigned long boot_time;

  int sensor_val = 0;
  int sensor_id = 1;

  int error_id =0;
  int error_val=0;
  int error_type=0;

  //indoor unit data
  int indoor_room_temp;  //01 Room temperature (remote controller)
  int indoor_ta = 0;     //02 Indoor unit intake air temperature (TA)
  int indoor_tcj = 0;    //03 Indoor unit heat exchanger (coil) temperature (TCJ)
  int indoor_tc = 0;     //04 Indoor unit heat exchanger (coil) temperature (TC)
  int indoor_filter_time = 0;   //F3 Filter sign time

  //outdoor unit data

  int outdoor_te = 0;    //60 Outdoor unit heat exchanger (coil) temperature (TE)
  int outdoor_to = 0;    //61 Outside air temperature (TO)
  int outdoor_td = 0;    //62 Compressor discharge temperature (TD)
  int outdoor_ts = 0;    //63 Compressor suction temperature (TS)
  int outdoor_ths = 0;   //64—65 Heatsink temperature (THS)
  int outdoor_current = 0; //6A Operating current (x1/10)
  int outdoor_cumhour = 0; //F1 Compressor cumulative operating hours (x100 h)


  int indoor_fan_speed = 0;
  int indoor_fan_run_time = 0;

  int outdoor_tl = 0;
  int outdoor_comp_freq = 0;
  int outdoor_lower_fan_speed = 0;
  int outdoor_upper_fan_speed = 0;
  
  float power_consumption = 0;
  String ip;
  
  rb_t rb;

  SoftwareSerial serial;
} air_status_t;
