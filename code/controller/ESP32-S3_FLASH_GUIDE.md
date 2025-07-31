# ESP32-S3 Flash Issues with ESP-IDF 6.0 - Complete Guide

## The Problem
ESP-IDF 6.0 introduced changes that break auto-reset functionality on many ESP32-S3 boards. This means the automatic bootloader mode detection no longer works reliably.

**Error you see**: `Wrong boot mode detected (0x2a)! The chip needs to be in download mode.`

## Root Cause
- ESP-IDF 6.0 changed reset timing and sequences
- ESP32-S3 auto-reset circuits are more sensitive than ESP32 classic
- Some boards lack proper auto-reset circuitry entirely

## 🛠️ SOLUTIONS (Try in Order)

### Solution 1: Manual Reset Method (Most Reliable)

1. **Put ESP32-S3 in Download Mode MANUALLY:**
   ```bash
   # On your ESP32-S3 board:
   # 1. HOLD DOWN the BOOT button (and keep holding)
   # 2. While holding BOOT, press and release the RESET button  
   # 3. RELEASE the BOOT button
   # 4. Board is now in download mode (usually LED changes or goes off)
   ```

2. **Flash with no_reset (already configured):**
   ```bash
   idf.py -p /dev/ttyUSB0 flash
   ```

### Solution 2: Direct esptool with Custom Timing
```bash
python -m esptool --chip esp32s3 -p /dev/ttyUSB0 -b 115200 \
    --before no_reset_no_sync --after hard_reset \
    --connect-attempts 1 \
    write_flash --flash_mode dio --flash_freq 80m --flash_size 16MB \
    0x0 build/bootloader/bootloader.bin \
    0x10000 build/controller.bin \
    0x8000 build/partition_table/partition-table.bin \
    0xe000 build/ota_data_initial.bin
```

### Solution 3: Enhanced Development Script (Recommended)
```bash
# Auto-detect port and show all options
./flash_esp32s3.sh

# Or specify port manually
./flash_esp32s3.sh /dev/ttyUSB0
```

**Features:**
- 🔍 **Auto-detects** USB ports (supports multiple devices)
- 🔨 **Build** option - compile project only
- 🔥 **Flash** options - multiple flash methods with manual reset guidance
- 📊 **Monitor** option - view serial output
- 🔄 **Full cycle** - build → flash → monitor in one command
- ⚡ **Smart workflow** - handles build failures and provides clear feedback

### Solution 4: Alternative Reset Configuration

If manual reset doesn't work, try changing back to default_reset with slower baud:

```bash
# Edit sdkconfig:
CONFIG_ESPTOOLPY_BEFORE="default_reset"
CONFIG_ESPTOOLPY_BEFORE_RESET=y
CONFIG_ESPTOOLPY_BAUD_115200B=y
CONFIG_ESPTOOLPY_BAUD="115200"

# Then rebuild and flash:
idf.py build
idf.py -p /dev/ttyUSB0 flash
```

## 🔍 Troubleshooting

### If Manual Reset Still Fails:
1. **Check connections**: Ensure USB cable is data-capable (not charge-only)
2. **Try different USB port**: Some ports have power/timing differences  
3. **Check board variant**: Some ESP32-S3 boards have different BOOT/RESET button layouts
4. **Verify download mode**: LED should change state when in download mode

### Board-Specific Notes:
- **ESP32-S3-DevKitC**: Hold BOOT (IO0), press RESET (EN), release BOOT
- **ESP32-S3-WROOM**: Some variants need longer BOOT hold time (3+ seconds)
- **Custom boards**: May not have auto-reset circuitry at all

### Success Indicators:
```
esptool.py v4.9.0
Serial port /dev/ttyUSB0
Connecting....
Chip is ESP32-S3 (revision v0.2)
```

### Still Having Issues?
1. Use a different ESP32-S3 board to verify
2. Try ESP-IDF 5.3 for comparison  
3. Check board documentation for specific reset procedure
4. Consider using JTAG instead of UART for programming

## 📋 Current Configuration
The controller has been configured with:
- `CONFIG_ESPTOOLPY_BEFORE="no_reset"` - Requires manual reset
- `CONFIG_HTTPD_WS_SUPPORT=y` - WebSocket support enabled
- `CONFIG_PARTITION_TABLE_CUSTOM=y` - Using custom 2M partition table
- `CONFIG_ESPTOOLPY_FLASHSIZE="16MB"` - 16MB flash size

This configuration builds successfully and is ready for manual flash. 