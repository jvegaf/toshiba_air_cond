#pragma once
#include <Arduino.h>

namespace Air
{
enum class Power {
  Off,
  On
};

enum class Temperature {
  Decrease,
  Increase
};

enum class Mode {
  Cool,
  Dry,
  Fan,
  Heat
};
  
enum class Fan {
  Low,
  Medium,
  High,
  Auto
};

enum class HWTimer {
  Cancel,
  Off,
  RepeatOff,
  On
};


} // namespace Air
