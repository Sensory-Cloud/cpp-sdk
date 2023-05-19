# Change Log

## 1.0.8

-   Fix authentication from token file example to persist token buffer until
    no longer needed by gRPC
-   Update global header to include optional components for ease of use
-   Update tutorial

## 1.0.7

-   Update management tools to print a group ID when creating enrollment groups
-   Update audio biometric examples (from files) with support for enrolling
    to binary feature vectors and authenticating against binary feature vectors

## 1.0.6

-   Improve documentation

## 1.0.5

-   Update `SensoryCloud` INI-based constructors to expect the device ID and
    name to exists as environment variables. When these values are not
    provided, automatically generate them and store them in the secure
    credential store for re-use

## 1.0.4

-   Update CLI short-hand for `language` to be the standard `-L` (instead of
    `-l`, which is reserved for liveness) in the audio file examples

## 1.0.3

-   Fix all usage of cmake to require version 1.14
-   Remove old build chain tools for audio file examples
-   Update verbose logging to output JSON format for all service examples

## 1.0.2

-   Update vision examples
    -   Incorporate new bounding box data from responses into the GUI
    -   Add command line argument for controlling the image transport codec
    -   Update logging to print server responses in JSON format
-   Fix the case of 0 chunk size in the audio file examples
    -   The expected functionality is to send the entire audio sample when the
        chunk size is 0
    -   a floating point exception was being raised due to an invalid
        divide-by-zero
        -   this has been corrected for all audio file examples

## 1.0.1

-   Fix [CMakeLists.txt](CMakeLists.txt) with a minimum version of 3.14. This
    version is required to use the `FetchContent_MakeAvailable` command.

## 1.0.0

-   Support for SensoryCloud 1.0.0

## 0.17.6

-   Move code definitions from header files to separate source cpp files where
    appropriate
-   Fix an issue in the audio file examples where the `FINAL` message was not
    sent for cases where the number of samples in an input file a mulitple of
    the specified chunk size.
-   Fix an issue in the STT transcript aggregator where non-alphanumeric
    characters were being trimmed from the aggregated transcripts

## 0.17.5

-   Update `sensory::util::{lstrip,rstrip,strip}` to more strictly adhere to
    the Python semantics that they derive from. Instead of only stripping
    white-space, the methods now strip any character outside of the range of
    (0x20, 0x7F) in arbitrary combinations (i.e., only simple characters are
    left.)
-   Implement new `sensory::io::path` namespace with methods for normalizing
    URIs to a standard `host:port` format.
-   Resolve a bug in config.ini parsing where fully qualified domain names
    formatted like `https://domain` would cause an error from the `stoi`
    function. Protocols are now stripped from URIs when parsed.

## 0.17.4

-   Resolve issue where OpenSSL was not linked in FetchContent install pathways
-   Update [examples](examples)
    -   Update [audio](examples/audio)
        -   Deprecate `--padding` flag for audio service examples, any padding
            will be calibrated and conducted server-side
        -   Add `--sample_rate`/`-fs` flag to speech synthesis examples to allow
            control over the sample rate of the audio that is returned.
        -   Move `dr_wav` examples to `audio/file` and replace local `dr_*`
            dependencies with libsndfile, pulled using `FetchContent`

## 0.17.3

-   INI parser bug-fixes and improvements (for constructing `SensoryCloud`)
    -   runtime error handling for irrecoverable failure cases:
        1.  a file-path points to a non-existent file,
        2.  a file contains improperly formatted INI data, and
        3.  a file is missing sections/keys that are required.

## 0.17.2

-   Bug fixes
    -   Add missing logic to query user enrollments to
        [examples/dr_wav/validate_enrolled_event.cpp](examples/dr_wav/validate_enrolled_event.cpp)
    -   Add missing `cloud.initialize` calls to example code
-   New features
    -   Support for capitalization and punctuation in transcription examples
    -   Support for single utterance mode in transcription examples
    -   Support for custom vocabulary in transcription examples
    -   Support for specifying reference IDs to audio and video enrollments
    -   Support for the minimum number of live frames for video enrollment
-   Removals
    -   Remove support for convenience initializers of gRPC stream configuration
        objects in favor of explicitly instantiating gRPC structures.
-   Namespace/symbolic changes
    -   Internal code style standardized to snake-case
    -   `sensory::service::audio::TranscriptAggregator` moved to
        `sensory::util::TranscriptAggregator`
    -   `sensory::service::audio::(l|r)?strip` moved to
        `sensory::util::(l|r)?strip`
    -   `sensory::token_manager::InsecureCredentialStore` renamed to
        `sensory::token_manager::FileSystemCredentialStore`
    -   Error structures moved from their respective namespaces to a new
        `sensory::error` namespace to contain all error structures.
        -   `sensory::service::NullStreamError` ->
            `sensory::error::NullStreamError`
        -   `sensory::service::WriteStreamError` ->
            `sensory::error::WriteStreamError`
        -   `sensory::service::ReadStreamError` ->
            `sensory::error::ReadStreamError`
        -   `sensory::ConfigError` -> `sensory::error::ConfigError`
    -   RPC data structures moved from global namespace to new `calldata`
        namespace. This includes:
        -   `sensory::calldata::AwaitableBidiReactor`
        -   `sensory::calldata::AwaitableReadReactor`
        -   `sensory::calldata::AwaitableWriteReactor`
        -   `sensory::CallData` -> `sensory::calldata::CallbackData`
        -   `sensory::calldata::AsyncReaderWriterCall`
        -   `sensory::calldata::AsyncResponseReaderCall`

## 0.15.2

-   Video service examples updated to support file IO (video and single images)
-   Audio service examples updated to support file IO
-   Implemented support for JSON web token (JWT) device enrollments
-   Deprecated internal implementation of RC4 cryptography algorithm in favor
    of `RAND_bytes` from required SSL dependency

## 0.15.1

-   Support for `v0.15.1` of the SensoryCloud API
    -   New `sensory::services::audio::TranscriptAggregator` for aggregating
        partial speech-to-text updates from the server into a final transcript.
    -   Audio transcription examples updated with
        `sensory::services::audio::TranscriptAggregator` and optional
        closed-captioning mode for rendering partial transcripts.

## 0.12.1

-   Support for `SynthesizeSpeech` (text-to-speech) endpoints

## 0.12.10

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

## 0.12.4

-   Support for `v0.12.4` of the SensoryCloud API
-   Updated speech-to-text (STT) example to use new `FINAL` post-processing
    action to indicate the end of a finite audio stream.

## 0.11.4

-   Support for `v0.11.4` of the SensoryCloud API

## 0.8.0

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

## 0.6.1

-   Fixed cryptographic-ally secure random string generation to produce
    alpha-numeric values with 62^24 possible combinations.
-   Resolve an issue on MacOS where cryptographic-ally secure random strings
    that were generated were not valid UTF-8 format.
-   Added Apache 2.0 license
-   Added CHANGELOG.md
-   Updated README.md for example projects with example CLI usage
-   Updated examples to not regenerate client ID and secret when attempting to
    register a device that has credentials but no OAuth token.

## 0.6.0

-   Initial release
