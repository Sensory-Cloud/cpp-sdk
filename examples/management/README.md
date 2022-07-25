# SensoryCloud Management Services

This project implements [SensoryCloud][sensory-cloud] management tools.

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

### Get server health

To return the health of the remote server:

```shell
./management <path to config.ini file> getHealth
```

### Get enrollments

To fetch the enrollments for a given user ID:

```shell
./management <path to config.ini file> getEnrollments -u <user ID>
```

### Delete enrollment

To delete a particular enrollment:

```shell
./management <path to config.ini file> deleteEnrollment -e <enrollment ID>
```

### Get enrollment groups

To fetch the enrollment groups a particular user belongs to:

```shell
./management <path to config.ini file> getEnrollmentGroups -u <user ID>
```

### Delete enrollment group

To delete an enrollment group:

```shell
./management <path to config.ini file> deleteEnrollmentGroup -e <group ID>
```

### Create enrollment group

To create a new enrollment group belonging to a particular user:

```shell
./management <path to config.ini file> createEnrollmentGroup \
    -u <user ID> \
    -n <group name> \
    -d <group description> \
    -m <model name> \
    -e <group ID>
```

### Append enrollment group

To append enrollments to an existing enrollment group:

```shell
./management <path to config.ini file> appendEnrollmentGroup \
    -e <group ID> \
    -E <enrollment ID>*
```

<!-- URLs -->

[sensory-cloud]: https://sensorycloud.ai/
