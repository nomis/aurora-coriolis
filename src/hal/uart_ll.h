#pragma once
#if defined(ARDUINO_LOLIN_S2_MINI)
# include "uart_ll_esp32s2.h"
#elif defined(ARDUINO_LOLIN_S3)
# include "uart_ll_esp32s3.h"
#endif
