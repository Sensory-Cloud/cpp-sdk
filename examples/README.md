# SensoryCloud C++ SDK Example Projects

This directory contains example projects for the services offered by
SensoryCloud. Each example project demonstrates the usage of different
services and provides a starting point for developers to integrate and utilize
SensoryCloud functionalities in their applications.

## Audio

The [audio](audio) example project implements interactive audio services. It
utilizes the `portaudio` library for real-time audio streaming and the
`libsndfile` library for offline processing of audio files. This example
project demonstrates how to integrate and interact with SensoryCloud's audio
services, including real-time streaming and file-based processing.

-   **Speech-to-Text**: Convert spoken language into written text. This
    service allows you to transcribe audio recordings or live speech in
    real-time.
-   **Text-to-Speech**: Generate natural-sounding speech from written text.
    This service enables you to convert text into spoken language with various
    voice options and customization.
-   **Sound Event Detection**: Identify and classify sound events in audio
    recordings or real-time audio streams. This service can detect specific
    sounds, such as doorbells, sirens, or alarms, and provide event
    notifications.
-   **Sound Event Identification**: Recognize and identify specific sounds or
    acoustic patterns in audio data. This service can identify sounds like dog
    barking, car engine revving, or musical instruments, among others.
-   **Voice Biometrics**: Perform speaker verification and identification
    based on voice characteristics. This service allows you to authenticate
    and verify individuals based on their unique vocal features.

The audio example project demonstrates the integration and usage of these
services, providing developers with a practical foundation for building
applications that leverage SensoryCloud's audio capabilities.

## Video

The [video](video) example project demonstrates the implementation of
interactive video services. It showcases the integration with various video
processing toolkits, such as OpenCV, to process and analyze video data. This
example project provides insights into integrating SensoryCloud's video
services with different video processing libraries.

-   **Face Biometrics**: Perform face recognition and identification based on
    facial features. This service enables you to authenticate and verify
    individuals using their facial characteristics.
-   **Face Liveness Detection**: Determine the authenticity of a face in
    real-time by detecting spoofing attempts or the presence of a live person.
    This service helps in ensuring the security and trustworthiness of
    face-based authentication systems.

The video example project demonstrates the usage of these services, providing
developers with a starting point for incorporating face biometrics and/or face
liveness detection into their video applications.

## Assistant

The [assistant](assistant) example project showcases the implementation of an
interactive assistant based on Language Models (LLMs). It illustrates how to
utilize SensoryCloud's LLM-based assistant services to create conversational
agents, chatbots, or voice assistants. This example project serves as a guide
for building interactive assistants using SensoryCloud's features.

## Management

The [management](management) example project focuses on the implementation of
user management services provided by SensoryCloud. It demonstrates how to
manage and control various aspects of SensoryCloud's biometric and
authentication features. This example project offers functionality for
managing users and enrollments, including basic CRUD operations. Additionally,
it provides the ability to create enrollment groups for 1-to-small-N
authentication purposes.
