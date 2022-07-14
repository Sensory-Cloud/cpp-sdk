// An insecure credential store for the SensoryCloud C++ SDK.
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

#ifndef SENSORYCLOUD_TOKEN_MANAGER_IN_MEMORY_CREDENTIAL_STORE_HPP_
#define SENSORYCLOUD_TOKEN_MANAGER_IN_MEMORY_CREDENTIAL_STORE_HPP_

#include <unordered_map>
#include <string>

/// @brief The SensoryCloud SDK.
namespace sensory {

/// @brief Modules for generating and storing secure credentials.
namespace token_manager {

/// @brief A mock secure credential store for testing the TokenManager.
struct InMemoryCredentialStore : public std::unordered_map<std::string, std::string> {
#if (__cplusplus <= 202002L)  // C++ 2020 and later defines contains for STL maps
    /// @brief Return true if the key exists in the key-value store.
    ///
    /// @returns `true` if the key exists, `false` otherwise.
    ///
    inline bool contains(const std::string& key) const {
        return find(key) != end();
    }
#endif

    /// @brief Emplace or replace a key/value pair in the key-chain.
    ///
    /// @param key the plain-text key of the value to store
    /// @param value the secure value to store
    ///
    inline void emplace(const std::string& key, const std::string& value) {
        if (contains(key)) erase(key);
        std::unordered_map<std::string, std::string>::emplace(key, value);
    }
};

}  // namespace token_manager

}  // namespace sensory

#endif  // SENSORYCLOUD_TOKEN_MANAGER_IN_MEMORY_CREDENTIAL_STORE_HPP_
