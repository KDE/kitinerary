# Static extractor executable build

For use e.g. as part of Nextcloud it can be useful to build a fully static executable binary.

This is done by the `static-extractor` Gitlab job, the scripts in this folder are the implementation detail of that job.

## Supported platforms

In its current form the target platform needs to have at least GLIBC 2.25. Further reducing that should be possible
by using an older image for the build and retrofitting newer CMake and newer compilers.

## Deployment

TODO where to put the translation catalogs and iso-codes files? can we embed them via qrc?

## Known Issues

- zlib isn't linked statically yet

## Local builds

If you want to locally reproduce the same build, this can be done by running the commands from the build scripts in the
order specified in `.gitlab-ci.yml` in an environment set up like follows:

```
docker run -it --rm opensuse/leap:15.2
export CI_PROJECT_PATH=pim/kitinerary
export BUILD_ROOT=/builds
export STAGING_ROOT=/builds/staging
```

Optionally, map a local folder into `/builds` in the Docker image to retain the checkouts and build results. This
is particularly useful when debugging/optimizing the build itself.

## Future Work

Ideally this would reuse the static build artifacts produced by the CI already anyway, and reuse the
existing dependency mechanism of the CI.
