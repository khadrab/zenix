#!/bin/bash
# Final test on real hardware

make clean && make && make iso

echo "======================================"
echo "  Zenix - Ready for Real Hardware"
echo "======================================"
echo ""
echo "Insert USB drive and run:"
echo "  sudo dd if=zenix.iso of=/dev/sdX bs=4M status=progress"
echo ""
echo "Then boot from USB on your HP machine"
echo "Expected: Intel HD 4600 initialization + graphics"