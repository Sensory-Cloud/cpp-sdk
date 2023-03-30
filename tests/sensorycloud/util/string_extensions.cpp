// Test cases for C++11 <string> header extensions.
//
// Copyright (c) 2023 Sensory, Inc.
//
// Author: Christian Kauten (ckauten@sensoryinc.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "sensorycloud/util/string_extensions.hpp"

using sensory::util::lstrip;
using sensory::util::rstrip;
using sensory::util::strip;

SCENARIO("Empty strings get stripped") {
    GIVEN("a string that is already stripped") {
        WHEN("the string is left stripped") {
            THEN("the output string is empty") {
                REQUIRE_THAT(lstrip(""), Catch::Equals(""));
            }
        }
        WHEN("the string is right stripped") {
            THEN("the output string is empty") {
                REQUIRE_THAT(rstrip(""), Catch::Equals(""));
            }
        }
        WHEN("the string is left+right stripped") {
            THEN("the output string is empty") {
                REQUIRE_THAT(strip(""), Catch::Equals(""));
            }
        }
    }
}

SCENARIO("Strings that are already normalized get stripped") {
    GIVEN("a string that is already stripped") {
        const std::string str = "foo";
        WHEN("the string is left stripped") {
            THEN("the output string is the input string") {
                REQUIRE_THAT(lstrip(str), Catch::Equals(str));
            }
        }
        WHEN("the string is right stripped") {
            THEN("the output string is the input string") {
                REQUIRE_THAT(rstrip(str), Catch::Equals(str));
            }
        }
        WHEN("the string is left+right stripped") {
            THEN("the output string is the input string") {
                REQUIRE_THAT(strip(str), Catch::Equals(str));
            }
        }
    }
}

SCENARIO("White-space needs to be stripped from strings") {
    GIVEN("a string with entirely white-space") {
        WHEN("the string is left stripped") {
            THEN("the output string is empty") {
                REQUIRE(lstrip(" ").empty());
                REQUIRE(lstrip("  ").empty());
            }
        }
        WHEN("the string is right stripped") {
            THEN("the output string is empty") {
                REQUIRE(rstrip(" ").empty());
                REQUIRE(rstrip("  ").empty());
            }
        }
        WHEN("the string is left+right stripped") {
            THEN("the output string is empty") {
                REQUIRE(strip(" ").empty());
                REQUIRE(strip("  ").empty());
            }
        }
    }
    GIVEN("a string with leading white-space") {
        WHEN("the string is left stripped") {
            THEN("the output string has no leading white-space") {
                REQUIRE_THAT(lstrip(" foo"), Catch::Equals("foo"));
                REQUIRE_THAT(lstrip("  foo"), Catch::Equals("foo"));
            }
        }
        WHEN("the string is right stripped") {
            THEN("the output string has no trailing white-space") {
                REQUIRE_THAT(rstrip(" foo"), Catch::Equals(" foo"));
                REQUIRE_THAT(rstrip("  foo"), Catch::Equals("  foo"));
            }
        }
        WHEN("the string is left+right stripped") {
            THEN("the output string has no leading or trailing white-space") {
                REQUIRE_THAT(strip(" foo"), Catch::Equals("foo"));
                REQUIRE_THAT(strip("  foo"), Catch::Equals("foo"));
            }
        }
    }
    GIVEN("a string with trailing white-space") {
        WHEN("the string is left stripped") {
            THEN("the output string has no leading white-space") {
                REQUIRE_THAT(lstrip("foo "), Catch::Equals("foo "));
                REQUIRE_THAT(lstrip("foo  "), Catch::Equals("foo  "));
            }
        }
        WHEN("the string is right stripped") {
            THEN("the output string has no trailing white-space") {
                REQUIRE_THAT(rstrip("foo "), Catch::Equals("foo"));
                REQUIRE_THAT(rstrip("foo  "), Catch::Equals("foo"));
            }
        }
        WHEN("the string is left+right stripped") {
            THEN("the output string has no leading or trailing white-space") {
                REQUIRE_THAT(strip("foo "), Catch::Equals("foo"));
                REQUIRE_THAT(strip("foo  "), Catch::Equals("foo"));
            }
        }
    }
}

SCENARIO("Horizontal tabs need to be stripped from strings") {
    GIVEN("a string with entirely tabs") {
        WHEN("the string is left stripped") {
            THEN("the output string is empty") {
                REQUIRE(lstrip("\t").empty());
                REQUIRE(lstrip("\t\t").empty());
            }
        }
        WHEN("the string is right stripped") {
            THEN("the output string is empty") {
                REQUIRE(rstrip("\t").empty());
                REQUIRE(rstrip("\t\t").empty());
            }
        }
        WHEN("the string is left+right stripped") {
            THEN("the output string is empty") {
                REQUIRE(strip("\t").empty());
                REQUIRE(strip("\t\t").empty());
            }
        }
    }
    GIVEN("a string with leading tabs") {
        WHEN("the string is left stripped") {
            THEN("the output string has no leading tabs") {
                REQUIRE_THAT(lstrip("\tfoo"), Catch::Equals("foo"));
                REQUIRE_THAT(lstrip("\t\tfoo"), Catch::Equals("foo"));
            }
        }
        WHEN("the string is right stripped") {
            THEN("the output string has no trailing tabs") {
                REQUIRE_THAT(rstrip("\tfoo"), Catch::Equals("\tfoo"));
                REQUIRE_THAT(rstrip("\t\tfoo"), Catch::Equals("\t\tfoo"));
            }
        }
        WHEN("the string is left+right stripped") {
            THEN("the output string has no leading or trailing tabs") {
                REQUIRE_THAT(strip("\tfoo"), Catch::Equals("foo"));
                REQUIRE_THAT(strip("\t\tfoo"), Catch::Equals("foo"));
            }
        }
    }
    GIVEN("a string with trailing tabs") {
        WHEN("the string is left stripped") {
            THEN("the output string has no leading tabs") {
                REQUIRE_THAT(lstrip("foo\t"), Catch::Equals("foo\t"));
                REQUIRE_THAT(lstrip("foo\t\t"), Catch::Equals("foo\t\t"));
            }
        }
        WHEN("the string is right stripped") {
            THEN("the output string has no trailing tabs") {
                REQUIRE_THAT(rstrip("foo\t"), Catch::Equals("foo"));
                REQUIRE_THAT(rstrip("foo\t\t"), Catch::Equals("foo"));
            }
        }
        WHEN("the string is left+right stripped") {
            THEN("the output string has no leading or trailing tabs") {
                REQUIRE_THAT(strip("foo\t"), Catch::Equals("foo"));
                REQUIRE_THAT(strip("foo\t\t"), Catch::Equals("foo"));
            }
        }
    }
}

SCENARIO("Vertical tabs need to be stripped from strings") {
    GIVEN("a string with entirely tabs") {
        WHEN("the string is left stripped") {
            THEN("the output string is empty") {
                REQUIRE(lstrip("\v").empty());
                REQUIRE(lstrip("\v\v").empty());
            }
        }
        WHEN("the string is right stripped") {
            THEN("the output string is empty") {
                REQUIRE(rstrip("\v").empty());
                REQUIRE(rstrip("\v\v").empty());
            }
        }
        WHEN("the string is left+right stripped") {
            THEN("the output string is empty") {
                REQUIRE(strip("\v").empty());
                REQUIRE(strip("\v\v").empty());
            }
        }
    }
    GIVEN("a string with leading tabs") {
        WHEN("the string is left stripped") {
            THEN("the output string has no leading tabs") {
                REQUIRE_THAT(lstrip("\vfoo"), Catch::Equals("foo"));
                REQUIRE_THAT(lstrip("\v\vfoo"), Catch::Equals("foo"));
            }
        }
        WHEN("the string is right stripped") {
            THEN("the output string has no trailing tabs") {
                REQUIRE_THAT(rstrip("\vfoo"), Catch::Equals("\vfoo"));
                REQUIRE_THAT(rstrip("\v\vfoo"), Catch::Equals("\v\vfoo"));
            }
        }
        WHEN("the string is left+right stripped") {
            THEN("the output string has no leading or trailing tabs") {
                REQUIRE_THAT(strip("\vfoo"), Catch::Equals("foo"));
                REQUIRE_THAT(strip("\v\vfoo"), Catch::Equals("foo"));
            }
        }
    }
    GIVEN("a string with trailing tabs") {
        WHEN("the string is left stripped") {
            THEN("the output string has no leading tabs") {
                REQUIRE_THAT(lstrip("foo\v"), Catch::Equals("foo\v"));
                REQUIRE_THAT(lstrip("foo\v\v"), Catch::Equals("foo\v\v"));
            }
        }
        WHEN("the string is right stripped") {
            THEN("the output string has no trailing tabs") {
                REQUIRE_THAT(rstrip("foo\v"), Catch::Equals("foo"));
                REQUIRE_THAT(rstrip("foo\v\v"), Catch::Equals("foo"));
            }
        }
        WHEN("the string is left+right stripped") {
            THEN("the output string has no leading or trailing tabs") {
                REQUIRE_THAT(strip("foo\v"), Catch::Equals("foo"));
                REQUIRE_THAT(strip("foo\v\v"), Catch::Equals("foo"));
            }
        }
    }
}

SCENARIO("Carriage-returns need to be stripped from strings") {
    GIVEN("a string with entirely carriage-returns") {
        WHEN("the string is left stripped") {
            THEN("the output string is empty") {
                REQUIRE(lstrip("\r").empty());
                REQUIRE(lstrip("\r\r").empty());
            }
        }
        WHEN("the string is right stripped") {
            THEN("the output string is empty") {
                REQUIRE(rstrip("\r").empty());
                REQUIRE(rstrip("\r\r").empty());
            }
        }
        WHEN("the string is left+right stripped") {
            THEN("the output string is empty") {
                REQUIRE(strip("\r").empty());
                REQUIRE(strip("\r\r").empty());
            }
        }
    }
    GIVEN("a string with leading carriage-returns") {
        WHEN("the string is left stripped") {
            THEN("the output string has no leading carriage-returns") {
                REQUIRE_THAT(lstrip("\rfoo"), Catch::Equals("foo"));
                REQUIRE_THAT(lstrip("\r\rfoo"), Catch::Equals("foo"));
            }
        }
        WHEN("the string is right stripped") {
            THEN("the output string has no trailing carriage-returns") {
                REQUIRE_THAT(rstrip("\rfoo"), Catch::Equals("\rfoo"));
                REQUIRE_THAT(rstrip("\r\rfoo"), Catch::Equals("\r\rfoo"));
            }
        }
        WHEN("the string is left+right stripped") {
            THEN("the output string has no leading or trailing carriage-returns") {
                REQUIRE_THAT(strip("\rfoo"), Catch::Equals("foo"));
                REQUIRE_THAT(strip("\r\rfoo"), Catch::Equals("foo"));
            }
        }
    }
    GIVEN("a string with trailing carriage-returns") {
        WHEN("the string is left stripped") {
            THEN("the output string has no leading carriage-returns") {
                REQUIRE_THAT(lstrip("foo\r"), Catch::Equals("foo\r"));
                REQUIRE_THAT(lstrip("foo\r\r"), Catch::Equals("foo\r\r"));
            }
        }
        WHEN("the string is right stripped") {
            THEN("the output string has no trailing carriage-returns") {
                REQUIRE_THAT(rstrip("foo\r"), Catch::Equals("foo"));
                REQUIRE_THAT(rstrip("foo\r\r"), Catch::Equals("foo"));
            }
        }
        WHEN("the string is left+right stripped") {
            THEN("the output string has no leading or trailing carriage-returns") {
                REQUIRE_THAT(strip("foo\r"), Catch::Equals("foo"));
                REQUIRE_THAT(strip("foo\r\r"), Catch::Equals("foo"));
            }
        }
    }
}

SCENARIO("Form-feeds need to be stripped from strings") {
    GIVEN("a string with entirely form-feeds") {
        WHEN("the string is left stripped") {
            THEN("the output string is empty") {
                REQUIRE(lstrip("\f").empty());
                REQUIRE(lstrip("\f\f").empty());
            }
        }
        WHEN("the string is right stripped") {
            THEN("the output string is empty") {
                REQUIRE(rstrip("\f").empty());
                REQUIRE(rstrip("\f\f").empty());
            }
        }
        WHEN("the string is left+right stripped") {
            THEN("the output string is empty") {
                REQUIRE(strip("\f").empty());
                REQUIRE(strip("\f\f").empty());
            }
        }
    }
    GIVEN("a string with leading form-feeds") {
        WHEN("the string is left stripped") {
            THEN("the output string has no leading form-feeds") {
                REQUIRE_THAT(lstrip("\ffoo"), Catch::Equals("foo"));
                REQUIRE_THAT(lstrip("\f\ffoo"), Catch::Equals("foo"));
            }
        }
        WHEN("the string is right stripped") {
            THEN("the output string has no trailing form-feeds") {
                REQUIRE_THAT(rstrip("\ffoo"), Catch::Equals("\ffoo"));
                REQUIRE_THAT(rstrip("\f\ffoo"), Catch::Equals("\f\ffoo"));
            }
        }
        WHEN("the string is left+right stripped") {
            THEN("the output string has no leading or trailing form-feeds") {
                REQUIRE_THAT(strip("\ffoo"), Catch::Equals("foo"));
                REQUIRE_THAT(strip("\f\ffoo"), Catch::Equals("foo"));
            }
        }
    }
    GIVEN("a string with trailing form-feeds") {
        WHEN("the string is left stripped") {
            THEN("the output string has no leading form-feeds") {
                REQUIRE_THAT(lstrip("foo\f"), Catch::Equals("foo\f"));
                REQUIRE_THAT(lstrip("foo\f\f"), Catch::Equals("foo\f\f"));
            }
        }
        WHEN("the string is right stripped") {
            THEN("the output string has no trailing form-feeds") {
                REQUIRE_THAT(rstrip("foo\f"), Catch::Equals("foo"));
                REQUIRE_THAT(rstrip("foo\f\f"), Catch::Equals("foo"));
            }
        }
        WHEN("the string is left+right stripped") {
            THEN("the output string has no leading or trailing form-feeds") {
                REQUIRE_THAT(strip("foo\f"), Catch::Equals("foo"));
                REQUIRE_THAT(strip("foo\f\f"), Catch::Equals("foo"));
            }
        }
    }
}

SCENARIO("New-lines need to be stripped from strings") {
    GIVEN("a string with entirely new-lines") {
        WHEN("the string is left stripped") {
            THEN("the output string is empty") {
                REQUIRE(lstrip("\n").empty());
                REQUIRE(lstrip("\n\n").empty());
            }
        }
        WHEN("the string is right stripped") {
            THEN("the output string is empty") {
                REQUIRE(rstrip("\n").empty());
                REQUIRE(rstrip("\n\n").empty());
            }
        }
        WHEN("the string is left+right stripped") {
            THEN("the output string is empty") {
                REQUIRE(strip("\n").empty());
                REQUIRE(strip("\n\n").empty());
            }
        }
    }
    GIVEN("a string with leading new-lines") {
        WHEN("the string is left stripped") {
            THEN("the output string has no leading new-lines") {
                REQUIRE_THAT(lstrip("\nfoo"), Catch::Equals("foo"));
                REQUIRE_THAT(lstrip("\n\nfoo"), Catch::Equals("foo"));
            }
        }
        WHEN("the string is right stripped") {
            THEN("the output string has no trailing new-lines") {
                REQUIRE_THAT(rstrip("\nfoo"), Catch::Equals("\nfoo"));
                REQUIRE_THAT(rstrip("\n\nfoo"), Catch::Equals("\n\nfoo"));
            }
        }
        WHEN("the string is left+right stripped") {
            THEN("the output string has no leading or trailing new-lines") {
                REQUIRE_THAT(strip("\nfoo"), Catch::Equals("foo"));
                REQUIRE_THAT(strip("\n\nfoo"), Catch::Equals("foo"));
            }
        }
    }
    GIVEN("a string with trailing new-lines") {
        WHEN("the string is left stripped") {
            THEN("the output string has no leading new-lines") {
                REQUIRE_THAT(lstrip("foo\n"), Catch::Equals("foo\n"));
                REQUIRE_THAT(lstrip("foo\n\n"), Catch::Equals("foo\n\n"));
            }
        }
        WHEN("the string is right stripped") {
            THEN("the output string has no trailing new-lines") {
                REQUIRE_THAT(rstrip("foo\n"), Catch::Equals("foo"));
                REQUIRE_THAT(rstrip("foo\n\n"), Catch::Equals("foo"));
            }
        }
        WHEN("the string is left+right stripped") {
            THEN("the output string has no leading or trailing new-lines") {
                REQUIRE_THAT(strip("foo\n"), Catch::Equals("foo"));
                REQUIRE_THAT(strip("foo\n\n"), Catch::Equals("foo"));
            }
        }
    }
}

SCENARIO("A combination of characters needs to be stripped from strings") {
    GIVEN("a string with entirely strip-able characters") {
        WHEN("the string is left stripped") {
            THEN("the output string is empty") {
                REQUIRE(lstrip("\n\t\r\f\v ").empty());
            }
        }
        WHEN("the string is right stripped") {
            THEN("the output string is empty") {
                REQUIRE(rstrip("\n\t\r\f\v ").empty());
            }
        }
        WHEN("the string is left+right stripped") {
            THEN("the output string is empty") {
                REQUIRE(strip("\n\t\r\f\v ").empty());
            }
        }
    }
    GIVEN("a string with leading characters") {
        WHEN("the string is left stripped") {
            THEN("the output string has no leading characters") {
                REQUIRE_THAT(lstrip("\n\t\r\f\v foo"), Catch::Equals("foo"));
            }
        }
        WHEN("the string is right stripped") {
            THEN("the output string has no trailing characters") {
                REQUIRE_THAT(rstrip("\n\t\r\f\v foo"), Catch::Equals("\n\t\r\f\v foo"));
            }
        }
        WHEN("the string is left+right stripped") {
            THEN("the output string has no leading or trailing characters") {
                REQUIRE_THAT(strip("\n\t\r\f\v foo"), Catch::Equals("foo"));
            }
        }
    }
    GIVEN("a string with trailing characters") {
        WHEN("the string is left stripped") {
            THEN("the output string has no leading characters") {
                REQUIRE_THAT(lstrip("foo\n\t\r\f\v "), Catch::Equals("foo\n\t\r\f\v "));
            }
        }
        WHEN("the string is right stripped") {
            THEN("the output string has no trailing characters") {
                REQUIRE_THAT(rstrip("foo\n\t\r\f\v "), Catch::Equals("foo"));
            }
        }
        WHEN("the string is left+right stripped") {
            THEN("the output string has no leading or trailing characters") {
                REQUIRE_THAT(strip("foo\n\t\r\f\v "), Catch::Equals("foo"));
            }
        }
    }
}

SCENARIO("Strip-able characters reside within a valid range") {
    GIVEN("a string with embedded strip-able characters") {
        const auto str = "foo\n\t\r\f\v bar";
        WHEN("the string is left stripped") {
            THEN("the inputs and outputs match") {
                REQUIRE_THAT(lstrip(str), Catch::Equals(str));
            }
        }
        WHEN("the string is right stripped") {
            THEN("the inputs and outputs match") {
                REQUIRE_THAT(rstrip(str), Catch::Equals(str));
            }
        }
        WHEN("the string is left+right stripped") {
            THEN("the inputs and outputs match") {
                REQUIRE_THAT(strip(str), Catch::Equals(str));
            }
        }
    }
}
