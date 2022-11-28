// Test cases for SDK configuration structures in the sensory namespace.
//
// Copyright (c) 2022 Sensory, Inc.
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
#include "sensorycloud/config.hpp"

SCENARIO("A user wants to initialize a Config with a fully qualified domain name") {
    GIVEN("a fully qualified domain name and security flag") {
        const std::string fqdn = "localhost:50051";
        const std::string tenant_id = "tenant_id";
        const std::string client_id = "client_id";
        WHEN("a Config is initialized") {
            const sensory::Config config(fqdn, tenant_id, client_id);
            THEN("the data is stored") {
                REQUIRE_THAT(fqdn, Catch::Equals(config.get_fully_qualified_domain_name()));
                REQUIRE_THAT(config.get_tenant_id(), Catch::Equals(tenant_id));
                REQUIRE_THAT(config.get_device_id(), Catch::Equals(client_id));
            }
            THEN("the connection is secure by default") {
                REQUIRE(config.get_is_secure());
            }
            THEN("the default gRPC timeout is 10 seconds (10 thousand ms)") {
                REQUIRE(10 * 1000 == config.get_timeout());
            }
            THEN("the host and port should be accessible") {
                REQUIRE_THAT(config.get_host(), Catch::Equals("localhost"));
                REQUIRE(config.get_port() == 50051);
            }
        }
    }
    GIVEN("an empty fully qualified domain name") {
        WHEN("the config is initialized") {
            THEN("an error is thrown") {
                REQUIRE_THROWS(sensory::Config("", "foo", "bar"));
            }
        }
    }
    GIVEN("port numbers ranging from 0 to 65535 (16-bit resolution)") {
        WHEN("the config is initialized at port -1") {
            THEN("an error is thrown") {
                REQUIRE_THROWS(sensory::Config("localhost:-1", "foo", "bar"));
            }
        }
        WHEN("the config is initialized at port 0") {
            auto config = sensory::Config("localhost:0", "foo", "bar");
            THEN("get_port returns 0") {
                REQUIRE(0 == config.get_port());
            }
        }
        WHEN("the config is initialized at port 1024") {
            auto config = sensory::Config("localhost:1024", "foo", "bar");
            THEN("get_port returns 1024") {
                REQUIRE(1024 == config.get_port());
            }
        }
        WHEN("the config is initialized at port 50050") {
            auto config = sensory::Config("localhost:50050", "foo", "bar");
            THEN("get_port returns 50050") {
                REQUIRE(50050 == config.get_port());
            }
        }
        WHEN("the config is initialized at port 65535") {
            auto config = sensory::Config("localhost:65535", "foo", "bar");
            THEN("get_port returns 65535") {
                REQUIRE(65535 == config.get_port());
            }
        }
        WHEN("the config is initialized at port 65536") {
            THEN("an error is thrown") {
                REQUIRE_THROWS(sensory::Config("localhost:65536", "foo", "bar"));
            }
        }
    }
    GIVEN("an improperly formatted fully qualified domain name") {
        WHEN("the config is initialized without a semicolon and port") {
            THEN("an error is thrown") {
                REQUIRE_THROWS(sensory::Config("localhost", "foo", "bar"));
            }
        }
        WHEN("the config is initialized without a semicolon and host") {
            THEN("an error is thrown") {
                REQUIRE_THROWS(sensory::Config("50051", "foo", "bar"));
            }
        }
        WHEN("the config is initialized with a semicolon but no port") {
            THEN("an error is thrown") {
                REQUIRE_THROWS(sensory::Config("localhost:", "foo", "bar"));
            }
        }
        WHEN("the config is initialized with a semicolon but no host") {
            THEN("an error is thrown") {
                REQUIRE_THROWS(sensory::Config(":50051", "foo", "bar"));
            }
        }
    }
    GIVEN("an empty tenant ID") {
        WHEN("the config is initialized") {
            THEN("an error is thrown") {
                REQUIRE_THROWS(sensory::Config("foo:50051", "", "bar"));
            }
        }
    }
    GIVEN("an empty device ID") {
        WHEN("the config is initialized") {
            THEN("an error is thrown") {
                REQUIRE_THROWS(sensory::Config("foo:50051", "bar", ""));
            }
        }
    }
}

SCENARIO("A user wants to initialize a Config with a host-port combination") {
    GIVEN("a hostname, port number, and security flag") {
        const std::string host = "localhost";
        const uint16_t port = 50051;
        const std::string tenant_id = "tenant_id";
        const std::string client_id = "client_id";
        WHEN("a Config is initialized") {
            const sensory::Config config(host, port, tenant_id, client_id);
            THEN("the data is stored") {
                REQUIRE_THAT(config.get_host(), Catch::Equals("localhost"));
                REQUIRE(config.get_port() == 50051);
                REQUIRE_THAT("localhost:50051", Catch::Equals(config.get_fully_qualified_domain_name()));
                REQUIRE_THAT(config.get_tenant_id(), Catch::Equals(tenant_id));
                REQUIRE_THAT(config.get_device_id(), Catch::Equals(client_id));
            }
            THEN("the connection is secure by default") {
                REQUIRE(config.get_is_secure());
            }
            THEN("the default gRPC timeout is 10 seconds (10 thousand ms)") {
                REQUIRE(10 * 1000 == config.get_timeout());
            }
        }
    }
    GIVEN("an empty host name") {
        WHEN("the config is initialized") {
            THEN("an error is thrown") {
                REQUIRE_THROWS(sensory::Config("", 50051, "foo", "bar"));
            }
        }
    }
}

SCENARIO("A user wants to change the gRPC timeout") {
    GIVEN("an initialized cloud host") {
        sensory::Config config("localhost:50051", "tenant_id", "deviceID");
        WHEN("the gRPC timeout is set") {
            const uint32_t timeout = 50;
            config.set_timeout(timeout);
            THEN("the gRPC timeout is stored") {
                REQUIRE(timeout == config.get_timeout());
            }
        }
    }
}

SCENARIO("A user wants to control the security of the connection to a cloud host") {
    WHEN("The cloud host is initialized with isSecure=true") {
        sensory::Config config("localhost:50051", "tenant_id", "deviceID", true);
        THEN("The cloud host connection is secured") {
            REQUIRE(config.get_is_secure());
        }
    }
    WHEN("The cloud host is initialized with isSecure=false") {
        sensory::Config config("localhost:50051", "tenant_id", "deviceID", false);
        THEN("The cloud host connection is not secured") {
            REQUIRE_FALSE(config.get_is_secure());
        }
    }
}
