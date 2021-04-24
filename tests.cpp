#define CATCH_CONFIG_MAIN

#include "Parser.hpp"
#include <cassert>
#include <catch2/catch.hpp>
#include <iostream>

using namespace Parsers;

TEST_CASE("Digit Parser") {
  auto digit_check = Digit.parse("12");
  REQUIRE(digit_check.has_value());
  REQUIRE(digit_check.value() == std::make_pair('1', string_view("2")));
  digit_check = Digit.parse(digit_check.value().second);
  REQUIRE(digit_check.has_value());
  REQUIRE(digit_check.value() == std::make_pair('2', string_view("")));
}

TEST_CASE("OneOrMore") {
  auto oneormore_check = AlphaNum.oneOrMore().parse("a12");
  REQUIRE(oneormore_check == std::make_optional(std::make_pair(
                                 std::vector{'a', '1', '2'}, string_view(""))));
}

TEST_CASE("Positive Number") {
  auto pos_num_check = PosNum.parse("123a");
  REQUIRE(pos_num_check.has_value());
  REQUIRE(pos_num_check.value() ==
          std::make_optional(
              std::make_pair<size_t, string_view>(123, string_view("a"))));
}

TEST_CASE("Brackets Check") {

  auto lcurly_check = LeftCurly.parse("{)");
  REQUIRE(lcurly_check ==
          std::make_optional(std::make_pair('{', string_view(")"))));
  lcurly_check = LeftCurly.parse(lcurly_check.value().second);
  REQUIRE(lcurly_check == std::nullopt);
}

TEST_CASE("andThen") {
  auto andThenCheck =
      String("Hi").andThen(String("Da")).andThen(String("F")).parse("HiDaD");
  REQUIRE(andThenCheck == std::nullopt);
  andThenCheck =
      String("Hi").andThen(String("Da")).andThen(String("D")).parse("HiDaD");
  REQUIRE(andThenCheck.has_value());
}

TEST_CASE("zips") {
  auto zips = zip3(Alpha, PosNum, Alpha);
  auto zipcheck = zips.parse("a12b");
  REQUIRE(zipcheck.has_value());
  zipcheck = zips.parse("a12");
  REQUIRE(not zipcheck.has_value());

  auto zipmanycheck = zipMany(Alpha, Digit, Alpha, Digit);
  auto zipmany_res = zipmanycheck.parse("a1a1");
  REQUIRE(zipmany_res.has_value());
  zipmany_res = zipmanycheck.parse("a1aa");
  REQUIRE(not zipmany_res.has_value());
}

TEST_CASE("WhiteSpace") {
  auto space_surr_skip = skipSurrWhitespace(Alpha).parse("    a  ");
  REQUIRE(space_surr_skip.has_value());

  auto space_pre_skip = skipPreWhitespace(Alpha).parse("    a  ");
  REQUIRE(space_pre_skip.has_value());
}

TEST_CASE("OR operator Check") {
  auto or_check = (Digit || Character('c')).parse("c1");
  REQUIRE(or_check.has_value());
  or_check = (Digit || Character('c')).parse("d1");
  REQUIRE(not or_check.has_value());
}

TEST_CASE("oneOf") {
  auto oneOfCheck = oneOf(Digit, Alpha).parse("a1");
  REQUIRE(oneOfCheck.value().first == 'a');
  auto oneOfCheck2 = oneOf(Digit, Alpha, Character('.')).parse(".a1");
  REQUIRE(oneOfCheck2.value().first == '.');
  auto oneOfCheck3 = oneOf(Digit, Alpha, Character('.')).parse(" a1");
  REQUIRE(oneOfCheck3.has_value() == false);
}

TEST_CASE("zipAndGet") {
  auto zipAndGetCheck = zipAndGet<0>(Alpha, Digit).parse("c1");
  REQUIRE(zipAndGetCheck.has_value());
}

TEST_CASE() {
  auto OptionalCheck1 = Optional(String("(")).parse("())");
  REQUIRE(OptionalCheck1.value().first == true);
  REQUIRE(OptionalCheck1.value().second == "))");

  auto OptionalCheck2 = Optional(String("(")).parse("}}}");
  REQUIRE(OptionalCheck2.value().first == false);
  REQUIRE(OptionalCheck2.value().second == "}}}");
}

TEST_CASE("Char Template Specialization") {
  auto char_template_specialization = Optional(Parser('.')).parse(".1");
  REQUIRE(char_template_specialization.has_value());
}

TEST_CASE("Character Parsers") {
  auto excludes1 = Char_excluding(']').parse("]Hi");
  REQUIRE(!excludes1.has_value());
  excludes1 = Char_excluding(']').parse("Hi");
  REQUIRE(excludes1.has_value());

  auto excludes2 = Char_excluding_many(std::array{'[', ']'}).parse("[Hi");
  REQUIRE(!excludes2.has_value());

  auto includes1 = Characters(std::array{'a', 'b', 'c'}).parse("ab");
  REQUIRE(includes1.has_value());
  includes1 = Characters(std::array{'a', 'b', 'c'}).parse("db");
  REQUIRE(!includes1.has_value());

  // auto error = Character('a').orThrow("Couldnt parse a").parse("bc"); //
  // throws
  auto error = Character('a').orThrow("Couldnt parse a").parse("abc");
  REQUIRE(error.has_value());
}

TEST_CASE("End") {
  auto end = End.parse("");
  REQUIRE(end.value().first);
  end = End.parse("a");
  REQUIRE(not end.value().first);
}

TEST_CASE("Skips") {
  auto skipmanycheck = skipMany(WhiteSpace).parse("a");
  REQUIRE(skipmanycheck.value().first == 0);
  skipmanycheck = skipMany1(WhiteSpace).parse("  a");
  REQUIRE(skipmanycheck.value().first == 2);
  skipmanycheck = skipMany1(WhiteSpace).parse("a");
  REQUIRE(!skipmanycheck.has_value());
  skipmanycheck = skipMany(WhiteSpace).parse(" a");
  REQUIRE(skipmanycheck.value().first == 1);
}

TEST_CASE("Inside Brackets Parser") {
  auto inside_parens = Parens(PosNum).parse("(12)");
  REQUIRE(inside_parens.has_value());
  REQUIRE(inside_parens.value().first == 12);
  REQUIRE(inside_parens.value().second == "");

  auto inside_curlies = Curlies(PosNum).parse("{12}");
  REQUIRE(inside_curlies.has_value());
  REQUIRE(inside_curlies.value().first == 12);
  REQUIRE(inside_curlies.value().second == "");
}

TEST_CASE("Sep By Checks") {
  auto sepby_check = sepBy(Char, Character(',')).parse("");
  REQUIRE(sepby_check.has_value());
  REQUIRE(sepby_check.value().first.empty());
  sepby_check = sepBy(Char, Character(',')).parse("a");
  REQUIRE(sepby_check.has_value());
  REQUIRE(sepby_check.value().first.size() == 1);
  sepby_check = sepBy(Char, Character(',')).parse("a,");
  REQUIRE(!sepby_check.has_value());
  sepby_check = sepBy(Char, Character(',')).parse("a,b");
  REQUIRE(sepby_check.has_value());
  REQUIRE(sepby_check.value().first.size() == 2);

  auto sepby1_check = sepBy1(Char, Character(',')).parse("");
  REQUIRE(!sepby1_check.has_value());
  sepby1_check = sepBy1(Char, Character(',')).parse("a");
  REQUIRE(sepby1_check.has_value());
  REQUIRE(sepby1_check.value().first.size() == 1);
  REQUIRE(sepby1_check.value().second == "");
  sepby1_check = sepBy1(Char, Character(',')).parse("a,");
  REQUIRE(!sepby1_check.has_value());
  sepby1_check = sepBy1(Char, Character(',')).parse("a,b");
  REQUIRE(sepby1_check.has_value());
  REQUIRE(sepby1_check.value().first.size() == 2);
  REQUIRE(sepby1_check.value().second == "");
}
