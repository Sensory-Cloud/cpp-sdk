# Release Check List

This document outlines the steps involved with releasing a new version of the
SDK.

1.  Pull latest proto files from `proto` submodule
1.  Recompile proto headers and source files
1.  Run unit tests against all target platforms and confirm passage
1.  Compile example projects against all target platforms using development
    branch
1.  Update `GIT_REPOSITORY` and `GIT_TAG` in README snippets and
    `CMakeLists.txt` of example projects.
1.  Update documentation with doxygen (`doxy`).
1.  Update `CHANGELOG.md` with the new features / bug fixes / changes
1.  Request a code review from another team member
1.  Merge changes into the main branch (they will be mirrored to GitHub)
1.  Tag the commit and document the changes in the release notes
1.  Compile example projects against all target platforms using new tag
