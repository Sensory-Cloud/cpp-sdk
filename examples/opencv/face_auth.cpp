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
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

// #include <grpc/grpc.h>
// #include <grpc++/channel.h>
// #include <grpc++/client_context.h>
// #include <grpc++/create_channel.h>
// #include <grpc++/security/credentials.h>

// #include <sensorycloud/generated/common/common.pb.h>
// #include <sensorycloud/generated/health/health.pb.h>
// #include <sensorycloud/generated/oauth/oauth.pb.h>
// #include <sensorycloud/generated/v1/audio/audio.pb.h>
// #include <sensorycloud/generated/v1/event/event.pb.h>
// #include <sensorycloud/generated/v1/file/file.pb.h>
// #include <sensorycloud/generated/v1/management/enrollment.pb.h>
// #include <sensorycloud/generated/v1/management/device.pb.h>
// #include <sensorycloud/generated/v1/video/video.pb.h>
// #include <sensorycloud/generated/validate/validate.pb.h>
#include <sensorycloud/sensorycloud.hpp>
#include <sensorycloud/config.hpp>
#include <sensorycloud/services/management_service.hpp>
#include <sensorycloud/services/oauth_service.hpp>

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

    sensory::Config config("io.stage.cloud.sensory.com", 443);

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
    cv::Mat frame;
    for (;;) {
        capture >> frame;
        if (frame.empty())
            break;
        // double t = 0;
        // t = (double)getTickCount();
        cv::imshow("Sensory Cloud C++ SDK OpenCV Face Authentication Example", frame);
        // t = (double)getTickCount() - t;
        // printf("render time = %g ms\n", t*1000/getTickFrequency());

        char c = (char) cv::waitKey(10);
        if (c == 27 || c == 'q' || c == 'Q')
            break;
    }

    return 0;
}
