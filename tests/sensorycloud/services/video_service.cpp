// Test cases for the video service.
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

using ::sensory::api::v1::video::AuthenticateRequest;
using ::sensory::api::v1::video::AuthenticateResponse;
using ::sensory::api::v1::video::CreateEnrollmentRequest;
using ::sensory::api::v1::video::CreateEnrollmentResponse;
using ::sensory::api::v1::video::GetModelsRequest;
using ::sensory::api::v1::video::GetModelsResponse;
using ::sensory::api::v1::video::LivenessRecognitionResponse;
using ::sensory::api::v1::video::RecognitionThreshold;
using ::sensory::api::v1::video::ValidateRecognitionRequest;

using ::sensory::service::video::newCreateEnrollmentConfig;
using ::sensory::service::video::newAuthenticateConfig;
using ::sensory::service::video::newValidateRecognitionConfig;

using testing::_;

// ---------------------------------------------------------------------------
// MARK: newCreateEnrollmentConfig
// ---------------------------------------------------------------------------

SCENARIO("A user needs to create a CreateEnrollmentConfig") {
    GIVEN("parameters for an enrollment creation stream") {
        const std::string modelName = "modelName";
        const std::string userID = "userID";
        const std::string description = "description";
        const bool isLivenessEnabled = true;
        const auto livenessThreshold = RecognitionThreshold::LOW;
        const int32_t numLivenessFramesRequired = 7;
        WHEN("the config is dynamically allocated from the parameters") {
            auto config = newCreateEnrollmentConfig(
                modelName,
                userID,
                description,
                isLivenessEnabled,
                livenessThreshold,
                numLivenessFramesRequired
            );
            THEN("a pointer is returned with the variables set") {
                REQUIRE(config != nullptr);
                REQUIRE(config->modelname() == modelName);
                REQUIRE(config->userid() == userID);
                REQUIRE(config->description() == description);
                REQUIRE(config->livenessthreshold() == livenessThreshold);
                REQUIRE(config->numlivenessframesrequired() == numLivenessFramesRequired);
            }
            delete config;
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: newAuthenticateConfig
// ---------------------------------------------------------------------------

SCENARIO("A user needs to create an AuthenticateConfig") {
    GIVEN("parameters for an authentication stream") {
        const std::string enrollmentID = "enrollmentID";
        const bool isLivenessEnabled = true;
        const auto livenessThreshold = RecognitionThreshold::LOW;
        WHEN("the config is dynamically allocated from the parameters") {
            auto config = newAuthenticateConfig(
                enrollmentID,
                isLivenessEnabled,
                livenessThreshold
            );
            THEN("a pointer is returned with the variables set") {
                REQUIRE(config != nullptr);
                REQUIRE(config->enrollmentid() == enrollmentID);
                REQUIRE(config->enrollmentgroupid() == "");
                REQUIRE(config->islivenessenabled() == isLivenessEnabled);
                REQUIRE(config->livenessthreshold() == livenessThreshold);
            }
            delete config;
        }
        WHEN("the config is dynamically allocated as an enrollment group") {
            auto config = newAuthenticateConfig(
                enrollmentID,
                isLivenessEnabled,
                livenessThreshold,
                true
            );
            THEN("a pointer is returned with the variables set") {
                REQUIRE(config != nullptr);
                REQUIRE(config->enrollmentid() == "");
                REQUIRE(config->enrollmentgroupid() == enrollmentID);
                REQUIRE(config->islivenessenabled() == isLivenessEnabled);
                REQUIRE(config->livenessthreshold() == livenessThreshold);
            }
            delete config;
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: newValidateRecognitionConfig
// ---------------------------------------------------------------------------

SCENARIO("A user needs to create a ValidateRecognitionConfig") {
    GIVEN("parameters for a recognition validation stream") {
        const std::string modelName = "modelName";
        const std::string userID = "userID";
        const auto threshold = RecognitionThreshold::LOW;
        WHEN("the config is dynamically allocated from the parameters") {
            auto config = newValidateRecognitionConfig(
                modelName,
                userID,
                threshold
            );
            THEN("a pointer is returned with the variables set") {
                REQUIRE(config != nullptr);
                REQUIRE(config->modelname() == modelName);
                REQUIRE(config->userid() == userID);
                REQUIRE(config->threshold() == threshold);
            }
            delete config;
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: VideoService
// ---------------------------------------------------------------------------

TEST_CASE("Should create VideoService from Config and TokenManager") {
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
    // Create the actual video service from the config and token manager.
    VideoService<InMemoryCredentialStore> service(config, tokenManager);
}

SCENARIO("A client requires a synchronous interface to the video service") {
    GIVEN("An initialized video service.") {
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
        // Create the actual video service from the config and token manager.
        auto modelsStub = new ::sensory::api::v1::video::MockVideoModelsStub;
        auto biometricsStub = new ::sensory::api::v1::video::MockVideoBiometricsStub;
        auto recognitionStub = new ::sensory::api::v1::video::MockVideoRecognitionStub;
        VideoService<InMemoryCredentialStore>
            service(config, tokenManager, modelsStub, biometricsStub, recognitionStub);

        // ----- GetModels -----------------------------------------------------

        WHEN("GetModels is called") {
            EXPECT_CALL(*modelsStub, GetModels(_, _, _))
                .Times(1)
                .WillOnce([] (ClientContext*, const GetModelsRequest& request, GetModelsResponse *response) {
                    auto model = response->add_models();
                    model->set_name("response model");
                    return Status::OK;
                }
            );
            GetModelsResponse response;
            auto status = service.getModels(&response);
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
            EXPECT_CALL(*biometricsStub, CreateEnrollmentRaw(_))
                .Times(1).WillOnce(testing::Return(nullptr));
            ClientContext context;
            THEN("the function catches the null stream and throws an error") {
                REQUIRE_THROWS_AS(service.createEnrollment(&context, newCreateEnrollmentConfig(
                    "modelName",
                    "userID",
                    "description",
                    true,
                    RecognitionThreshold::LOW
                )), sensory::service::NullStreamError);
            }
        }

        WHEN("CreateEnrollment is called and the first Write fails") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<CreateEnrollmentRequest, CreateEnrollmentResponse>();
            EXPECT_CALL(*biometricsStub, CreateEnrollmentRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the video config.
            EXPECT_CALL(*mock_stream, Write(_, _))
                .Times(1).WillOnce(testing::Return(false));
            ClientContext context;
            THEN("the function catches the write failure and throws an error") {
                REQUIRE_THROWS_AS(service.createEnrollment(&context, newCreateEnrollmentConfig(
                    "modelName",
                    "userID",
                    "description",
                    true,
                    RecognitionThreshold::LOW
                )), sensory::service::WriteStreamError);
            }
        }

        WHEN("CreateEnrollment is called with a valid connection") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<CreateEnrollmentRequest, CreateEnrollmentResponse>();
            EXPECT_CALL(*biometricsStub, CreateEnrollmentRaw(_))
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
            auto stream = service.createEnrollment(&context, newCreateEnrollmentConfig(
                "modelName",
                "userID",
                "description",
                true,
                RecognitionThreshold::LOW
            ));
            THEN("a unique pointer to the mock stream is returned") {
                REQUIRE(stream.get() == mock_stream);
            }
        }

        // ----- Authenticate --------------------------------------------------

        WHEN("Authenticate is called without a valid connection") {
            // Expect the call and return a null pointer for the stream.
            EXPECT_CALL(*biometricsStub, AuthenticateRaw(_))
                .Times(1).WillOnce(testing::Return(nullptr));
            ClientContext context;
            THEN("the function catches the null stream and throws an error") {
                REQUIRE_THROWS_AS(service.authenticate(&context, newAuthenticateConfig(
                    "enrollmentID",
                    true,
                    RecognitionThreshold::LOW
                )), sensory::service::NullStreamError);
            }
        }

        WHEN("Authenticate is called and the first Write fails") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<AuthenticateRequest, AuthenticateResponse>();
            EXPECT_CALL(*biometricsStub, AuthenticateRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the video config.
            EXPECT_CALL(*mock_stream, Write(_, _))
                .Times(1).WillOnce(testing::Return(false));
            ClientContext context;
            THEN("the function catches the write failure and throws an error") {
                REQUIRE_THROWS_AS(service.authenticate(&context, newAuthenticateConfig(
                    "enrollmentID",
                    true,
                    RecognitionThreshold::LOW
                )), sensory::service::WriteStreamError);
            }
        }

        WHEN("Authenticate is called with a valid connection") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<AuthenticateRequest, AuthenticateResponse>();
            EXPECT_CALL(*biometricsStub, AuthenticateRaw(_))
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
            auto stream = service.authenticate(&context, newAuthenticateConfig(
                "enrollmentID",
                true,
                RecognitionThreshold::LOW
            ));
            THEN("a unique pointer to the mock stream is returned") {
                REQUIRE(stream.get() == mock_stream);
            }
        }

        // ----- ValidateRecognition -------------------------------------------

        WHEN("ValidateRecognition is called without a valid connection") {
            // Expect the call and return a null pointer for the stream.
            EXPECT_CALL(*recognitionStub, ValidateLivenessRaw(_))
                .Times(1).WillOnce(testing::Return(nullptr));
            ClientContext context;
            THEN("the function catches the null stream and throws an error") {
                REQUIRE_THROWS_AS(service.validateLiveness(&context, newValidateRecognitionConfig(
                    "modelName",
                    "userID",
                    RecognitionThreshold::LOW
                )), sensory::service::NullStreamError);
            }
        }

        WHEN("ValidateRecognition is called and the first Write fails") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<ValidateRecognitionRequest, LivenessRecognitionResponse>();
            EXPECT_CALL(*recognitionStub, ValidateLivenessRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the video config.
            EXPECT_CALL(*mock_stream, Write(_, _))
                .Times(1).WillOnce(testing::Return(false));
            ClientContext context;
            THEN("the function catches the write failure and throws an error") {
                REQUIRE_THROWS_AS(service.validateLiveness(&context, newValidateRecognitionConfig(
                    "modelName",
                    "userID",
                    RecognitionThreshold::LOW
                )), sensory::service::WriteStreamError);
            }
        }

        WHEN("ValidateRecognition is called with a valid connection") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<ValidateRecognitionRequest, LivenessRecognitionResponse>();
            EXPECT_CALL(*recognitionStub, ValidateLivenessRaw(_))
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
            auto stream = service.validateLiveness(&context, newValidateRecognitionConfig(
                "modelName",
                "userID",
                RecognitionThreshold::LOW
            ));
            THEN("a unique pointer to the mock stream is returned") {
                REQUIRE(stream.get() == mock_stream);
            }
        }

    }
}
