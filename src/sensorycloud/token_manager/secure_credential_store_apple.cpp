// A secure credential store interface for the SensoryCloud C++ SDK.
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

#if defined(BUILD_SECURE_CREDENTIAL_STORE) && defined(__APPLE__) && defined(__MACH__)

#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#include <exception>
#include "sensorycloud/token_manager/secure_credential_store.hpp"

namespace sensory {

namespace token_manager {

void SecureCredentialStore::emplace(const std::string& key, const std::string& value) const {
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

bool SecureCredentialStore::contains(const std::string& key) const {
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

std::string SecureCredentialStore::at(const std::string& key) const {
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

void SecureCredentialStore::erase(const std::string& key) const {
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

}  // namespace token_manager

}  // namespace sensory

#endif
