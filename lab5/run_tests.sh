#!/usr/bin/env bash
set -euo pipefail
IFS=$'\n\t'
echo "Running tests..."
echo
echo
echo "========= Compiling the OS ======"
cd flat/os && make
cd -

echo "======== Running os tests ======="
cd flat/apps/ostests && make run
cd -

echo "======== Running file tests ======="
cd flat/apps/file_test && make run
cd -
echo "Done! Bye..."
echo ""
echo ""
