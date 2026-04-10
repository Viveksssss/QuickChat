#!/bin/bash

set -euo pipefail

OUT_DIR=.

protoc --cpp_out="$OUT_DIR" \
  ./*.proto

echo "protoc完毕"

