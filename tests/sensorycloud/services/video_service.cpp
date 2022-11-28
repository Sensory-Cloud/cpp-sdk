// Test cases for the video service.
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
#include <grpcpp/test/mock_stream.h>
#include "sensorycloud/services/video_service.hpp"
#include "sensorycloud/services/oauth_service.hpp"
#include "sensorycloud/token_manager/token_manager.hpp"
#include "sensorycloud/token_manager/in_memory_credential_store.hpp"
#include "sensorycloud/generated/v1/video/video_mock.grpc.pb.h"

using ::grpc::ClientContext;
using ::grpc::Status;
using ::grpc::testing::MockClientReaderWriter;

using ::sensory::Config;
using ::sensory::token_manager::InMemoryCredentialStore;
using ::sensory::token_manager::TokenManager;
using ::sensory::service::OAuthService;
using ::sensory::service::VideoService;
using ::sensory::error::WriteStreamError;
using ::sensory::error::NullStreamError;
using ::sensory::error::ReadStreamError;

using ::sensory::api::v1::video::AuthenticateRequest;
using ::sensory::api::v1::video::AuthenticateResponse;
using ::sensory::api::v1::video::CreateEnrollmentRequest;
using ::sensory::api::v1::video::CreateEnrollmentResponse;
using ::sensory::api::v1::video::GetModelsRequest;
using ::sensory::api::v1::video::GetModelsResponse;
using ::sensory::api::v1::video::LivenessRecognitionResponse;
using ::sensory::api::v1::video::RecognitionThreshold;
using ::sensory::api::v1::video::ValidateRecognitionRequest;

using testing::_;

// ---------------------------------------------------------------------------
// MARK: VideoService
// ---------------------------------------------------------------------------

TEST_CASE("Should create VideoService from Config and TokenManager") {
    // Create the configuration that provides information about the remote host.
    Config config("hostname.com", 443, "tenant ID", "device ID");
    // Create the OAuth service for requesting and managing OAuth tokens through
    // a token manager instance.
    OAuthService oauth_service(config);
    // Create a credential store for keeping the clientID, clientSecret,
    // token, and expiration time.
    InMemoryCredentialStore keychain;
    TokenManager<InMemoryCredentialStore> token_manager(oauth_service, keychain);
    // Create the actual video service from the config and token manager.
    VideoService<InMemoryCredentialStore> service(config, token_manager);
}

SCENARIO("A client requires a synchronous interface to the video service") {
    GIVEN("An initialized video service.") {
        // Create the configuration that provides information about the remote host.
        Config config("hostname.com", 443, "tenant ID", "device ID", false);
        // Create the OAuth service for requesting and managing OAuth tokens through
        // a token manager instance.
        OAuthService oauth_service(config);
        // Create a credential store for keeping the clientID, clientSecret,
        // token, and expiration time.
        InMemoryCredentialStore keychain;
        TokenManager<InMemoryCredentialStore> token_manager(oauth_service, keychain);
        // Create the actual video service from the config and token manager.
        auto models_stub = new ::sensory::api::v1::video::MockVideoModelsStub;
        auto biometrics_stub = new ::sensory::api::v1::video::MockVideoBiometricsStub;
        auto recognition_stub = new ::sensory::api::v1::video::MockVideoRecognitionStub;
        VideoService<InMemoryCredentialStore>
            service(config, token_manager, models_stub, biometrics_stub, recognition_stub);

        // ----- GetModels -----------------------------------------------------

        WHEN("GetModels is called") {
            EXPECT_CALL(*models_stub, GetModels(_, _, _))
                .Times(1)
                .WillOnce([] (ClientContext*, const GetModelsRequest& request, GetModelsResponse *response) {
                    auto model = response->add_models();
                    model->set_name("response model");
                    return Status::OK;
                }
            );
            GetModelsResponse response;
            auto status = service.get_models(&response);
            THEN("The status is OK") {
                REQUIRE(status.ok());
            }
            THEN("the response contains the updated data") {
                REQUIRE(1 == response.models_size());
                REQUIRE("response model" == response.models(0).name());
            }
        }

        // ----- CreateEnrollment ----------------------------------------------

        WHEN("CreateEnrollment is called without a valid connection") {
            // Expect the call and return a null pointer for the stream.
            EXPECT_CALL(*biometrics_stub, CreateEnrollmentRaw(_))
                .Times(1).WillOnce(testing::Return(nullptr));
            ClientContext context;
            auto config = new ::sensory::api::v1::video::CreateEnrollmentConfig;
            config->set_modelname("modelName");
            config->set_userid("userID");
            config->set_description("description");
            config->set_islivenessenabled(true);
            config->set_livenessthreshold(RecognitionThreshold::LOW);
            config->set_numlivenessframesrequired(0);
            config->set_referenceid("referenceId");
            THEN("the function catches the null stream and throws an error") {
                REQUIRE_THROWS_AS(service.create_enrollment(&context, config), NullStreamError);
            }
        }

        WHEN("CreateEnrollment is called and the first Write fails") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<CreateEnrollmentRequest, CreateEnrollmentResponse>();
            EXPECT_CALL(*biometrics_stub, CreateEnrollmentRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the video config.
            EXPECT_CALL(*mock_stream, Write(_, _))
                .Times(1).WillOnce(testing::Return(false));
            ClientContext context;
            auto config = new ::sensory::api::v1::video::CreateEnrollmentConfig;
            config->set_modelname("modelName");
            config->set_userid("userID");
            config->set_description("description");
            config->set_islivenessenabled(true);
            config->set_livenessthreshold(RecognitionThreshold::LOW);
            config->set_numlivenessframesrequired(0);
            config->set_referenceid("referenceId");
            THEN("the function catches the write failure and throws an error") {
                REQUIRE_THROWS_AS(service.create_enrollment(&context, config), WriteStreamError);
            }
        }

        WHEN("CreateEnrollment is called with a valid connection") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<CreateEnrollmentRequest, CreateEnrollmentResponse>();
            EXPECT_CALL(*biometrics_stub, CreateEnrollmentRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the video config.
            // Check that the config is properly set with the parameters.
            EXPECT_CALL(*mock_stream, Write(_, _)).Times(1).WillOnce(
                [] (const CreateEnrollmentRequest& request, ::grpc::WriteOptions) {
                    REQUIRE("device ID" == request.config().deviceid());
                    REQUIRE("modelName" == request.config().modelname());
                    REQUIRE("userID" == request.config().userid());
                    REQUIRE("description" == request.config().description());
                    REQUIRE(true == request.config().islivenessenabled());
                    REQUIRE(RecognitionThreshold::LOW == request.config().livenessthreshold());
                    return true;
                }
            );
            ClientContext context;
            auto config = new ::sensory::api::v1::video::CreateEnrollmentConfig;
            config->set_modelname("modelName");
            config->set_userid("userID");
            config->set_description("description");
            config->set_islivenessenabled(true);
            config->set_livenessthreshold(RecognitionThreshold::LOW);
            config->set_numlivenessframesrequired(0);
            config->set_referenceid("referenceId");
            auto stream = service.create_enrollment(&context, config);
            THEN("a unique pointer to the mock stream is returned") {
                REQUIRE(stream.get() == mock_stream);
            }
        }

        // ----- Authenticate --------------------------------------------------

        WHEN("Authenticate is called without a valid connection") {
            // Expect the call and return a null pointer for the stream.
            EXPECT_CALL(*biometrics_stub, AuthenticateRaw(_))
                .Times(1).WillOnce(testing::Return(nullptr));
            ClientContext context;
            auto config = new ::sensory::api::v1::video::AuthenticateConfig;
            config->set_enrollmentid("enrollmentID");
            config->set_islivenessenabled(true);
            config->set_livenessthreshold(RecognitionThreshold::LOW);
            THEN("the function catches the null stream and throws an error") {
                REQUIRE_THROWS_AS(service.authenticate(&context, config), NullStreamError);
            }
        }

        WHEN("Authenticate is called and the first Write fails") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<AuthenticateRequest, AuthenticateResponse>();
            EXPECT_CALL(*biometrics_stub, AuthenticateRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the video config.
            EXPECT_CALL(*mock_stream, Write(_, _))
                .Times(1).WillOnce(testing::Return(false));
            ClientContext context;
            auto config = new ::sensory::api::v1::video::AuthenticateConfig;
            config->set_enrollmentid("enrollmentID");
            config->set_islivenessenabled(true);
            config->set_livenessthreshold(RecognitionThreshold::LOW);
            THEN("the function catches the write failure and throws an error") {
                REQUIRE_THROWS_AS(service.authenticate(&context, config), WriteStreamError);
            }
        }

        WHEN("Authenticate is called with a valid connection") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<AuthenticateRequest, AuthenticateResponse>();
            EXPECT_CALL(*biometrics_stub, AuthenticateRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the video config.
            // Check that the config is properly set with the parameters.
            EXPECT_CALL(*mock_stream, Write(_, _)).Times(1).WillOnce(
                [] (const AuthenticateRequest& request, ::grpc::WriteOptions) {
                    REQUIRE("enrollmentID" == request.config().enrollmentid());
                    REQUIRE(true == request.config().islivenessenabled());
                    REQUIRE(RecognitionThreshold::LOW == request.config().livenessthreshold());
                    return true;
                }
            );
            ClientContext context;
            auto config = new ::sensory::api::v1::video::AuthenticateConfig;
            config->set_enrollmentid("enrollmentID");
            config->set_islivenessenabled(true);
            config->set_livenessthreshold(RecognitionThreshold::LOW);
            auto stream = service.authenticate(&context, config);
            THEN("a unique pointer to the mock stream is returned") {
                REQUIRE(stream.get() == mock_stream);
            }
        }

        // ----- ValidateRecognition -------------------------------------------

        WHEN("ValidateRecognition is called without a valid connection") {
            // Expect the call and return a null pointer for the stream.
            EXPECT_CALL(*recognition_stub, ValidateLivenessRaw(_))
                .Times(1).WillOnce(testing::Return(nullptr));
            ClientContext context;
            auto config = new ::sensory::api::v1::video::ValidateRecognitionConfig;
            config->set_modelname("modelName");
            config->set_userid("userID");
            config->set_threshold(RecognitionThreshold::LOW);
            THEN("the function catches the null stream and throws an error") {
                REQUIRE_THROWS_AS(service.validate_liveness(&context, config), NullStreamError);
            }
        }

        WHEN("ValidateRecognition is called and the first Write fails") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<ValidateRecognitionRequest, LivenessRecognitionResponse>();
            EXPECT_CALL(*recognition_stub, ValidateLivenessRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the video config.
            EXPECT_CALL(*mock_stream, Write(_, _))
                .Times(1).WillOnce(testing::Return(false));
            ClientContext context;
            auto config = new ::sensory::api::v1::video::ValidateRecognitionConfig;
            config->set_modelname("modelName");
            config->set_userid("userID");
            config->set_threshold(RecognitionThreshold::LOW);
            THEN("the function catches the write failure and throws an error") {
                REQUIRE_THROWS_AS(service.validate_liveness(&context, config), WriteStreamError);
            }
        }

        WHEN("ValidateRecognition is called with a valid connection") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<ValidateRecognitionRequest, LivenessRecognitionResponse>();
            EXPECT_CALL(*recognition_stub, ValidateLivenessRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the video config.
            // Check that the config is properly set with the parameters.
            EXPECT_CALL(*mock_stream, Write(_, _)).Times(1).WillOnce(
                [] (const ValidateRecognitionRequest& request, ::grpc::WriteOptions) {
                    REQUIRE("modelName" == request.config().modelname());
                    REQUIRE("userID" == request.config().userid());
                    REQUIRE(RecognitionThreshold::LOW == request.config().threshold());
                    return true;
                }
            );
            ClientContext context;
            auto config = new ::sensory::api::v1::video::ValidateRecognitionConfig;
            config->set_modelname("modelName");
            config->set_userid("userID");
            config->set_threshold(RecognitionThreshold::LOW);
            auto stream = service.validate_liveness(&context, config);
            THEN("a unique pointer to the mock stream is returned") {
                REQUIRE(stream.get() == mock_stream);
            }
        }
    }
}
