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
