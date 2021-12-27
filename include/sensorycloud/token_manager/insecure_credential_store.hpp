// An insecure credential store for the Sensory Cloud C++ SDK.
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

#ifndef SENSORY_CLOUD_TOKEN_MANAGER_INSECURE_CREDENTIAL_STORE_HPP_
#define SENSORY_CLOUD_TOKEN_MANAGER_INSECURE_CREDENTIAL_STORE_HPP_

#include <exception>
#include <string>
#include <cstdio>

/// @brief The Sensory Cloud SDK.
namespace sensory {

/// @brief Modules for generating and storing secure credentials.
namespace token_manager {

/// @brief An insecure credential storage manager.
class InsecureCredentialStore {
 private:
    /// The root path to write files to for this insecure store.
    const std::string root_path;
    /// The package name that identifies the owner of the keys.
    const std::string package;

 public:
    /// @brief Initialize a new secure credential storage interface.
    ///
    /// @param root_path_ A root path for storing files in the insecure store
    /// @param package_ A package identifier in `"com.package.product"` format.
    /// @details
    /// The value of `package_` should remain constant among compatible versions
    /// of the calling application.
    ///
    explicit InsecureCredentialStore(const std::string& root_path_, const std::string& package_) :
        root_path(root_path_),
        package(package_) { }

    /// @brief Return the path of the given key.
    ///
    /// @param key The key to return the pathname of.
    /// @returns The full pathname for the file associated with the given key.
    ///
    inline std::string key_path(const std::string& key) const {
        return root_path + "/" + package + "." + key;
    }

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
    inline void emplace(const std::string& key, const std::string& value) const {
        std::ofstream outputs;
        outputs.open(key_path(key));
        outputs << value;
        outputs.close();
    }

    /// @brief Return true if the key exists in the secure credential store.
    ///
    /// @param key The key to check for the existence of.
    ///
    inline bool contains(const std::string& key) const {
        std::ifstream inputs;
        inputs.open(key_path(key));
        auto exists = static_cast<bool>(inputs);
        inputs.close();
        return exists;
    }

    /// @brief Look-up a secret value in the secure credential store.
    ///
    /// @param key The key of the value to return.
    /// @returns The secret value indexed by the given key.
    ///
    inline std::string at(const std::string& key) const {
        std::ifstream inputs;
        inputs.open(key_path(key));
        std::string value;
        std::getline(inputs, value);
        inputs.close();
        return value;
    }

    /// @brief Remove a secret key-value pair in the secure credential store.
    ///
    /// @param key The key to remove from the secure credential store.
    ///
    inline void erase(const std::string& key) const {
        remove(key_path(key).c_str());
    }
};

}  // namespace token_manager

}  // namespace sensory

#endif  // SENSORY_CLOUD_TOKEN_MANAGER_INSECURE_CREDENTIAL_STORE_HPP_
