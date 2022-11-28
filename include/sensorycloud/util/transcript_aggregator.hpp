// A structure for aggregating transcript data.
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

#ifndef SENSORYCLOUD_UTIL_TRANSCRIPT_AGGREGATOR_HPP_
#define SENSORYCLOUD_UTIL_TRANSCRIPT_AGGREGATOR_HPP_

#include <string>
#include <vector>
#include "sensorycloud/generated/v1/audio/audio.pb.h"
#include "sensorycloud/generated/v1/audio/audio.grpc.pb.h"
#include "sensorycloud/util/string_extensions.hpp"

/// @brief The SensoryCloud SDK.
namespace sensory {

/// @brief Utility functions.
namespace util {

/// @brief A structure that aggregates and stores transcription responses.
/// @details
/// This class can maintain the full transcript returned from the server's
/// windows responses.
class TranscriptAggregator {
 private:
    /// An internal buffer of the complete transcript from the server.
    std::vector<::sensory::api::v1::audio::TranscribeWord> word_list;

 public:
    /// @brief Process a single sliding-window response from the server.
    ///
    /// @param response The current word list from the server.
    ///
    void process_response(const ::sensory::api::v1::audio::TranscribeWordResponse& response) {
        if (response.words().empty()) return;
        // Get the expected transcript size from the index of the last word.
        const auto response_size = response.lastwordindex() + 1;
        // Grow the word buffer if the incoming transcript is larger.
        if (response_size > word_list.size())
            word_list.resize(response_size);
        // Loop through returned words and replace buffered words that changed.
        for (const auto& word : response.words()) {
            // Sanity check the word index to prevent the possibility of
            // unexpected segmentation faults in favor of descriptive errors.
            if (word.wordindex() >= word_list.size())
                throw std::runtime_error(
                    "Attempting to update word at index " +
                    std::to_string(word.wordindex()) +
                    " that exceeds the expected buffer size of " +
                    std::to_string(word_list.size())
                );
            word_list[word.wordindex()] = word;
        }
        // Shrink the word list if the incoming transcript is smaller.
        if (response_size < word_list.size())
            word_list.erase(word_list.begin() + response_size, word_list.end());
    }

    /// @brief Return a constant reference to the complete transcript.
    ///
    /// @returns A vector with the transcribed words and associated metadata.
    ///
    inline const std::vector<::sensory::api::v1::audio::TranscribeWord>& get_word_list() const {
        return word_list;
    }

    /// @brief Return the full transcript as computed from the current word list.
    ///
    /// @param delimiter An optional delimiter for controlling the separation
    /// of individual words in the transcript.
    /// @returns An imploded string representation of the underlying word list.
    ///
    std::string get_transcript(const std::string& delimiter=" ") const {
        if (word_list.empty()) return "";
        // Iterate over the words to accumulate the transcript.
        std::string transcript = "";
        for (const auto& word : word_list)
            transcript += delimiter + ::sensory::util::strip(word.word());
        // Remove the extra space at the front of the transcript.
        transcript.erase(0, 1);
        return transcript;
    }
};

}  // namespace util

}  // namespace sensory

#endif  // SENSORYCLOUD_UTIL_TRANSCRIPT_AGGREGATOR_HPP_
