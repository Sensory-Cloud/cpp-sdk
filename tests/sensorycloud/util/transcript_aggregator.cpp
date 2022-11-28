// Test cases for the transcript aggregator.
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
#include <catch2/catch.hpp>
#include <grpcpp/test/mock_stream.h>
#include "sensorycloud/util/transcript_aggregator.hpp"

using ::sensory::util::TranscriptAggregator;
using ::sensory::api::v1::audio::TranscribeWord;
using ::sensory::api::v1::audio::TranscribeWordResponse;

SCENARIO("A client needs to track a full transcript using the STT engine") {
    WHEN("A transcript aggregator is initialized") {
        TranscriptAggregator aggregator;
        THEN("The transcript aggregator embodies a null state") {
            REQUIRE(aggregator.get_word_list().empty());
            REQUIRE(aggregator.get_transcript().empty());
        }
    }
    GIVEN("An empty transcript aggregator") {
        TranscriptAggregator aggregator;
        WHEN("An empty response is passed to the aggregator") {
            aggregator.process_response({});
            THEN("The initial state of the aggregator does not change") {
                REQUIRE(aggregator.get_word_list().empty());
                REQUIRE(aggregator.get_transcript().empty());
            }
        }
        WHEN("A single word response is passed to the aggregator") {
            // Create a mock word at index 0.
            TranscribeWord foo;
            foo.set_word("foo");
            foo.set_wordindex(0);
            // Create the transcription response with the word update.
            TranscribeWordResponse rsp;
            rsp.set_firstwordindex(0);
            rsp.set_lastwordindex(0);
            (*rsp.mutable_words()->Add()) = foo;
            // Update the structure with the single word transcript.
            aggregator.process_response(rsp);
            THEN("The aggregator is updated with the transcript state") {
                REQUIRE(1 == aggregator.get_word_list().size());
                REQUIRE_THAT("foo", Catch::Equals(aggregator.get_transcript()));
            }
        }
        WHEN("A multi-word response is passed to the aggregator") {
            // Create a mock word at index 0.
            TranscribeWord foo;
            foo.set_word("foo ");
            foo.set_wordindex(0);
            TranscribeWord bar;
            bar.set_word("bar");
            bar.set_wordindex(1);
            // Create the transcription response with the word update.
            TranscribeWordResponse rsp;
            rsp.set_firstwordindex(0);
            rsp.set_lastwordindex(1);
            (*rsp.mutable_words()->Add()) = foo;
            (*rsp.mutable_words()->Add()) = bar;
            aggregator.process_response(rsp);
            THEN("The aggregator is updated with the transcript state") {
                REQUIRE(2 == aggregator.get_word_list().size());
                REQUIRE_THAT("foo bar", Catch::Equals(aggregator.get_transcript()));
            }
        }
    }
    GIVEN("A transcript aggregator with existing state") {
        TranscriptAggregator aggregator;
        // Create mock words.
        TranscribeWord foo;
        foo.set_word("foo");
        foo.set_wordindex(0);
        TranscribeWord bar;
        bar.set_word("bar");
        bar.set_wordindex(1);
        // Create the transcription response with the word update.
        TranscribeWordResponse rsp0;
        rsp0.set_firstwordindex(0);
        rsp0.set_lastwordindex(1);
        (*rsp0.mutable_words()->Add()) = foo;
        (*rsp0.mutable_words()->Add()) = bar;
        aggregator.process_response(rsp0);
        WHEN("An update response is passed to the aggregator that adds a word") {
            TranscribeWord baz;
            baz.set_word("baz");
            baz.set_wordindex(2);
            TranscribeWordResponse rsp1;
            rsp1.set_firstwordindex(0);
            rsp1.set_lastwordindex(2);
            (*rsp1.mutable_words()->Add()) = baz;
            aggregator.process_response(rsp1);
            THEN("The aggregator is updated with the new word") {
                REQUIRE(3 == aggregator.get_word_list().size());
                REQUIRE_THAT("foo bar baz", Catch::Equals(aggregator.get_transcript()));
            }
        }
        WHEN("An update response is passed to the aggregator that replaces a word") {
            TranscribeWord food;
            food.set_word("food");
            food.set_wordindex(0);
            TranscribeWordResponse rsp1;
            rsp1.set_firstwordindex(0);
            rsp1.set_lastwordindex(1);
            (*rsp1.mutable_words()->Add()) = food;
            aggregator.process_response(rsp1);
            THEN("The aggregator is updated with the replacement word") {
                REQUIRE(2 == aggregator.get_word_list().size());
                REQUIRE_THAT("food bar", Catch::Equals(aggregator.get_transcript()));
            }
        }
        WHEN("An update response is passed to the aggregator that replaces a sub-string") {
            TranscribeWord word;
            word.set_word("foobar");
            word.set_wordindex(0);
            TranscribeWordResponse rsp1;
            rsp1.set_firstwordindex(0);
            rsp1.set_lastwordindex(0);
            (*rsp1.mutable_words()->Add()) = word;
            aggregator.process_response(rsp1);
            THEN("The aggregator is updated with the sub-string replacement") {
                REQUIRE(1 == aggregator.get_word_list().size());
                REQUIRE_THAT("foobar", Catch::Equals(aggregator.get_transcript()));
            }
        }
    }
    WHEN("A transcript aggregator is passed an invalid index") {
        TranscriptAggregator aggregator;
        TranscribeWord word;
        word.set_word("foobar");
        word.set_wordindex(1);
        TranscribeWordResponse rsp;
        rsp.set_firstwordindex(0);
        rsp.set_lastwordindex(0);
        (*rsp.mutable_words()->Add()) = word;
        THEN("An expected runtime error is raised") {
            REQUIRE_THROWS(aggregator.process_response(rsp));
        }
    }
}
