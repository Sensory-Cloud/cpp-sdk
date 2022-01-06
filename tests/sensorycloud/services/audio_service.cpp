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
#include "sensorycloud/services/audio_service.hpp"
#include "sensorycloud/services/oauth_service.hpp"
#include "sensorycloud/token_manager/token_manager.hpp"
#include "sensorycloud/token_manager/insecure_credential_store.hpp"

using sensory::Config;
using sensory::token_manager::InsecureCredentialStore;
using sensory::token_manager::TokenManager;
using sensory::service::OAuthService;
using sensory::service::AudioService;

TEST_CASE("Should create AudioService from Config and TokenManager") {
    // Create the configuration that provides information about the remote host.
    Config config("hostname.com", 443, "tenant ID", "device ID");
    // Create the OAuth service for requesting and managing OAuth tokens through
    // a token manager instance.
    OAuthService oauthService(config);
    // Create a credential store for keeping the clientID, clientSecret,
    // token, and expiration time.
    InsecureCredentialStore keychain(".", "com.sensory.cloud.examples");
    TokenManager<InsecureCredentialStore> tokenManager(oauthService, keychain);
    // Create the actual audio service from the config and token manager.
    AudioService<InsecureCredentialStore> service(config, tokenManager);
}

SCENARIO("A user needs to create an AudioConfig") {
    GIVEN("parameters for an audio config that describe the input stream") {
        const sensory::api::v1::audio::AudioConfig_AudioEncoding& encoding =
            sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16;
        const float sampleRateHertz = 16000;
        const uint32_t audioChannelCount = 1;
        const std::string languageCode = "en-US";
        WHEN("an audio config is dynamically allocated from the parameters") {
            auto config = sensory::service::newAudioConfig(
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

SCENARIO("A user needs to create a CreateEnrollmentConfig") {
    GIVEN("parameters for the enrollment based on a text-independent model") {
        const std::string& modelName = "modelName";
        const std::string& userID = "userID";
        const std::string& description = "Description";
        const bool& isLivenessEnabled = true;
        const float enrollmentDuration = 10.f;
        const int32_t numUtterances = 0;
        WHEN("a CreateEnrollmentConfig is allocated from the parameters") {
            auto config = sensory::service::newCreateEnrollmentConfig(
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
        WHEN("a CreateEnrollmentConfig is allocated from the parameters") {
            auto config = sensory::service::newCreateEnrollmentConfig(
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
    GIVEN("invalid parameters for the enrollment (both enrollmentDuration and numUtterances provided)") {
        const std::string& modelName = "modelName";
        const std::string& userID = "userID";
        const std::string& description = "Description";
        const bool& isLivenessEnabled = true;
        const float enrollmentDuration = 10.f;
        const int32_t numUtterances = 4;
        WHEN("a CreateEnrollmentConfig is allocated from the parameters") {
            THEN("an error is thrown") {
                REQUIRE_THROWS(sensory::service::newCreateEnrollmentConfig(
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
