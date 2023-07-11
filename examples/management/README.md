# SensoryCloud Management Services

This project provides a demonstration of SensoryCloud management tools. This
includes create, read, update, and delete (CRUD) operations for enrollments
and enrollment groups.

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

### Server Health Check

To retrieve the health status of your inference server, use the following
command:

```shell
./management <path to config.ini file> get_health
```

-   `<path to config.ini file>` is a path to your SensoryCloud config file.

Upon successful connection to the server, a JSON response will be returned that
relays the server's version, unique identifier, individual sub-system status,
and aggregate health.

### Enrollments

The SensoryCloud management service allows you to interact with enrollments in
the system using simple read and delete operations. It is worth noting that
enrollments can only be _created_ using one of the video and/or audio services
that provides enroll-able models. In most cases, enrollments are immutable
following their creation.

### Get Enrollments

To query enrollments in the system, one must know the user ID that was provided
upon enrollment in the system. Depending on the architecture of your
application this may be an email address, alpha-numeric string, or potentially
a UUID. Once a user ID has been identified, enrollments belonging to the user
may be fetched using the following command:

```shell
./management <path to config.ini file> get_enrollments -u <user ID>
```

-   `<path to config.ini file>` is a path to your SensoryCloud config file.
-   `<user ID>` is the ID of the user that was provided during enrollment

This will return a series of JSON objects, each describing an enrollment in the
system corresponding to the given user ID. The JSON includes the unique
identifier of the enrollment, metadata about the model the user enrolled with,
whether the enrollment is biometric and/or required liveness verification, and
time-stamps of the enrollment's creation and last update dates.

### Delete Enrollment

In some cases you may need to remove an enrollment from the system. In order to
do so, you must identify the unique identifier of the enrollment in question.
If you don't know this value, it may be queried from the server based a user
ID using the above [Get Enrollments](#get-enrollments) operation. Once an ID
of an enrollment to delete has been identified, it may be deleted by executing:

```shell
./management <path to config.ini file> delete_enrollment -e <enrollment ID>
```

-   `<path to config.ini file>` is a path to your SensoryCloud config file.
-   `<enrollment ID>` is the ID of the enrollment to remove from the system.

**N.B.:** Due to the cross-device trust design of SensoryCloud, a
bio-metrically enrolled user must always have at least one enrollment in the
system. This ensures that users cannot have their last enrollment deleted,
which would lock them out of the system. If the last of a user's enrollments
must be removed, the user must enroll a second time using any of the biometric
enrollment services.

### Enrollment Groups

A unique feature of SensoryCloud is its ability to group enrollments that
correspond to the same model. This allows for _1-to-N_ lookup and
authentication applications where _N_ is relatively small. The following
passages describe how to create, read, update, and delete enrollment groups.

### Create Enrollment Group

Enrollment groups provide a mechanism for grouping enrollments from the same
model into a shared representation that can both authenticate and identify
among _N_ individual enrollments. In order to create an enrollment group,
a user ID and model name are required. The user ID describes the user acting
as the owner and administrator of the group, and the model name describes the
model that enrollments belonging to the group correspond to. Because enrollment
groups may contain zero enrollments, the model name is provided explicitly, as
opposed to be implied by the enrollments in the group. To create a new
enrollment group, the following may be used:

```shell
./management <path to config.ini file> create_enrollment_group \
    -u <user ID> \
    -n <group name> \
    -d <group description> \
    -m <model name> \
    -e <group ID> \
    -E <enrollment ID*>
```

-   `<path to config.ini file>` is a path to your SensoryCloud config file.
-   `<user ID>` is the ID of the user that is administrating the group.
-   `<group name>` is a string describing the name of the enrollment group,
    e.g., `"Access Control Group #3"`.
-   `<group description>` is a human read-able string that provides additional
    descriptive context of the enrollment group, e.g.,
    `"Users with access to server room 4"`. This argument is optional and may
    be omitted in cases where a description is not necessary.
-   `<model name>` is the name of the model that enrollments added to the group
    must correspond to, e.g., `face_recognition`.
-   `<group ID>` is an optional unique identifier to assign to the enrollment
    group. This value is optional and will be automatically generated and
    returned in cases where none is explicitly given to the CLI.
-   `<enrollment ID*>` is a space delimited list of the enrollment IDs to
    create the group from. Enrollment groups can be empty, so this value is
    optional upon creation. Enrollments may be added to the group
    retro-actively using, e.g., the
    [Append Enrollment Group](#append-enrollment-group) operation below.

Once an enrollment group has been created, its group ID may be used in place
of an individual enrollment ID in any audio/video authentication services.
Please refer to the [audio](../audio) and [video](../video) examples for more
information about authenticating against enrollments and enrollment groups.

### Get Enrollment Groups

Enrollment groups in the cloud are owned by the user that was specified upon
creation of the group. As such, to query enrollment groups from the cloud, the
user ID of the group's owner must be known. Below provides the command to query
enrollment groups belonging to a particular user:

```shell
./management <path to config.ini file> get_enrollment_groups -u <user ID>
```

-   `<path to config.ini file>` is a path to your SensoryCloud config file.
-   `<user ID>` is the ID of the user that is administrating the group(s).

This will return a JSON formatted output of the enrollment groups belonging to
the user. This includes information about the group itself, as well as the
individual enrollments that are contained by the group.

### Append Enrollment Group

In many cases it is desirable to add enrollments to an existing enrollment
group. In order to do so, both the ID of the enrollment group, as well as the
ID(s) of new enrollment(s) to add to the group must be known. Appending
supports adding multiple enrollments to a group in one call. To append
enrollments to an existing group, the following may be used:

```shell
./management <path to config.ini file> append_enrollment_group \
    -e <group ID> \
    -E <enrollment ID*>
```

-   `<path to config.ini file>` is a path to your SensoryCloud config file.
-   `<group ID>` is the ID of the group to add enrollments to
-   `<enrollment ID*>` is a space delimited list of the enrollment IDs to
    add to the group.

### Remove Enrollments From Group

Sometimes it may be necessary to remove an enrollment from an existing group.
In a similar fashion to appending enrollments to a group, enrollments may be
removed from a group using the following:

```shell
./management <path to config.ini file> remove_enrollments_from_group \
    -e <group ID> \
    -E <enrollment ID*>
```

-   `<path to config.ini file>` is a path to your SensoryCloud config file.
-   `<group ID>` is the ID of the group to add enrollments to
-   `<enrollment ID*>` is a space delimited list of the enrollment IDs to
    detach from the group. Note that these enrollments are not deleted
    entirely, they are simply removed from the enrollment group in question.

### Delete Enrollment Group

In certain cases enrollment groups will need to be deleted entirely. When an
enrollment group is deleted, its associated enrollments are removed from the
group, _but not deleted from the server._ In order to delete enrollments
entirely, please refer to [Delete Enrollment](#delete-enrollment). The
following command allows one to delete an enrollment group by ID:

```shell
./management <path to config.ini file> delete_enrollment_group -e <group ID>
```

-   `<path to config.ini file>` is a path to your SensoryCloud config file.
-   `<group ID>` is the ID of the group to delete

## Verbose Outputs

If you're curious about the inner workings of the SensoryCloud API and SDK,
verbose output may be enabled using the `-v` flag. This results in the
emission of each response from the server to the standard output in JSON
format. This can be useful for debugging cases that are otherwise not obviated
by our command line/graphical user interfaces.
