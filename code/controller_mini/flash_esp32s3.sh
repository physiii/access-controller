#!/bin/bash

# Function to auto-detect ESP32 port
detect_port() {
    local ports=($(ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null))
    
    if [ ${#ports[@]} -eq 0 ]; then
        echo "❌ No USB serial ports found" >&2
        echo "💡 Make sure your ESP32-S3 is connected via USB" >&2
        exit 1
    elif [ ${#ports[@]} -eq 1 ]; then
        echo "🔍 Auto-detected port: ${ports[0]}" >&2
        echo "${ports[0]}"
    else
        echo "🔍 Multiple ports found:" >&2
        for i in "${!ports[@]}"; do
            echo "   [$((i+1))] ${ports[$i]}" >&2
        done
        echo "" >&2
        read -p "Select port (1-${#ports[@]}): " port_choice >&2
        if [[ "$port_choice" =~ ^[0-9]+$ ]] && [ "$port_choice" -ge 1 ] && [ "$port_choice" -le ${#ports[@]} ]; then
            echo "${ports[$((port_choice-1))]}"
        else
            echo "❌ Invalid selection" >&2
            exit 1
        fi
    fi
}

# Auto-detect port if not provided
if [ -z "$1" ]; then
    PORT=$(detect_port)
else
    PORT="$1"
fi

echo "============================================="
echo "ESP32-S3 Development Script - ESP-IDF 6.0"
echo "============================================="
echo "🔌 Port: $PORT"
echo ""
echo "🚨 ESP-IDF 6.0 REQUIRES MANUAL RESET FOR ESP32-S3"
echo "💡 Tip: Press ENTER to use defaults for quick flashing"
echo ""
echo "📋 STEP 1: Choose Action"
echo "   [1] Build only"
echo "   [2] Build and Flash"
echo "   [3] Flash only (requires build first)"
echo "   [4] Monitor only"
echo "   [5] Flash and Monitor"
echo "   [6] Full cycle (Build → Flash → Monitor) [DEFAULT]"
echo "   [7] Show manual flash commands"
echo ""
read -p "Enter choice (1-7) [6]: " action
action=${action:-6}

case $action in
    1)
        echo ""
        echo "🔨 Building project..."
        idf.py build
        echo ""
        echo "✅ Build complete!"
        echo "💡 Next: Use option 3 to flash, or option 2 to build+flash"
        exit 0
        ;;
    4)
        echo ""
        echo "📊 Starting monitor on $PORT..."
        echo "💡 Press Ctrl+] to exit monitor"
        idf.py -p $PORT monitor
        exit 0
        ;;
    7)
        echo ""
        echo "📋 Manual Flash Commands:"
        echo ""
        echo "Method 1 - idf.py:"
        echo "idf.py -p $PORT flash"
        echo ""
        echo "Method 2 - Direct esptool:"
        echo "python -m esptool --chip esp32s3 -p $PORT -b 115200 \\"
        echo "    --before no_reset_no_sync --after hard_reset \\"
        echo "    write_flash --flash_mode dio --flash_freq 80m --flash_size 16MB \\"
        echo "    0x0 build/bootloader/bootloader.bin \\"
        echo "    0x10000 build/controller.bin \\"
        echo "    0x8000 build/partition_table/partition-table.bin \\"
        echo "    0xe000 build/ota_data_initial.bin"
        echo ""
        echo "⚠️  Put ESP32-S3 in download mode first (BOOT+RESET sequence)"
        exit 0
        ;;
esac

# Build step for options that need it
if [[ "$action" =~ ^[256]$ ]]; then
    echo ""
    echo "🔨 Building project..."
    if ! idf.py build; then
        echo "❌ Build failed!"
        exit 1
    fi
    echo "✅ Build complete!"
fi

# Flash step for options that need it
if [[ "$action" =~ ^[2356]$ ]]; then
    echo ""
    echo "📋 FLASH PREPARATION: ESP32-S3 Download Mode"
    echo "   1. Locate BOOT and RESET buttons on your board"
    echo "   2. HOLD DOWN the BOOT button (keep holding!)"
    echo "   3. While holding BOOT, press and release RESET"
    echo "   4. RELEASE the BOOT button"
    echo "   5. Board LED should change (indicates download mode)"
    echo ""
    echo "⚡ Choose Flash Method:"
    echo "   [1] Use idf.py (recommended) [DEFAULT]"
    echo "   [2] Use direct esptool with custom timing"
    echo ""
    read -p "Enter flash method (1-2) [1]: " flash_choice
    flash_choice=${flash_choice:-1}

    case $flash_choice in
        1)
            echo ""
            echo "🔥 Starting idf.py flash..."
            echo "Press ENTER after putting ESP32-S3 in download mode:"
            read -p ""
            if ! idf.py -p $PORT flash; then
                echo "❌ Flash failed!"
                exit 1
            fi
            ;;
        2)
            echo ""
            echo "🔥 Starting direct esptool flash..."
            echo "Press ENTER after putting ESP32-S3 in download mode:"
            read -p ""
            if ! python -m esptool --chip esp32s3 -p $PORT -b 115200 \
                --before no_reset_no_sync --after hard_reset \
                write_flash --flash_mode dio --flash_freq 80m --flash_size 16MB \
                0x0 build/bootloader/bootloader.bin \
                0x10000 build/controller.bin \
                0x8000 build/partition_table/partition-table.bin \
                0xe000 build/ota_data_initial.bin; then
                echo "❌ Flash failed!"
                exit 1
            fi
            ;;
        *)
            echo "❌ Invalid flash method choice"
            exit 1
            ;;
    esac
    
    echo ""
    echo "✅ Flash complete!"
    echo "The device should reset automatically."
fi

# Monitor step for options that need it
if [[ "$action" =~ ^[56]$ ]]; then
    echo ""
    echo "📊 Starting monitor..."
    echo "💡 Press Ctrl+] to exit monitor"
    echo "⚠️  If device doesn't appear immediately, press RESET button"
    echo ""
    echo "Starting monitor in 3 seconds..."
    sleep 3
    idf.py -p $PORT monitor
fi

echo ""
echo "============================================="
echo "🎉 Operation complete!"
case $action in
    1) echo "📁 Project built successfully" ;;
    2) echo "🔥 Project built and flashed" ;;
    3) echo "🔥 Project flashed" ;;
    4) echo "📊 Monitor session ended" ;;
    5) echo "🔥📊 Project flashed and monitored" ;;
    6) echo "🔨🔥📊 Full development cycle complete" ;;
esac
echo ""
echo "💡 Quick commands:"
echo "   Build:   idf.py build"
echo "   Flash:   idf.py -p $PORT flash"
echo "   Monitor: idf.py -p $PORT monitor"
echo "   Full:    ./flash_esp32s3.sh (press ENTER for defaults)"
echo "=============================================" 