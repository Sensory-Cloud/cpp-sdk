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

#ifndef SENSORYCLOUD_TOKEN_MANAGER_SECURE_CREDENTIAL_STORE_HPP_
#define SENSORYCLOUD_TOKEN_MANAGER_SECURE_CREDENTIAL_STORE_HPP_

#include <string>

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
    void emplace(const std::string& key, const std::string& value) const;

    /// @brief Return true if the key exists in the secure credential store.
    ///
    /// @param key The key to check for the existence of.
    ///
    bool contains(const std::string& key) const;

    /// @brief Look-up a secret value in the secure credential store.
    ///
    /// @param key The key of the value to return.
    /// @returns The secret value indexed by the given key.
    ///
    std::string at(const std::string& key) const;

    /// @brief Remove a secret key-value pair in the secure credential store.
    ///
    /// @param key The key to remove from the secure credential store.
    ///
    void erase(const std::string& key) const;
};

}  // namespace token_manager

}  // namespace sensory

#endif  // SENSORYCLOUD_TOKEN_MANAGER_SECURE_CREDENTIAL_STORE_HPP_
