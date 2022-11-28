// Test cases for the audio service.
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
#include "sensorycloud/services/audio_service.hpp"
#include "sensorycloud/services/oauth_service.hpp"
#include "sensorycloud/token_manager/token_manager.hpp"
#include "sensorycloud/token_manager/in_memory_credential_store.hpp"
#include "sensorycloud/generated/v1/audio/audio_mock.grpc.pb.h"

using ::grpc::ClientContext;
using ::grpc::Status;
using ::grpc::testing::MockClientReaderWriter;
using ::grpc::testing::MockClientReader;
using ::grpc::testing::MockClientWriter;

using ::sensory::Config;
using ::sensory::token_manager::InMemoryCredentialStore;
using ::sensory::token_manager::TokenManager;
using ::sensory::service::OAuthService;
using ::sensory::service::AudioService;
using ::sensory::error::WriteStreamError;
using ::sensory::error::NullStreamError;
using ::sensory::error::ReadStreamError;

using ::sensory::api::v1::audio::AudioModel;
using ::sensory::api::v1::audio::AuthenticateRequest;
using ::sensory::api::v1::audio::AuthenticateResponse;
using ::sensory::api::v1::audio::CreateEnrolledEventRequest;
using ::sensory::api::v1::audio::CreateEnrollmentRequest;
using ::sensory::api::v1::audio::CreateEnrollmentResponse;
using ::sensory::api::v1::audio::GetModelsRequest;
using ::sensory::api::v1::audio::GetModelsResponse;
using ::sensory::api::v1::audio::SynthesizeSpeechRequest;
using ::sensory::api::v1::audio::SynthesizeSpeechResponse;
using ::sensory::api::v1::audio::TranscribeRequest;
using ::sensory::api::v1::audio::TranscribeResponse;
using ::sensory::api::v1::audio::ValidateEnrolledEventRequest;
using ::sensory::api::v1::audio::ValidateEnrolledEventResponse;
using ::sensory::api::v1::audio::ValidateEventRequest;
using ::sensory::api::v1::audio::ValidateEventResponse;
using ::sensory::api::v1::audio::TranscribeWordResponse;
using ::sensory::api::v1::audio::TranscribeWord;

using testing::_;

// ---------------------------------------------------------------------------
// MARK: AudioService
// ---------------------------------------------------------------------------

TEST_CASE("Should create AudioService from Config and TokenManager") {
    // Create the configuration that provides information about the remote host.
    Config config("hostname.com", 443, "tenant ID", "device ID");
    // Create the OAuth service for requesting and managing OAuth tokens through
    // a token manager instance.
    OAuthService oauth_service(config);
    // Create a credential store for keeping the clientID, clientSecret,
    // token, and expiration time.
    InMemoryCredentialStore keychain;
    TokenManager<InMemoryCredentialStore> token_manager(oauth_service, keychain);
    // Create the actual audio service from the config and token manager.
    AudioService<InMemoryCredentialStore> service(config, token_manager);
}

SCENARIO("A client requires a synchronous interface to the audio service") {
    GIVEN("An initialized audio service.") {
        // Create the configuration that provides information about the remote host.
        Config config("hostname.com", 443, "tenant ID", "device ID", false);
        // Create the OAuth service for requesting and managing OAuth tokens through
        // a token manager instance.
        OAuthService oauth_service(config);
        // Create a credential store for keeping the clientID, clientSecret,
        // token, and expiration time.
        InMemoryCredentialStore keychain;
        TokenManager<InMemoryCredentialStore> token_manager(oauth_service, keychain);
        // Create the actual audio service from the config and token manager.
        auto models_stub = new ::sensory::api::v1::audio::MockAudioModelsStub;
        auto biometrics_stub = new ::sensory::api::v1::audio::MockAudioBiometricsStub;
        auto events_stub = new ::sensory::api::v1::audio::MockAudioEventsStub;
        auto transcription_stub = new ::sensory::api::v1::audio::MockAudioTranscriptionsStub;
        auto synthesis_stub = new ::sensory::api::v1::audio::MockAudioSynthesisStub;
        AudioService<InMemoryCredentialStore> service(config,
            token_manager,
            models_stub,
            biometrics_stub,
            events_stub,
            transcription_stub,
            synthesis_stub
        );

        auto audio_config = new ::sensory::api::v1::audio::AudioConfig;
        audio_config->set_encoding(sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16);
        audio_config->set_sampleratehertz(16000);
        audio_config->set_audiochannelcount(1);
        audio_config->set_languagecode("en-US");

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
            THEN("the function catches the null stream and throws an error") {
                auto config = new ::sensory::api::v1::audio::CreateEnrollmentConfig;
                config->set_modelname("modelName");
                config->set_userid("userID");
                config->set_description("description");
                config->set_islivenessenabled(true);
                config->set_enrollmentduration(10.f);
                REQUIRE_THROWS_AS(service.create_enrollment(&context, audio_config, config), NullStreamError);
            }
        }

        WHEN("CreateEnrollment is called and the first Write fails") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<CreateEnrollmentRequest, CreateEnrollmentResponse>();
            EXPECT_CALL(*biometrics_stub, CreateEnrollmentRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the audio config.
            EXPECT_CALL(*mock_stream, Write(_, _))
                .Times(1).WillOnce(testing::Return(false));
            ClientContext context;
            THEN("the function catches the write failure and throws an error") {
                auto config = new ::sensory::api::v1::audio::CreateEnrollmentConfig;
                config->set_modelname("modelName");
                config->set_userid("userID");
                config->set_description("description");
                config->set_islivenessenabled(true);
                config->set_enrollmentduration(10.f);
                REQUIRE_THROWS_AS(service.create_enrollment(&context, audio_config, config), WriteStreamError);
            }
        }

        WHEN("CreateEnrollment is called with a valid connection") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<CreateEnrollmentRequest, CreateEnrollmentResponse>();
            EXPECT_CALL(*biometrics_stub, CreateEnrollmentRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the audio config.
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
            auto audio_config = new ::sensory::api::v1::audio::AudioConfig;
            audio_config->set_encoding(sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16);
            audio_config->set_sampleratehertz(16000);
            audio_config->set_audiochannelcount(1);
            audio_config->set_languagecode("en-US");
            auto config = new ::sensory::api::v1::audio::CreateEnrollmentConfig;
            config->set_modelname("modelName");
            config->set_userid("userID");
            config->set_description("description");
            config->set_islivenessenabled(true);
            config->set_enrollmentduration(10.f);
            auto stream = service.create_enrollment(&context, audio_config, config);
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
            auto config = new ::sensory::api::v1::audio::AuthenticateConfig;
            config->set_enrollmentid("enrollmentID");
            config->set_islivenessenabled(true);
            config->set_sensitivity(sensory::api::v1::audio::ThresholdSensitivity::LOW);
            config->set_security(sensory::api::v1::audio::AuthenticateConfig_ThresholdSecurity_LOW);
            THEN("the function catches the null stream and throws an error") {
                REQUIRE_THROWS_AS(service.authenticate(&context, audio_config, config), NullStreamError);
            }
        }

        WHEN("Authenticate is called and the first Write fails") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<AuthenticateRequest, AuthenticateResponse>();
            EXPECT_CALL(*biometrics_stub, AuthenticateRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the audio config.
            EXPECT_CALL(*mock_stream, Write(_, _))
                .Times(1).WillOnce(testing::Return(false));
            ClientContext context;
            auto config = new ::sensory::api::v1::audio::AuthenticateConfig;
            config->set_enrollmentid("enrollmentID");
            config->set_islivenessenabled(true);
            config->set_sensitivity(sensory::api::v1::audio::ThresholdSensitivity::LOW);
            config->set_security(sensory::api::v1::audio::AuthenticateConfig_ThresholdSecurity_LOW);
            THEN("the function catches the write failure and throws an error") {
                REQUIRE_THROWS_AS(service.authenticate(&context, audio_config, config), WriteStreamError);
            }
        }

        WHEN("Authenticate is called with a valid connection") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<AuthenticateRequest, AuthenticateResponse>();
            EXPECT_CALL(*biometrics_stub, AuthenticateRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the audio config.
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
            auto audio_config = new ::sensory::api::v1::audio::AudioConfig;
            audio_config->set_encoding(sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16);
            audio_config->set_sampleratehertz(16000);
            audio_config->set_audiochannelcount(1);
            audio_config->set_languagecode("en-US");
            auto config = new ::sensory::api::v1::audio::AuthenticateConfig;
            config->set_enrollmentid("enrollmentID");
            config->set_islivenessenabled(true);
            config->set_sensitivity(sensory::api::v1::audio::ThresholdSensitivity::LOW);
            config->set_security(sensory::api::v1::audio::AuthenticateConfig_ThresholdSecurity_LOW);
            auto stream = service.authenticate(&context, audio_config, config);
            THEN("a unique pointer to the mock stream is returned") {
                REQUIRE(stream.get() == mock_stream);
            }
        }

        // ----- ValidateEvent -------------------------------------------------

        WHEN("ValidateEvent is called without a valid connection") {
            // Expect the call and return a null pointer for the stream.
            EXPECT_CALL(*events_stub, ValidateEventRaw(_))
                .Times(1).WillOnce(testing::Return(nullptr));
            ClientContext context;
            THEN("the function catches the null stream and throws an error") {
                auto config = new ::sensory::api::v1::audio::ValidateEventConfig;
                config->set_modelname("modelName");
                config->set_userid("userID");
                config->set_sensitivity(sensory::api::v1::audio::ThresholdSensitivity::LOW);
                REQUIRE_THROWS_AS(service.validate_event(&context, audio_config, config), NullStreamError);
            }
        }

        WHEN("ValidateEvent is called and the first Write fails") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<ValidateEventRequest, ValidateEventResponse>();
            EXPECT_CALL(*events_stub, ValidateEventRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the audio config.
            EXPECT_CALL(*mock_stream, Write(_, _))
                .Times(1).WillOnce(testing::Return(false));
            ClientContext context;
            THEN("the function catches the write failure and throws an error") {
                auto config = new ::sensory::api::v1::audio::ValidateEventConfig;
                config->set_modelname("modelName");
                config->set_userid("userID");
                config->set_sensitivity(sensory::api::v1::audio::ThresholdSensitivity::LOW);
                REQUIRE_THROWS_AS(service.validate_event(&context, audio_config, config), WriteStreamError);
            }
        }

        WHEN("ValidateEvent is called with a valid connection") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<ValidateEventRequest, ValidateEventResponse>();
            EXPECT_CALL(*events_stub, ValidateEventRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the audio config.
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
            auto audio_config = new ::sensory::api::v1::audio::AudioConfig;
            audio_config->set_encoding(sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16);
            audio_config->set_sampleratehertz(16000);
            audio_config->set_audiochannelcount(1);
            audio_config->set_languagecode("en-US");
            auto config = new ::sensory::api::v1::audio::ValidateEventConfig;
            config->set_modelname("modelName");
            config->set_userid("userID");
            config->set_sensitivity(sensory::api::v1::audio::ThresholdSensitivity::LOW);
            auto stream = service.validate_event(&context, audio_config, config);
            THEN("a unique pointer to the mock stream is returned") {
                REQUIRE(stream.get() == mock_stream);
            }
        }

        // ----- CreateEnrolledEvent -------------------------------------------

        WHEN("CreateEnrolledEvent is called without a valid connection") {
            // Expect the call and return a null pointer for the stream.
            EXPECT_CALL(*events_stub, CreateEnrolledEventRaw(_))
                .Times(1).WillOnce(testing::Return(nullptr));
            ClientContext context;
            auto config = new ::sensory::api::v1::audio::CreateEnrollmentEventConfig;
            config->set_modelname("modelName");
            config->set_userid("userID");
            config->set_description("description");
            config->set_enrollmentduration(10.f);
            THEN("the function catches the null stream and throws an error") {
                REQUIRE_THROWS_AS(service.create_event_enrollment(&context, audio_config, config), NullStreamError);
            }
        }

        WHEN("CreateEnrolledEvent is called and the first Write fails") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<CreateEnrolledEventRequest, CreateEnrollmentResponse>();
            EXPECT_CALL(*events_stub, CreateEnrolledEventRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the audio config.
            EXPECT_CALL(*mock_stream, Write(_, _))
                .Times(1).WillOnce(testing::Return(false));
            ClientContext context;
            auto config = new ::sensory::api::v1::audio::CreateEnrollmentEventConfig;
            config->set_modelname("modelName");
            config->set_userid("userID");
            config->set_description("description");
            config->set_enrollmentduration(10.f);
            THEN("the function catches the write failure and throws an error") {
                REQUIRE_THROWS_AS(service.create_event_enrollment(&context, audio_config, config), WriteStreamError);
            }
        }

        WHEN("CreateEnrolledEvent is called with a valid connection") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<CreateEnrolledEventRequest, CreateEnrollmentResponse>();
            EXPECT_CALL(*events_stub, CreateEnrolledEventRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the audio config.
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
            auto audio_config = new ::sensory::api::v1::audio::AudioConfig;
            audio_config->set_encoding(sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16);
            audio_config->set_sampleratehertz(16000);
            audio_config->set_audiochannelcount(1);
            audio_config->set_languagecode("en-US");
            auto config = new ::sensory::api::v1::audio::CreateEnrollmentEventConfig;
            config->set_modelname("modelName");
            config->set_userid("userID");
            config->set_description("description");
            config->set_enrollmentduration(10.f);
            auto stream = service.create_event_enrollment(&context, audio_config, config);
            THEN("a unique pointer to the mock stream is returned") {
                REQUIRE(stream.get() == mock_stream);
            }
        }

        // ----- ValidateEnrolledEvent -----------------------------------------

        WHEN("ValidateEnrolledEvent is called without a valid connection") {
            // Expect the call and return a null pointer for the stream.
            EXPECT_CALL(*events_stub, ValidateEnrolledEventRaw(_))
                .Times(1).WillOnce(testing::Return(nullptr));
            ClientContext context;
            auto config = new ::sensory::api::v1::audio::ValidateEnrolledEventConfig;
            config->set_enrollmentid("enrollmentID");
            config->set_sensitivity(sensory::api::v1::audio::ThresholdSensitivity::LOW);
            THEN("the function catches the null stream and throws an error") {
                REQUIRE_THROWS_AS(service.validate_enrolled_event(&context, audio_config, config), NullStreamError);
            }
        }

        WHEN("ValidateEnrolledEvent is called and the first Write fails") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<ValidateEnrolledEventRequest, ValidateEnrolledEventResponse>();
            EXPECT_CALL(*events_stub, ValidateEnrolledEventRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the audio config.
            EXPECT_CALL(*mock_stream, Write(_, _))
                .Times(1).WillOnce(testing::Return(false));
            ClientContext context;
            auto config = new ::sensory::api::v1::audio::ValidateEnrolledEventConfig;
            config->set_enrollmentid("enrollmentID");
            config->set_sensitivity(sensory::api::v1::audio::ThresholdSensitivity::LOW);
            THEN("the function catches the write failure and throws an error") {
                REQUIRE_THROWS_AS(service.validate_enrolled_event(&context, audio_config, config), WriteStreamError);
            }
        }

        WHEN("ValidateEnrolledEvent is called with a valid connection") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<ValidateEnrolledEventRequest, ValidateEnrolledEventResponse>();
            EXPECT_CALL(*events_stub, ValidateEnrolledEventRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the audio config.
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
            auto audio_config = new ::sensory::api::v1::audio::AudioConfig;
            audio_config->set_encoding(sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16);
            audio_config->set_sampleratehertz(16000);
            audio_config->set_audiochannelcount(1);
            audio_config->set_languagecode("en-US");
            auto config = new ::sensory::api::v1::audio::ValidateEnrolledEventConfig;
            config->set_enrollmentid("enrollmentID");
            config->set_sensitivity(sensory::api::v1::audio::ThresholdSensitivity::LOW);
            auto stream = service.validate_enrolled_event(&context, audio_config, config);
            THEN("a unique pointer to the mock stream is returned") {
                REQUIRE(stream.get() == mock_stream);
            }
        }

        // ----- Transcribe ----------------------------------------------------

        WHEN("Transcribe is called without a valid connection") {
            // Expect the call and return a null pointer for the stream.
            EXPECT_CALL(*transcription_stub, TranscribeRaw(_))
                .Times(1).WillOnce(testing::Return(nullptr));
            ClientContext context;
            THEN("the function catches the null stream and throws an error") {
                auto config = new ::sensory::api::v1::audio::TranscribeConfig;
                config->set_modelname("modelName");
                config->set_userid("userID");
                REQUIRE_THROWS_AS(service.transcribe(&context, audio_config, config), NullStreamError);
            }
        }

        WHEN("Transcribe is called and the first Write fails") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<TranscribeRequest, TranscribeResponse>();
            EXPECT_CALL(*transcription_stub, TranscribeRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the audio config.
            EXPECT_CALL(*mock_stream, Write(_, _))
                .Times(1).WillOnce(testing::Return(false));
            ClientContext context;
            THEN("the function catches the write failure and throws an error") {
                auto config = new ::sensory::api::v1::audio::TranscribeConfig;
                config->set_modelname("modelName");
                config->set_userid("userID");
                REQUIRE_THROWS_AS(service.transcribe(&context, audio_config, config), WriteStreamError);
            }
        }

        WHEN("Transcribe is called with a valid connection") {
            // Expect the call and return a mock stream.
            auto mock_stream = new MockClientReaderWriter<TranscribeRequest, TranscribeResponse>();
            EXPECT_CALL(*transcription_stub, TranscribeRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the audio config.
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
            auto config = new ::sensory::api::v1::audio::TranscribeConfig;
            config->set_modelname("modelName");
            config->set_userid("userID");
            auto stream = service.transcribe(&context, audio_config, config);
            THEN("a unique pointer to the mock stream is returned") {
                REQUIRE(stream.get() == mock_stream);
            }
        }

        // ----- Synthesize Speech ---------------------------------------------

        WHEN("SynthesizeSpeech is called without a valid connection") {
            EXPECT_CALL(*synthesis_stub, SynthesizeSpeechRaw(_, _))
                .Times(1).WillOnce(testing::Return(nullptr));
            ClientContext context;
            THEN("the function catches the null stream and throws an error") {
                REQUIRE_THROWS_AS(service.synthesize_speech(
                    &context,
                    audio_config,
                    "craig",
                    "Hello, World!"
                ), NullStreamError);
            }
        }

        WHEN("SynthesizeSpeech is called with a valid connection") {
            auto mock_stream = new MockClientReader<SynthesizeSpeechResponse>();
            EXPECT_CALL(*synthesis_stub, SynthesizeSpeechRaw(_, _))
                .Times(1).WillOnce(testing::Return(mock_stream));
            ClientContext context;
            auto stream = service.synthesize_speech(&context, audio_config, "craig", "Hello, World!");
            THEN("a unique pointer to the mock stream is returned") {
                REQUIRE(stream.get() == mock_stream);
            }
        }
    }
}
