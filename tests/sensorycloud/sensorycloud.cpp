// Test cases for the sensorycloud module.
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
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#define CATCH_CONFIG_MAIN
#include <cstdio>
#include <cstdlib>
#include <catch2/catch.hpp>
#include "sensorycloud/sensorycloud.hpp"
#include "sensorycloud/token_manager/in_memory_credential_store.hpp"

using sensory::parse_enrollment_type;
using sensory::EnrollmentType;
using sensory::SensoryCloud;
using sensory::token_manager::InMemoryCredentialStore;

// ---------------------------------------------------------------------------
// MARK: parse_enrollment_type
// ---------------------------------------------------------------------------

SCENARIO("a user wants to parse an EnrollmentType from a string") {
    GIVEN("the string \"none\"") {
        const auto input = "none";
        WHEN("The string is parsed") {
            const auto enrollment_type = parse_enrollment_type(input);
            THEN("EnrollmentType::None is returned") {
                REQUIRE(EnrollmentType::None == enrollment_type);
            }
        }
    }
    GIVEN("the string \"sharedSecret\"") {
        const auto input = "sharedSecret";
        WHEN("The string is parsed") {
            const auto enrollment_type = parse_enrollment_type(input);
            THEN("EnrollmentType::SharedSecret is returned") {
                REQUIRE(EnrollmentType::SharedSecret == enrollment_type);
            }
        }
    }
    GIVEN("the string \"jwt\"") {
        const auto input = "jwt";
        WHEN("The string is parsed") {
            const auto enrollment_type = parse_enrollment_type(input);
            THEN("EnrollmentType::JWT is returned") {
                REQUIRE(EnrollmentType::JWT == enrollment_type);
            }
        }
    }
    GIVEN("the string \"foo\"") {
        const auto input = "foo";
        WHEN("The string is parsed") {
            THEN("an std::runtime_error is thrown") {
                REQUIRE_THROWS(parse_enrollment_type(input));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: SensoryCloud
// ---------------------------------------------------------------------------

/// @brief A temporary C-style file that deletes itself when de-allocated.
/// @details
/// The implementation of this temporary file follows C++ context conventions
/// whereby the constructor sets up the context and the de-constructor tears it
/// down. As such, it is typically recommended to initialize the structure on
/// the stack unless persistence via heap allocation is a strict requirement.
struct TemporaryFile {
    /// @brief An immutable pointer to the temporary file.
    FILE* const file = nullptr;

    /// @brief Initialize and open a new temporary file.
    TemporaryFile() : file(tmpfile()) { }

    /// @brief Close and delete the file when the object is de-allocated.
    ~TemporaryFile() { if (is_open()) fclose(file); }

    /// @brief Return true if the file is open, false otherwise.
    inline bool is_open() const { return file != NULL; }

    /// @brief Write the given string data to the file.
    /// @param content The string content to write to the file
    /// @details
    /// The file cursor is rewound after this write operation (for reading.)
    inline void write(const std::string& content) const {
        if (!is_open())
            throw std::runtime_error("Attempted write, but the file is not open");
        fputs(content.c_str(), file);
        rewind(file);
    }
};

SCENARIO("a user attempts to create a new SensoryCloud instance with a broken INI file") {
    GIVEN("a path to a non-existent INI file") {
        const std::string path = "/foo/bar/baz";
        InMemoryCredentialStore keychain;
        WHEN("a new instance of SensoryCloud is instantiated") {
            THEN("a runtime error is thrown") {
                REQUIRE_THROWS_WITH(SensoryCloud<InMemoryCredentialStore>(path, keychain), Catch::Contains("Path does not refer to an INI file \"/foo/bar/baz\""));
            }
        }
    }
    GIVEN("a path to a file that exists, but is not formatted as INI") {
        TemporaryFile file;
        if (!file.is_open()) FAIL("Failed to create temporary file for test");
        file.write("Hello, INI!");
        // Check that an error is thrown by the initializer.
        InMemoryCredentialStore keychain;
        WHEN("a new instance of SensoryCloud is instantiated") {
            THEN("a runtime error is thrown") {
                REQUIRE_THROWS_WITH(SensoryCloud<InMemoryCredentialStore>(file.file, keychain), Catch::Contains("Failed to parse INI file at line 1"));
            }
        }
    }
    GIVEN("a path to a valid INI file that has no SDK-Configuration section") {
        TemporaryFile file;
        if (!file.is_open()) FAIL("Failed to create temporary file for test");
        InMemoryCredentialStore keychain;
        WHEN("a new instance of SensoryCloud is instantiated") {
            THEN("a runtime error is thrown") {
                REQUIRE_THROWS_WITH(SensoryCloud<InMemoryCredentialStore>(file.file, keychain), Catch::Contains("Failed to find key \"tenantID\" in section [SDK-configuration]"));
            }
        }
    }
    GIVEN("a path to a valid INI file that has an [SDK-Configuration] section with no `tenantID`") {
        TemporaryFile file;
        if (!file.is_open()) FAIL("Failed to create temporary file for test");
        file.write("[SDK-Configuration]\nfullyQualifiedDomainName=10.10.28.51:50050");
        InMemoryCredentialStore keychain;
        WHEN("a new instance of SensoryCloud is instantiated") {
            THEN("a runtime error is thrown") {
                REQUIRE_THROWS_WITH(SensoryCloud<InMemoryCredentialStore>(file.file, keychain), Catch::Contains("Failed to find key \"tenantID\" in section [SDK-configuration]"));
            }
        }
    }
}
