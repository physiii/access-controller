# ESP32-S3 Access Controller - Quick Usage Guide

## 🚀 Enhanced Development Workflow

### One-Command Development
```bash
# Run the enhanced script - it auto-detects everything!
./flash_esp32s3.sh
```

### Available Options:
1. **Build only** - Compile project without flashing
2. **Build and Flash** - Full build → flash cycle  
3. **Flash only** - Flash pre-built binary (requires build first)
4. **Monitor only** - View serial output from device
5. **Flash and Monitor** - Flash then immediately monitor
6. **Full cycle** - Build → Flash → Monitor (complete development cycle)
7. **Show manual commands** - Display raw commands for manual use

### Port Detection
- **Auto-detects** USB ports (`/dev/ttyUSB*`, `/dev/ttyACM*`)
- **Single device**: Automatically selects the port
- **Multiple devices**: Prompts you to choose
- **Manual override**: `./flash_esp32s3.sh /dev/ttyUSB1`

### ESP32-S3 Manual Reset (Required for ESP-IDF 6.0)
When flashing, you'll need to manually put ESP32-S3 in download mode:

1. **HOLD** the BOOT button (keep holding)
2. **PRESS and RELEASE** the RESET button  
3. **RELEASE** the BOOT button
4. Board LED should change (indicates download mode)
5. Press ENTER to start flashing

## 🛠️ Manual Commands

### Individual Commands
```bash
# Build
idf.py build

# Flash (after manual reset)
idf.py -p /dev/ttyUSB0 flash

# Monitor
idf.py -p /dev/ttyUSB0 monitor

# All-in-one
idf.py -p /dev/ttyUSB0 flash monitor
```

### Direct esptool (if needed)
```bash
python -m esptool --chip esp32s3 -p /dev/ttyUSB0 -b 115200 \
    --before no_reset_no_sync --after hard_reset \
    write_flash --flash_mode dio --flash_freq 80m --flash_size 16MB \
    0x0 build/bootloader/bootloader.bin \
    0x10000 build/controller.bin \
    0x8000 build/partition_table/partition-table.bin \
    0xe000 build/ota_data_initial.bin
```

## ⚡ Quick Workflow

### Development Cycle
1. Make code changes
2. Run `./flash_esp32s3.sh`
3. Choose option **6** (Full cycle)
4. Follow manual reset instructions when prompted
5. View output in monitor

### Just Testing
1. Run `./flash_esp32s3.sh`  
2. Choose option **4** (Monitor only)
3. See live output from your device

## 🔧 Troubleshooting

### If Script Fails
- Check USB connection
- Try different USB port  
- Verify ESP32-S3 is in download mode
- See `ESP32-S3_FLASH_GUIDE.md` for detailed troubleshooting

### Build Issues
- Run `idf.py build` first to see detailed error messages
- All ESP-IDF 6.0 compatibility issues have been resolved

### Monitor Issues  
- Press device RESET button if no output appears
- Verify correct baud rate (default: 115200)
- Use Ctrl+] to exit monitor

## 📁 Files Overview
- `flash_esp32s3.sh` - Enhanced development script
- `ESP32-S3_FLASH_GUIDE.md` - Detailed troubleshooting guide  
- `USAGE.md` - This quick reference
- `sdkconfig` - ESP-IDF 6.0 compatible configuration 