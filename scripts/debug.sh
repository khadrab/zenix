#!/bin/bash
# scripts/debug_run.sh - QEMU Debug Script

echo "Running Zenix in QEMU with debug output..."

qemu-system-i386 \
    -kernel bin/zenix.bin \
    -serial stdio \
    -vga std \
    -d guest_errors,int \
    -no-reboot \
    -monitor telnet:127.0.0.1:5555,server,nowait

# Alternative: with more verbose output
# qemu-system-i386 \
#     -kernel bin/zenix.bin \
#     -serial file:serial.log \
#     -vga std \
#     -d cpu_reset,int,guest_errors \
#     -no-reboot