/**
 * @file SerialInterface.h
 */

#pragma once
#include <stdint.h>

namespace SerialInterface
{
  bool init();
  void commandHandler();
  void update();
}
