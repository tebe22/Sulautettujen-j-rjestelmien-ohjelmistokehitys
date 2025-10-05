#pragma once
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

enum TimeParseError {
    TP_ERR_NULL      = -1, // null-pointteri
    TP_ERR_LEN       = -2, // väärä pituus
    TP_ERR_NAN       = -3, // ei-numeroita
    TP_ERR_RANGE_H   = -4, // tunnit [0..23]
    TP_ERR_RANGE_M   = -5, // minuutit [0..59]
    TP_ERR_RANGE_S   = -6  // sekunnit [0..59]
};

int time_parse(const char* timestr);

#ifdef __cplusplus
}
#endif
