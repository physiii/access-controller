#!/bin/bash

echo "=========================================="
echo "ESP32-S3 Interface & Reset Diagnostic"
echo "=========================================="
echo ""

# Check available interfaces
echo "🔍 Step 1: Detecting Available Interfaces"
echo "----------------------------------------"

USB_PORTS=($(ls /dev/ttyUSB* 2>/dev/null))
ACM_PORTS=($(ls /dev/ttyACM* 2>/dev/null))

if [ ${#USB_PORTS[@]} -gt 0 ]; then
    echo "✅ UART Interface found: ${USB_PORTS[@]}"
else
    echo "❌ No UART interfaces (/dev/ttyUSB*) found"
fi

if [ ${#ACM_PORTS[@]} -gt 0 ]; then
    echo "✅ USB Interface found: ${ACM_PORTS[@]}"
else
    echo "❌ No USB interfaces (/dev/ttyACM*) found"
fi

ALL_PORTS=("${USB_PORTS[@]}" "${ACM_PORTS[@]}")

if [ ${#ALL_PORTS[@]} -eq 0 ]; then
    echo ""
    echo "❌ ERROR: No ESP32-S3 interfaces detected!"
    echo "💡 Check USB cable connection and try again."
    exit 1
fi

echo ""
echo "🧪 Step 2: Testing Interface Communication"
echo "------------------------------------------"

for port in "${ALL_PORTS[@]}"; do
    echo "Testing $port..."
    
    # Test normal communication
    echo -n "  Normal mode: "
    if python -m esptool --chip esp32s3 -p $port chip_id 2>/dev/null | grep -q "Chip is ESP32-S3"; then
        echo "✅ WORKING (ESP32-S3 detected)"
        echo "  🎉 SUCCESS: Auto-reset is working on $port"
        echo "  💡 Use: idf.py -p $port flash"
        exit 0
    else
        boot_mode=$(python -m esptool --chip esp32s3 -p $port chip_id 2>&1 | grep -o "boot mode detected ([^)]*)" | head -1)
        if [[ "$boot_mode" ]]; then
            echo "⚠️  DETECTED but wrong $boot_mode"
        else
            echo "❌ No response"
        fi
    fi
done

echo ""
echo "🔧 Step 3: Manual Reset Required"
echo "---------------------------------"
echo "None of the interfaces support auto-reset."
echo "You need to manually put ESP32-S3 in download mode."
echo ""
echo "📋 MANUAL RESET PROCEDURE:"
echo "1. Locate BOOT and EN (RESET) buttons on your board"
echo "2. HOLD DOWN the BOOT button"
echo "3. While holding BOOT, press and release EN button" 
echo "4. RELEASE the BOOT button"
echo "5. ESP32-S3 should now be in download mode"
echo ""

for port in "${ALL_PORTS[@]}"; do
    echo "🧪 Testing $port with manual reset:"
    echo "   Put ESP32-S3 in download mode now, then press ENTER..."
    read -p "   Ready? "
    
    echo -n "   Download mode test: "
    output=$(python -m esptool --chip esp32s3 -p $port chip_id 2>&1)
    if echo "$output" | grep -q "Chip is ESP32-S3"; then
        echo "✅ SUCCESS!"
        echo ""
        echo "🎉 FOUND WORKING CONFIGURATION:"
        echo "   Interface: $port"
        echo "   Method: Manual reset required"
        echo ""
        echo "💡 Flash command:"
        echo "   1. Put ESP32-S3 in download mode (repeat button procedure)"
        echo "   2. Run: idf.py -p $port flash"
        echo ""
        echo "📝 Save this configuration in your notes!"
        exit 0
    else
        if echo "$output" | grep -q "Wrong boot mode"; then
            echo "❌ Still in normal mode (manual reset didn't work)"
        else
            echo "❌ Communication failed"
        fi
    fi
    echo ""
done

echo "❌ DIAGNOSIS FAILED"
echo ""
echo "🔧 ADDITIONAL TROUBLESHOOTING:"
echo "1. Check button labels on your board:"
echo "   - BOOT button: might be labeled GPIO0, BOOT, or FLASH"
echo "   - RESET button: might be labeled EN, RESET, or RST"
echo ""
echo "2. Try extended hold times:"
echo "   - Hold BOOT for 5+ seconds before pressing RESET"
echo "   - Keep BOOT held for 2+ seconds after RESET"
echo ""
echo "3. Board identification:"
echo "   - What ESP32-S3 board model do you have?"
echo "   - Does it have one USB port or two?"
echo "   - Are there clear BOOT/RESET button labels?"
echo ""
echo "4. USB cable verification:"
echo "   - Try a different USB cable (must be data-capable)"
echo "   - Try a different USB port on your computer"
echo ""
echo "Report your findings and board model for further assistance." 