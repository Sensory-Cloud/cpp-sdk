// The SensoryCloud C++ SDK management service demo (synchronous interface).
//
// Copyright (c) 2022 Sensory, Inc.
//
// Author: Jonathan Welch (jwelch@sensoryinc.com)
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
#include <list>
#include <mutex>
#include <thread>
#include <google/protobuf/util/time_util.h>
#include <grpcpp/completion_queue.h>
#include <sensorycloud/sensorycloud.hpp>
#include <sensorycloud/token_manager/file_system_credential_store.hpp>
#include "dep/argparse.hpp"

using SensoryCloudInstance =
    sensory::SensoryCloud<sensory::token_manager::FileSystemCredentialStore>;

struct modelData {
  std::string modelName;
  bool isEnrollable;
  modelData() : modelName(""), isEnrollable(false) {}
};

std::list<modelData> GetSpeechModels(SensoryCloudInstance *cloud,
                                     grpc::CompletionQueue *queue);
std::list<modelData> GetVideoModels(SensoryCloudInstance *cloud,
                                    grpc::CompletionQueue *queue);
std::list<modelData> GetSoundIDModels(SensoryCloudInstance *cloud,
                                      grpc::CompletionQueue *queue);

void ListAllModels(SensoryCloudInstance *cloud, grpc::CompletionQueue *queue);

int main(int argc, const char **argv) {
  // Create an argument parser to parse inputs from the command line
  auto parser = argparse::ArgumentParser(argc, argv)
                    .prog("apitool")
                    .description("A tool for exploring the sensory cloud API");

  parser.add_argument({"--getmodels"})
      .action("store_true")
      .help("List all available models for your tenant by type");
  parser.add_argument({"path"}).help(
      "The path to an INI file containing server metadata.");
  parser.add_argument({"-v", "--verbose"})
      .action("store_true")
      .help("Produce verbose output.");

  const auto args = parser.parse_args();
  const auto GETMODELS = args.get<bool>("getmodels");
  const auto PATH = args.get<std::string>("path");
  const auto VERBOSE = args.get<bool>("verbose");

  // Create a credential store for keeping OAuth credentials
  sensory::token_manager::FileSystemCredentialStore keychain(
      ".", "com.sensory.cloud.examples");

  // Create the cloud services handle
  SensoryCloudInstance cloud(PATH, keychain);

  // Query the health of the remote services
  sensory::api::common::ServerHealthResponse server_health;
  auto status = cloud.health.get_health(&server_health);
  if (!status.ok()) { // the call failed, print a descriptive message
    std::cout << "Failed to get server health (" << status.error_code()
              << "): " << status.error_message() << std::endl;
    return 1;
  }
  if (VERBOSE) {
    std::cout << "Server status:" << std::endl;
    std::cout << "\tisHealthy: " << server_health.ishealthy() << std::endl;
    std::cout << "\tserverVersion: " << server_health.serverversion()
              << std::endl;
    std::cout << "\tid: " << server_health.id() << std::endl;
  }

  // Initialize the client.
  sensory::api::v1::management::DeviceResponse response;
  status = cloud.initialize(&response);
  if (!status.ok()) { // the call failed, print a descriptive message
    std::cout << "Failed to initialize (" << status.error_code()
              << "): " << status.error_message() << std::endl;
    return 1;
  }

  grpc::CompletionQueue queue;

  if (GETMODELS)
    ListAllModels(&cloud, &queue);
}

void ListAllModels(SensoryCloudInstance *cloud, grpc::CompletionQueue *queue) {
  auto speechModels = GetSpeechModels(cloud, queue);
  auto videoModels = GetVideoModels(cloud, queue);
  auto soundIDModels = GetSoundIDModels(cloud, queue);

  // List out all the Audio Models
  for (auto &model : speechModels) {
    std::cout << "Speech to Text [" << model.modelName
              << "] -- isEnrollable:" << (model.isEnrollable ? "True" : "False")
              << std::endl;
  }

  // List out all Video Models
  for (auto &model : videoModels) {
    std::cout << "Face Biometric Model [" << model.modelName
              << "] -- isEnrollable:" << (model.isEnrollable ? "True" : "False")
              << std::endl;
  }

  // List out all SoundID Models
  for (auto &model : soundIDModels) {
    std::cout << "SoundID Models [" << model.modelName
              << "] -- isEnrollable:" << (model.isEnrollable ? "True" : "False")
              << std::endl;
  }
}

std::list<modelData> GetSpeechModels(SensoryCloudInstance *cloud,
                                     grpc::CompletionQueue *queue) {
  // We are going to list all available models

  /********** AUDIO MODELS ***************/
  // Get Models Through Audio Interface
  auto get_models_rpc = cloud->audio.get_models(queue);

  // Execute the async RPC in this thread (which will technically block)
  void *tag(nullptr);
  bool ok(false);
  // This is a blocking call -- AsyncNext is also available if you want this to
  // be non-blocking
  queue->Next(&tag, &ok);
  int error_code = 0;
  std::list<modelData> modelNames;
  if (ok && tag == get_models_rpc) {
    for (auto &model : get_models_rpc->getResponse().models()) {
      if (model.modeltype() ==
          sensory::api::common::ModelType::VOICE_TRANSCRIBE_GRAMMAR) {
        modelData mdata;
        mdata.modelName = model.name();
        mdata.isEnrollable = model.isenrollable();
        modelNames.push_back(mdata);
      }
    }
  }
  return modelNames;
}

std::list<modelData> GetSoundIDModels(SensoryCloudInstance *cloud,
                                      grpc::CompletionQueue *queue) {
  // We are going to list all available models

  /********** AUDIO MODELS ***************/
  // Get Models Through Audio Interface
  auto get_models_rpc = cloud->audio.get_models(queue);

  // Execute the async RPC in this thread (which will technically block)
  void *tag(nullptr);
  bool ok(false);
  // This is a blocking call -- AsyncNext is also available if you want this to
  // be non-blocking
  queue->Next(&tag, &ok);
  int error_code = 0;
  std::list<modelData> modelNames;
  if (ok && tag == get_models_rpc) {
    for (auto &model : get_models_rpc->getResponse().models()) {
      modelData mdata;

      switch (model.modeltype()) {
      case sensory::api::common::ModelType::SOUND_SCENE_FIXED:
      case sensory::api::common::ModelType::SOUND_EVENT_FIXED:
      case sensory::api::common::ModelType::SOUND_EVENT_ENROLLABLE:
      case sensory::api::common::ModelType::SOUND_EVENT_REVALIDATION:
        mdata.modelName = model.name();
        mdata.isEnrollable = model.isenrollable();
        modelNames.push_back(mdata);
        break;

      default:
        break;
      }
    }
  }
  return modelNames;
}

std::list<modelData> GetVideoModels(SensoryCloudInstance *cloud,
                                    grpc::CompletionQueue *queue) {
  // We are going to list all available models

  /********** VIDEO MODELS ***************/
  // Get Models Through Video Interface
  auto get_models_rpc = cloud->video.get_models(queue);

  // Execute the async RPC in this thread (which will technically block)
  void *tag(nullptr);
  bool ok(false);
  // This is a blocking call -- AsyncNext is also available if you want this to
  // be non-blocking
  queue->Next(&tag, &ok);
  int error_code = 0;
  std::list<modelData> modelNames;
  if (ok && tag == get_models_rpc) {
    for (auto &model : get_models_rpc->getResponse().models()) {
      if (model.modeltype() ==
              sensory::api::common::ModelType::FACE_BIOMETRIC ||
          model.modeltype() == sensory::api::common::FACE_RECOGNITION) {
        modelData mdata;
        mdata.modelName = model.name();
        mdata.isEnrollable = model.isenrollable();
        modelNames.push_back(mdata);
      }
    }
  }
  return modelNames;
}
