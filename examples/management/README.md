# SensoryCloud Management Services

This project implements SensoryCloud management tools.

## Compilation

To compile the tools in this project:

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

### Get server health

To return the health of the remote server:

```shell
./examples/management/build/management getHealth \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID>
```

### Get enrollments

To fetch the enrollments for a given user ID:

```shell
./examples/management/build/management getEnrollments \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID> \
    -u <user ID>
```

### Delete enrollment

To delete a particular enrollment:

```shell
./examples/management/build/management deleteEnrollment \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID> \
    -e <enrollment ID>
```

### Get enrollment groups

To fetch the enrollment groups a particular user belongs to:

```shell
./examples/management/build/management getEnrollmentGroups \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID> \
    -u <user ID>
```

### Delete enrollment group

To delete an enrollment group:

```shell
./examples/management/build/management deleteEnrollmentGroup \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID> \
    -e <group ID>
```

### Create enrollment group

To create a new enrollment group belonging to a particular user:

```shell
./examples/management/build/management createEnrollmentGroup \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID> \
    -u <user ID> \
    -n <group name> \
    -d <group description> \
    -m <model name> \
    -e <group ID>
```

### Append enrollment group

To append enrollments to an existing enrollment group:

```shell
./examples/management/build/management appendEnrollmentGroup \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID> \
    -e <group ID> \
    -E <enrollment ID>*
```
