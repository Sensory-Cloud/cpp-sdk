// Test cases for types and structures for the structures in sensory.
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

// --- sensory::CloudHost Unit Tests ---------------------------------

SCENARIO("A user wants to initialize a CloudHost") {
    GIVEN("a hostname, port number, and security flag") {
        std::string host = "localhost";
        uint32_t port = 443;
        bool isSecure = false;
        WHEN("a CloudHost is initialized") {
            sensory::CloudHost cloudHost{host, port, isSecure};
            THEN("the data is stored") {
                REQUIRE_THAT(host, Catch::Equals(cloudHost.host));
                REQUIRE(port == cloudHost.port);
                REQUIRE(isSecure == cloudHost.isSecure);
            }
        }
    }
}

// --- sensory::Config Unit Tests ------------------------------------

SCENARIO("A user wants to initialization a Config") {
    GIVEN("no config parameters") {
        WHEN("a Config is initialized") {
            sensory::Config config;
            THEN("the default values are stored in the struct") {
                REQUIRE(nullptr == config.getCloudHost());
                REQUIRE_THAT(config.tenantID, Catch::Equals(""));
                REQUIRE_THAT(config.deviceID, Catch::Equals(""));
                REQUIRE(16000.0 == config.audioSampleRate);
                REQUIRE(720 == config.photoHeight);
                REQUIRE(480 == config.photoWidth);
                REQUIRE(0.5 == config.getJpegCompression());
                REQUIRE_THAT(config.languageCode, Catch::Equals(""));
                REQUIRE(10 == config.grpcTimeout);
            }
        }
    }
}

SCENARIO("A user wants to use a different JPEG Quality factor") {
    GIVEN("An initialized Config object") {
        sensory::Config config;
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

SCENARIO("A user wants to create a secure connection to a secure cloud host") {
    GIVEN("An initialized Config object") {
        sensory::Config config;
        WHEN("The cloud host is set to its initial value") {
            std::string host = "localhost";
            uint32_t port = 443;
            config.setCloudHost(host, port);
            THEN("The cloud host is set") {
                REQUIRE(nullptr != config.getCloudHost());
                REQUIRE(host == config.getCloudHost()->host);
                REQUIRE(port == config.getCloudHost()->port);
                REQUIRE(true == config.getCloudHost()->isSecure);
            }
            THEN("The gRPC host-name is set") {
                REQUIRE_THAT("localhost:443", Catch::Equals(config.getCloudHost()->getGRPCHost()));
            }
        }
    }
    GIVEN("An initialized Config object with an existing connection") {
        sensory::Config config;
        config.setCloudHost("localhost", 8080);
        WHEN("The cloud host is set to its initial value") {
            std::string host = "cloud.sensory.com";
            uint32_t port = 443;
            config.setCloudHost(host, port);
            THEN("The cloud host is set") {
                REQUIRE(nullptr != config.getCloudHost());
                REQUIRE(host == config.getCloudHost()->host);
                REQUIRE(port == config.getCloudHost()->port);
            }
            THEN("The gRPC host-name is set") {
                REQUIRE_THAT("cloud.sensory.com:443", Catch::Equals(config.getCloudHost()->getGRPCHost()));
            }
        }
    }
}

SCENARIO("A user wants to create an insecure connection to a secure cloud host") {
    GIVEN("An initialized Config object") {
        sensory::Config config;
        WHEN("The cloud host is set to its initial value") {
            std::string host = "localhost";
            uint32_t port = 443;
            config.setInsecureCloudHost(host, port);
            THEN("The cloud host is set") {
                REQUIRE(nullptr != config.getCloudHost());
                REQUIRE(host == config.getCloudHost()->host);
                REQUIRE(port == config.getCloudHost()->port);
                REQUIRE(false == config.getCloudHost()->isSecure);
            }
            THEN("The gRPC host-name is set") {
                REQUIRE_THAT("localhost:443", Catch::Equals(config.getCloudHost()->getGRPCHost()));
            }
        }
    }
    GIVEN("An initialized Config object with an existing connection") {
        sensory::Config config;
        config.setInsecureCloudHost("localhost", 8080);
        WHEN("The cloud host is set to its initial value") {
            std::string host = "cloud.sensory.com";
            uint32_t port = 443;
            config.setCloudHost(host, port);
            THEN("The cloud host is set") {
                REQUIRE(nullptr != config.getCloudHost());
                REQUIRE(host == config.getCloudHost()->host);
                REQUIRE(port == config.getCloudHost()->port);
            }
            THEN("The gRPC host-name is set") {
                REQUIRE_THAT("cloud.sensory.com:443", Catch::Equals(config.getCloudHost()->getGRPCHost()));
            }
        }
    }
}
