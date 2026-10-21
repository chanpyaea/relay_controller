#ifndef AC_MONITOR_H
#define AC_MONITOR_H

#include <Arduino.h>

void acMonitorInit();
void acMonitorLoop();
bool isElectricityOn();

#endif
