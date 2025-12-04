#!/usr/bin/env python3
"""
Pre-build script for PlatformIO
Validates environment and prepares build
"""

import os
import sys
import datetime
import subprocess

Import("env")

def get_git_commit():
    """Get current git commit hash"""
    try:
        commit = subprocess.check_output(
            ["git", "rev-parse", "--short", "HEAD"],
            stderr=subprocess.DEVNULL
        ).decode().strip()
        return commit
    except:
        return "unknown"

def validate_environment():
    """Validate required environment variables"""
    required_vars = []
    
    # Check which environment we're building
    env_name = env.get("PIOENV", "")
    
    if "production" in env_name:
        required_vars = [
            "PROD_WIFI_SSID",
            "PROD_WIFI_PASS", 
            "PROD_OTA_PASS",
            "DEVICE_ID"
        ]
    elif "staging" in env_name:
        required_vars = [
            "DEVICE_ID"
        ]
    
    missing_vars = []
    for var in required_vars:
        if not os.environ.get(var):
            missing_vars.append(var)
    
    if missing_vars:
        print(f"ERROR: Missing required environment variables: {', '.join(missing_vars)}")
        sys.exit(1)

def inject_build_flags():
    """Inject dynamic build flags"""
    # Set build timestamp
    if not os.environ.get("BUILD_TIMESTAMP"):
        os.environ["BUILD_TIMESTAMP"] = str(int(datetime.datetime.now().timestamp()))
    
    # Set git commit
    if not os.environ.get("GIT_COMMIT"):
        os.environ["GIT_COMMIT"] = get_git_commit()
    
    # Add build info
    env.Append(CPPDEFINES=[
        ("BUILD_DATE", f'\\"{datetime.datetime.now().strftime("%Y-%m-%d")}\\"'),
        ("BUILD_TIME", f'\\"{datetime.datetime.now().strftime("%H:%M:%S")}\\"'),
        ("BUILD_HOST", f'\\"{os.uname().nodename}\\"')
    ])

def create_version_header():
    """Create version header file"""
    version_h = """
#ifndef VERSION_H
#define VERSION_H

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 1
#define VERSION_STRING "1.0.1"

#define BUILD_TIMESTAMP {timestamp}
#define GIT_COMMIT "{commit}"
#define BUILD_ENV "{env}"

#endif // VERSION_H
""".format(
        timestamp=os.environ.get("BUILD_TIMESTAMP", "0"),
        commit=os.environ.get("GIT_COMMIT", "unknown"),
        env=env.get("PIOENV", "unknown")
    )
    
    # Write version header
    include_dir = os.path.join(env.get("PROJECT_DIR"), "include")
    if not os.path.exists(include_dir):
        os.makedirs(include_dir)
    
    with open(os.path.join(include_dir, "version.h"), "w") as f:
        f.write(version_h)

def pre_build_actions():
    """Main pre-build actions"""
    print("=" * 50)
    print("OTAManager Pre-Build Script")
    print("=" * 50)
    print(f"Environment: {env.get('PIOENV')}")
    print(f"Platform: {env.get('PIOPLATFORM')}")
    print(f"Board: {env.get('BOARD')}")
    print(f"Build Type: {env.get('BUILD_TYPE', 'release')}")
    print("=" * 50)
    
    # Validate environment
    validate_environment()
    
    # Inject build flags
    inject_build_flags()
    
    # Create version header
    create_version_header()
    
    print("Pre-build checks completed successfully!")
    print("=" * 50)

# Run pre-build actions
pre_build_actions()