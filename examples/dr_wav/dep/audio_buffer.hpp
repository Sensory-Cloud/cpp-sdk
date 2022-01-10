// A structure for working with audio sample file IO.
//
// Author: Christian Kauten (ckauten@sensoryinc.com)
//
// Copyright (c) 2021 Sensory, Inc.
// Copyright (c) 2020 Christian Kauten
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef AUDIO_BUFFER_HPP
#define AUDIO_BUFFER_HPP

#include <algorithm>
#include <string>
#include <vector>
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#define DR_FLAC_IMPLEMENTATION
#include "dr_flac.h"
#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"

/// @brief Extract the file extension from the given path.
///
/// @param path the path to extract the extension from
/// @returns the extension of the file.
/// @details
/// If no extension is found, returns an empty string.
///
inline std::string getExtension(const std::string& path) {
    // Lookup the index of the last dot in the path.
    const auto index = path.find_last_of(".");
    // If no dot was found, return an empty string
    if (index == std::string::npos) return "";
    // Return a sub-string starting past the index of the dot
    return path.substr(index + 1);
}

// /// @brief Write the contents of the given vector to a WAV file.
// ///
// /// @param path the path of the WAV file to write to
// /// @param data the IEEE754 floating point samples to write to the file
// /// @param sample_rate the sample rate of the data
// /// @param channels the number of channels of interleaved samples
// ///
// int writeWAV(const std::string& path, const std::vector<float>& data, int sample_rate, int channels) {
//     drwav_data_format format;
//     format.container = drwav_container_riff;
//     format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
//     format.channels = channels;
//     format.sampleRate = sample_rate;
//     format.bitsPerSample = 32;
//     drwav file;
//     if (!drwav_init_file_write(&file, path.c_str(), &format, NULL)) return 1;
//     drwav_write_pcm_frames(&file, data.size()/channels, data.data());
//     drwav_uninit(&file);
//     return 0;
// }

/// @brief An audio buffer.
struct AudioBuffer {
 private:
    /// the rate of the internal sample
    float sample_rate = 44100;
    /// the bit depth of the internal sample before converting to 32-bit
    float bit_depth = 16;
    /// the number of channels of audio data in the sample buffer
    float channels = 1;
    /// the samples of the "sample" being played by the player
    std::vector<int16_t> samples = {0};
    /// the path of the file that the sample originates from
    std::string path = "";

 public:
    /// @brief Channels on stereo samples.
    enum class Channel {
        /// 1 channel, mono playback
        Mono = 0,
        /// 1st channel of stereo (or greater) playback, i.e., left channel
        Left = 0,
        /// 2nd channel of stereo (or greater) playback, i.e., right channel
        Right = 1
    };

    /// @brief the status of a load call
    enum class LoadStatus {
        /// load succeeded
        Success = 0,
        /// the file was invalid
        InvalidFile,
        /// the extension was invalid
        InvalidExtension
    };

    /// @brief Return the sample rate of the internal sample.
    inline const float& getSampleRate() const { return sample_rate; }

    /// @brief Return the bit depth of the internal sample.
    inline const float& getBitDepth() const { return bit_depth; }

    /// @brief Return the number of channels in the sample.
    inline const float& getChannels() { return channels; }

    /// @brief Return true if the sample is mono.
    inline bool isMono() const { return channels == 1; }

    /// @brief Return true if the sample is stereo.
    inline bool isStereo() const { return channels == 2; }

    /// @brief Return the total length of the sample in samples.
    inline float getNumSamples() const { return samples.size() / channels; }

    /// @brief Return the file-path that this sample was loaded from.
    inline const std::string& getPath() const { return path; }

    /// @brief Return the sample for the given channel and index.
    ///
    /// @param channel the stereo channel to get a sample from
    /// @param index the index of the sample to return
    ///
    inline const int16_t& getSample(const Channel& channel, const uint64_t& index) const {
        return samples[static_cast<uint64_t>(channels) * index + static_cast<uint64_t>(channel)];
    }

    inline const std::vector<int16_t>& getSamples() const { return samples; }

    /// @brief Load the given file into the sample player.
    ///
    /// @param file_path the path of the file to load
    /// @details
    /// https://mackron.github.io/dr_wav
    ///
    inline LoadStatus loadWAV(const std::string& file_path) {
        // Load the file metadata structure.
        drwav file;
        if (!drwav_init_file(&file, file_path.c_str(), NULL))
            return LoadStatus::InvalidFile;
        sample_rate = file.sampleRate;
        channels = file.channels;
        bit_depth = file.bitsPerSample;
        // Load the samples into 32-bit floating point container
        samples.resize(file.totalPCMFrameCount * file.channels);
        drwav_read_pcm_frames_s16(&file, file.totalPCMFrameCount, samples.data());
        // Cleanup and release operating system resources.
        drwav_uninit(&file);
        path = file_path;
        return LoadStatus::Success;
    }

    /// @brief Load the given file into the sample player.
    ///
    /// @param file_path the path of the file to load
    /// @details
    /// https://mackron.github.io/dr_flac
    ///
    inline LoadStatus loadFLAC(const std::string& file_path) {
        // Load the file metadata structure.
        drflac* file = drflac_open_file(file_path.c_str(), NULL);
        if (file == nullptr)
            return LoadStatus::InvalidFile;
        sample_rate = file->sampleRate;
        channels = file->channels;
        bit_depth = file->bitsPerSample;
        // Load the samples into 32-bit floating point container
        samples.resize(file->totalPCMFrameCount * file->channels);
        drflac_read_pcm_frames_s16(file, file->totalPCMFrameCount, samples.data());
        // Cleanup and release operating system resources.
        drflac_close(file);
        path = file_path;
        return LoadStatus::Success;
    }

    /// @brief Load the given file into the sample player.
    ///
    /// @param file_path the path of the file to load
    /// @details
    /// https://github.com/mackron/dr_libs/blob/master/dr_mp3.h
    ///
    inline LoadStatus loadMP3(const std::string& file_path) {
        // Load the file metadata structure.
        drmp3 file;
        if (!drmp3_init_file(&file, file_path.c_str(), NULL))
            return LoadStatus::InvalidFile;
        sample_rate = file.sampleRate;
        channels = file.channels;
        bit_depth = 16;
        // Load the samples into 32-bit floating point container
        samples.resize(drmp3_get_pcm_frame_count(&file) * file.channels);
        drmp3_read_pcm_frames_s16(&file, drmp3_get_pcm_frame_count(&file), samples.data());
        // Cleanup and release operating system resources.
        drmp3_uninit(&file);
        path = file_path;
        return LoadStatus::Success;
    }

    /// @brief Load the given file into the sample player.
    ///
    /// @param file the path of the file to load
    /// @returns an integer code determining whether an error occurred.
    /// @details
    /// Error codes:
    /// 0: load succeeded
    /// 1: unsupported file-type
    /// 2: file contents invalid / file not found
    ///
    inline LoadStatus load(const std::string& file) {
        // Get the extension from the path.
        std::string extension = getExtension(file);
        // Transform the extension from any case to lower case. (TODO: make generic function)
        std::transform(extension.begin(), extension.end(), extension.begin(),
            [](unsigned char c){ return std::tolower(c); });
        // Load the file based on the extension.
        if (extension.compare("wav") == 0)
            return loadWAV(file);
        else if (extension.compare("flac") == 0)
            return loadFLAC(file);
        else if (extension.compare("mp3") == 0)
            return loadMP3(file);
        else  // unsupported file-type
            return LoadStatus::InvalidExtension;
    }

    /// @brief Unload the sample from memory.
    inline void unload() {
        samples.clear();
        samples.resize(1);
        sample_rate = 44100;
        bit_depth = 32;
        channels = 1;
        path = "";
    }

    /// @brief Zero-pad the audio buffer for the given duration.
    ///
    /// @param duration the duration in milliseconds to pad the buffer to
    ///
    inline void padBack(const float& duration) {
        std::vector<uint16_t> zeros(static_cast<int>(channels * sample_rate * duration / 1000), 0);
        samples.reserve(samples.size() + zeros.size());
        std::move(std::begin(zeros), std::end(zeros), std::back_inserter(samples));
        zeros.clear();
    }

    // /// @brief Write the contents of the audio buffer as a WAV file.
    // ///
    // /// @param path the path to write the buffer to as a WAV file
    // ///
    // inline int writeWAV(const std::string& path) const {
    //     return writeWAV(path, samples, sample_rate, channels);
    // }

    // /// @brief Remove the DC offset from the sample.
    // inline void removeDC() {
    //     std::vector<double> mean(static_cast<unsigned>(channels));
    //     for (int c = 0; c < channels; c++) mean[c] = 0;
    //     // Calculate the mean of the sample.
    //     for (int i = 0; i < getNumSamples(); i++)
    //         for (int c = 0; c < channels; c++)
    //             mean[c] += getSample(static_cast<Channel>(c), i);
    //     for (int c = 0; c < channels; c++) mean[c] /= getNumSamples();
    //     // Remove the mean from the sample.
    //     for (int i = 0; i < getNumSamples(); i++)
    //         for (int c = 0; c < channels; c++)
    //             samples[channels * i + c] -= mean[c];
    // }
};

#endif  // AUDIO_BUFFER_HPP
