// Test cases for the management service.
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

using ::sensory::api::v1::management::AppendEnrollmentGroupRequest;
using ::sensory::api::v1::management::CreateEnrollmentGroupRequest;
using ::sensory::api::v1::management::DeleteEnrollmentGroupRequest;
using ::sensory::api::v1::management::DeleteEnrollmentRequest;
using ::sensory::api::v1::management::EnrollmentGroupResponse;
using ::sensory::api::v1::management::EnrollmentResponse;
using ::sensory::api::v1::management::GetEnrollmentGroupsResponse;
using ::sensory::api::v1::management::GetEnrollmentsRequest;
using ::sensory::api::v1::management::GetEnrollmentsResponse;

using testing::_;

TEST_CASE("Should create ManagementService from Config and TokenManager") {
    // Create the configuration that provides information about the remote host.
    Config config("hostname.com", 443, "tenant ID", "device ID");
    // Create the OAuth service for requesting and managing OAuth tokens through
    // a token manager instance.
    OAuthService oauth_service(config);
    // Create a credential store for keeping the clientID, clientSecret,
    // token, and expiration time.
    InMemoryCredentialStore keychain;
    TokenManager<InMemoryCredentialStore> token_manager(oauth_service, keychain);
    // Create the management service from the config and token manager.
    ManagementService<InMemoryCredentialStore> service(config, token_manager);
}

SCENARIO("A client requires a synchronous interface to the management service") {
    GIVEN("An initialized management service.") {
        // Create the configuration that provides information about the remote host.
        Config config("hostname.com", 443, "tenant ID", "device ID", false);
        // Create the OAuth service for requesting and managing OAuth tokens through
        // a token manager instance.
        OAuthService oauth_service(config);
        // Create a credential store for keeping the clientID, clientSecret,
        // token, and expiration time.
        InMemoryCredentialStore keychain;
        TokenManager<InMemoryCredentialStore> token_manager(oauth_service, keychain);
        // Create the management service from the config and token manager, but
        // use a GMock stub instead of the default constructor.
        auto stub = new ::sensory::api::v1::management::MockEnrollmentServiceStub;
        ManagementService<InMemoryCredentialStore> service(config, token_manager, stub);

        // ----- GetEnrollments ------------------------------------------------

        WHEN("GetEnrollments is called") {
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
            GetEnrollmentsResponse response;
            auto status = service.get_enrollments(&response, "foo user");
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

        // ----- GetEnrollmentGroups -------------------------------------------

        WHEN("GetEnrollmentGroups is called") {
            EXPECT_CALL(*stub, GetEnrollmentGroups(_, _, _))
                .Times(1)
                .WillOnce([] (ClientContext*, const GetEnrollmentsRequest& request, GetEnrollmentGroupsResponse *response) {
                    REQUIRE(request.userid() == "foo user");
                    auto enrollment_group = response->add_enrollmentgroups();
                    enrollment_group->set_id("foo ID");
                    enrollment_group->set_description("foo description");
                    return Status::OK;
                }
            );
            GetEnrollmentGroupsResponse response;
            auto status = service.get_enrollment_groups(&response, "foo user");
            THEN("The status is OK") {
                REQUIRE(status.ok());
            }
            THEN("the list of enrollments contains the enrollment group") {
                REQUIRE(1 == response.enrollmentgroups_size());
                auto& enrollment_group = response.enrollmentgroups(0);
                REQUIRE("foo ID" == enrollment_group.id());
                REQUIRE("foo description" == enrollment_group.description());
            }
        }

        // ----- CreateEnrollmentGroup -----------------------------------------

        WHEN("CreateEnrollmentGroup is called") {
            EXPECT_CALL(*stub, CreateEnrollmentGroup(_, _, _))
                .Times(1)
                .WillOnce([] (ClientContext*, const CreateEnrollmentGroupRequest& request, EnrollmentGroupResponse *response) {
                    REQUIRE(request.userid() == "foo user");
                    REQUIRE(request.id() == "foo group");
                    REQUIRE(request.name() == "foo name");
                    REQUIRE(request.description() == "foo description");
                    REQUIRE(request.modelname() == "foo model");
                    REQUIRE(1 == request.enrollmentids_size());
                    REQUIRE("enrollment ID" == request.enrollmentids(0));
                    response->set_id("response group");
                    response->set_description("response description");
                    return Status::OK;
                }
            );
            EnrollmentGroupResponse response;
            auto status = service.create_enrollment_group(&response,
                "foo user",
                "foo group",
                "foo name",
                "foo description",
                "foo model",
                {"enrollment ID"}
            );
            THEN("The status is OK") {
                REQUIRE(status.ok());
            }
            THEN("the response is updated from the call") {
                REQUIRE("response group" == response.id());
                REQUIRE("response description" == response.description());
            }
        }

        WHEN("CreateEnrollmentGroup is called with no group ID") {
            EXPECT_CALL(*stub, CreateEnrollmentGroup(_, _, _))
                .Times(1)
                .WillOnce([] (ClientContext*, const CreateEnrollmentGroupRequest& request, EnrollmentGroupResponse *response) {
                    REQUIRE(request.userid() == "foo user");
                    REQUIRE(request.id().length() == 36);  // UUIDv4 will have 36 characters
                    REQUIRE(request.name() == "foo name");
                    REQUIRE(request.description() == "foo description");
                    REQUIRE(request.modelname() == "foo model");
                    REQUIRE(0 == request.enrollmentids_size());
                    return Status::OK;
                }
            );
            EnrollmentGroupResponse response;
            auto status = service.create_enrollment_group(&response,
                "foo user", "", "foo name", "foo description", "foo model", {});
            THEN("The status is OK") {
                REQUIRE(status.ok());
            }
        }

        // ----- AppendEnrollmentGroup -----------------------------------------

        WHEN("AppendEnrollmentGroup is called") {
            EXPECT_CALL(*stub, AppendEnrollmentGroup(_, _, _))
                .Times(1)
                .WillOnce([] (ClientContext*, const AppendEnrollmentGroupRequest& request, EnrollmentGroupResponse *response) {
                    REQUIRE(request.groupid() == "foo ID");
                    REQUIRE(2 == request.enrollmentids_size());
                    REQUIRE("ID0" == request.enrollmentids(0));
                    REQUIRE("ID1" == request.enrollmentids(1));
                    return Status::OK;
                }
            );
            EnrollmentGroupResponse response;
            auto status = service.append_enrollment_group(&response, "foo ID", {"ID0", "ID1"});
        }

        // ----- DeleteEnrollment ----------------------------------------------

        WHEN("DeleteEnrollment is called") {
            EXPECT_CALL(*stub, DeleteEnrollment(_, _, _))
                .Times(1)
                .WillOnce([] (ClientContext*, const DeleteEnrollmentRequest& request, EnrollmentResponse *response) {
                    REQUIRE(request.id() == "foo ID");
                    response->set_id("response ID");
                    return Status::OK;
                }
            );
            EnrollmentResponse response;
            auto status = service.delete_enrollment(&response, "foo ID");
            THEN("The status is OK") {
                REQUIRE(status.ok());
            }
            THEN("The response is updated with the enrollment") {
                REQUIRE(response.id() == "response ID");
            }
        }

        // ----- DeleteEnrollmentGroup -----------------------------------------

        WHEN("DeleteEnrollmentGroup is called") {
            EXPECT_CALL(*stub, DeleteEnrollmentGroup(_, _, _))
                .Times(1)
                .WillOnce([] (ClientContext*, const DeleteEnrollmentGroupRequest& request, EnrollmentGroupResponse *response) {
                    REQUIRE(request.id() == "foo ID");
                    response->set_id("response ID");
                    return Status::OK;
                }
            );
            EnrollmentGroupResponse response;
            auto status = service.delete_enrollment_group(&response, "foo ID");
            THEN("The status is OK") {
                REQUIRE(status.ok());
            }
            THEN("The response is updated with the enrollment group") {
                REQUIRE(response.id() == "response ID");
            }
        }
    }
}
