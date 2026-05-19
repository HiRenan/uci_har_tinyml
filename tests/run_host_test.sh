#!/usr/bin/env bash
set -euo pipefail
cc -std=c99 -O2 -I.. tests/host_model_test.c -lm -o tests/host_model_test
./tests/host_model_test | tee tests/host_model_test_output.txt
