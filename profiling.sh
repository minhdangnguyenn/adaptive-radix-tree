#!/bin/bash
set -e

BUILD_DIR="build"

# configure and build
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
make art_bench

echo ""
echo "===== ns/lookup ====="
./art_bench

echo ""
echo "===== perf stat (counters) ====="
perf stat ./art_bench

echo ""
echo "===== perf record (call graph) ====="
perf record ./art_bench
perf report --stdio | head -20
echo ""
echo "Full interactive report: perf report"
