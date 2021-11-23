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

#ifndef SENSORY_CLOUD_KEYCHAIN_HPP_
#define SENSORY_CLOUD_KEYCHAIN_HPP_

#include <exception>
#include <string>

#if defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#endif

/// @brief The Sensory Cloud SDK.
namespace sensory {

/// @brief Modules for generating and storing secure credentials.
namespace token_manager {

#if defined(_WIN32)
    // #define PLATFORM_NAME "windows" // Windows
#elif defined(_WIN64)
    // #define PLATFORM_NAME "windows" // Windows
#elif defined(__CYGWIN__) && !defined(_WIN32)
    // #define PLATFORM_NAME "windows" // Windows (Cygwin POSIX under Microsoft Window)
#elif defined(__ANDROID__)
    // #define PLATFORM_NAME "android" // Android (implies Linux, so it must come first)
#elif defined(__linux__)
    // #define PLATFORM_NAME "linux" // Debian, Ubuntu, Gentoo, Fedora, openSUSE, RedHat, Centos and other
#elif defined(__unix__) || !defined(__APPLE__) && defined(__MACH__)
    // #define PLATFORM_NAME "bsd" // FreeBSD, NetBSD, OpenBSD, DragonFly BSD
#elif defined(__hpux)
    // #define PLATFORM_NAME "hp-ux" // HP-UX
#elif defined(_AIX)
    // #define PLATFORM_NAME "aix" // IBM AIX
#elif defined(__APPLE__) && defined(__MACH__) // Apple OSX and iOS (Darwin)
/// @brief A keychain manager for interacting with the keychain.
class Keychain{
 private:
    /// the package name that identifies the owner of the keys
    std::string package;

 public:
    /// @brief Initialize a new Apple Keychain interface.
    ///
    /// @param package_ the package identifier in "com.package.product" format
    ///
    explicit Keychain(const std::string& package_) : package(package_) { }

    /// @brief Insert / Update a key/value pair in the key-chain.
    ///
    /// @param key the plain-text ID of the value to store
    /// @param value the secure value to store
    ///
    void insert(const std::string& key, const std::string& value) {
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

        if (status == errSecDuplicateItem)  // password exists, overwrite
            return update(key, value);

        if (status != errSecSuccess)
            throw std::runtime_error("failed to set value");
    }

    /// @brief Update a key/value pair in the key-chain.
    ///
    /// @param key the plain-text ID of the value to update
    /// @param value the new secure value to store
    ///
    void update(const std::string& key, const std::string& value) {
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

        if (status != errSecSuccess)
            throw std::runtime_error("failed to update value");
    }

    /// @brief Look-up a secret value in the key-chain.
    ///
    /// @param key the plain-text ID of the value to return the secure value of
    /// @returns the secret value indexed by the given key
    ///
    std::string get(const std::string& key) {
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

    /// @brief Remove a secret key-value pair in the key-chain.
    ///
    /// @param key the plain-text ID of the key to remove from the keychain
    ///
    void remove(const std::string& key) {
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
};
#elif defined(__sun) && defined(__SVR4)
    // #define PLATFORM_NAME "solaris" // Oracle Solaris, Open Indiana
#else
    // #define PLATFORM_NAME NULL
#endif

}  // namespace token_manager

}  // namespace sensory

#endif  // SENSORY_CLOUD_KEYCHAIN_HPP_



