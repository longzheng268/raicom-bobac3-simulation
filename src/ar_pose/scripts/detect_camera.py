#!/usr/bin/env python
# -*- coding: utf-8 -*-
import glob
import os
import sys

def detect_camera():
    """Auto-detect available video devices, return first capture device"""
    devices = sorted(glob.glob('/dev/video*'))
    for dev in devices:
        basename = os.path.basename(dev)
        if basename.startswith('video') and basename[5:].isdigit():
            dev_id = int(basename[5:])
            if dev_id % 2 == 0:  # Even = capture, odd = metadata
                return dev
    # Fallback
    for dev in devices:
        basename = os.path.basename(dev)
        if basename.startswith('video') and basename[5:].isdigit():
            return dev
    return '/dev/video0'

if __name__ == '__main__':
    print(detect_camera())
