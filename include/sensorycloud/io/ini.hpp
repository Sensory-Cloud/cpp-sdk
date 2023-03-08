// Read an INI file into easy-to-access name/value pairs.

// inih and INIReader are released under the New BSD license (see LICENSE.txt).
// Go to the project home page for more info:
//
// https://github.com/benhoyt/inih
/* inih -- simple .INI file parser

inih is released under the New BSD license (see LICENSE.txt). Go to the project
home page for more info:

https://github.com/benhoyt/inih

*/

#ifndef SENSORYCLOUD_IO_INI_HPP_
#define SENSORYCLOUD_IO_INI_HPP_

#ifdef __cplusplus
#include "sensorycloud/io/path.hpp"
extern "C" {
#endif

#include <stdio.h>

/// Typedef for prototype of handler function.
typedef int (*ini_handler)(void* user, const char* section, const char* name, const char* value);

/// Typedef for prototype of fgets-style reader function.
typedef char* (*ini_reader)(char* str, int num, void* stream);

/// Parse given INI-style file. May have [section]s, name=value pairs
/// (whitespace stripped), and comments starting with ';' (semicolon). Section
/// is "" if name=value pair parsed before any section heading. name:value
/// pairs are also supported as a concession to Python's configparser.///
///
/// For each name=value pair parsed, call handler function with given user
/// pointer as well as section, name, and value (data only valid for duration
/// of handler call). Handler should return nonzero on success, zero on error.///
///
/// Returns 0 on success, line number of first error on parse error (doesn't
/// stop on first error), -1 on file open error, or -2 on memory allocation
/// error (only when INI_USE_STACK is zero).
void ini_parse(const char* filename, ini_handler handler, void* user);

/// Same as ini_parse(), but takes a FILE* instead of filename. This doesn't
/// close the file when it's finished -- the caller must do that.
void ini_parse_file(FILE* file, ini_handler handler, void* user);

/// Same as ini_parse(), but takes an ini_reader function pointer instead of
/// filename. Used for implementing custom or string-based I/O.
void ini_parse_stream(ini_reader reader, void* stream, ini_handler handler, void* user);

/// Nonzero to allow multi-line value parsing, in the style of Python's
/// configparser. If allowed, ini_parse() will call the handler with the same
/// name for each subsequent line parsed.
#ifndef INI_ALLOW_MULTILINE
#define INI_ALLOW_MULTILINE 1
#endif

/// Nonzero to allow a UTF-8 BOM sequence (0xEF 0xBB 0xBF) at the start of
/// the file. See http://code.google.com/p/inih/issues/detail?id=21
#ifndef INI_ALLOW_BOM
#define INI_ALLOW_BOM 1
#endif

/// Nonzero to allow inline comments (with valid inline comment characters
/// specified by INI_INLINE_COMMENT_PREFIXES). Set to 0 to turn off and match
/// Python 3.2+ configparser behaviour.
#ifndef INI_ALLOW_INLINE_COMMENTS
#define INI_ALLOW_INLINE_COMMENTS 1
#endif
#ifndef INI_INLINE_COMMENT_PREFIXES
#define INI_INLINE_COMMENT_PREFIXES ";"
#endif

/// Nonzero to use stack, zero to use heap (malloc/free).
#ifndef INI_USE_STACK
#define INI_USE_STACK 1
#endif

/// Stop parsing on first error (default is to keep parsing).
#ifndef INI_STOP_ON_FIRST_ERROR
#define INI_STOP_ON_FIRST_ERROR 0
#endif

/// Maximum line length for any line in INI file.
#ifndef INI_MAX_LINE
#define INI_MAX_LINE 200
#endif

#ifdef __cplusplus
}
#endif

/* inih -- simple .INI file parser

inih is released under the New BSD license (see LICENSE.txt). Go to the project
home page for more info:

https://github.com/benhoyt/inih

*/

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#if !INI_USE_STACK
#include <stdlib.h>
#endif

#define MAX_SECTION 50
#define MAX_NAME 50

/* Strip whitespace chars off end of given string, in place. Return s. */
inline static char* rstrip(char* s) {
    char* p = s + strlen(s);
    while (p > s && isspace((unsigned char)(*--p)))
        *p = '\0';
    return s;
}

/* Return pointer to first non-whitespace char in given string. */
inline static char* lskip(const char* s) {
    while (*s && isspace((unsigned char)(*s)))
        s++;
    return (char*)s;
}

/* Return pointer to first char (of chars) or inline comment in given string,
   or pointer to null at end of string if neither found. Inline comment must
   be prefixed by a whitespace character to register as a comment. */
inline static char* find_chars_or_comment(const char* s, const char* chars) {
#if INI_ALLOW_INLINE_COMMENTS
    int was_space = 0;
    while (*s && (!chars || !strchr(chars, *s)) &&
           !(was_space && strchr(INI_INLINE_COMMENT_PREFIXES, *s))) {
        was_space = isspace((unsigned char)(*s));
        s++;
    }
#else
    while (*s && (!chars || !strchr(chars, *s))) {
        s++;
    }
#endif
    return (char*)s;
}

/* Version of strncpy that ensures dest (size bytes) is null-terminated. */
inline static char* strncpy0(char* dest, const char* src, size_t size) {
    strncpy(dest, src, size);
    dest[size - 1] = '\0';
    return dest;
}

inline void ini_parse_stream(
    ini_reader reader,
    void* stream,
    ini_handler handler,
    void* user
) {
    /* Uses a fair bit of stack (use heap instead if you need to) */
#if INI_USE_STACK
    char line[INI_MAX_LINE];
#else
    char* line;
#endif
    char section[MAX_SECTION] = "";
    char prev_name[MAX_NAME] = "";

    char* start;
    char* end;
    char* name;
    char* value;
    int lineno = 0;
    int error = 0;

#if !INI_USE_STACK
    line = (char*)malloc(INI_MAX_LINE);
    if (!line)
        throw std::runtime_error("Failed to allocate onto heap for INI contents!");
#endif

    /* Scan through stream line by line */
    while (reader(line, INI_MAX_LINE, stream) != NULL) {
        lineno++;

        start = line;
#if INI_ALLOW_BOM
        if (lineno == 1 && (unsigned char)start[0] == 0xEF &&
                           (unsigned char)start[1] == 0xBB &&
                           (unsigned char)start[2] == 0xBF) {
            start += 3;
        }
#endif
        start = lskip(rstrip(start));

        if (*start == ';' || *start == '#') {
            /* Per Python configparser, allow both ; and # comments at the
               start of a line */
        }
#if INI_ALLOW_MULTILINE
        else if (*prev_name && *start && start > line) {

#if INI_ALLOW_INLINE_COMMENTS
        end = find_chars_or_comment(start, NULL);
        if (*end)
            *end = '\0';
        rstrip(start);
#endif

            /* Non-blank line with leading whitespace, treat as continuation
               of previous name's value (as per Python configparser). */
            if (!handler(user, section, prev_name, start) && !error)
                error = lineno;
        }
#endif
        else if (*start == '[') {
            /* A "[section]" line */
            end = find_chars_or_comment(start + 1, "]");
            if (*end == ']') {
                *end = '\0';
                strncpy0(section, start + 1, sizeof(section));
                *prev_name = '\0';
            }
            else if (!error) {
                /* No ']' found on section line */
                error = lineno;
            }
        }
        else if (*start) {
            /* Not a comment, must be a name[=:]value pair */
            end = find_chars_or_comment(start, "=:");
            if (*end == '=' || *end == ':') {
                *end = '\0';
                name = rstrip(start);
                value = lskip(end + 1);
#if INI_ALLOW_INLINE_COMMENTS
                end = find_chars_or_comment(value, NULL);
                if (*end)
                    *end = '\0';
#endif
                rstrip(value);

                /* Valid name[=:]value pair found, call handler */
                strncpy0(prev_name, name, sizeof(prev_name));
                if (!handler(user, section, name, value) && !error)
                    error = lineno;
            }
            else if (!error) {
                /* No '=' or ':' found on name[=:]value line */
                error = lineno;
            }
        }

#if INI_STOP_ON_FIRST_ERROR
        if (error)
            break;
#endif
    }

#if !INI_USE_STACK
    free(line);
#endif

    if (error)  // An error occurred while parsing the file contents.
        throw std::runtime_error("Failed to parse INI file at line " + std::to_string(error));
}

/* See documentation in header file. */
inline void ini_parse_file(FILE* file, ini_handler handler, void* user) {
    ini_parse_stream((ini_reader)fgets, file, handler, user);
}

/* See documentation in header file. */
inline void ini_parse(const char* filename, ini_handler handler, void* user) {
    if (!sensory::io::path::is_file(filename))
        throw std::runtime_error("Path does not refer to an INI file \"" + std::string(filename) + "\"");
    FILE* file;
    file = fopen(filename, "r");
    if (!file)
        throw std::runtime_error("Failed to open INI file at path \"" + std::string(filename) + "\"");
    ini_parse_file(file, handler, user);
    fclose(file);
}

#endif  // SENSORYCLOUD_IO_INI_HPP_



#ifndef SENSORYCLOUD_IO_INIREADER_HPP_
#define SENSORYCLOUD_IO_INIREADER_HPP_

#include <map>
#include <set>
#include <string>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <exception>

/// @brief The SensoryCloud SDK.
namespace sensory {

/// @brief File IO components.
namespace io {

/// @brief An IO structure for parsing INI files into key-value pairs.
class INIReader {
 private:
    /// @brief Initialize a null INIReader with no content.
    INIReader() { }

 protected:
    /// A mapping of flattened section/key names to their associated values.
    std::map<std::string, std::string> values;
    /// A collection of the unique sections in the file.
    std::set<std::string> sections;

    /// @brief Create a combined section/name key from a section and name.
    /// @param section The section that contains the key with given name.
    /// @param name The name of the key in the given section
    /// @returns A unique combination of the section and key for flat mapping.
    /// @details
    /// Keys are case insensitive.
    inline static std::string get_key_from_section_and_name(
        const std::string& section,
        const std::string& name
    ) {
        // `=` is the INI assignment operator; it's safe to use as a delimiter.
        std::string key = section + "=" + name;
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        return key;
    }

    /// @brief A callback for handling values during parsing.
    /// @param reader_ The instance of INIReader that is being parsed into.
    /// @param section The section that is being parsed
    /// @param name The name of the key to handle
    /// @param value The value of the key to handle
    static int handle_value(
        void* reader_,
        const char* section,
        const char* name,
        const char* value
    ) {
        INIReader* reader = reinterpret_cast<INIReader*>(reader_);
        std::string key = get_key_from_section_and_name(section, name);
        if (reader->values[key].size() > 0) reader->values[key] += "\n";
        reader->values[key] += value;
        reader->sections.insert(section);
        return 1;
    }

 public:
    /// @brief Construct a new INI reader from a file with given path name.
    /// @param filename The path to the file on the file-system.
    explicit INIReader(const std::string& filename) {
        ini_parse(filename.c_str(), handle_value, this);
    }

    // Construct INIReader and parse given file. See ini.h for more info
    // about the parsing.
    explicit INIReader(FILE *file) {
        ini_parse_file(file, handle_value, this);
    }

    // Return the list of sections found in ini file
    inline const std::set<std::string>& get_sections() const {
        return sections;
    }

    /// @brief Fetch a value from the ini file.
    /// @tparam T The data-type to cast the return value to.
    /// @param section The section of the file to constrain the search to.
    /// @param name The name of the key to look for within the section.
    /// @param default_value A default value to use if the key does not exist.
    /// @param required True to throw an error if the key does not exist.
    /// @returns The value of the given key in the given section.
    /// @throws A `std::runtime_error` when `required` is true and the section
    ///         doesn't exist, or the key doesn't exist within the section.
    template<typename T>
    T get(
        const std::string& section,
        const std::string& name,
        const T& default_value = T(),
        const bool& required = false) const;
};

template<>
std::string INIReader::get<std::string>(
    const std::string& section,
    const std::string& name,
    const std::string& default_value,
    const bool& required
) const {
    std::string key = get_key_from_section_and_name(section, name);
    if (values.count(key))  // section exists and has a value for the key.
        return values.at(key);
    if (!required)  // Section/key doesn't exist, but the value is optional.
        return default_value;
    // The value should exist but doesn't, throw an error.
    throw std::runtime_error(
        "Failed to find key \"" + name + "\" in section [" + section + "]");
}

template<>
bool INIReader::get<bool>(
    const std::string& section,
    const std::string& name,
    const bool& default_value,
    const bool& required
) const {
    std::string value = get<std::string>(section, name, "", required);
    // Convert to lower case to make string comparisons case-insensitive.
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    if (value == "true" || value == "yes" || value == "on" || value == "1")
        return true;
    if (value == "false" || value == "no" || value == "off" || value == "0")
        return false;
    return default_value;
}

template<>
inline int32_t INIReader::get<int32_t>(
    const std::string& section,
    const std::string& name,
    const int32_t& default_value,
    const bool& required
) const {
    const std::string value = get<std::string>(section, name, "", required);
    return value.empty() ? default_value : std::stoi(value);
}

template<>
inline int64_t INIReader::get<int64_t>(
    const std::string& section,
    const std::string& name,
    const int64_t& default_value,
    const bool& required
) const {
    const std::string value = get<std::string>(section, name, "", required);
    return value.empty() ? default_value : std::stol(value);
}

template<>
inline float INIReader::get<float>(
    const std::string& section,
    const std::string& name,
    const float& default_value,
    const bool& required
) const {
    const std::string value = get<std::string>(section, name, "", required);
    return value.empty() ? default_value : std::stof(value);
}

template<>
inline double INIReader::get<double>(
    const std::string& section,
    const std::string& name,
    const double& default_value,
    const bool& required
) const {
    const std::string value = get<std::string>(section, name, "", required);
    return value.empty() ? default_value : std::stod(value);
}

}  // namespace io

}  // namespace sensory

#endif  // SENSORYCLOUD_IO_INIREADER_HPP_
