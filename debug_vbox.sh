#!/bin/bash
# debug_vbox.sh

echo "=== VirtualBox Zenix Debug ==="
echo ""

# Check if VM is running
if VBoxManage list runningvms | grep -q "Zenix_Test"; then
    echo "✓ VM is running"
    
    # Get VM info
    echo ""
    echo "VM Configuration:"
    VBoxManage showvminfo Zenix_Test | grep -E "(Memory|VRAM|Graphics|Boot)"
    
    echo ""
    echo "=== Serial Output (last 50 lines) ==="
    if [ -f /tmp/zenix_serial.log ]; then
        tail -50 /tmp/zenix_serial.log
    else
        echo "No serial log found"
    fi
else
    echo "✗ VM is not running"
    echo ""
    echo "Start with: ./vbox_test.sh"
fi