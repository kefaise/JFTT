#include <iostream>

#include "Fixture.hpp"

#include "catch.hpp"
#include "interpreter.hpp"

#include "logging.h"

TEST_CASE("simple increment", "While")
{
  std::string program = R"DELIM(
VAR
  a
BEGIN
  a := 1;
  WHILE a < 10 DO
    WRITE a;
    a := a + 1;
  ENDWHILE
END
)DELIM";

  auto compiled = Fixture().compile(program);

  std::istringstream compiledFile(compiled);
  std::ostringstream stdOut;
  std::istringstream stdIn;
  //std::ostream& out, std::istream& programStdIn, std::ostream& programStdOut
  std::ostringstream programOutput;
  auto result = interpret(compiledFile, stdOut, stdIn, programOutput);

  LOG("program out:\n" << programOutput.str() << "=====");

  REQUIRE(result > 0);
  Fixture().checkOutput(programOutput, {1, 2, 3, 4, 5, 6, 7, 8, 9});
}