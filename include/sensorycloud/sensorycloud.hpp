// The SensoryCloud C++ SDK.
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
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXTERNRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifndef SENSORYCLOUD_SENSORYCLOUD_HPP_
#define SENSORYCLOUD_SENSORYCLOUD_HPP_

#include <string>
#include "sensorycloud/config.hpp"
#include "sensorycloud/services/health_service.hpp"
#include "sensorycloud/services/oauth_service.hpp"
#include "sensorycloud/services/management_service.hpp"
#include "sensorycloud/services/audio_service.hpp"
#include "sensorycloud/services/video_service.hpp"
#include "sensorycloud/token_manager/token_manager.hpp"
#include "sensorycloud/token_manager/in_memory_credential_store.hpp"
#include "sensorycloud/token_manager/file_system_credential_store.hpp"
#include "sensorycloud/io/ini.hpp"
#include "sensorycloud/io/path.hpp"
#include "sensorycloud/util/string_extensions.hpp"
#include "sensorycloud/util/jwt.h"
#include "sensorycloud/util/transcript_aggregator.hpp"
#include "sensorycloud/sys/env.hpp"

/// @brief The SensoryCloud SDK.
namespace sensory {

/// @brief The possible types of device enrollment.
enum class EnrollmentType {
    /// Insecure device enrollment.
    None = 0,
    /// Shared secret (i.e., pass-phrase) enrollment.
    SharedSecret,
    /// JSON Web Token (JWT) enrollment.
    JWT
};

/// @brief Parse an EnrollmentType from the given string
///
/// @param enrollment_type The string value to map to an EnrollmentType.
/// @returns The EnrollmentType corresponding to the string key.
/// @exception runtime_error If the given string value is not recognized.
static inline EnrollmentType parse_enrollment_type(const std::string& enrollment_type) {
    if (enrollment_type == "none")
        return EnrollmentType::None;
    else if (enrollment_type == "sharedSecret")
        return EnrollmentType::SharedSecret;
    else if (enrollment_type == "jwt")
        return EnrollmentType::JWT;
    else
        throw std::runtime_error("unrecognized enrollment type: " + enrollment_type);
}

/// @brief A structure containing credentials for device registration.
struct RegistrationCredentials {
    /// The friendly name for the device.
    std::string device_name;
    /// The type of enrollment (e.g., shared credential or JWT).
    EnrollmentType enrollment_type;
    /// The value of the credential.
    std::string credential;

    /// @brief Initialize a new registration credentials.
    ///
    /// @param device_name_ The friendly name for the device.
    /// @param enrollment_type_ The type of enrollment.
    /// @param credential_ The value of the credential.
    ///
    RegistrationCredentials(
        const std::string& device_name_,
        const EnrollmentType& enrollment_type_,
        const std::string& credential_
    ) :
        device_name(device_name_),
        enrollment_type(enrollment_type_),
        credential(credential_) { }

    /// @brief Initialize a new registration credentials.
    ///
    /// @param device_name_ The friendly name for the device.
    /// @param enrollment_type_ The type of enrollment.
    /// @param credential_ The value of the credential.
    ///
    /// @exception std::runtime_error If `enrollment_type_` cannot be parsed
    ///     into an EnrollmentType.
    ///
    RegistrationCredentials(
        const std::string& device_name_,
        const std::string& enrollment_type_,
        const std::string& credential_
    ) :
        RegistrationCredentials(
            device_name_,
            parse_enrollment_type(enrollment_type_),
            credential_
        ) {}
};

/// The key of the device ID environment variable.
static const std::string DEVICE_ID_ENV_KEY = "SENSORYCLOUD_DEVICE_ID";
/// The tag of the device ID in the secure credential store.
static const std::string DEVICE_ID_KEYCHAIN_TAG = "deviceID";
/// The key of the device name environment variable.
static const std::string DEVICE_NAME_ENV_KEY = "SENSORYCLOUD_DEVICE_NAME";
/// The tag of the device name in the secure credential store.
static const std::string DEVICE_NAME_KEYCHAIN_TAG = "deviceName";

/// @brief Get the system device ID.
/// @tparam CredentialStore A key-value store for storing and fetching
/// credentials and tokens.
/// @param keychain The secure credential store to check for a device ID in.
/// @returns The device ID from the credential store if found, otherwise the
/// device ID provided by the `SENSORYCLOUD_DEVICE_NAME` environment variable.
/// If this value is consumed from the environment, a key will be created in the
/// credential store for future usage without environment configuration. If
/// there is neither a value in the keychain, nor provided by the environment,
/// one will be automatically generated and stored in the keychain for future
/// use.
template<typename CredentialStore>
static inline std::string get_system_device_id(CredentialStore& keychain) {
    if (keychain.contains(DEVICE_ID_KEYCHAIN_TAG))
        return keychain.at(DEVICE_ID_KEYCHAIN_TAG);
    auto device_id = sys::get_env_var(DEVICE_ID_ENV_KEY);
    if (device_id.empty())
        device_id = util::uuid_v4();
    keychain.emplace(DEVICE_ID_KEYCHAIN_TAG, device_id);
    return device_id;
}

/// @brief Get the system device name.
/// @tparam CredentialStore A key-value store for storing and fetching
/// credentials and tokens.
/// @param keychain The secure credential store to check for a device name in.
/// @returns The device name from the credential store if found, otherwise the
/// device name provided by the `SENSORYCLOUD_DEVICE_NAME` environment variable.
/// If this value is consumed from the environment, a key will be created in the
/// credential store for future usage without environment configuration.
template<typename CredentialStore>
static inline std::string get_system_device_name(CredentialStore& keychain) {
    if (keychain.contains(DEVICE_NAME_KEYCHAIN_TAG))
        return keychain.at(DEVICE_NAME_KEYCHAIN_TAG);
    auto name = sys::get_env_var(DEVICE_NAME_ENV_KEY);
    if (name.empty())
        name = util::uuid_v4();
    keychain.emplace(DEVICE_NAME_KEYCHAIN_TAG, name);
    return name;
}

/// @brief The SensoryCloud service.
/// @tparam CredentialStore A key-value store for storing and fetching
/// credentials and tokens.
template<typename CredentialStore>
class SensoryCloud {
 private:
    /// The configuration for the remote service.
    const ::sensory::Config config;
    /// The credentials for registering the device with the server.
    const ::sensory::RegistrationCredentials registration_credentials;

 public:
    /// The health service.
    ::sensory::service::HealthService health;
    /// The OAuth service.
    ::sensory::service::OAuthService oauth;
    /// The token manager.
    ::sensory::token_manager::TokenManager<CredentialStore> token_manager;
    /// The management service.
    ::sensory::service::ManagementService<CredentialStore> management;
    /// The audio service.
    ::sensory::service::AudioService<CredentialStore> audio;
    /// The video service.
    ::sensory::service::VideoService<CredentialStore> video;

    /// @brief Initialize the SensoryCloud service.
    ///
    /// @param config_ The config for the remote service.
    /// @param registration_credentials_ The device registration credentials.
    /// @param keychain The secure credential store.
    ///
    SensoryCloud(
        const ::sensory::Config& config_,
        const ::sensory::RegistrationCredentials& registration_credentials_,
        CredentialStore& keychain
    ) :
        config(config_),
        registration_credentials(registration_credentials_),
        health(config),
        oauth(config),
        token_manager(oauth, keychain),
        management(config, token_manager),
        audio(config, token_manager),
        video(config, token_manager) { }

    /// @brief Initialize the SensoryCloud service.
    ///
    /// @param reader The INI file read to parse the contents from.
    /// @param keychain The secure credential store.
    /// @details
    /// The configuration file should contain the following section:
    ///
    /// ```
    /// [SDK-configuration]
    /// fullyQualifiedDomainName = localhost:50051
    /// tenantID = <your tenant ID>
    /// isSecure = <0 for insecure connections, 1 for TLS>
    /// enrollmentType = <one of [none,sharedSecret,jwt]>
    /// credential = <your credential>
    /// ```
    ///
    /// When using this INI construction interface, the device ID and name are
    /// expected to exist as environment variables if needed, otherwise a
    /// device ID and/or name will automatically generated and stored in the
    /// keychain.
    ///
    SensoryCloud(const io::INIReader& reader, CredentialStore& keychain) :
        SensoryCloud({
            io::path::normalize_uri(util::strip(
                reader.get<std::string>("SDK-configuration", "fullyQualifiedDomainName", "localhost:50051")
            )),
            reader.get<std::string>("SDK-configuration", "tenantID", "", true),
            get_system_device_id(keychain),
            reader.get<bool>("SDK-configuration", "isSecure", false)
        }, {
            get_system_device_name(keychain),
            reader.get<std::string>("SDK-configuration", "enrollmentType", "none"),
            reader.get<std::string>("SDK-configuration", "credential", "")
        }, keychain) { }

    /// @brief Initialize the SensoryCloud service.
    ///
    /// @param path The path to an INI file to read the configuration from.
    /// @param keychain The secure credential store.
    /// @details
    /// The configuration file pointed to by the given path should contain the
    /// following section:
    ///
    /// ```
    /// [SDK-configuration]
    /// fullyQualifiedDomainName = localhost:50051
    /// tenantID = <your tenant ID>
    /// isSecure = <0 for insecure connections, 1 for TLS>
    /// enrollmentType = <one of [none,sharedSecret,jwt]>
    /// credential = <your credential>
    /// ```
    ///
    /// When using this INI construction interface, the device ID and name are
    /// expected to exist as environment variables if needed, otherwise a
    /// device ID and/or name will automatically generated and stored in the
    /// keychain.
    ///
    SensoryCloud(const std::string& path, CredentialStore& keychain) :
        SensoryCloud(io::INIReader{path}, keychain) { }

    /// @brief Initialize the SensoryCloud service.
    ///
    /// @param file The INI file to read the configuration from.
    /// @param keychain The secure credential store.
    /// @details
    /// The configuration file should contain the following section:
    ///
    /// ```
    /// [SDK-configuration]
    /// fullyQualifiedDomainName = localhost:50051
    /// tenantID = <your tenant ID>
    /// isSecure = <0 for insecure connections, 1 for TLS>
    /// enrollmentType = <one of [none,sharedSecret,jwt]>
    /// credential = <your credential>
    /// ```
    ///
    /// When using this INI construction interface, the device ID and name are
    /// expected to exist as environment variables if needed, otherwise a
    /// device ID and/or name will automatically generated and stored in the
    /// keychain.
    ///
    SensoryCloud(FILE* file, CredentialStore& keychain) :
        SensoryCloud(io::INIReader{file}, keychain) { }

    /// @brief Initialize the client connection synchronously.
    ///
    /// @param response A pointer to a device response to populate if device
    ///     registration is required.
    /// @returns the gRPC status as a result of the device registration call.
    ///
    ::grpc::Status initialize(sensory::api::v1::management::DeviceResponse* response) {
        if (token_manager.has_token()) return {};  // already registered.
        const auto device_credentials = token_manager.has_saved_credentials() ?
            token_manager.get_saved_credentials() : token_manager.generate_credentials();
        // If we're using a JWT token, it needs to be signed.
        auto credential = registration_credentials.credential;
        std::string key = "-----BEGIN PRIVATE KEY-----\n" + credential + "\n-----END PRIVATE KEY-----";
        if (registration_credentials.enrollment_type == EnrollmentType::JWT)
            credential = jwt::create()
                .set_issuer("sensorycloud-cpp-sdk")
                .set_issued_at(std::chrono::system_clock::now())
                // .set_expires_at(std::chrono::system_clock::now() + std::chrono::minutes{1})
                .set_type("JWT")
                .set_payload_claim("device_name", jwt::claim(registration_credentials.device_name))
                .set_payload_claim("tenant_id", jwt::claim(token_manager.get_service().get_config().get_tenant_id()))
                .set_payload_claim("client_id", jwt::claim(device_credentials.id))
                .sign(jwt::algorithm::ed25519{"", key, "", ""});
        return oauth.register_device(response,
            registration_credentials.device_name,
            credential,
            device_credentials.id,
            device_credentials.secret
        );
    }
};

}  // namespace sensory

#endif  // SENSORYCLOUD_SENSORYCLOUD_HPP_
