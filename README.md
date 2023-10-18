# qst-backend-go

## Introduction

This is the backend of [qst](https://github.com/kumudiaorong/qst-grpc) written in rust.
## Getting Started

### Get Release

No release yet.

### Build from Source

#### Dependencies

First please install [go](https://go.dev/dl/).

Then see [grpc](https://grpc.io/docs/languages/go/quickstart/) for other dependencies.

#### Get Source

```bash
git clone https://github.com/kumudiaorong/qst-backend-go.git
git submodule update --init
```
then change `go_package` in daemon.proto and defs.proto to `qst-backend-go/...`.
and run `./run.sh go` to generate go code.
#### Build

```bash
cargo b --bin qst-f --release
```
The binary will be in `target/release/qst-f`.