// Test cases for the io::INIReader structure.
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
#include "sensorycloud/io/ini.hpp"

using sensory::io::INIReader;

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

// ---------------------------------------------------------------------------
// MARK: INIReader
// ---------------------------------------------------------------------------

SCENARIO("a user attempts to create an INIReader with a faulty INI file") {
    GIVEN("a path to a non-existent INI file") {
        const std::string path = "/foo/bar/baz";
        WHEN("an INIReader is instantiated") {
            THEN("a runtime error is thrown") {
                REQUIRE_THROWS_WITH(INIReader(path), Catch::Contains("Path does not refer to an INI file \"" + path + "\""));
            }
        }
    }
    GIVEN("a file that is not formatted as INI") {
        TemporaryFile file;
        if (!file.is_open()) FAIL("Failed to create temporary file for test");
        file.write("Hello, INI!");
        WHEN("an INIReader is instantiated") {
            THEN("a runtime error is thrown") {
                REQUIRE_THROWS_WITH(INIReader(file.file), Catch::Contains("Failed to parse INI file at line 1"));
            }
        }
    }
}

SCENARIO("a user attempts to create an INIReader with a valid INI file") {
    GIVEN("an empty file") {
        TemporaryFile file;
        if (!file.is_open()) FAIL("Failed to create temporary file for test");
        WHEN("an INIReader is instantiated") {
            THEN("no error is thrown") {
                REQUIRE_NOTHROW(INIReader(file.file));
            }
        }
    }
    GIVEN("a file with a single section and key") {
        TemporaryFile file;
        if (!file.is_open()) FAIL("Failed to create temporary file for test");
        file.write("[foo]\nbar=baz");
        WHEN("an INIReader is instantiated") {
            THEN("no error is thrown") {
                REQUIRE_NOTHROW(INIReader(file.file));
            }
            THEN("The list of sections contains the 'foo' section") {
                auto sections = INIReader(file.file).get_sections();
                REQUIRE(1 == sections.size());
                REQUIRE(1 == sections.count("foo"));
            }
        }
    }
}

SCENARIO("a user wants to parse string keys") {
    GIVEN("a file with string keys") {
        TemporaryFile file;
        if (!file.is_open()) FAIL("Failed to create temporary file for test");
        file.write("[foo]\nbar=baz");
        WHEN("an INIReader is instantiated") {
            INIReader ini(file.file);
            // sections with keys that exist
            THEN("the 'bar' key can be parsed from the 'foo' section") {
                REQUIRE_THAT(ini.get<std::string>("foo", "bar"), Catch::Equals("baz"));
            }
            THEN("existence of the 'bar' key in the 'key' section can be strictly required") {
                REQUIRE_NOTHROW(ini.get<std::string>("foo", "bar", "", true));
            }
            // sections that exist, but keys that don't
            THEN("the 'nan' key can be parsed from the 'foo' section without a null value") {
                REQUIRE_THAT(ini.get<std::string>("foo", "nan"), Catch::Equals(""));
            }
            THEN("the 'nan' key can be parsed from the 'foo' section with a default value") {
                REQUIRE_THAT(ini.get<std::string>("foo", "nan", "default"), Catch::Equals("default"));
            }
            THEN("existence of the 'nan' key in the 'foo' section can be strictly required") {
                REQUIRE_THROWS_WITH(ini.get<std::string>("foo", "nan", "", true), Catch::Contains("Failed to find key \"nan\" in section [foo]"));
            }
            // sections that do not exist
            THEN("the 'nan' key can be parsed from the 'nan' section with a null value") {
                REQUIRE_THAT(ini.get<std::string>("nan", "nan"), Catch::Equals(""));
            }
            THEN("the 'nan' key can be parsed from the 'nan' section with a default value") {
                REQUIRE_THAT(ini.get<std::string>("nan", "nan", "default"), Catch::Equals("default"));
            }
            THEN("existence of the 'nan' key in the 'nan' section can be strictly required") {
                REQUIRE_THROWS_WITH(ini.get<std::string>("nan", "nan", "", true), Catch::Contains("Failed to find key \"nan\" in section [nan]"));
            }
        }
    }
}

SCENARIO("a user wants to parse boolean keys") {
    GIVEN("a file with integer keys") {
        TemporaryFile file;
        if (!file.is_open()) FAIL("Failed to create temporary file for test");
        file.write("[foo]\nbar=true");
        WHEN("an INIReader is instantiated") {
            INIReader ini(file.file);
            // sections with keys that exist
            THEN("the 'bar' key can be parsed from the 'foo' section") {
                REQUIRE(ini.get<bool>("foo", "bar") == true);
            }
            THEN("existence of the 'bar' key in the 'key' section can be strictly required") {
                REQUIRE_NOTHROW(ini.get<bool>("foo", "bar", false, true));
            }
            // sections that exist, but keys that don't
            THEN("the 'nan' key can be parsed from the 'foo' section without a null value") {
                REQUIRE(ini.get<bool>("foo", "nan") == false);
            }
            THEN("the 'nan' key can be parsed from the 'foo' section with a default value") {
                REQUIRE(ini.get<bool>("foo", "nan", true) == true);
            }
            THEN("existence of the 'nan' key in the 'foo' section can be strictly required") {
                REQUIRE_THROWS_WITH(ini.get<bool>("foo", "nan", false, true), Catch::Contains("Failed to find key \"nan\" in section [foo]"));
            }
            // sections that do not exist
            THEN("the 'nan' key can be parsed from the 'nan' section with a null value") {
                REQUIRE(ini.get<bool>("nan", "nan") == false);
            }
            THEN("the 'nan' key can be parsed from the 'nan' section with a default value") {
                REQUIRE(ini.get<bool>("nan", "nan", true) == true);
            }
            THEN("existence of the 'nan' key in the 'nan' section can be strictly required") {
                REQUIRE_THROWS_WITH(ini.get<bool>("nan", "nan", false, true), Catch::Contains("Failed to find key \"nan\" in section [nan]"));
            }
        }
    }
}

SCENARIO("a user wants to parse 32-bit integer keys") {
    GIVEN("a file with 32-bit integer keys") {
        TemporaryFile file;
        if (!file.is_open()) FAIL("Failed to create temporary file for test");
        file.write("[foo]\nbar=7");
        WHEN("an INIReader is instantiated") {
            INIReader ini(file.file);
            // sections with keys that exist
            THEN("the 'bar' key can be parsed from the 'foo' section") {
                REQUIRE(ini.get<int32_t>("foo", "bar") == 7);
            }
            THEN("existence of the 'bar' key in the 'key' section can be strictly required") {
                REQUIRE_NOTHROW(ini.get<int32_t>("foo", "bar", 0, true));
            }
            // sections that exist, but keys that don't
            THEN("the 'nan' key can be parsed from the 'foo' section without a null value") {
                REQUIRE(ini.get<int32_t>("foo", "nan") == 0);
            }
            THEN("the 'nan' key can be parsed from the 'foo' section with a default value") {
                REQUIRE(ini.get<int32_t>("foo", "nan", 9) == 9);
            }
            THEN("existence of the 'nan' key in the 'foo' section can be strictly required") {
                REQUIRE_THROWS_WITH(ini.get<int32_t>("foo", "nan", 0, true), Catch::Contains("Failed to find key \"nan\" in section [foo]"));
            }
            // sections that do not exist
            THEN("the 'nan' key can be parsed from the 'nan' section with a null value") {
                REQUIRE(ini.get<int32_t>("nan", "nan") == 0);
            }
            THEN("the 'nan' key can be parsed from the 'nan' section with a default value") {
                REQUIRE(ini.get<int32_t>("nan", "nan", 9) == 9);
            }
            THEN("existence of the 'nan' key in the 'nan' section can be strictly required") {
                REQUIRE_THROWS_WITH(ini.get<int32_t>("nan", "nan", 0, true), Catch::Contains("Failed to find key \"nan\" in section [nan]"));
            }
        }
    }
}

SCENARIO("a user wants to parse 64-bit integer keys") {
    GIVEN("a file with 64-bit integer keys") {
        TemporaryFile file;
        if (!file.is_open()) FAIL("Failed to create temporary file for test");
        file.write("[foo]\nbar=7");
        WHEN("an INIReader is instantiated") {
            INIReader ini(file.file);
            // sections with keys that exist
            THEN("the 'bar' key can be parsed from the 'foo' section") {
                REQUIRE(ini.get<int64_t>("foo", "bar") == 7);
            }
            THEN("existence of the 'bar' key in the 'key' section can be strictly required") {
                REQUIRE_NOTHROW(ini.get<int64_t>("foo", "bar", 0, true));
            }
            // sections that exist, but keys that don't
            THEN("the 'nan' key can be parsed from the 'foo' section without a null value") {
                REQUIRE(ini.get<int64_t>("foo", "nan") == 0);
            }
            THEN("the 'nan' key can be parsed from the 'foo' section with a default value") {
                REQUIRE(ini.get<int64_t>("foo", "nan", 9) == 9);
            }
            THEN("existence of the 'nan' key in the 'foo' section can be strictly required") {
                REQUIRE_THROWS_WITH(ini.get<int64_t>("foo", "nan", 0, true), Catch::Contains("Failed to find key \"nan\" in section [foo]"));
            }
            // sections that do not exist
            THEN("the 'nan' key can be parsed from the 'nan' section with a null value") {
                REQUIRE(ini.get<int64_t>("nan", "nan") == 0);
            }
            THEN("the 'nan' key can be parsed from the 'nan' section with a default value") {
                REQUIRE(ini.get<int64_t>("nan", "nan", 9) == 9);
            }
            THEN("existence of the 'nan' key in the 'nan' section can be strictly required") {
                REQUIRE_THROWS_WITH(ini.get<int64_t>("nan", "nan", 0, true), Catch::Contains("Failed to find key \"nan\" in section [nan]"));
            }
        }
    }
}

SCENARIO("a user wants to parse 32-bit floating point keys") {
    GIVEN("a file with 32-bit float keys") {
        TemporaryFile file;
        if (!file.is_open()) FAIL("Failed to create temporary file for test");
        file.write("[foo]\nbar=7.7");
        WHEN("an INIReader is instantiated") {
            INIReader ini(file.file);
            // sections with keys that exist
            THEN("the 'bar' key can be parsed from the 'foo' section") {
                REQUIRE(ini.get<float>("foo", "bar") == 7.7f);
            }
            THEN("existence of the 'bar' key in the 'key' section can be strictly required") {
                REQUIRE_NOTHROW(ini.get<float>("foo", "bar", 0, true));
            }
            // sections that exist, but keys that don't
            THEN("the 'nan' key can be parsed from the 'foo' section without a null value") {
                REQUIRE(ini.get<float>("foo", "nan") == 0);
            }
            THEN("the 'nan' key can be parsed from the 'foo' section with a default value") {
                REQUIRE(ini.get<float>("foo", "nan", 9.5f) == 9.5f);
            }
            THEN("existence of the 'nan' key in the 'foo' section can be strictly required") {
                REQUIRE_THROWS_WITH(ini.get<float>("foo", "nan", 0, true), Catch::Contains("Failed to find key \"nan\" in section [foo]"));
            }
            // sections that do not exist
            THEN("the 'nan' key can be parsed from the 'nan' section with a null value") {
                REQUIRE(ini.get<float>("nan", "nan") == 0);
            }
            THEN("the 'nan' key can be parsed from the 'nan' section with a default value") {
                REQUIRE(ini.get<float>("nan", "nan", 9.5f) == 9.5f);
            }
            THEN("existence of the 'nan' key in the 'nan' section can be strictly required") {
                REQUIRE_THROWS_WITH(ini.get<float>("nan", "nan", 0, true), Catch::Contains("Failed to find key \"nan\" in section [nan]"));
            }
        }
    }
}

SCENARIO("a user wants to parse 64-bit floating point keys") {
    GIVEN("a file with 64-bit float keys") {
        TemporaryFile file;
        if (!file.is_open()) FAIL("Failed to create temporary file for test");
        file.write("[foo]\nbar=7.7");
        WHEN("an INIReader is instantiated") {
            INIReader ini(file.file);
            // sections with keys that exist
            THEN("the 'bar' key can be parsed from the 'foo' section") {
                REQUIRE(ini.get<double>("foo", "bar") == 7.7);
            }
            THEN("existence of the 'bar' key in the 'key' section can be strictly required") {
                REQUIRE_NOTHROW(ini.get<double>("foo", "bar", 0, true));
            }
            // sections that exist, but keys that don't
            THEN("the 'nan' key can be parsed from the 'foo' section without a null value") {
                REQUIRE(ini.get<double>("foo", "nan") == 0);
            }
            THEN("the 'nan' key can be parsed from the 'foo' section with a default value") {
                REQUIRE(ini.get<double>("foo", "nan", 9.5) == 9.5);
            }
            THEN("existence of the 'nan' key in the 'foo' section can be strictly required") {
                REQUIRE_THROWS_WITH(ini.get<double>("foo", "nan", 0, true), Catch::Contains("Failed to find key \"nan\" in section [foo]"));
            }
            // sections that do not exist
            THEN("the 'nan' key can be parsed from the 'nan' section with a null value") {
                REQUIRE(ini.get<double>("nan", "nan") == 0);
            }
            THEN("the 'nan' key can be parsed from the 'nan' section with a default value") {
                REQUIRE(ini.get<double>("nan", "nan", 9.5) == 9.5);
            }
            THEN("existence of the 'nan' key in the 'nan' section can be strictly required") {
                REQUIRE_THROWS_WITH(ini.get<double>("nan", "nan", 0, true), Catch::Contains("Failed to find key \"nan\" in section [nan]"));
            }
        }
    }
}
