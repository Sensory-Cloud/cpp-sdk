// A secure credential store interface for the SensoryCloud C++ SDK.
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

#if defined(BUILD_SECURE_CREDENTIAL_STORE) && defined(_WIN32) || defined(_WIN64)

#include <windows.h>
#include <wincred.h>
#include <intsafe.h>
#include <exception>
#include "sensorycloud/token_manager/secure_credential_store.hpp"

namespace sensory {

namespace token_manager {

void SecureCredentialStore::emplace(const std::string& key, const std::string& value) const { }

bool SecureCredentialStore::contains(const std::string& key) const { return false; }

std::string SecureCredentialStore::at(const std::string& key) const { return ""; }

void SecureCredentialStore::erase(const std::string& key) const { }

}  // namespace token_manager

}  // namespace sensory

#endif
