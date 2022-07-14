// A secure credential store interface for the SensoryCloud C++ SDK.
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

#ifndef SENSORYCLOUD_TOKEN_MANAGER_SECURE_CREDENTIAL_STORE_HPP_
#define SENSORYCLOUD_TOKEN_MANAGER_SECURE_CREDENTIAL_STORE_HPP_

#include <exception>
#include <string>

#if defined(_WIN32) || defined(_WIN64)  // Windows
#include <windows.h>
#include <wincred.h>
#include <intsafe.h>
#elif defined(__CYGWIN__) && !defined(_WIN32)  // Cygwin POSIX Windows

#elif defined(__ANDROID__)  // Android flavored Linux

#elif defined(__linux__)  // Debian, Ubuntu, Gentoo, Fedora, openSUSE, RedHat, Centos and other
#include <libsecret/secret.h>
#elif defined(__unix__) || !defined(__APPLE__) && defined(__MACH__)  // FreeBSD, NetBSD, OpenBSD, DragonFly BSD

#elif defined(__hpux)  // HP-UX

#elif defined(_AIX)  // IBM AIX

#elif defined(__APPLE__) && defined(__MACH__) // Apple OSX and iOS (Darwin)
#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#elif defined(__sun) && defined(__SVR4)  // Oracle Solaris, Open Indiana

#else  // Unrecognized system architecture

#endif

/// @brief The SensoryCloud SDK.
namespace sensory {

/// @brief Modules for generating and storing secure credentials.
namespace token_manager {

/// @brief A secure credential storage manager.
class SecureCredentialStore {
 private:
    /// The package name that identifies the owner of the keys.
    const std::string package;

 public:
    /// @brief Initialize a new secure credential storage interface.
    ///
    /// @param package_ A package identifier in `"com.package.product"` format.
    /// @details
    /// The value of `package_` should remain constant among compatible versions
    /// of the calling application.
    ///
    explicit SecureCredentialStore(const std::string& package_) :
        package(package_) { }

    /// @brief Emplace or replace a key/value pair in the secure credential
    /// store.
    ///
    /// @param key The key of the value to store.
    /// @param value The secure value to store.
    /// @details
    /// Unlike most key-value store abstractions in the STL, this
    /// implementation of emplace will overwrite existing values in the
    /// key-value store.
    ///
    inline void emplace(const std::string& key, const std::string& value) const;

    /// @brief Return true if the key exists in the secure credential store.
    ///
    /// @param key The key to check for the existence of.
    ///
    inline bool contains(const std::string& key) const;

    /// @brief Look-up a secret value in the secure credential store.
    ///
    /// @param key The key of the value to return.
    /// @returns The secret value indexed by the given key.
    ///
    inline std::string at(const std::string& key) const;

    /// @brief Remove a secret key-value pair in the secure credential store.
    ///
    /// @param key The key to remove from the secure credential store.
    ///
    inline void erase(const std::string& key) const;
};

#if defined(_WIN32) || defined(_WIN64)  // Windows

inline void SecureCredentialStore::emplace(const std::string& key, const std::string& value) const {

}

inline bool SecureCredentialStore::contains(const std::string& key) const {
    return false;
}

inline std::string SecureCredentialStore::at(const std::string& key) const {
    return "";
}

inline void SecureCredentialStore::erase(const std::string& key) const {

}

#elif defined(__CYGWIN__) && !defined(_WIN32)  // Cygwin POSIX Windows

#elif defined(__ANDROID__)  // Android flavored Linux

#elif defined(__linux__)  // Debian, Ubuntu, Gentoo, Fedora, openSUSE, RedHat, Centos and other

inline void SecureCredentialStore::emplace(const std::string& key, const std::string& value) const {
    const SecretSchema schema{package.c_str(), SECRET_SCHEMA_NONE, {
        {"key", SECRET_SCHEMA_ATTRIBUTE_STRING},
        {NULL, SecretSchemaAttributeType(0)},
    }};

    GError* error = NULL;
    secret_password_store_sync(
        &schema,
        SECRET_COLLECTION_DEFAULT,
        (package + "." + key).c_str(),
        value.c_str(),
        NULL,  // not cancellable
        &error,
        "key",
        key.c_str(),
        NULL
    );

    if (error != NULL) {  // An error occurred.
        g_error_free(error);
    }
}

inline bool SecureCredentialStore::contains(const std::string& key) const {
    const SecretSchema schema{package.c_str(), SECRET_SCHEMA_NONE, {
        {"key", SECRET_SCHEMA_ATTRIBUTE_STRING},
        {NULL, SecretSchemaAttributeType(0)},
    }};

    GError* error = NULL;
    gchar* raw_values = secret_password_lookup_sync(
        &schema,
        NULL,  // not cancellable
        &error,
        "key",
        key.c_str(),
        NULL
    );

    if (error != NULL) {  // An error occurred.
        g_error_free(error);
        return false;
    } else if (raw_values == NULL) {  // Key-value pair not found.
        return false;
    } else {  // Key-value pair located.
        secret_password_free(raw_values);
        return true;
    }
}

inline std::string SecureCredentialStore::at(const std::string& key) const {
    const SecretSchema schema{package.c_str(), SECRET_SCHEMA_NONE, {
        {"key", SECRET_SCHEMA_ATTRIBUTE_STRING},
        {NULL, SecretSchemaAttributeType(0)},
    }};

    GError* error = NULL;
    gchar* raw_values = secret_password_lookup_sync(
        &schema,
        NULL,  // not cancellable
        &error,
        "key",
        key.c_str(),
        NULL
    );

    if (error != NULL) {  // An error occurred.
        g_error_free(error);
        return "";
    } else if (raw_values == NULL) {  // Key-value pair not found.
        return "";
    } else {  // Key-value pair located.
        const std::string value(raw_values);
        secret_password_free(raw_values);
        return value;
    }
}

inline void SecureCredentialStore::erase(const std::string& key) const {
    const SecretSchema schema{package.c_str(), SECRET_SCHEMA_NONE, {
        {"key", SECRET_SCHEMA_ATTRIBUTE_STRING},
        {NULL, SecretSchemaAttributeType(0)},
    }};

    GError* error = NULL;
    bool deleted = secret_password_clear_sync(
        &schema,
        NULL, // not cancellable
        &error,
        "key",
        key.c_str(),
        NULL
    );

    if (error != NULL) {  // An error occurred
        g_error_free(error);
    } else if (!deleted) {  // Failed to delete.

    }
}

#elif defined(__unix__) || !defined(__APPLE__) && defined(__MACH__)  // FreeBSD, NetBSD, OpenBSD, DragonFly BSD

#elif defined(__hpux)  // HP-UX

#elif defined(_AIX)  // IBM AIX

#elif defined(__APPLE__) && defined(__MACH__) // Apple OSX and iOS (Darwin)

inline void SecureCredentialStore::emplace(const std::string& key, const std::string& value) const {
    OSStatus status = SecKeychainAddGenericPassword(
        NULL,  // default key-chain
        static_cast<UInt32>(package.length()),
        package.data(),
        static_cast<UInt32>(key.length()),
        key.data(),
        static_cast<UInt32>(value.length()),
        value.data(),
        NULL  // unused output parameter
    );

    if (status == errSecDuplicateItem) {  // password exists, overwrite
        SecKeychainItemRef item = NULL;
        status = SecKeychainFindGenericPassword(
            NULL,  // default key-chain
            static_cast<UInt32>(package.length()),
            package.data(),
            static_cast<UInt32>(key.length()),
            key.data(),
            NULL,  // unused output parameter
            NULL,  // unused output parameter
            &item
        );

        if (status == errSecSuccess) {
            status = SecKeychainItemModifyContent(item, NULL,
                static_cast<UInt32>(value.length()),
                value.data()
            );
        }

        if (item)
            CFRelease(item);
    }

    if (status != errSecSuccess)
        throw std::runtime_error("failed to set value");
}

inline bool SecureCredentialStore::contains(const std::string& key) const {
    SecKeychainItemRef item = NULL;
    OSStatus status = SecKeychainFindGenericPassword(
        NULL,  // default key-chain
        static_cast<UInt32>(package.length()),
        package.data(),
        static_cast<UInt32>(key.length()),
        key.data(),
        NULL,  // unused output parameter
        NULL,  // unused output parameter
        NULL
    );

    return status == errSecSuccess;
}

inline std::string SecureCredentialStore::at(const std::string& key) const {
    void *data;
    UInt32 length;
    OSStatus status = SecKeychainFindGenericPassword(
        NULL,  // default key-chain
        static_cast<UInt32>(package.length()),
        package.data(),
        static_cast<UInt32>(key.length()),
        key.data(),
        &length,
        &data,
        NULL  // unused output parameter
    );

    std::string value = "";

    if (status != errSecSuccess) {
        throw std::runtime_error("failed to get value");
    } else if (data != NULL) {
        value = std::string(reinterpret_cast<const char*>(data), length);
        SecKeychainItemFreeContent(NULL, data);
    }

    return value;
}

inline void SecureCredentialStore::erase(const std::string& key) const {
    SecKeychainItemRef item = NULL;
    OSStatus status = SecKeychainFindGenericPassword(
        NULL,  // default key-chain
        static_cast<UInt32>(package.length()),
        package.data(),
        static_cast<UInt32>(key.length()),
        key.data(),
        NULL,  // unused output parameter
        NULL,  // unused output parameter
        &item
    );

    if (status == errSecSuccess)
        status = SecKeychainItemDelete(item);

    if (item)
        CFRelease(item);

    if (status != errSecSuccess)
        std::runtime_error("failed to find key to delete");
}

#elif defined(__sun) && defined(__SVR4)  // Oracle Solaris, Open Indiana

#else  // Unrecognized system architecture

#endif

}  // namespace token_manager

}  // namespace sensory

#endif  // SENSORYCLOUD_TOKEN_MANAGER_SECURE_CREDENTIAL_STORE_HPP_



