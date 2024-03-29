// The SensoryCloud C++ SDK management service demo (synchronous interface).
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

#include <iostream>
#include <regex>
#include <google/protobuf/util/json_util.h>
#include <sensorycloud/sensorycloud.hpp>
#include <sensorycloud/token_manager/file_system_credential_store.hpp>
#include "dep/argparse.hpp"

using sensory::SensoryCloud;
using sensory::token_manager::FileSystemCredentialStore;
using sensory::service::ManagementService;

int main(int argc, const char** argv) {
    // Create an argument parser to parse inputs from the command line.
    auto parser = argparse::ArgumentParser(argc, argv)
        .prog("assistant")
        .description("A tool for using the SensoryCloud assistant.");
    parser.add_argument({ "path" })
        .help("The path to an INI file containing server metadata.");
    parser.add_argument({ "-g", "--getmodels" })
        .action("store_true")
        .help("Whether to query for a list of available models.");
    parser.add_argument({ "-m", "--model" })
        .help("The model to use for the session.");
    parser.add_argument({ "-v", "--verbose" })
        .action("store_true")
        .help("Produce verbose output during authentication.");
    // Parse the arguments from the command line.
    const auto args = parser.parse_args();
    const auto PATH = args.get<std::string>("path");
    const auto GETMODELS = args.get<bool>("getmodels");
    const auto MODEL = args.get<std::string>("model");
    const auto VERBOSE = args.get<bool>("verbose");

    // Create a credential store for keeping OAuth credentials in.
    FileSystemCredentialStore keychain(".", "com.sensory.cloud.examples");

    // Create the cloud services handle.
    SensoryCloud<FileSystemCredentialStore> cloud(PATH, keychain);

    // Query the health of the remote service.
    sensory::api::common::ServerHealthResponse server_health;
    auto status = cloud.health.get_health(&server_health);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to get server health ("
            << status.error_code() << "): "
            << status.error_message() << std::endl;
        return 1;
    }
    if (VERBOSE) {
        google::protobuf::util::JsonPrintOptions options;
        options.add_whitespace = true;
        options.always_print_primitive_fields = true;
        options.always_print_enums_as_ints = false;
        options.preserve_proto_field_names = true;
        std::string server_health_json;
        google::protobuf::util::MessageToJsonString(server_health, &server_health_json, options);
        std::cout << server_health_json << std::endl;
    }

    // Initialize the client.
    sensory::api::v1::management::DeviceResponse device_response;
    status = cloud.initialize(&device_response);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to initialize (" << status.error_code() << "): " << status.error_message() << std::endl;
        return 1;
    }

    // Fetch the available language models.
    if (GETMODELS) {
        sensory::api::v1::audio::GetModelsResponse audioModelsResponse;
        status = cloud.audio.get_models(&audioModelsResponse);
        if (!status.ok()) {  // the call failed, print a descriptive message
            std::cout << "Failed to get audio models ("
                << status.error_code() << "): "
                << status.error_message() << std::endl;
            return 1;
        }
        for (auto& model : audioModelsResponse.models()) {
            if (model.modeltype() != sensory::api::common::LLM_GPT_3_5)
                continue;
            google::protobuf::util::JsonPrintOptions options;
            options.add_whitespace = true;
            options.always_print_primitive_fields = true;
            options.always_print_enums_as_ints = false;
            options.preserve_proto_field_names = true;
            std::string model_json;
            google::protobuf::util::MessageToJsonString(model, &model_json, options);
            std::cout << model_json << std::endl;
        }
        return 0;
    }

    // Create a request and set the model name. We'll use the request to cache
    // all communication to-and-from the assistant.
    ::sensory::api::v1::assistant::TextChatRequest request;
    request.set_modelname(MODEL);

    while (true) {
        // Parse a command from the terminal.
        std::cout << ">>> ";
        std::string line;
        std::getline(std::cin, line);
        // Detect the shutdown signal
        if (line == "exit" || line == "exit()" || line == "quit") break;
        // Create the chat request
        auto message = request.add_messages();
        message->set_role(::sensory::api::v1::assistant::ChatRole::USER);
        message->set_content(line);
        // Print the response to the terminal
        ::sensory::api::v1::assistant::TextChatResponse response;
        auto status = cloud.assistant.text_chat(request, &response);
        if (!status.ok()) {  // The call failed, print a descriptive message.
            std::cout << "Chat request failed with ("
                << status.error_code() << "): "
                << status.error_message() << std::endl;
            continue;
        }
        std::cout << response.message().content() << std::endl;
        // Put the response in the request for context tracing
        message = request.add_messages();
        message->set_role(::sensory::api::v1::assistant::ChatRole::ASSISTANT);
        message->set_content(response.message().content());
    }
}
