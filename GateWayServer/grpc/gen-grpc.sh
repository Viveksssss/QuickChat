#!/bin/bash

set -euo pipefail

OUT_DIR=.

protoc --cpp_out="$OUT_DIR" \
    --grpc_out="$OUT_DIR" \
    --plugin=protoc-gen-grpc=$(which grpc_cpp_plugin) \
    ./*.proto
echo "protoc完毕"
