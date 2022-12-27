#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_COLORS_ANSI
#include <doctest/doctest.h>
#include "utils/Timestamp.h"

TEST_CASE("Timestamp_Test"){
    auto time = Timestamp::now();

    Timestamp zero;
    zero = zero + 3.14;
    CHECK(zero.toString() == "3.140000");
    zero += 3.14;
    CHECK(zero.toFormattedString() == "1970-01-01 08:00:06.280000");
    CHECK(zero.toFormattedString(false) == "1970-01-01 08:00:06");

    zero = zero - 1;
    CHECK(zero.toFormattedString() == "1970-01-01 08:00:05.280000");
    zero -= 1;
    CHECK(zero.toFormattedString() == "1970-01-01 08:00:04.280000");

    Timestamp ts(5 * Timestamp::kMicroSecondsPerSecond);
    CHECK(ts - zero == 0.72);
    CHECK(!(ts == zero));
    CHECK(ts != zero);
    Timestamp copy(ts);
    CHECK(copy == ts);
    CHECK(!(copy != ts));
}