// Test cases for sensorycloud/io/path.hpp
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
#include "sensorycloud/io/path.hpp"

using sensory::io::path::normalize_uri;
using sensory::io::path::is_file;

// ---------------------------------------------------------------------------
// MARK: normalize_uri
// ---------------------------------------------------------------------------

SCENARIO("URIs need to be normalized to a host(:port)? format") {
    GIVEN("an empty host-name") {
        const std::string URI = "";
        WHEN("the string is normalized") {
            THEN("the output is unchanged") {
                REQUIRE_THAT(normalize_uri(URI), Catch::Equals(URI));
            }
        }
    }
    GIVEN("an arbitrary host-name") {
        const std::string URI = "foo";
        WHEN("the string is normalized") {
            THEN("the output is unchanged") {
                REQUIRE_THAT(normalize_uri(URI), Catch::Equals(URI));
            }
        }
    }
    GIVEN("a host-name and port in host:port format") {
        const std::string URI = "foo:50051";
        WHEN("the string is normalized") {
            THEN("the output is unchanged") {
                REQUIRE_THAT(normalize_uri(URI), Catch::Equals(URI));
            }
        }
    }
    GIVEN("a URL specifying the HTTPS protocol") {
        WHEN("the string is normalized") {
            THEN("the protocol prefix is removed") {
                REQUIRE_THAT(normalize_uri("https://foo"), Catch::Equals("foo"));
            }
        }
    }
    GIVEN("a URL specifying the HTTPS protocol without a FQDN") {
        WHEN("the string is normalized") {
            THEN("an empty string is returned") {
                REQUIRE_THAT(normalize_uri("https://"), Catch::Equals(""));
            }
        }
    }
    GIVEN("a '://' string") {
        WHEN("the string is normalized") {
            THEN("an empty string is returned") {
                REQUIRE_THAT(normalize_uri("://"), Catch::Equals(""));
            }
        }
    }
    GIVEN("a URL with a protocol delimiter but no protocol") {
        WHEN("the string is normalized") {
            THEN("the host-name is returned") {
                REQUIRE_THAT(normalize_uri("://foo"), Catch::Equals("foo"));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: is_file
// ---------------------------------------------------------------------------

SCENARIO("Paths need to be checked to determine if they are files or not") {
    GIVEN("a path to a file") {
        WHEN("is_file is called") {
            THEN("true is returned") {
                REQUIRE(is_file("/bin/ls"));
            }
        }
    }
    GIVEN("a path to a directory") {
        WHEN("is_file is called") {
            THEN("false is returned") {
                REQUIRE_FALSE(is_file("/usr"));
            }
        }
    }
    GIVEN("an invalid path") {
        WHEN("is_file is called") {
            THEN("false is returned") {
                REQUIRE_FALSE(is_file("/foo/bar/zam"));

            }
        }
    }
}
