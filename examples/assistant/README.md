# SensoryCloud Assistant Services

This project provides a demonstration of SensoryCloud assistant services.

## Compilation

To compile the applications in this project:

```shell
mkdir -p build
cd build
cmake ..
make
```

## Usage

Before getting started you must spin up a [SensoryCloud][sensory-cloud]
inference server or have [SensoryCloud][sensory-cloud] spin one up for you. You
must also have the following pieces of information:

-   your inference server address and port number,
-   your SensoryCloud tenant ID, and
-   your configured secret key used for registering OAuth clients.

[sensory-cloud]: https://sensorycloud.ai/

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
