// The SensoryCloud C++ SDK.
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
#include "./config.hpp"
#include "./services/health_service.hpp"
#include "./services/oauth_service.hpp"
#include "./services/management_service.hpp"
#include "./services/audio_service.hpp"
#include "./services/video_service.hpp"
#include "./token_manager/token_manager.hpp"
#include "./io/ini.hpp"

/// @brief The SensoryCloud SDK.
namespace sensory {

/// @brief The possible types of device enrollment.
enum class EnrollmentType {
    /// Insecure device enrollment.
    None = 0,
    /// Shared secret (i.e., pass-phrase) enrollment.
    SharedSecret,
    /// JavaScript web token enrollment.
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
        throw std::runtime_error("unrecognized enrollment type");
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

/// @brief The SensoryCloud service.
/// @tparam CredentialStore The type for the secure credential store.
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

 private:
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
    /// deviceID = <the UUID for the device running the SDK>
    /// isSecure = <0 for insecure connections, 1 for TLS>
    /// deviceName = <the name of the device>
    /// enrollmentType = <one of [none,sharedSecret,jwt]>
    /// credential = <your credential>
    /// ```
    ///
    SensoryCloud(const io::INIReader& reader, CredentialStore& keychain) :
        config{
            reader.Get("SDK-configuration", "fullyQualifiedDomainName", "localhost:50051"),
            reader.Get("SDK-configuration", "tenantID", ""),
            reader.Get("SDK-configuration", "deviceID", ""),
            reader.GetBoolean("SDK-configuration", "isSecure", false)
        },
        registration_credentials{
            reader.Get("SDK-configuration", "deviceName", ""),
            reader.Get("SDK-configuration", "enrollmentType", "none"),
            reader.Get("SDK-configuration", "credential", "")
        },
        health(config),
        oauth(config),
        token_manager(oauth, keychain),
        management(config, token_manager),
        audio(config, token_manager),
        video(config, token_manager) { }

 public:
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
    /// deviceID = <the UUID for the device running the SDK>
    /// isSecure = <0 for insecure connections, 1 for TLS>
    /// deviceName = <the name of the device>
    /// enrollmentType = <one of [none,sharedSecret,jwt]>
    /// credential = <your credential>
    /// ```
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
    /// deviceID = <the UUID for the device running the SDK>
    /// isSecure = <0 for insecure connections, 1 for TLS>
    /// deviceName = <the name of the device>
    /// enrollmentType = <one of [none,sharedSecret,jwt]>
    /// credential = <your credential>
    /// ```
    ///
    SensoryCloud(FILE* file, CredentialStore& keychain) :
        SensoryCloud(io::INIReader{file}, keychain) { }

    /// @brief Initialize the client connection synchronously.
    ///
    /// @param response A pointer to a device response to populate if device
    ///     registration is required.
    /// @returns the gRPC status as a result of the
    ///
    ::grpc::Status initialize(sensory::api::v1::management::DeviceResponse* response) {
        if (token_manager.has_token()) return {};  // already registered.
        const auto device_credentials = token_manager.has_saved_credentials() ?
            token_manager.get_saved_credentials() : token_manager.generate_credentials();
        // TODO: switch logic to support [none,sharedSecret,jwt]
        return oauth.registerDevice(response,
            registration_credentials.device_name,
            registration_credentials.credential,
            device_credentials.id,
            device_credentials.secret
        );
    }
};

}  // namespace sensory

#endif  // SENSORYCLOUD_SENSORYCLOUD_HPP_
