#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2020 Nuclex Development Labs

This library is free software; you can redistribute it and/or
modify it under the terms of the IBM Common Public License as
published by the IBM Corporation; either version 1.0 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
IBM Common Public License for more details.

You should have received a copy of the IBM Common Public
License along with this library
*/
#pragma endregion // CPL License

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/Text/Lexical.h"

#include <cmath>
#include <clocale>

#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Checks if a string starts with the letters NaN (ignoring case)</summary>
  /// <param name="text">String that will be checked for NaN</param>
  /// <returns>True if the string started with NaN, false otherwise</returns>
  bool textStartsWithNaN(const std::string &text) {
    if(text.length() >= 3) {
      return (
        ((text[0] == 'n') || (text[0] == 'N')) &&
        ((text[1] == 'a') || (text[1] == 'A')) &&
        ((text[2] == 'n') || (text[2] == 'N'))
      );
    } else {
      return false;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Checks if a string starts with the letters Inf or -Inf (ignoring case)</summary>
  /// <param name="text">String that will be checked for Inf</param>
  /// <returns>True if the string started with Inf or -Inf, false otherwise</returns>
  bool textStartsWithInfOrMinusInf(const std::string &text) {
    bool startsWithInf, startsWithMinusInf;

    std::string::size_type length = text.length();

    // Check for -inf
    if(length >= 4) {
      startsWithMinusInf = (
        ((text[0] == '+') || (text[0] == '-')) &&
        ((text[1] == 'i') || (text[1] == 'I')) &&
        ((text[2] == 'n') || (text[2] == 'N')) &&
        ((text[3] == 'f') || (text[3] == 'F'))
      );
    } else {
      startsWithMinusInf = false;
    }

    // Check for inf
    if(length >= 3) {
      startsWithInf = (
        ((text[0] == 'i') || (text[0] == 'I')) &&
        ((text[1] == 'n') || (text[1] == 'N')) &&
        ((text[2] == 'f') || (text[2] == 'F'))
      );
    } else {
      startsWithInf = false;
    }

    return (startsWithInf || startsWithMinusInf);
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Checks if a string starts with a minus character</summary>
  /// <param name="text">String that will be checked for starting with a minus</param>
  /// <returns>True if the string starts with a minus character, false otherwise</returns>
  bool textStartsWithMinus(const std::string &text) {
    if(text.size() >= 1) {
      return text[0] == '-';
    } else {
      return false;
    }
  }
    
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, CanConvertBoolToString) {
    std::string text = lexical_cast<std::string>(true);
    EXPECT_EQ(text, "true");
    text = lexical_cast<std::string>(false);
    EXPECT_EQ(text, "false");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, CanConvertStringToBool) {
    bool boolean_from_true_string = lexical_cast<bool>("true");
    EXPECT_TRUE(boolean_from_true_string);
    bool boolean_from_false_string = lexical_cast<bool>("false");
    EXPECT_FALSE(boolean_from_false_string);

    bool boolean_from_invalid_string = lexical_cast<bool>("hi there, how goes?");
    EXPECT_FALSE(boolean_from_invalid_string);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, CanConvertUInt8ToString) {
    std::string text = lexical_cast<std::string>(std::uint8_t(234));
    EXPECT_EQ(text, "234");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, CanConvertStringToUInt8) {
    std::uint8_t value = lexical_cast<std::uint8_t>("235");
    EXPECT_EQ(value, std::uint8_t(235));

    value = lexical_cast<std::uint8_t>(std::string("236"));
    EXPECT_EQ(value, std::uint8_t(236));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, CanConvertInt8ToString) {
    std::string text = lexical_cast<std::string>(std::int8_t(-123));
    EXPECT_EQ(text, "-123");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, CanConvertStringToInt8) {
    std::int8_t value = lexical_cast<std::int8_t>("-124");
    EXPECT_EQ(value, std::int8_t(-124));

    value = lexical_cast<std::int8_t>(std::string("-125"));
    EXPECT_EQ(value, std::int8_t(-125));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, CanConvertUInt16ToString) {
    std::string text = lexical_cast<std::string>(std::uint16_t(56789));
    EXPECT_EQ(text, "56789");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, CanConvertStringToUInt16) {
    std::uint16_t value = lexical_cast<std::uint16_t>("56790");
    EXPECT_EQ(value, std::uint16_t(56790));

    value = lexical_cast<std::uint16_t>(std::string("56791"));
    EXPECT_EQ(value, std::uint16_t(56791));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, CanConvertInt16ToString) {
    std::string text = lexical_cast<std::string>(std::int16_t(-23456));
    EXPECT_EQ(text, "-23456");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, CanConvertStringToInt16) {
    std::int16_t value = lexical_cast<std::int16_t>("-23457");
    EXPECT_EQ(value, std::int16_t(-23457));

    value = lexical_cast<std::int16_t>(std::string("-23458"));
    EXPECT_EQ(value, std::int16_t(-23458));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, CanConvertUInt32ToString) {
    std::string text = lexical_cast<std::string>(std::uint32_t(3456789012));
    EXPECT_EQ(text, "3456789012");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, CanConvertStringToUInt32) {
    std::uint32_t value = lexical_cast<std::uint32_t>("3456789013");
    EXPECT_EQ(value, std::uint32_t(3456789013));

    value = lexical_cast<std::uint32_t>(std::string("3456789014"));
    EXPECT_EQ(value, std::uint32_t(3456789014));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, CanConvertInt32ToString) {
    std::string text = lexical_cast<std::string>(std::int32_t(-1234567890));
    EXPECT_EQ(text, "-1234567890");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, CanConvertStringToInt32) {
    std::int32_t value = lexical_cast<std::int32_t>("-1234567891");
    EXPECT_EQ(value, std::int32_t(-1234567891));

    value = lexical_cast<std::int32_t>(std::string("-1234567892"));
    EXPECT_EQ(value, std::int32_t(-1234567892));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, CanConvertUInt64ToString) {
    std::string text = lexical_cast<std::string>(std::uint64_t(12345678901234567890ULL));
    EXPECT_EQ(text, "12345678901234567890");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, CanConvertStringToUInt64) {
    std::uint64_t value = lexical_cast<std::uint64_t>("12345678901234567891");
    EXPECT_EQ(value, std::uint64_t(12345678901234567891ULL));

    value = lexical_cast<std::uint64_t>(std::string("12345678901234567892"));
    EXPECT_EQ(value, std::uint64_t(12345678901234567892ULL));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, CanConvertInt64ToString) {
    std::string text = lexical_cast<std::string>(std::int64_t(-8901234567890123456LL));
    EXPECT_EQ(text, "-8901234567890123456");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, CanConvertStringToInt64) {
    std::int64_t value = lexical_cast<std::int64_t>("-8901234567890123457");
    EXPECT_EQ(value, std::int64_t(-8901234567890123457LL));

    value = lexical_cast<std::int64_t>(std::string("-8901234567890123458"));
    EXPECT_EQ(value, std::int64_t(-8901234567890123458LL));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, CanConvertFloatToString) {
    std::string text = lexical_cast<std::string>(float(0.0009765625f));
    EXPECT_EQ(text, "0.0009765625");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, CanConvertStringToFloat) {
    float value = lexical_cast<float>("0.0009765625");
    EXPECT_EQ(value, float(0.0009765625));

    value = lexical_cast<float>(std::string("0.0009765625"));
    EXPECT_EQ(value, float(0.0009765625));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, CanConvertDoubleToString) {
    std::string text = lexical_cast<std::string>(double(0.00000190735));
    EXPECT_EQ(text, "0.00000190735");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, CanConvertStringToDouble) {
    double value = lexical_cast<double>("0.00000190735");
    EXPECT_EQ(value, double(0.00000190735));

    value = lexical_cast<double>(std::string("0.00000190735"));
    EXPECT_EQ(value, double(0.00000190735));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, FloatToStringIsLocaleIndependent) {
    std::setlocale(LC_NUMERIC, "de_DE.UTF-8");
    std::string text = lexical_cast<std::string>(0.125f);
    EXPECT_EQ(text, "0.125");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, FloatToStringAlwaysIncludesLeadingZero) {
    std::string text = lexical_cast<std::string>(0.1f);
    EXPECT_EQ(text, "0.1");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, FloatToStringDecimalsAreOptional) {
    std::string text = lexical_cast<std::string>(1.0f);
    EXPECT_EQ(text, "1");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, FloatToStringHandlesNaN) {
    std::string text = lexical_cast<std::string>(std::numeric_limits<float>::quiet_NaN());
    EXPECT_TRUE(textStartsWithNaN(text));

    text = lexical_cast<std::string>(std::numeric_limits<float>::signaling_NaN());
    EXPECT_TRUE(textStartsWithNaN(text));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, FloatToStringHandlesInfinity) {
    std::string text = lexical_cast<std::string>(std::numeric_limits<float>::infinity());
    EXPECT_TRUE(textStartsWithInfOrMinusInf(text));

    text = lexical_cast<std::string>(-std::numeric_limits<float>::infinity());
    EXPECT_TRUE(textStartsWithMinus(text));
    EXPECT_TRUE(textStartsWithInfOrMinusInf(text));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, FloatToStringOutputCanBeLong) {
    const float PI = 3.14159265358979323846264338327950288419716939937510582097494459230781640628f;
    std::string text = lexical_cast<std::string>(PI);

    // Why this number and this many decimals? Floating point numbers can only represent
    // certain values exactly (specifically binary fractions, i.e. 1/1024 or 5/4096 but not
    // the numbers between them).
    //
    // This is the number of decimals after which adding decimals that selects the closest
    // respresentable float (for round-trip parsing) and adding more decimals would not
    // actually change the resulting floating point value.
    EXPECT_EQ(text, "3.1415927");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, DoubleToStringIsLocaleIndependent) {
    std::setlocale(LC_NUMERIC, "de_DE.UTF-8");
    std::string text = lexical_cast<std::string>(0.125);
    EXPECT_EQ(text, "0.125");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, DoubleToStringAlwaysIncludesLeadingZero) {
    std::string text = lexical_cast<std::string>(0.1);
    EXPECT_EQ(text, "0.1");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, DoubleToStringDecimalsAreOptional) {
    std::string text = lexical_cast<std::string>(1.0);
    EXPECT_EQ(text, "1");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, DoubleToStringHandlesNaN) {
    std::string text = lexical_cast<std::string>(std::numeric_limits<double>::quiet_NaN());
    EXPECT_TRUE(textStartsWithNaN(text));

    text = lexical_cast<std::string>(std::numeric_limits<double>::signaling_NaN());
    EXPECT_TRUE(textStartsWithNaN(text));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, DoubleToStringHandlesInfinity) {
    std::string text = lexical_cast<std::string>(std::numeric_limits<double>::infinity());
    EXPECT_TRUE(textStartsWithInfOrMinusInf(text));

    text = lexical_cast<std::string>(-std::numeric_limits<double>::infinity());
    EXPECT_TRUE(textStartsWithMinus(text));
    EXPECT_TRUE(textStartsWithInfOrMinusInf(text));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalTest, DoubleToStringOutputCanBeLong) {
    const double PI = 3.14159265358979323846264338327950288419716939937510582097494459230781640628;
    std::string text = lexical_cast<std::string>(PI);

    // Why this number and this many decimals? Floating point numbers can only represent
    // certain values exactly (specifically binary fractions, i.e. 1/1024 or 5/4096 but not
    // the numbers between them).
    //
    // This is the number of decimals after which adding decimals that selects the closest
    // respresentable float (for round-trip parsing) and adding more decimals would not
    // actually change the resulting double-precision floating point value.
    EXPECT_EQ(text, "3.141592653589793");
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
