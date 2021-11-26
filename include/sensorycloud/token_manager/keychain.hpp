// A secure credential provider for the Sensory Cloud C++ SDK.
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

#ifndef SENSORY_CLOUD_TOKEN_MANAGER_KEYCHAIN_HPP_
#define SENSORY_CLOUD_TOKEN_MANAGER_KEYCHAIN_HPP_

#include <exception>
#include <string>

#if defined(_WIN32) || defined(_WIN64)  // Windows
#include <windows.h>
#include <wincred.h>
#include <intsafe.h>
#elif defined(__CYGWIN__) && !defined(_WIN32)  // Cygwin POSIX Windows

#elif defined(__ANDROID__)  // Android flavored Linux

#elif defined(__linux__)  // Debian, Ubuntu, Gentoo, Fedora, openSUSE, RedHat, Centos and other

#elif defined(__unix__) || !defined(__APPLE__) && defined(__MACH__)  // FreeBSD, NetBSD, OpenBSD, DragonFly BSD

#elif defined(__hpux)  // HP-UX

#elif defined(_AIX)  // IBM AIX

#elif defined(__APPLE__) && defined(__MACH__) // Apple OSX and iOS (Darwin)
#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#elif defined(__sun) && defined(__SVR4)  // Oracle Solaris, Open Indiana

#else  // Unrecognized system architecture

#endif

/// @brief The Sensory Cloud SDK.
namespace sensory {

/// @brief Modules for generating and storing secure credentials.
namespace token_manager {

/// @brief A keychain manager for interacting with the OS credential manager.
class Keychain {
 private:
    /// the package name that identifies the owner of the keys
    std::string package;

 public:
    /// @brief Initialize a new Keychain interface.
    ///
    /// @param package_ the package identifier in `"com.package.product"` format
    ///
    /// @details
    /// The value of `package_` should remain constant among compatible versions
    /// of the calling application.
    ///
    explicit Keychain(const std::string& package_) : package(package_) { }

    /// @brief Emplace or replace a key/value pair in the key-chain.
    ///
    /// @param key the plain-text key of the value to store
    /// @param value the secure value to store
    /// @details
    /// Unlike most key-value store abstractions in the STL, this
    /// implementation of emplace will overwrite existing values in the
    /// key-value store.
    ///
    inline void emplace(const std::string& key, const std::string& value) const;

    /// @brief Return true if the key exists in the key-chain.
    ///
    /// @param key the plain-text key to check for the existence of
    ///
    inline bool contains(const std::string& key) const;

    /// @brief Look-up a secret value in the key-chain.
    ///
    /// @param key the plain-text key of the value to return
    /// @returns the secret value indexed by the given key
    ///
    inline std::string at(const std::string& key) const;

    /// @brief Remove a secret key-value pair in the key-chain.
    ///
    /// @param key the plain-text key of the pair to remove from the keychain
    ///
    inline void erase(const std::string& key) const;
};

#if defined(_WIN32) || defined(_WIN64)  // Windows

#elif defined(__CYGWIN__) && !defined(_WIN32)  // Cygwin POSIX Windows

#elif defined(__ANDROID__)  // Android flavored Linux

#elif defined(__linux__)  // Debian, Ubuntu, Gentoo, Fedora, openSUSE, RedHat, Centos and other

#elif defined(__unix__) || !defined(__APPLE__) && defined(__MACH__)  // FreeBSD, NetBSD, OpenBSD, DragonFly BSD

#elif defined(__hpux)  // HP-UX

#elif defined(_AIX)  // IBM AIX

#elif defined(__APPLE__) && defined(__MACH__) // Apple OSX and iOS (Darwin)

inline void Keychain::emplace(const std::string& key, const std::string& value) const {
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

inline bool Keychain::contains(const std::string& key) const {
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

inline std::string Keychain::at(const std::string& key) const {
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

inline void Keychain::erase(const std::string& key) const {
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

#endif  // SENSORY_CLOUD_TOKEN_MANAGER_KEYCHAIN_HPP_



