// Test cases for the audio service.
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
#include "sensorycloud/services/audio_service.hpp"
#include "sensorycloud/services/oauth_service.hpp"
#include "sensorycloud/token_manager/token_manager.hpp"
#include "sensorycloud/token_manager/in_memory_credential_store.hpp"
#include "sensorycloud/generated/v1/audio/audio_mock.grpc.pb.h"

using ::grpc::ClientContext;
using ::grpc::Status;
using ::grpc::testing::MockClientReaderWriter;

using ::sensory::Config;
using ::sensory::token_manager::InMemoryCredentialStore;
using ::sensory::token_manager::TokenManager;
using ::sensory::service::OAuthService;
using ::sensory::service::AudioService;

using ::sensory::api::v1::audio::AudioModel;
using ::sensory::api::v1::audio::AuthenticateRequest;
using ::sensory::api::v1::audio::AuthenticateResponse;
using ::sensory::api::v1::audio::CreateEnrolledEventRequest;
using ::sensory::api::v1::audio::CreateEnrollmentRequest;
using ::sensory::api::v1::audio::CreateEnrollmentResponse;
using ::sensory::api::v1::audio::GetModelsRequest;
using ::sensory::api::v1::audio::GetModelsResponse;
using ::sensory::api::v1::audio::TranscribeRequest;
using ::sensory::api::v1::audio::TranscribeResponse;
using ::sensory::api::v1::audio::ValidateEnrolledEventRequest;
using ::sensory::api::v1::audio::ValidateEnrolledEventResponse;
using ::sensory::api::v1::audio::ValidateEventRequest;
using ::sensory::api::v1::audio::ValidateEventResponse;

using ::sensory::service::audio::newAudioConfig;
using ::sensory::service::audio::newCreateEnrollmentConfig;
using ::sensory::service::audio::newAuthenticateConfig;
using ::sensory::service::audio::newValidateEventConfig;
using ::sensory::service::audio::newCreateEnrollmentEventConfig;
using ::sensory::service::audio::newValidateEnrolledEventConfig;
using ::sensory::service::audio::newTranscribeConfig;

using testing::_;

// ---------------------------------------------------------------------------
// MARK: newAudioConfig
// ---------------------------------------------------------------------------

SCENARIO("A user needs to create an AudioConfig") {
    GIVEN("parameters for an audio config that describe the input stream") {
        const sensory::api::v1::audio::AudioConfig_AudioEncoding& encoding =
            sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16;
        const float sampleRateHertz = 16000;
        const uint32_t audioChannelCount = 1;
        const std::string languageCode = "en-US";
        WHEN("the config is dynamically allocated from the parameters") {
            auto config = newAudioConfig(
                encoding,
                sampleRateHertz,
                audioChannelCount,
                languageCode
            );
            THEN("a pointer is returned with the variables set") {
                REQUIRE(config != nullptr);
                REQUIRE(config->encoding() == encoding);
                REQUIRE(config->sampleratehertz() == sampleRateHertz);
                REQUIRE(config->audiochannelcount() == audioChannelCount);
                REQUIRE(config->languagecode() == languageCode);
            }
            delete config;
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: newCreateEnrollmentConfig
// ---------------------------------------------------------------------------

SCENARIO("A user needs to create a CreateEnrollmentConfig") {
    GIVEN("parameters for the enrollment based on a text-independent model") {
        const std::string& modelName = "modelName";
        const std::string& userID = "userID";
        const std::string& description = "Description";
        const bool& isLivenessEnabled = true;
        const float enrollmentDuration = 10.f;
        const int32_t numUtterances = 0;
        WHEN("the config is allocated from the parameters") {
            auto config = newCreateEnrollmentConfig(
                modelName,
                userID,
                description,
                isLivenessEnabled,
                enrollmentDuration,
                numUtterances
            );
            THEN("a pointer is returned with the variables set") {
                REQUIRE(config != nullptr);
                REQUIRE(config->modelname() == modelName);
                REQUIRE(config->userid() == userID);
                REQUIRE(config->description() == description);
                REQUIRE(config->islivenessenabled() == isLivenessEnabled);
                REQUIRE(config->enrollmentduration() == enrollmentDuration);
                REQUIRE(config->enrollmentnumutterances() == numUtterances);
            }
            delete config;
        }
    }
    GIVEN("parameters for the enrollment based on a text-independent model") {
        const std::string& modelName = "modelName";
        const std::string& userID = "userID";
        const std::string& description = "Description";
        const bool& isLivenessEnabled = true;
        const float enrollmentDuration = 0.f;
        const int32_t numUtterances = 4;
        WHEN("the config is allocated from the parameters") {
            auto config = newCreateEnrollmentConfig(
                modelName,
                userID,
                description,
                isLivenessEnabled,
                enrollmentDuration,
                numUtterances
            );
            THEN("a pointer is returned with the variables set") {
                REQUIRE(config != nullptr);
                REQUIRE(config->modelname() == modelName);
                REQUIRE(config->userid() == userID);
                REQUIRE(config->description() == description);
                REQUIRE(config->islivenessenabled() == isLivenessEnabled);
                REQUIRE(config->enrollmentduration() == enrollmentDuration);
                REQUIRE(config->enrollmentnumutterances() == numUtterances);
            }
            delete config;
        }
    }
    GIVEN("both enrollmentDuration and numUtterances provided") {
        const std::string& modelName = "modelName";
        const std::string& userID = "userID";
        const std::string& description = "Description";
        const bool& isLivenessEnabled = true;
        const float enrollmentDuration = 10.f;
        const int32_t numUtterances = 4;
        WHEN("the config is allocated from the parameters") {
            THEN("an error is thrown") {
                REQUIRE_THROWS(newCreateEnrollmentConfig(
                    modelName,
                    userID,
                    description,
                    isLivenessEnabled,
                    enrollmentDuration,
                    numUtterances
                ));
            }
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
        const sensory::api::v1::audio::ThresholdSensitivity sensitivity =
            sensory::api::v1::audio::ThresholdSensitivity::LOW;
        const sensory::api::v1::audio::AuthenticateConfig_ThresholdSecurity security =
            sensory::api::v1::audio::AuthenticateConfig_ThresholdSecurity_LOW;
        WHEN("the config is dynamically allocated from the parameters") {
            auto config = newAuthenticateConfig(
                enrollmentID,
                isLivenessEnabled,
                sensitivity,
                security
            );
            THEN("a pointer is returned with the variables set") {
                REQUIRE(config != nullptr);
                REQUIRE(config->enrollmentid() == enrollmentID);
                REQUIRE(config->enrollmentgroupid() == "");
                REQUIRE(config->islivenessenabled() == isLivenessEnabled);
                REQUIRE(config->sensitivity() == sensitivity);
                REQUIRE(config->security() == security);
            }
            delete config;
        }
        WHEN("the config is dynamically allocated as an enrollment group") {
            auto config = newAuthenticateConfig(
                enrollmentID,
                isLivenessEnabled,
                sensitivity,
                security,
                true
            );
            THEN("a pointer is returned with the variables set") {
                REQUIRE(config != nullptr);
                REQUIRE(config->enrollmentid() == "");
                REQUIRE(config->enrollmentgroupid() == enrollmentID);
                REQUIRE(config->islivenessenabled() == isLivenessEnabled);
                REQUIRE(config->sensitivity() == sensitivity);
                REQUIRE(config->security() == security);
            }
            delete config;
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: newValidateEventConfig
// ---------------------------------------------------------------------------

SCENARIO("A user needs to create a ValidateEventConfig") {
    GIVEN("parameters for an event validation stream") {
        const std::string modelName = "modelName";
        const std::string userID = "userID";
        const sensory::api::v1::audio::ThresholdSensitivity sensitivity =
            sensory::api::v1::audio::ThresholdSensitivity::LOW;
        WHEN("the config is dynamically allocated from the parameters") {
            auto config = newValidateEventConfig(
                modelName,
                userID,
                sensitivity
            );
            THEN("a pointer is returned with the variables set") {
                REQUIRE(config != nullptr);
                REQUIRE(config->modelname() == modelName);
                REQUIRE(config->userid() == userID);
                REQUIRE(config->sensitivity() == sensitivity);
            }
            delete config;
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: newCreateEnrollmentEventConfig
// ---------------------------------------------------------------------------

SCENARIO("A user needs to create a CreateEnrollmentEventConfig") {
    GIVEN("parameters for the enrollment based on a text-independent model") {
        const std::string& modelName = "modelName";
        const std::string& userID = "userID";
        const std::string& description = "Description";
        const float enrollmentDuration = 10.f;
        const int32_t numUtterances = 0;
        WHEN("the config is allocated from the parameters") {
            auto config = newCreateEnrollmentEventConfig(
                modelName,
                userID,
                description,
                enrollmentDuration,
                numUtterances
            );
            THEN("a pointer is returned with the variables set") {
                REQUIRE(config != nullptr);
                REQUIRE(config->modelname() == modelName);
                REQUIRE(config->userid() == userID);
                REQUIRE(config->description() == description);
                REQUIRE(config->enrollmentduration() == enrollmentDuration);
                REQUIRE(config->enrollmentnumutterances() == numUtterances);
            }
            delete config;
        }
    }
    GIVEN("parameters for the enrollment based on a text-independent model") {
        const std::string& modelName = "modelName";
        const std::string& userID = "userID";
        const std::string& description = "Description";
        const float enrollmentDuration = 0.f;
        const int32_t numUtterances = 4;
        WHEN("the config is allocated from the parameters") {
            auto config = newCreateEnrollmentEventConfig(
                modelName,
                userID,
                description,
                enrollmentDuration,
                numUtterances
            );
            THEN("a pointer is returned with the variables set") {
                REQUIRE(config != nullptr);
                REQUIRE(config->modelname() == modelName);
                REQUIRE(config->userid() == userID);
                REQUIRE(config->description() == description);
                REQUIRE(config->enrollmentduration() == enrollmentDuration);
                REQUIRE(config->enrollmentnumutterances() == numUtterances);
            }
            delete config;
        }
    }
    GIVEN("both enrollmentDuration and numUtterances provided") {
        const std::string& modelName = "modelName";
        const std::string& userID = "userID";
        const std::string& description = "Description";
        const float enrollmentDuration = 10.f;
        const int32_t numUtterances = 4;
        WHEN("the config is allocated from the parameters") {
            THEN("an error is thrown") {
                REQUIRE_THROWS(newCreateEnrollmentEventConfig(
                    modelName,
                    userID,
                    description,
                    enrollmentDuration,
                    numUtterances
                ));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: newValidateEnrolledEventConfig
// ---------------------------------------------------------------------------

SCENARIO("A user needs to create an ValidateEnrolledEventConfig") {
    GIVEN("parameters for an authentication stream") {
        const std::string enrollmentID = "enrollmentID";
        const sensory::api::v1::audio::ThresholdSensitivity sensitivity =
            sensory::api::v1::audio::ThresholdSensitivity::LOW;
        WHEN("the config is dynamically allocated from the parameters") {
            auto config = newValidateEnrolledEventConfig(
                enrollmentID,
                sensitivity
            );
            THEN("a pointer is returned with the variables set") {
                REQUIRE(config != nullptr);
                REQUIRE(config->enrollmentid() == enrollmentID);
                REQUIRE(config->enrollmentgroupid() == "");
                REQUIRE(config->sensitivity() == sensitivity);
            }
            delete config;
        }
        WHEN("the config is dynamically allocated for group enrollment") {
            auto config = newValidateEnrolledEventConfig(
                enrollmentID,
                sensitivity,
                true
            );
            THEN("a pointer is returned with the variables set") {
                REQUIRE(config != nullptr);
                REQUIRE(config->enrollmentid() == "");
                REQUIRE(config->enrollmentgroupid() == enrollmentID);
                REQUIRE(config->sensitivity() == sensitivity);
            }
            delete config;
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: newTranscribeConfig
// ---------------------------------------------------------------------------

SCENARIO("A user needs to create a TranscribeConfig") {
    GIVEN("parameters for an audio transcription stream") {
        const std::string modelName = "modelName";
        const std::string userID = "userID";
        WHEN("the config is dynamically allocated from the parameters") {
            auto config = newTranscribeConfig(modelName, userID);
            THEN("a pointer is returned with the variables set") {
                REQUIRE(config != nullptr);
                REQUIRE(config->modelname() == modelName);
                REQUIRE(config->userid() == userID);
            }
            delete config;
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: AudioService
// ---------------------------------------------------------------------------

TEST_CASE("Should create AudioService from Config and TokenManager") {
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
    // Create the actual audio service from the config and token manager.
    AudioService<InMemoryCredentialStore> service(config, tokenManager);
}

SCENARIO("A client requires a synchronous interface to the audio service") {
    GIVEN("An initialized audio service.") {
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
        // Create the actual audio service from the config and token manager.
        auto modelsStub = new ::sensory::api::v1::audio::MockAudioModelsStub;
        auto biometricsStub = new ::sensory::api::v1::audio::MockAudioBiometricsStub;
        auto eventsStub = new ::sensory::api::v1::audio::MockAudioEventsStub;
        auto transcriptionStub = new ::sensory::api::v1::audio::MockAudioTranscriptionsStub;
        AudioService<InMemoryCredentialStore>
            service(config, tokenManager, modelsStub, biometricsStub, eventsStub, transcriptionStub);

        auto audioConfig = newAudioConfig(
            sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16,
            16000,
            1,
            "en-US"
        );

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
                REQUIRE_THROWS_AS(service.createEnrollment(&context, audioConfig, newCreateEnrollmentConfig(
                    "modelName",
                    "userID",
                    "description",
                    true,
                    10.f,
                    0
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
                REQUIRE_THROWS_AS(service.createEnrollment(&context, audioConfig, newCreateEnrollmentConfig(
                    "modelName",
                    "userID",
                    "description",
                    true,
                    10.f,
                    0
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
                    REQUIRE(sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16 == request.config().audio().encoding());
                    REQUIRE(16000 == request.config().audio().sampleratehertz());
                    REQUIRE(1 == request.config().audio().audiochannelcount());
                    REQUIRE("en-US" == request.config().audio().languagecode());
                    REQUIRE("device ID" == request.config().deviceid());
                    REQUIRE("modelName" == request.config().modelname());
                    REQUIRE("userID" == request.config().userid());
                    REQUIRE("description" == request.config().description());
                    REQUIRE(true == request.config().islivenessenabled());
                    REQUIRE(10.f == request.config().enrollmentduration());
                    REQUIRE(0 == request.config().enrollmentnumutterances());
                    return true;
                }
            );
            ClientContext context;
            auto audioConfig = newAudioConfig(
                sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16,
                16000,
                1,
                "en-US"
            );
            auto stream = service.createEnrollment(&context, audioConfig, newCreateEnrollmentConfig(
                "modelName",
                "userID",
                "description",
                true,
                10.f,
                0
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
                REQUIRE_THROWS_AS(service.authenticate(&context, audioConfig, newAuthenticateConfig(
                    "enrollmentID",
                    true,
                    sensory::api::v1::audio::ThresholdSensitivity::LOW,
                    sensory::api::v1::audio::AuthenticateConfig_ThresholdSecurity_LOW
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
                REQUIRE_THROWS_AS(service.authenticate(&context, audioConfig, newAuthenticateConfig(
                    "enrollmentID",
                    true,
                    sensory::api::v1::audio::ThresholdSensitivity::LOW,
                    sensory::api::v1::audio::AuthenticateConfig_ThresholdSecurity_LOW
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
                    REQUIRE(sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16 == request.config().audio().encoding());
                    REQUIRE(16000 == request.config().audio().sampleratehertz());
                    REQUIRE(1 == request.config().audio().audiochannelcount());
                    REQUIRE("en-US" == request.config().audio().languagecode());
                    REQUIRE("enrollmentID" == request.config().enrollmentid());
                    REQUIRE(true == request.config().islivenessenabled());
                    REQUIRE(sensory::api::v1::audio::ThresholdSensitivity::LOW == request.config().sensitivity());
                    REQUIRE(sensory::api::v1::audio::AuthenticateConfig_ThresholdSecurity_LOW == request.config().security());
                    return true;
                }
            );
            ClientContext context;
            auto audioConfig = newAudioConfig(
                sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16,
                16000,
                1,
                "en-US"
            );
            auto stream = service.authenticate(&context, audioConfig, newAuthenticateConfig(
                "enrollmentID",
                true,
                sensory::api::v1::audio::ThresholdSensitivity::LOW,
                sensory::api::v1::audio::AuthenticateConfig_ThresholdSecurity_LOW
            ));
            THEN("a unique pointer to the mock stream is returned") {
                REQUIRE(stream.get() == mock_stream);
            }
        }

        // ----- ValidateEvent -------------------------------------------------

        WHEN("ValidateEvent is called without a valid connection") {
            // Expect the call and return a null pointer for the stream.
            EXPECT_CALL(*eventsStub, ValidateEventRaw(_))
                .Times(1).WillOnce(testing::Return(nullptr));
            ClientContext context;
            THEN("the function catches the null stream and throws an error") {
                REQUIRE_THROWS_AS(service.validateEvent(&context, audioConfig, newValidateEventConfig(
                    "modelName",
                    "userID",
                    sensory::api::v1::audio::ThresholdSensitivity::LOW
                )), sensory::service::NullStreamError);
            }
        }

        WHEN("ValidateEvent is called and the first Write fails") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<ValidateEventRequest, ValidateEventResponse>();
            EXPECT_CALL(*eventsStub, ValidateEventRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the video config.
            EXPECT_CALL(*mock_stream, Write(_, _))
                .Times(1).WillOnce(testing::Return(false));
            ClientContext context;
            THEN("the function catches the write failure and throws an error") {
                REQUIRE_THROWS_AS(service.validateEvent(&context, audioConfig, newValidateEventConfig(
                    "modelName",
                    "userID",
                    sensory::api::v1::audio::ThresholdSensitivity::LOW
                )), sensory::service::WriteStreamError);
            }
        }

        WHEN("ValidateEvent is called with a valid connection") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<ValidateEventRequest, ValidateEventResponse>();
            EXPECT_CALL(*eventsStub, ValidateEventRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the video config.
            // Check that the config is properly set with the parameters.
            EXPECT_CALL(*mock_stream, Write(_, _)).Times(1).WillOnce(
                [] (const ValidateEventRequest& request, ::grpc::WriteOptions) {
                    REQUIRE(sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16 == request.config().audio().encoding());
                    REQUIRE(16000 == request.config().audio().sampleratehertz());
                    REQUIRE(1 == request.config().audio().audiochannelcount());
                    REQUIRE("en-US" == request.config().audio().languagecode());
                    REQUIRE("modelName" == request.config().modelname());
                    REQUIRE("userID" == request.config().userid());
                    REQUIRE(sensory::api::v1::audio::ThresholdSensitivity::LOW == request.config().sensitivity());
                    return true;
                }
            );
            ClientContext context;
            auto audioConfig = newAudioConfig(
                sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16,
                16000,
                1,
                "en-US"
            );
            auto stream = service.validateEvent(&context, audioConfig, newValidateEventConfig(
                "modelName",
                "userID",
                sensory::api::v1::audio::ThresholdSensitivity::LOW
            ));
            THEN("a unique pointer to the mock stream is returned") {
                REQUIRE(stream.get() == mock_stream);
            }
        }

        // ----- CreateEnrolledEvent -------------------------------------------

        WHEN("CreateEnrolledEvent is called without a valid connection") {
            // Expect the call and return a null pointer for the stream.
            EXPECT_CALL(*eventsStub, CreateEnrolledEventRaw(_))
                .Times(1).WillOnce(testing::Return(nullptr));
            ClientContext context;
            THEN("the function catches the null stream and throws an error") {
                REQUIRE_THROWS_AS(service.createEventEnrollment(&context, audioConfig, newCreateEnrollmentEventConfig(
                    "modelName",
                    "userID",
                    "Description",
                    10.f,
                    0
                )), sensory::service::NullStreamError);
            }
        }

        WHEN("CreateEnrolledEvent is called and the first Write fails") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<CreateEnrolledEventRequest, CreateEnrollmentResponse>();
            EXPECT_CALL(*eventsStub, CreateEnrolledEventRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the video config.
            EXPECT_CALL(*mock_stream, Write(_, _))
                .Times(1).WillOnce(testing::Return(false));
            ClientContext context;
            THEN("the function catches the write failure and throws an error") {
                REQUIRE_THROWS_AS(service.createEventEnrollment(&context, audioConfig, newCreateEnrollmentEventConfig(
                    "modelName",
                    "userID",
                    "Description",
                    10.f,
                    0
                )), sensory::service::WriteStreamError);
            }
        }

        WHEN("CreateEnrolledEvent is called with a valid connection") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<CreateEnrolledEventRequest, CreateEnrollmentResponse>();
            EXPECT_CALL(*eventsStub, CreateEnrolledEventRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the video config.
            // Check that the config is properly set with the parameters.
            EXPECT_CALL(*mock_stream, Write(_, _)).Times(1).WillOnce(
                [] (const CreateEnrolledEventRequest& request, ::grpc::WriteOptions) {
                    REQUIRE(sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16 == request.config().audio().encoding());
                    REQUIRE(16000 == request.config().audio().sampleratehertz());
                    REQUIRE(1 == request.config().audio().audiochannelcount());
                    REQUIRE("en-US" == request.config().audio().languagecode());
                    REQUIRE("modelName" == request.config().modelname());
                    REQUIRE("userID" == request.config().userid());
                    REQUIRE("description" == request.config().description());
                    REQUIRE(10.f == request.config().enrollmentduration());
                    REQUIRE(0 == request.config().enrollmentnumutterances());
                    return true;
                }
            );
            ClientContext context;
            auto audioConfig = newAudioConfig(
                sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16,
                16000,
                1,
                "en-US"
            );
            auto stream = service.createEventEnrollment(&context, audioConfig, newCreateEnrollmentEventConfig(
                "modelName",
                "userID",
                "description",
                10.f,
                0
            ));
            THEN("a unique pointer to the mock stream is returned") {
                REQUIRE(stream.get() == mock_stream);
            }
        }

        // ----- ValidateEnrolledEvent -----------------------------------------

        WHEN("ValidateEnrolledEvent is called without a valid connection") {
            // Expect the call and return a null pointer for the stream.
            EXPECT_CALL(*eventsStub, ValidateEnrolledEventRaw(_))
                .Times(1).WillOnce(testing::Return(nullptr));
            ClientContext context;
            THEN("the function catches the null stream and throws an error") {
                REQUIRE_THROWS_AS(service.validateEnrolledEvent(&context, audioConfig, newValidateEnrolledEventConfig(
                    "enrollmentID",
                    sensory::api::v1::audio::ThresholdSensitivity::LOW
                )), sensory::service::NullStreamError);
            }
        }

        WHEN("ValidateEnrolledEvent is called and the first Write fails") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<ValidateEnrolledEventRequest, ValidateEnrolledEventResponse>();
            EXPECT_CALL(*eventsStub, ValidateEnrolledEventRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the video config.
            EXPECT_CALL(*mock_stream, Write(_, _))
                .Times(1).WillOnce(testing::Return(false));
            ClientContext context;
            THEN("the function catches the write failure and throws an error") {
                REQUIRE_THROWS_AS(service.validateEnrolledEvent(&context, audioConfig, newValidateEnrolledEventConfig(
                    "enrollmentID",
                    sensory::api::v1::audio::ThresholdSensitivity::LOW
                )), sensory::service::WriteStreamError);
            }
        }

        WHEN("ValidateEnrolledEvent is called with a valid connection") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<ValidateEnrolledEventRequest, ValidateEnrolledEventResponse>();
            EXPECT_CALL(*eventsStub, ValidateEnrolledEventRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the video config.
            // Check that the config is properly set with the parameters.
            EXPECT_CALL(*mock_stream, Write(_, _)).Times(1).WillOnce(
                [] (const ValidateEnrolledEventRequest& request, ::grpc::WriteOptions) {
                    REQUIRE(sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16 == request.config().audio().encoding());
                    REQUIRE(16000 == request.config().audio().sampleratehertz());
                    REQUIRE(1 == request.config().audio().audiochannelcount());
                    REQUIRE("en-US" == request.config().audio().languagecode());
                    REQUIRE("enrollmentID" == request.config().enrollmentid());
                    REQUIRE(sensory::api::v1::audio::ThresholdSensitivity::LOW == request.config().sensitivity());
                    return true;
                }
            );
            ClientContext context;
            auto audioConfig = newAudioConfig(
                sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16,
                16000,
                1,
                "en-US"
            );
            auto stream = service.validateEnrolledEvent(&context, audioConfig, newValidateEnrolledEventConfig(
                "enrollmentID",
                sensory::api::v1::audio::ThresholdSensitivity::LOW
            ));
            THEN("a unique pointer to the mock stream is returned") {
                REQUIRE(stream.get() == mock_stream);
            }
        }

        // ----- Transcribe ----------------------------------------------------

        WHEN("Transcribe is called without a valid connection") {
            // Expect the call and return a null pointer for the stream.
            EXPECT_CALL(*transcriptionStub, TranscribeRaw(_))
                .Times(1).WillOnce(testing::Return(nullptr));
            ClientContext context;
            THEN("the function catches the null stream and throws an error") {
                REQUIRE_THROWS_AS(service.transcribe(&context, audioConfig, newTranscribeConfig(
                    "modelName",
                    "userID"
                )), sensory::service::NullStreamError);
            }
        }

        WHEN("Transcribe is called and the first Write fails") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<TranscribeRequest, TranscribeResponse>();
            EXPECT_CALL(*transcriptionStub, TranscribeRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the video config.
            EXPECT_CALL(*mock_stream, Write(_, _))
                .Times(1).WillOnce(testing::Return(false));
            ClientContext context;
            THEN("the function catches the write failure and throws an error") {
                REQUIRE_THROWS_AS(service.transcribe(&context, audioConfig, newTranscribeConfig(
                    "modelName",
                    "userID"
                )), sensory::service::WriteStreamError);
            }
        }

        WHEN("Transcribe is called with a valid connection") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<TranscribeRequest, TranscribeResponse>();
            EXPECT_CALL(*transcriptionStub, TranscribeRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the video config.
            // Check that the config is properly set with the parameters.
            EXPECT_CALL(*mock_stream, Write(_, _)).Times(1).WillOnce(
                [] (const TranscribeRequest& request, ::grpc::WriteOptions) {
                    REQUIRE(sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16 == request.config().audio().encoding());
                    REQUIRE(16000 == request.config().audio().sampleratehertz());
                    REQUIRE(1 == request.config().audio().audiochannelcount());
                    REQUIRE("en-US" == request.config().audio().languagecode());
                    REQUIRE("modelName" == request.config().modelname());
                    REQUIRE("userID" == request.config().userid());
                    return true;
                }
            );
            ClientContext context;
            auto stream = service.transcribe(&context, audioConfig, newTranscribeConfig(
                "modelName",
                "userID"
            ));
            THEN("a unique pointer to the mock stream is returned") {
                REQUIRE(stream.get() == mock_stream);
            }
        }
    }
}
