#include "TimeParser.h"
#include <cctype>
#include <cstring>

static inline int to_two(const char* p) {
    return (p[0]-'0')*10 + (p[1]-'0');
}

int time_parse(const char* timestr) {
    if (!timestr) return TP_ERR_NULL;
    if (std::strlen(timestr) != 6) return TP_ERR_LEN;

    for (int i = 0; i < 6; ++i) {
        if (!std::isdigit(static_cast<unsigned char>(timestr[i])))
            return TP_ERR_NAN;
    }

    const int h = to_two(timestr + 0);
    const int m = to_two(timestr + 2);
    const int s = to_two(timestr + 4);

    if (h < 0 || h > 23) return TP_ERR_RANGE_H;
    if (m < 0 || m > 59) return TP_ERR_RANGE_M;
    if (s < 0 || s > 59) return TP_ERR_RANGE_S;

    return m * 60 + s;
}
