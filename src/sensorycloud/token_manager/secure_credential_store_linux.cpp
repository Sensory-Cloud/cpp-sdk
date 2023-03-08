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

#if defined(BUILD_SECURE_CREDENTIAL_STORE) && defined(__linux__) && !defined(__ANDROID__) && !defined(__APPLE__)

#include <libsecret/secret.h>
#include <exception>
#include "sensorycloud/token_manager/secure_credential_store.hpp"

namespace sensory {

namespace token_manager {

void SecureCredentialStore::emplace(const std::string& key, const std::string& value) const {
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

bool SecureCredentialStore::contains(const std::string& key) const {
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

std::string SecureCredentialStore::at(const std::string& key) const {
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

void SecureCredentialStore::erase(const std::string& key) const {
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

}  // namespace token_manager

}  // namespace sensory

#endif
