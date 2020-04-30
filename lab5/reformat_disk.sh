#!/usr/bin/env bash
set -euo pipefail
IFS=$'\n\t'

echo "Reformating the disk !!..."
echo
echo
echo "========= Compiling the OS ======"
cd flat/os && make
cd -

echo "======== Running fdisk with Dfs Module disabled ======="
cd flat/apps/fdisk && make run
cd -

echo "======== Making disk +rwx ======="
chmod a+rwx /tmp/ee469g55.img

echo "Done! Bye..."
echo ""
echo ""
