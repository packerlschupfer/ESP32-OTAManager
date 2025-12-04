#!/usr/bin/env python3
"""
Post-build script for PlatformIO
Generates build artifacts and metadata
"""

import os
import json
import hashlib
import datetime
import shutil

Import("env")

def calculate_checksum(filepath):
    """Calculate SHA256 checksum of a file"""
    sha256_hash = hashlib.sha256()
    with open(filepath, "rb") as f:
        for byte_block in iter(lambda: f.read(4096), b""):
            sha256_hash.update(byte_block)
    return sha256_hash.hexdigest()

def get_firmware_size(filepath):
    """Get firmware file size"""
    return os.path.getsize(filepath)

def create_manifest(firmware_path):
    """Create firmware manifest file"""
    manifest = {
        "version": env.get("CPPDEFINES", {}).get("APP_VERSION", "1.0.0"),
        "build": {
            "timestamp": int(os.environ.get("BUILD_TIMESTAMP", 0)),
            "date": datetime.datetime.now().isoformat(),
            "git_commit": os.environ.get("GIT_COMMIT", "unknown"),
            "environment": env.get("PIOENV", "unknown"),
            "platform": env.get("PIOPLATFORM", "unknown"),
            "board": env.get("BOARD", "unknown")
        },
        "firmware": {
            "filename": os.path.basename(firmware_path),
            "size": get_firmware_size(firmware_path),
            "checksum": {
                "type": "sha256",
                "value": calculate_checksum(firmware_path)
            }
        },
        "ota": {
            "hostname": env.get("CPPDEFINES", {}).get("OTA_HOSTNAME", "esp32-ota"),
            "port": int(env.get("CPPDEFINES", {}).get("OTA_PORT", 3232)),
            "password_required": True
        }
    }
    
    # Save manifest
    manifest_path = firmware_path.replace(".bin", ".json")
    with open(manifest_path, "w") as f:
        json.dump(manifest, f, indent=2)
    
    return manifest_path

def create_update_package(firmware_path):
    """Create update package with firmware and metadata"""
    # Create package directory
    package_dir = os.path.join(
        env.get("PROJECT_DIR"),
        "releases",
        env.get("PIOENV"),
        datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    )
    os.makedirs(package_dir, exist_ok=True)
    
    # Copy firmware
    firmware_dest = os.path.join(package_dir, os.path.basename(firmware_path))
    shutil.copy2(firmware_path, firmware_dest)
    
    # Create and copy manifest
    manifest_path = create_manifest(firmware_path)
    manifest_dest = os.path.join(package_dir, os.path.basename(manifest_path))
    shutil.move(manifest_path, manifest_dest)
    
    # Create README for the package
    readme_content = f"""# Firmware Update Package

## Version Information
- Environment: {env.get("PIOENV")}
- Build Date: {datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")}
- Git Commit: {os.environ.get("GIT_COMMIT", "unknown")}

## Files
- `{os.path.basename(firmware_path)}` - Firmware binary
- `{os.path.basename(manifest_path)}` - Build manifest with checksums

## Installation

### Via OTA Upload:
```bash
pio run -e {env.get("PIOENV")} --target uploadota
```

### Via esptool:
```bash
esptool.py write_flash 0x10000 {os.path.basename(firmware_path)}
```

## Verification
SHA256: {calculate_checksum(firmware_path)}
"""
    
    readme_path = os.path.join(package_dir, "README.md")
    with open(readme_path, "w") as f:
        f.write(readme_content)
    
    return package_dir

def generate_metrics():
    """Generate build metrics"""
    metrics = {
        "build_time": datetime.datetime.now().isoformat(),
        "memory_usage": {
            "ram": env.get("RAM_SIZE", 0),
            "flash": env.get("FLASH_SIZE", 0)
        },
        "dependencies": []
    }
    
    # Get library dependencies
    lib_deps = env.get("LIB_DEPS", [])
    for dep in lib_deps:
        metrics["dependencies"].append(str(dep))
    
    # Save metrics
    metrics_path = os.path.join(
        env.get("PROJECT_DIR"),
        "build_metrics.json"
    )
    with open(metrics_path, "w") as f:
        json.dump(metrics, f, indent=2)

def post_build_actions(source, target, env):
    """Main post-build actions"""
    print("=" * 50)
    print("OTAManager Post-Build Script")
    print("=" * 50)
    
    firmware_path = str(target[0])
    print(f"Firmware: {firmware_path}")
    print(f"Size: {get_firmware_size(firmware_path)} bytes")
    
    # Create manifest
    manifest_path = create_manifest(firmware_path)
    print(f"Manifest: {manifest_path}")
    
    # Create update package for production builds
    if "production" in env.get("PIOENV", ""):
        package_dir = create_update_package(firmware_path)
        print(f"Update package: {package_dir}")
    
    # Generate metrics
    generate_metrics()
    
    print("=" * 50)
    print("Post-build actions completed!")
    print("=" * 50)

# Register post-build action
env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", post_build_actions)