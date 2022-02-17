// Test cases for the health service.
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
#include "sensorycloud/services/health_service.hpp"
#include "sensorycloud/generated/health/health_mock.grpc.pb.h"

using ::grpc::ClientContext;
using ::grpc::Status;

using ::sensory::Config;
using ::sensory::service::HealthService;
using ::sensory::api::health::HealthRequest;
using ::sensory::api::common::ServerHealthResponse;

using testing::_;

TEST_CASE("Should create HealthService from Config") {
    // Create the configuration that provides information about the remote host.
    Config config("hostname.com", 443, "tenant ID", "device ID");
    config.connect();
    // Create the health service.
    HealthService service(config);
}

SCENARIO("A client requires a synchronous interface to the management service") {
    GIVEN("An initialized management service.") {
        // Create the configuration that provides information about the remote host.
        Config config("hostname.com", 443, "tenant ID", "device ID", false);
        config.connect();
        auto stub = new ::sensory::api::health::MockHealthServiceStub;
        HealthService service(config, stub);

        // ----- GetHealth -----------------------------------------------------

        WHEN("GetHealth is called") {
            EXPECT_CALL(*stub, GetHealth(_, _, _))
                .Times(1)
                .WillOnce([] (ClientContext*, const HealthRequest& request, ServerHealthResponse *response) {
                    response->set_ishealthy(true);
                    response->set_serverversion("0.0.0");
                    response->set_id("response ID");
                    return Status::OK;
                }
            );
            ServerHealthResponse response;
            auto status = service.getHealth(&response);
            THEN("The status is OK") {
                REQUIRE(status.ok());
            }
            THEN("the response contains the updated data") {
                REQUIRE(response.ishealthy());
                REQUIRE("0.0.0" == response.serverversion());
                REQUIRE("response ID" == response.id());
            }
        }
    }
}
