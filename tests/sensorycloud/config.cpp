// Test cases for SDK configuration structures in the sensory namespace.
//
// Author: Christian Kauten (ckauten@sensoryinc.com)
//
// Copyright (c) 2021 Sensory, Inc.
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
#include "sensorycloud/config.hpp"

SCENARIO("A user wants to initialize a Config") {
    GIVEN("a hostname, port number, and security flag") {
        const std::string host = "localhost";
        const uint32_t port = 50051;
        WHEN("a Config is initialized") {
            const sensory::Config config(host, port);
            THEN("the host data is stored") {
                REQUIRE_THAT(host, Catch::Equals(config.getHost()));
                REQUIRE(port == config.getPort());
            }
            THEN("the connection is secure by default") {
                REQUIRE(config.getIsSecure());
            }
            THEN("the fully qualified domain name is correct") {
                const auto expected = "localhost:50051";
                const auto actual = config.getFullyQualifiedDomainName();
                REQUIRE_THAT(expected, Catch::Equals(actual));
            }
            THEN ("the device and tenant information is empty") {
                REQUIRE_THAT(config.tenantID, Catch::Equals(""));
                REQUIRE_THAT(config.deviceID, Catch::Equals(""));
            }
            THEN("the default gRPC timeout is 10 seconds") {
                REQUIRE(10 == config.getTimeout());
            }
            THEN("the default JPEG setting is 0.5") {
                REQUIRE(0.5 == config.getJpegCompression());
            }
            THEN("the default audio sample rate is 16000kHz") {
                REQUIRE(16000.0 == config.audioSampleRate);
            }
            THEN("the default image dimensions are 480x720") {
                REQUIRE(720 == config.photoHeight);
                REQUIRE(480 == config.photoWidth);
            }
            THEN("the language code is english (US") {
                REQUIRE_THAT(config.languageCode, Catch::Equals("en-US"));
            }
        }
    }
}

SCENARIO("A user wants to change the gRPC timeout") {
    GIVEN("an initialized cloud host") {
        sensory::Config config("localhost", 50051);
        WHEN("the gRPC timeout is set") {
            const uint32_t timeout = 50;
            config.setTimeout(timeout);
            THEN("the gRPC timeout is stored") {
                REQUIRE(timeout == config.getTimeout());
            }
        }
    }
}

SCENARIO("A user wants to use a different JPEG Quality factor") {
    GIVEN("An initialized Config object") {
        sensory::Config config("localhost", 50051);
        WHEN("The JPEG Quality factor is set to an arbitrary value") {
            config.setJpegCompression(0.25);
            THEN("The JPEG Quality factor is set to an arbitrary value") {
                REQUIRE(0.25 == config.getJpegCompression());
            }
        }
        WHEN("The JPEG Quality factor is set below the minimum value") {
            config.setJpegCompression(-1.0);
            THEN("The JPEG Quality factor is set to the minimum value") {
                REQUIRE(0.0 == config.getJpegCompression());
            }
        }
        WHEN("The JPEG Quality factor is set above the maximum value") {
            config.setJpegCompression(2.0);
            THEN("The JPEG Quality factor is set to the maximum value") {
                REQUIRE(1.0 == config.getJpegCompression());
            }
        }
    }
}

SCENARIO("A user wants to control the security of the connection to a cloud host") {
    WHEN("The cloud host is initialized with isSecure=true") {
        sensory::Config config("localhost", 50051, true);
        THEN("The cloud host connection is secured") {
            REQUIRE(config.getIsSecure());
        }
    }
    WHEN("The cloud host is initialized with isSecure=false") {
        sensory::Config config("localhost", 50051, false);
        THEN("The cloud host connection is not secured") {
            REQUIRE_FALSE(config.getIsSecure());
        }
    }
}

SCENARIO("A user wants to determine if a configured connection is valid") {
    GIVEN("An initialized config with default values") {
        sensory::Config config("localhost", 50051);
        WHEN("the validity of the configuration is queried") {
            THEN("the validity is reported as invalid") {
                REQUIRE_FALSE(config.isValid());
            }
        }
        WHEN("a tenant ID is assigned to the configuration with no device ID") {
            config.tenantID = "foo";
            THEN("the validity is reported as invalid") {
                REQUIRE_FALSE(config.isValid());
            }
        }
        WHEN("a device ID is assigned to the configuration with no tenant ID") {
            config.deviceID = "bar";
            THEN("the validity is reported as invalid") {
                REQUIRE_FALSE(config.isValid());
            }
        }
        WHEN("both tenant ID and device ID are assigned to the configuration") {
            config.tenantID = "foo";
            config.deviceID = "bar";
            THEN("the validity is reported as valid") {
                REQUIRE(config.isValid());
            }
        }
    }
}
