# SensoryCloud Management Services

This project provides a demonstration of SensoryCloud management tools.

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

### Get server health

To return the health of the remote server:

```shell
./management <path to config.ini file> get_health
```

### Get enrollments

To fetch the enrollments for a given user ID:

```shell
./management <path to config.ini file> get_enrollments -u <user ID>
```

### Delete enrollment

To delete a particular enrollment:

```shell
./management <path to config.ini file> delete_enrollment -e <enrollment ID>
```

### Get enrollment groups

To fetch the enrollment groups a particular user belongs to:

```shell
./management <path to config.ini file> get_enrollment_groups -u <user ID>
```

### Delete enrollment group

To delete an enrollment group:

```shell
./management <path to config.ini file> delete_enrollment_group -e <group ID>
```

### Create enrollment group

To create a new enrollment group belonging to a particular user:

```shell
./management <path to config.ini file> create_enrollment_group \
    -u <user ID> \
    -n <group name> \
    -d <group description> \
    -m <model name> \
    -e <group ID>
```

### Append enrollment group

To append enrollments to an existing enrollment group:

```shell
./management <path to config.ini file> append_enrollment_group \
    -e <group ID> \
    -E <enrollment ID>*
```

<!-- URLs -->

[sensory-cloud]: https://sensorycloud.ai/
