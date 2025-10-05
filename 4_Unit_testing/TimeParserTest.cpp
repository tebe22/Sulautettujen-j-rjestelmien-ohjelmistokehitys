

#include "gtest/gtest.h"
#include "TimeParser.h"

// Perustapaukset
TEST(TimeParserTest, ParsesBasic) {
    EXPECT_EQ(time_parse("000120"), 80);                
    EXPECT_EQ(time_parse("123456"), 34*60 + 56);         
}

// Boundaryt
TEST(TimeParserTest, BoundaryZeros) {
    EXPECT_EQ(time_parse("000000"), 0);
}

TEST(TimeParserTest, BoundaryUpperMinuteSecond) {
    EXPECT_EQ(time_parse("005959"), 59*60 + 59);
}

TEST(TimeParserTest, BoundaryUpperHour) {
    EXPECT_EQ(time_parse("235900"), 59*60 + 0);
}

// Range-virheet
TEST(TimeParserTest, MinutesSixtyIsError) {
    EXPECT_EQ(time_parse("006000"), TP_ERR_RANGE_M);
}

TEST(TimeParserTest, SecondsSixtyIsError) {
    EXPECT_EQ(time_parse("000060"), TP_ERR_RANGE_S);
}

TEST(TimeParserTest, Hours24IsError) {
    EXPECT_EQ(time_parse("245900"), TP_ERR_RANGE_H);
}

// Formaattivirheet
TEST(TimeParserTest, LengthNotSix) {
    EXPECT_EQ(time_parse("01234"),   TP_ERR_LEN);
    EXPECT_EQ(time_parse("00112233"),TP_ERR_LEN);
}

TEST(TimeParserTest, NonDigits) {
    EXPECT_EQ(time_parse("12AB56"), TP_ERR_NAN);
}

TEST(TimeParserTest, NullPtr) {
    EXPECT_EQ(time_parse(nullptr), TP_ERR_NULL);
}
