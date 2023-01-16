# Static extractor executable build

For use e.g. as part of Nextcloud it can be useful to build a fully static executable binary.

This is done by the `static-extractor` Gitlab job, the scripts in this folder are the implementation detail of that job.

## Supported platforms

In its current form the target platform needs to have at least GLIBC 2.25. Further reducing that should be possible
by using an older image for the build and retrofitting newer CMake and newer compilers.

## Deployment

The extractor needs external data in the form of translation catalogs and iso-codes files. Those are provided
in the same archive as the binary, and need to be placed in the same relative location to the binary to be found.

Alternatively, they can be placed in a path listed in the `XDG_DATA_DIRS` environment variable.

## Translations

When using the iCal output format, translations are relevant. This is done using Gettext and thus follows the
environment variables that influence that (`LANG`, `LANGUAGE`, `LC_ALL`, etc).

This also implies that Glibc locale data has to be installed on the system, Gettext will not work without those.

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
