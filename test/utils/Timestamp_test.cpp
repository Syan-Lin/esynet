#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_COLORS_ANSI
#include <doctest/doctest.h>
#include "utils/Timestamp.h"

using namespace esynet::utils;

TEST_CASE("Timestamp_Test"){
    Timestamp time0;    /* 1970-01-01 08:00:00 */
    Timestamp time1(123456);
    CHECK(!time0.valid());
    CHECK(time1.valid());
    CHECK(time0 < time1);
    CHECK(time1 > time0);
    CHECK(time0 != time1);
    CHECK(!(time0 == time1));

    CHECK(time0.toString() == "0.000000");
    CHECK(time1.toString() == "0.123456");
    CHECK(time0.toFormattedString() == "1970-01-01 08:00:00.000000");
    CHECK(time1.toFormattedString() == "1970-01-01 08:00:00.123456");
    CHECK(time0.toFormattedString(false) == "1970-01-01 08:00:00");
    CHECK(time1.toFormattedString(false) == "1970-01-01 08:00:00");

    time0 += 1;
    time0 = time0 + 1;
    time0 -= 0.5;
    time0 = time0 - 0.5;

    CHECK(time0.microSecondsSinceEpoch() == 1000);
    time0 += 1000;
    CHECK(time0.secondsSinceEpoch() == 1);
    CHECK(time0 - time1 == 877.544);

    auto time = Timestamp::now();

    Timestamp zero;
    zero = zero + 3.14;
    CHECK(zero.toString() == "0.003140");
    zero += 3.14;
    CHECK(zero.toFormattedString() == "1970-01-01 08:00:00.006280");
    CHECK(zero.toFormattedString(false) == "1970-01-01 08:00:00");

    zero = zero - 1;
    CHECK(zero.toFormattedString() == "1970-01-01 08:00:00.005280");
    zero -= 1;
    CHECK(zero.toFormattedString() == "1970-01-01 08:00:00.004280");

    Timestamp ts(5 * Timestamp::kMicroSecondsPerSecond);
    CHECK(ts - zero == 4995.72);
    CHECK(!(ts == zero));
    CHECK(ts != zero);
    Timestamp copy(ts);
    CHECK(copy == ts);
    CHECK(!(copy != ts));
}