# SensoryCloud Assistant Services

This project provides a demonstration of SensoryCloud assistant services.

## Compilation

To compile the applications in this project, follow these steps:

```shell
mkdir -p build
cd build
cmake ..
make
```

## Usage

Before you start using this example project, it's important to have a
SensoryCloud inference server set up. If you're new to SensoryCloud, we offer
a free trial server that allows you to test our cloud platform and determine
its suitability for your product. To learn more about deploying an inference
server with SensoryCloud, please visit the [SensoryCloud website][trial].

Once your server is up and running, you'll need the following information to
effectively interact with it using this SDK:

-   The address and port number of your inference server,
-   Your SensoryCloud tenant ID, and
-   Your configured secret key used for registering OAuth clients.

If you have any questions or need assistance with server setup or
configuration, please don't hesitate to [contact our sales team][sales]. We
are here to help ensure a smooth and successful integration of SensoryCloud
into your product.

[trial]: https://sensorycloud.ai/free-credits/
[sales]: https://sensorycloud.ai/resources/contact-us/

### Text Chat

To fetch assistant models:

```shell
./assistant <path to config.ini file> -g
```

To start an interactive chat session:

```shell
./assistant <path to config.ini file> -m <model name>
```

This will start an interactive shell similar to a Python interpreter. Simply
chat with the assistant as you would any other instant messaging client. When
you are ready to terminate the session, enter the command `exit`.
