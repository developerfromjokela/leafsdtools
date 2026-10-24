# Building

Both variants are built with the open
[CeGCC toolchain](https://github.com/palian-tech/cegcc-build), available as a
Docker image:

```sh
docker run --rm -v "$PWD":/w -w /w ghcr.io/palian-tech/cegcc:v9.3.0-ce.1 \
  sh -c 'make TARGET=arm && make TARGET=sh4'
# build/arm/LeafSDTools.exe (QY8XXX), build/sh4/LeafSDTools.exe (QY7XXX)
```

## Card image

The card image is the upstream `LeafSDTools-111-beta.img` with the updater
replaced by the new build:

```sh
python3 ci/fetch-base-image.py   # downloads and verifies the upstream image
make card-image                  # in the same Docker image
# build/LeafSDTools.img
```
