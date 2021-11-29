// An example of face services based on OpenCV camera streams.
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
#include <sensorycloud/config.hpp>
#include <sensorycloud/services/health_service.hpp>
#include <sensorycloud/services/oauth_service.hpp>
#include <sensorycloud/services/management_service.hpp>
#include <sensorycloud/services/audio_service.hpp>
#include <sensorycloud/services/video_service.hpp>
#include <sensorycloud/token_manager/keychain.hpp>
#include <sensorycloud/token_manager/token_manager.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>

int main(int argc, const char** argv) {
    cv::CommandLineParser parser(argc, argv, "{help h||}{@device||}");
    // if (parser.has("help")) {
    //     help(argv);
    //     return 0;
    // }
    // double scale = parser.get<double>("scale");
    // if (scale < 1) scale = 1;
    // bool tryflip = parser.has("try-flip");
    std::string device = parser.get<std::string>("@device");
    if (!parser.check()) {
        parser.printErrors();
        return 0;
    }



    // Initialize the configuration to the host for given address and port
    sensory::Config config(
        "io.stage.cloud.sensory.com",
        443,
        "cabb7700-206f-4cc7-8e79-cd7f288aa78d",
        "D895F447-91E8-486F-A783-6E3A33E4C7C5"
    );
    std::cout << "Connecting to remote host: " << config.getFullyQualifiedDomainName() << std::endl;

    // Query the health of the remote service.
    sensory::service::HealthService healthService(config);
    sensory::api::common::ServerHealthResponse serverHealth;
    auto status = healthService.getHealth(&serverHealth);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "GetHealth failed with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }
    // Report the health of the remote service
    std::cout << "Server status:" << std::endl;
    std::cout << "\tisHealthy: " << serverHealth.ishealthy() << std::endl;
    std::cout << "\tserverVersion: " << serverHealth.serverversion() << std::endl;
    std::cout << "\tid: " << serverHealth.id() << std::endl;

    // Query the user ID
    std::string userID = "";
    std::cout << "user ID: ";
    std::cin >> userID;
    // Query the shared pass-phrase
    std::string password = "";
    std::cout << "password: ";
    std::cin >> password;

    // Create an OAuth service
    sensory::service::OAuthService oauthService(config);
    sensory::token_manager::Keychain keychain("com.sensory.cloud");
    sensory::token_manager::TokenManager<sensory::token_manager::Keychain> token_manager(oauthService, keychain);

    // Get the client ID and client secret from the secure credential store
    const auto clientID = keychain.at("clientID");
    const auto clientSecret = keychain.at("clientSecret");

    // Query the available video models
    std::cout << "Available video models:" << std::endl;
    sensory::service::VideoService<sensory::token_manager::Keychain> videoService(config, token_manager);
    sensory::api::v1::video::GetModelsResponse videoModelsResponse;
    status = videoService.getModels(&videoModelsResponse);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "GetVideoModels failed with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }
    for (auto& model : videoModelsResponse.models())
        std::cout << "\t" << model.name() << std::endl;

    std::string videoModel = "";
    std::cout << "Video model: ";
    std::cin >> videoModel;

    // Query this user's active enrollments
    std::cout << "Active enrollments:" << std::endl;
    sensory::service::ManagementService<sensory::token_manager::Keychain> mgmtService(config, token_manager);
    sensory::api::v1::management::GetEnrollmentsResponse enrollmentResponse;
    status = mgmtService.getEnrollments(&enrollmentResponse, userID);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "GetEnrollments failed with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }
    for (auto& enrollment : enrollmentResponse.enrollments()) {
        std::cout << "\tDesc: "            << enrollment.description()  << std::endl;
        std::cout << "\t\tModel Name: "    << enrollment.modelname()    << std::endl;
        std::cout << "\t\tModel Type: "    << enrollment.modeltype()    << std::endl;
        std::cout << "\t\tModel Version: " << enrollment.modelversion() << std::endl;
        std::cout << "\t\tUser ID: "       << enrollment.userid()       << std::endl;
        std::cout << "\t\tDevice ID: "     << enrollment.deviceid()     << std::endl;
        // std::cout << "\t\tCreated: "       << enrollment.createdat()    << std::endl;
        std::cout << "\t\tID: "            << enrollment.id()    << std::endl;
    }

    std::string enrollmentID = "";
    std::cout << "Enrollment ID: ";
    std::cin >> enrollmentID;

    // Create the stream
    auto stream = videoService.authenticate(enrollmentID);

    // Create an image capture object
    cv::VideoCapture capture;
    // Look for a device ID from the command line
    if (device.empty() || (isdigit(device[0]) && device.size() == 1)) {
        // Default to device 0 if the device was not provided. Use ASCII
        // subtraction to convert the digit to an integer.
        int camera = device.empty() ? 0 : device[0] - '0';
        if (!capture.open(camera)) {
            std::cout << "Capture from camera #" << camera << " didn't work" << std::endl;
            return 1;
        }
    } else {  // Device ID was not a valid integer value.
        std::cout << "Device ID \"" << device << "\" is not a valid integer!" << std::endl;
    }

    // Start capturing frames from the device.
    std::cout << "Video capturing has been started ..." << std::endl;
    for (;;) {
        cv::Mat frame;
        capture >> frame;
        if (frame.empty())
            break;

        cv::imshow("Sensory Cloud C++ SDK OpenCV Face Authentication Example", frame);
        std::vector<unsigned char> buffer;
        cv::imencode(".jpg", frame, buffer);

        double t = (double) cv::getTickCount();

        sensory::api::v1::video::AuthenticateRequest request;
        request.set_imagecontent(buffer.data(), buffer.size());
        stream->Write(request);
        sensory::api::v1::video::AuthenticateResponse response;
        stream->Read(&response);

        t = (double) cv::getTickCount() - t;

        std::cout << "Frame Response:" << std::endl;
        printf("\tResponse time: %g ms\n", t * 1000.f / cv::getTickFrequency());
        std::cout << "\tSuccess: "  << response.success() << std::endl;
        std::cout << "\tScore: "    << response.score() << std::endl;
        std::cout << "\tIs Alive: " << response.isalive() << std::endl;

        if (response.success()) break;

        char c = (char) cv::waitKey(10);
        if (c == 27 || c == 'q' || c == 'Q')
            break;
    }

    stream->WritesDone();
    status = stream->Finish();
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Authenticate stream failed with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }

    return 0;
}
