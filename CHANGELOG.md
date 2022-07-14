# 0.6.0

-   Initial release

# 0.6.1

-   Fixed cryptographic-ally secure random string generation to produce
    alpha-numeric values with 62^24 possible combinations.
-   Resolve an issue on MacOS where cryptographic-ally secure random strings
    that were generated were not valid UTF-8 format.
-   Added Apache 2.0 license
-   Added CHANGELOG.md
-   Updated README.md for example projects with example CLI usage
-   Updated examples to not regenerate client ID and secret when attempting to
    register a device that has credentials but no OAuth token.

# 0.8.0

-   Updated to support features in SensoryCloud version 0.8.0
    -   Implemented `RenewDeviceCredential` end-points in the `OAuthService`
    -   Updated `Authenticate` end-point in the `VideoService` to support
        enrollment groups
    -   Updated `CreateEnrollment` end-point in the `VideoService` to support
        number of required liveness frames
-   Updated `CreateEnrollmentGroup` end-points in the `ManagementService` to
    accept a vector of enrollment IDs to create the group with
-   Implemented a constructor in each service for building from pointers to
    each of the necessary stubs

# 0.11.4

-   Support for `v0.11.4` of the SensoryCloud API

# 0.12.4

-   Support for `v0.12.4` of the SensoryCloud API
-   Updated speech-to-text (STT) example to use new `FINAL` post-processing
    action to indicate the end of a finite audio stream.

# 0.12.10

-   Support for `v0.12.10` of the SensoryCloud API
-   Re-organize example code according to programming model.
-   Updates and breaking changes to `sensory::Config`
    -   Update to expect unary gRPC timeouts in milliseconds instead of seconds.
    -   Update to treat fully qualified domain name as the default storage
        medium for server addresses.
        -   Refactor internal data structures to favor fully qualified domain
            name over host-port combination.
        -   Implement a new constructor that expects a formatted FQDN.
        -   Leave all old functionality in place to prevent downstream breakage.
    -   Update `connect` to throw a `sensory::ConfigError` if called when a
        gRPC channel already exists.
    -   Update `get_channel` to throw `sensory::ConfigError` if called when a
        gRPC channel does not exist.
    -   Update with new `is_connected` getter for determining if `connect` has
        been called.
-   Updates relevant to all services:
    -   Added `get_config` getter for accessing the immutable config being used
        by a particular service.
    -   Refactored functional envy between `Config` and `TokenManager`; the
        token manager now handles all gRPC context setup outside of the scope
        of the config.
        -   `setup_unary_client_context` and `setup_bidi_client_context` both
            moved from `::sensory::Config` to
            `::sensory::token_manager::TokenManager`

# 0.12.1

-   Support for `SynthesizeSpeech` (text-to-speech) endpoints
