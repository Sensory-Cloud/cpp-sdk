// Test cases for the audio service.
//
// Copyright (c) 2021 Sensory, Inc.
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

using ::sensory::service::audio::new_audio_config;
using ::sensory::service::audio::new_create_enrollment_config;
using ::sensory::service::audio::new_authenticate_config;
using ::sensory::service::audio::new_validate_event_config;
using ::sensory::service::audio::new_create_enrollment_event_config;
using ::sensory::service::audio::new_validate_enrolled_event_config;
using ::sensory::service::audio::new_transcribe_config;
using ::sensory::service::audio::TranscriptAggregator;

using testing::_;

// ---------------------------------------------------------------------------
// MARK: new_audio_config
// ---------------------------------------------------------------------------

SCENARIO("A user needs to create an AudioConfig") {
    GIVEN("parameters for an audio config that describe the input stream") {
        const sensory::api::v1::audio::AudioConfig_AudioEncoding& encoding =
            sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16;
        const float sample_rate_hertz = 16000;
        const uint32_t audio_channel_count = 1;
        const std::string language_code = "en-US";
        WHEN("the config is dynamically allocated from the parameters") {
            auto config = new_audio_config(
                encoding,
                sample_rate_hertz,
                audio_channel_count,
                language_code
            );
            THEN("a pointer is returned with the variables set") {
                REQUIRE(config != nullptr);
                REQUIRE(config->encoding() == encoding);
                REQUIRE(config->sampleratehertz() == sample_rate_hertz);
                REQUIRE(config->audiochannelcount() == audio_channel_count);
                REQUIRE(config->languagecode() == language_code);
            }
            delete config;
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: new_create_enrollment_config
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
            auto config = new_create_enrollment_config(
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
                REQUIRE_THAT("", Catch::Equals(config->referenceid()));
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
            auto config = new_create_enrollment_config(
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
                REQUIRE_THAT("", Catch::Equals(config->referenceid()));
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
                REQUIRE_THROWS(new_create_enrollment_config(
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
    GIVEN("parameters for the enrollment with a reference ID") {
        const std::string& modelName = "modelName";
        const std::string& userID = "userID";
        const std::string& description = "Description";
        const bool& isLivenessEnabled = true;
        const float enrollmentDuration = 0.f;
        const int32_t numUtterances = 0;
        const std::string reference_id = "reference_id";
        WHEN("the config is allocated from the parameters") {
            auto config = new_create_enrollment_config(
                modelName,
                userID,
                description,
                isLivenessEnabled,
                enrollmentDuration,
                numUtterances,
                reference_id
            );
            THEN("a pointer is returned with the variables set") {
                REQUIRE(config != nullptr);
                REQUIRE(config->modelname() == modelName);
                REQUIRE(config->userid() == userID);
                REQUIRE(config->description() == description);
                REQUIRE(config->islivenessenabled() == isLivenessEnabled);
                REQUIRE(config->enrollmentduration() == enrollmentDuration);
                REQUIRE(config->enrollmentnumutterances() == numUtterances);
                REQUIRE_THAT(reference_id, Catch::Equals(config->referenceid()));
            }
            delete config;
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: new_authenticate_config
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
            auto config = new_authenticate_config(
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
            auto config = new_authenticate_config(
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
// MARK: new_validate_event_config
// ---------------------------------------------------------------------------

SCENARIO("A user needs to create a ValidateEventConfig") {
    GIVEN("parameters for an event validation stream") {
        const std::string modelName = "modelName";
        const std::string userID = "userID";
        const sensory::api::v1::audio::ThresholdSensitivity sensitivity =
            sensory::api::v1::audio::ThresholdSensitivity::LOW;
        WHEN("the config is dynamically allocated from the parameters") {
            auto config = new_validate_event_config(
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
// MARK: new_create_enrollment_event_config
// ---------------------------------------------------------------------------

SCENARIO("A user needs to create a CreateEnrollmentEventConfig") {
    GIVEN("parameters for the enrollment based on a text-independent model") {
        const std::string& modelName = "modelName";
        const std::string& userID = "userID";
        const std::string& description = "Description";
        const float enrollmentDuration = 10.f;
        const int32_t numUtterances = 0;
        WHEN("the config is allocated from the parameters") {
            auto config = new_create_enrollment_event_config(
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
                REQUIRE_THAT("", Catch::Equals(config->referenceid()));
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
            auto config = new_create_enrollment_event_config(
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
                REQUIRE_THAT("", Catch::Equals(config->referenceid()));
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
                REQUIRE_THROWS(new_create_enrollment_event_config(
                    modelName,
                    userID,
                    description,
                    enrollmentDuration,
                    numUtterances
                ));
            }
        }
    }
    GIVEN("parameters for the enrollment with a reference ID") {
        const std::string& modelName = "modelName";
        const std::string& userID = "userID";
        const std::string& description = "Description";
        const float enrollmentDuration = 0.f;
        const int32_t numUtterances = 0;
        const std::string reference_id = "reference_id";
        WHEN("the config is allocated from the parameters") {
            auto config = new_create_enrollment_event_config(
                modelName,
                userID,
                description,
                enrollmentDuration,
                numUtterances,
                reference_id
            );
            THEN("a pointer is returned with the variables set") {
                REQUIRE(config != nullptr);
                REQUIRE(config->modelname() == modelName);
                REQUIRE(config->userid() == userID);
                REQUIRE(config->description() == description);
                REQUIRE(config->enrollmentduration() == enrollmentDuration);
                REQUIRE(config->enrollmentnumutterances() == numUtterances);
                REQUIRE_THAT(reference_id, Catch::Equals(config->referenceid()));
            }
            delete config;
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: new_validate_enrolled_event_config
// ---------------------------------------------------------------------------

SCENARIO("A user needs to create an ValidateEnrolledEventConfig") {
    GIVEN("parameters for an authentication stream") {
        const std::string enrollmentID = "enrollmentID";
        const sensory::api::v1::audio::ThresholdSensitivity sensitivity =
            sensory::api::v1::audio::ThresholdSensitivity::LOW;
        WHEN("the config is dynamically allocated from the parameters") {
            auto config = new_validate_enrolled_event_config(
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
            auto config = new_validate_enrolled_event_config(
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
// MARK: new_transcribe_config
// ---------------------------------------------------------------------------

SCENARIO("A user needs to create a TranscribeConfig") {
    GIVEN("parameters for an audio transcription stream") {
        const std::string modelName = "modelName";
        const std::string userID = "userID";
        WHEN("the config is dynamically allocated from the parameters") {
            auto config = new_transcribe_config(modelName, userID);
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
// MARK: TranscriptAggregator
// ---------------------------------------------------------------------------

SCENARIO("A client needs to track a full transcript using the STT engine") {
    WHEN("A transcript aggregator is initialized") {
        TranscriptAggregator aggregator;
        THEN("The transcript aggregator embodies a null state") {
            REQUIRE(aggregator.get_word_list().empty());
            REQUIRE(aggregator.get_transcript().empty());
        }
    }
    GIVEN("An empty transcript aggregator") {
        TranscriptAggregator aggregator;
        WHEN("An empty response is passed to the aggregator") {
            aggregator.process_response({});
            THEN("The initial state of the aggregator does not change") {
                REQUIRE(aggregator.get_word_list().empty());
                REQUIRE(aggregator.get_transcript().empty());
            }
        }
        WHEN("A single word response is passed to the aggregator") {
            // Create a mock word at index 0.
            TranscribeWord foo;
            foo.set_word("foo");
            foo.set_wordindex(0);
            // Create the transcription response with the word update.
            TranscribeWordResponse rsp;
            rsp.set_firstwordindex(0);
            rsp.set_lastwordindex(0);
            (*rsp.mutable_words()->Add()) = foo;
            // Update the structure with the single word transcript.
            aggregator.process_response(rsp);
            THEN("The aggregator is updated with the transcript state") {
                REQUIRE(1 == aggregator.get_word_list().size());
                REQUIRE_THAT("foo", Catch::Equals(aggregator.get_transcript()));
            }
        }
        WHEN("A multi-word response is passed to the aggregator") {
            // Create a mock word at index 0.
            TranscribeWord foo;
            foo.set_word("foo ");
            foo.set_wordindex(0);
            TranscribeWord bar;
            bar.set_word("bar");
            bar.set_wordindex(1);
            // Create the transcription response with the word update.
            TranscribeWordResponse rsp;
            rsp.set_firstwordindex(0);
            rsp.set_lastwordindex(1);
            (*rsp.mutable_words()->Add()) = foo;
            (*rsp.mutable_words()->Add()) = bar;
            aggregator.process_response(rsp);
            THEN("The aggregator is updated with the transcript state") {
                REQUIRE(2 == aggregator.get_word_list().size());
                REQUIRE_THAT("foo bar", Catch::Equals(aggregator.get_transcript()));
            }
        }
    }
    GIVEN("A transcript aggregator with existing state") {
        TranscriptAggregator aggregator;
        // Create mock words.
        TranscribeWord foo;
        foo.set_word("foo");
        foo.set_wordindex(0);
        TranscribeWord bar;
        bar.set_word("bar");
        bar.set_wordindex(1);
        // Create the transcription response with the word update.
        TranscribeWordResponse rsp0;
        rsp0.set_firstwordindex(0);
        rsp0.set_lastwordindex(1);
        (*rsp0.mutable_words()->Add()) = foo;
        (*rsp0.mutable_words()->Add()) = bar;
        aggregator.process_response(rsp0);
        WHEN("An update response is passed to the aggregator that adds a word") {
            TranscribeWord baz;
            baz.set_word("baz");
            baz.set_wordindex(2);
            TranscribeWordResponse rsp1;
            rsp1.set_firstwordindex(0);
            rsp1.set_lastwordindex(2);
            (*rsp1.mutable_words()->Add()) = baz;
            aggregator.process_response(rsp1);
            THEN("The aggregator is updated with the new word") {
                REQUIRE(3 == aggregator.get_word_list().size());
                REQUIRE_THAT("foo bar baz", Catch::Equals(aggregator.get_transcript()));
            }
        }
        WHEN("An update response is passed to the aggregator that replaces a word") {
            TranscribeWord food;
            food.set_word("food");
            food.set_wordindex(0);
            TranscribeWordResponse rsp1;
            rsp1.set_firstwordindex(0);
            rsp1.set_lastwordindex(1);
            (*rsp1.mutable_words()->Add()) = food;
            aggregator.process_response(rsp1);
            THEN("The aggregator is updated with the replacement word") {
                REQUIRE(2 == aggregator.get_word_list().size());
                REQUIRE_THAT("food bar", Catch::Equals(aggregator.get_transcript()));
            }
        }
        WHEN("An update response is passed to the aggregator that replaces a sub-string") {
            TranscribeWord word;
            word.set_word("foobar");
            word.set_wordindex(0);
            TranscribeWordResponse rsp1;
            rsp1.set_firstwordindex(0);
            rsp1.set_lastwordindex(0);
            (*rsp1.mutable_words()->Add()) = word;
            aggregator.process_response(rsp1);
            THEN("The aggregator is updated with the sub-string replacement") {
                REQUIRE(1 == aggregator.get_word_list().size());
                REQUIRE_THAT("foobar", Catch::Equals(aggregator.get_transcript()));
            }
        }
    }
    WHEN("A transcript aggregator is passed an invalid index") {
        TranscriptAggregator aggregator;
        TranscribeWord word;
        word.set_word("foobar");
        word.set_wordindex(1);
        TranscribeWordResponse rsp;
        rsp.set_firstwordindex(0);
        rsp.set_lastwordindex(0);
        (*rsp.mutable_words()->Add()) = word;
        THEN("An expected runtime error is raised") {
            REQUIRE_THROWS(aggregator.process_response(rsp));
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: AudioService
// ---------------------------------------------------------------------------

TEST_CASE("Should create AudioService from Config and TokenManager") {
    // Create the configuration that provides information about the remote host.
    Config config("hostname.com", 443, "tenant ID", "device ID");
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
        // Create the OAuth service for requesting and managing OAuth tokens through
        // a token manager instance.
        OAuthService oauthService(config);
        // Create a credential store for keeping the clientID, clientSecret,
        // token, and expiration time.
        InMemoryCredentialStore keychain;
        TokenManager<InMemoryCredentialStore> tokenManager(oauthService, keychain);
        // Create the actual audio service from the config and token manager.
        auto models_stub = new ::sensory::api::v1::audio::MockAudioModelsStub;
        auto biometrics_stub = new ::sensory::api::v1::audio::MockAudioBiometricsStub;
        auto events_stub = new ::sensory::api::v1::audio::MockAudioEventsStub;
        auto transcription_stub = new ::sensory::api::v1::audio::MockAudioTranscriptionsStub;
        auto synthesis_stub = new ::sensory::api::v1::audio::MockAudioSynthesisStub;
        AudioService<InMemoryCredentialStore> service(config,
            tokenManager,
            models_stub,
            biometrics_stub,
            events_stub,
            transcription_stub,
            synthesis_stub
        );

        auto audioConfig = new_audio_config(
            sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16,
            16000,
            1,
            "en-US"
        );

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
            EXPECT_CALL(*biometrics_stub, CreateEnrollmentRaw(_))
                .Times(1).WillOnce(testing::Return(nullptr));
            ClientContext context;
            THEN("the function catches the null stream and throws an error") {
                REQUIRE_THROWS_AS(service.createEnrollment(&context, audioConfig, new_create_enrollment_config(
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
            EXPECT_CALL(*biometrics_stub, CreateEnrollmentRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the audio config.
            EXPECT_CALL(*mock_stream, Write(_, _))
                .Times(1).WillOnce(testing::Return(false));
            ClientContext context;
            THEN("the function catches the write failure and throws an error") {
                REQUIRE_THROWS_AS(service.createEnrollment(&context, audioConfig, new_create_enrollment_config(
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
            auto audioConfig = new_audio_config(
                sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16,
                16000,
                1,
                "en-US"
            );
            auto stream = service.createEnrollment(&context, audioConfig, new_create_enrollment_config(
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
            EXPECT_CALL(*biometrics_stub, AuthenticateRaw(_))
                .Times(1).WillOnce(testing::Return(nullptr));
            ClientContext context;
            THEN("the function catches the null stream and throws an error") {
                REQUIRE_THROWS_AS(service.authenticate(&context, audioConfig, new_authenticate_config(
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
            EXPECT_CALL(*biometrics_stub, AuthenticateRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the audio config.
            EXPECT_CALL(*mock_stream, Write(_, _))
                .Times(1).WillOnce(testing::Return(false));
            ClientContext context;
            THEN("the function catches the write failure and throws an error") {
                REQUIRE_THROWS_AS(service.authenticate(&context, audioConfig, new_authenticate_config(
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
            auto audioConfig = new_audio_config(
                sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16,
                16000,
                1,
                "en-US"
            );
            auto stream = service.authenticate(&context, audioConfig, new_authenticate_config(
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
            EXPECT_CALL(*events_stub, ValidateEventRaw(_))
                .Times(1).WillOnce(testing::Return(nullptr));
            ClientContext context;
            THEN("the function catches the null stream and throws an error") {
                REQUIRE_THROWS_AS(service.validateEvent(&context, audioConfig, new_validate_event_config(
                    "modelName",
                    "userID",
                    sensory::api::v1::audio::ThresholdSensitivity::LOW
                )), sensory::service::NullStreamError);
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
                REQUIRE_THROWS_AS(service.validateEvent(&context, audioConfig, new_validate_event_config(
                    "modelName",
                    "userID",
                    sensory::api::v1::audio::ThresholdSensitivity::LOW
                )), sensory::service::WriteStreamError);
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
            auto audioConfig = new_audio_config(
                sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16,
                16000,
                1,
                "en-US"
            );
            auto stream = service.validateEvent(&context, audioConfig, new_validate_event_config(
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
            EXPECT_CALL(*events_stub, CreateEnrolledEventRaw(_))
                .Times(1).WillOnce(testing::Return(nullptr));
            ClientContext context;
            THEN("the function catches the null stream and throws an error") {
                REQUIRE_THROWS_AS(service.createEventEnrollment(&context, audioConfig, new_create_enrollment_event_config(
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
            EXPECT_CALL(*events_stub, CreateEnrolledEventRaw(_))
                .Times(1).WillOnce(testing::Return(mock_stream));
            // Expect the SDK to call the first write to send the audio config.
            EXPECT_CALL(*mock_stream, Write(_, _))
                .Times(1).WillOnce(testing::Return(false));
            ClientContext context;
            THEN("the function catches the write failure and throws an error") {
                REQUIRE_THROWS_AS(service.createEventEnrollment(&context, audioConfig, new_create_enrollment_event_config(
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
            auto audioConfig = new_audio_config(
                sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16,
                16000,
                1,
                "en-US"
            );
            auto stream = service.createEventEnrollment(&context, audioConfig, new_create_enrollment_event_config(
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
            EXPECT_CALL(*events_stub, ValidateEnrolledEventRaw(_))
                .Times(1).WillOnce(testing::Return(nullptr));
            ClientContext context;
            THEN("the function catches the null stream and throws an error") {
                REQUIRE_THROWS_AS(service.validateEnrolledEvent(&context, audioConfig, new_validate_enrolled_event_config(
                    "enrollmentID",
                    sensory::api::v1::audio::ThresholdSensitivity::LOW
                )), sensory::service::NullStreamError);
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
            THEN("the function catches the write failure and throws an error") {
                REQUIRE_THROWS_AS(service.validateEnrolledEvent(&context, audioConfig, new_validate_enrolled_event_config(
                    "enrollmentID",
                    sensory::api::v1::audio::ThresholdSensitivity::LOW
                )), sensory::service::WriteStreamError);
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
            auto audioConfig = new_audio_config(
                sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16,
                16000,
                1,
                "en-US"
            );
            auto stream = service.validateEnrolledEvent(&context, audioConfig, new_validate_enrolled_event_config(
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
            EXPECT_CALL(*transcription_stub, TranscribeRaw(_))
                .Times(1).WillOnce(testing::Return(nullptr));
            ClientContext context;
            THEN("the function catches the null stream and throws an error") {
                REQUIRE_THROWS_AS(service.transcribe(&context, audioConfig, new_transcribe_config(
                    "modelName",
                    "userID"
                )), sensory::service::NullStreamError);
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
                REQUIRE_THROWS_AS(service.transcribe(&context, audioConfig, new_transcribe_config(
                    "modelName",
                    "userID"
                )), sensory::service::WriteStreamError);
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
            auto stream = service.transcribe(&context, audioConfig, new_transcribe_config(
                "modelName",
                "userID"
            ));
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
                    audioConfig,
                    "craig",
                    "Hello, World!"
                ), sensory::service::NullStreamError);
            }
        }

        WHEN("SynthesizeSpeech is called with a valid connection") {
            auto mock_stream = new MockClientReader<SynthesizeSpeechResponse>();
            EXPECT_CALL(*synthesis_stub, SynthesizeSpeechRaw(_, _))
                .Times(1).WillOnce(testing::Return(mock_stream));
            ClientContext context;
            auto stream = service.synthesize_speech(&context, audioConfig, "craig", "Hello, World!");
            THEN("a unique pointer to the mock stream is returned") {
                REQUIRE(stream.get() == mock_stream);
            }
        }
    }
}
