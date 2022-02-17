// Test cases for the management service.
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
#include "sensorycloud/services/management_service.hpp"
#include "sensorycloud/services/oauth_service.hpp"
#include "sensorycloud/token_manager/token_manager.hpp"
#include "sensorycloud/token_manager/in_memory_credential_store.hpp"
#include "sensorycloud/generated/v1/management/enrollment_mock.grpc.pb.h"

using ::grpc::ClientContext;
using ::grpc::Status;

using ::sensory::Config;
using ::sensory::token_manager::TokenManager;
using ::sensory::token_manager::InMemoryCredentialStore;
using ::sensory::service::OAuthService;
using ::sensory::service::ManagementService;

using ::sensory::api::v1::management::GetEnrollmentsRequest;
using ::sensory::api::v1::management::GetEnrollmentsResponse;
using ::sensory::api::v1::management::EnrollmentResponse;

using testing::_;

TEST_CASE("Should create ManagementService from Config and TokenManager") {
    // Create the configuration that provides information about the remote host.
    Config config("hostname.com", 443, "tenant ID", "device ID");
    config.connect();
    // Create the OAuth service for requesting and managing OAuth tokens through
    // a token manager instance.
    OAuthService oauthService(config);
    // Create a credential store for keeping the clientID, clientSecret,
    // token, and expiration time.
    InMemoryCredentialStore keychain;
    TokenManager<InMemoryCredentialStore> tokenManager(oauthService, keychain);
    // Create the management service from the config and token manager.
    ManagementService<InMemoryCredentialStore> service(config, tokenManager);
}

SCENARIO("A client requires a synchronous interface to the management service") {
    GIVEN("An initialized management service.") {
        // Create the configuration that provides information about the remote host.
        Config config("hostname.com", 443, "tenant ID", "device ID", false);
        config.connect();
        // Create the OAuth service for requesting and managing OAuth tokens through
        // a token manager instance.
        OAuthService oauthService(config);
        // Create a credential store for keeping the clientID, clientSecret,
        // token, and expiration time.
        InMemoryCredentialStore keychain;
        TokenManager<InMemoryCredentialStore> tokenManager(oauthService, keychain);
        // Create the management service from the config and token manager, but
        // use a GMock stub instead of the default constructor.
        auto stub = new ::sensory::api::v1::management::MockEnrollmentServiceStub;
        ManagementService<InMemoryCredentialStore> service(config, tokenManager, stub);
        WHEN("GetEnrollments is called") {
            // Expect the GetEnrollments call and setup the expected response.
            EXPECT_CALL(*stub, GetEnrollments(_, _, _))
                .Times(1)
                .WillOnce([] (ClientContext*, const GetEnrollmentsRequest& request, GetEnrollmentsResponse *response) {
                    REQUIRE(request.userid() == "foo user");
                    auto enrollment = response->add_enrollments();
                    enrollment->set_id("foo ID");
                    enrollment->set_description("foo description");
                    response->set_isrequestortrusted(false);
                    return Status::OK;
                }
            );
            // Make the call that will update this locally scoped response.
            GetEnrollmentsResponse response;
            auto status = service.getEnrollments(&response, "foo user");
            THEN("The status is OK") {
                REQUIRE(status.ok());
            }
            THEN("the list of enrollments contains the enrollment") {
                REQUIRE(1 == response.enrollments_size());
                auto& enrollment = response.enrollments(0);
                REQUIRE("foo ID" == enrollment.id());
                REQUIRE("foo description" == enrollment.description());
            }
            THEN("the `isRequestorTrusted` flag is False") {
                REQUIRE_FALSE(response.isrequestortrusted());
            }
        }
    }
}
