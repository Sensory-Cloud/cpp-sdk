// The Sensory Cloud C++ SDK Hello, World!
//
// Author: Christian Kauten (ckauten@sensoryinc.com)
//
// Copyright (c) 2021 Sensory, Inc.
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
// #include <grpc/grpc.h>
// #include <grpc++/channel.h>
// #include <grpc++/client_context.h>
// #include <grpc++/create_channel.h>
// #include <grpc++/security/credentials.h>
// #include <sensorycloud/sensorycloud.hpp>
#include <sensorycloud/config.hpp>
#include <sensorycloud/services/service.hpp>
// #include <sensorycloud/services/management_service.hpp>
#include <sensorycloud/services/oauth_service.hpp>
// #include <sensorycloud/services/audio_service.hpp>
// #include <sensorycloud/services/video_service.hpp>

int main() {
    // Initialize the configuration to the host for given address and port
    std::string host = "io.stage.cloud.sensory.com";
    uint16_t port = 50051;
    std::shared_ptr<sensory::Config> config(new sensory::Config);
    config->setCloudHost(host, port);
    // Set the Tenant ID for the default tenant
    config->tenantID = "cabb7700-206f-4cc7-8e79-cd7f288aa78d";

    // Create the base service
    sensory::service::Service service(config);
    // Create an OAuth service from the base service
    auto oauthService = sensory::service::OAuthService(service.getGRPCChannel());

    auto response = oauthService.getWhoAmI(*config);

    std::cout << "Hello, Sensory Cloud C++ SDK!" << std::endl;
}
