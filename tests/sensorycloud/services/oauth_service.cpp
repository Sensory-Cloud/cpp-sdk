// Test cases for the OAuth service.
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
#include "sensorycloud/services/oauth_service.hpp"
#include "sensorycloud/generated/oauth/oauth_mock.grpc.pb.h"
#include "sensorycloud/generated/v1/management/device_mock.grpc.pb.h"

using ::grpc::ClientContext;
using ::grpc::Status;

using ::sensory::Config;
using ::sensory::service::OAuthService;

using ::sensory::api::common::TokenResponse;
using ::sensory::api::oauth::PublicKeyRequest;
using ::sensory::api::oauth::PublicKeyResponse;
using ::sensory::api::oauth::SignTokenRequest;
using ::sensory::api::oauth::TokenRequest;
using ::sensory::api::oauth::WhoAmIRequest;
using ::sensory::api::oauth::WhoAmIResponse;
using ::sensory::api::v1::management::DeviceResponse;
using ::sensory::api::v1::management::EnrollDeviceRequest;
using ::sensory::api::v1::management::RenewDeviceCredentialRequest;

using testing::_;

TEST_CASE("Should create OAuthService from Config") {
    // Create the configuration that provides information about the remote host.
    Config config("hostname.com", 443, "tenant ID", "device ID");
    // Create the OAuth service for requesting and managing OAuth tokens through
    // a token manager instance.
    OAuthService service(config);
}

SCENARIO("A client requires a synchronous interface to the video service") {
    GIVEN("An initialized video service.") {
        // Create the configuration that provides information about the remote host.
        Config config("hostname.com", 443, "tenant ID", "device ID", false);
        // Create the service with the mock stubs.
        auto device_stub = new ::sensory::api::v1::management::MockDeviceServiceStub;
        auto oauth_stub = new ::sensory::api::oauth::MockOauthServiceStub;
        OAuthService service(config, device_stub, oauth_stub);

        // ----- EnrollDevice --------------------------------------------------

        WHEN("EnrollDevice is called") {
            EXPECT_CALL(*device_stub, EnrollDevice(_, _, _))
                .Times(1)
                .WillOnce([] (ClientContext*, const EnrollDeviceRequest& request, DeviceResponse *response) {
                    REQUIRE("device ID" == request.deviceid());
                    REQUIRE("tenant ID" == request.tenantid());
                    REQUIRE("foo name" == request.name());
                    REQUIRE("foo credential" == request.credential());
                    REQUIRE("foo client ID" == request.client().clientid());
                    REQUIRE("foo client secret" == request.client().secret());
                    response->set_name("response name");
                    response->set_deviceid("response ID");
                    return Status::OK;
                }
            );
            DeviceResponse response;
            auto status = service.register_device(&response, "foo name", "foo credential", "foo client ID", "foo client secret");
            THEN("The status is OK") {
                REQUIRE(status.ok());
            }
            THEN("the response contains the updated data") {
                REQUIRE("response name" == response.name());
                REQUIRE("response ID" == response.deviceid());
            }
        }

        // ----- RenewDeviceCredential -----------------------------------------

        WHEN("RenewDeviceCredential is called") {
            EXPECT_CALL(*device_stub, RenewDeviceCredential(_, _, _))
                .Times(1)
                .WillOnce([] (ClientContext*, const RenewDeviceCredentialRequest& request, DeviceResponse *response) {
                    REQUIRE("device ID" == request.deviceid());
                    REQUIRE("tenant ID" == request.tenantid());
                    REQUIRE("foo credential" == request.credential());
                    REQUIRE("foo client ID" == request.clientid());
                    response->set_name("response name");
                    response->set_deviceid("response ID");
                    return Status::OK;
                }
            );
            DeviceResponse response;
            auto status = service.renew_device_credential(&response, "foo credential", "foo client ID");
            THEN("The status is OK") {
                REQUIRE(status.ok());
            }
            THEN("the response contains the updated data") {
                REQUIRE("response name" == response.name());
                REQUIRE("response ID" == response.deviceid());
            }
        }

        // ----- GetToken ------------------------------------------------------

        WHEN("GetToken is called") {
            EXPECT_CALL(*oauth_stub, GetToken(_, _, _))
                .Times(1)
                .WillOnce([] (ClientContext*, const TokenRequest& request, TokenResponse *response) {
                    REQUIRE("foo client ID" == request.clientid());
                    REQUIRE("foo client secret" == request.secret());
                    response->set_accesstoken("response access token");
                    return Status::OK;
                }
            );
            TokenResponse response;
            auto status = service.get_token(&response, "foo client ID", "foo client secret");
            THEN("The status is OK") {
                REQUIRE(status.ok());
            }
            THEN("the response contains the updated data") {
                REQUIRE("response access token" == response.accesstoken());
            }
        }
    }
}
