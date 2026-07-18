#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Auto-detecting camera node.
Scans /dev/video* for available capture devices and starts usb_cam with the first one found.
"""
import rospy
import subprocess
import glob
import os
import signal
import sys

def detect_camera():
    """Return first available video capture device"""
    devices = sorted(glob.glob('/dev/video*'))
    for dev in devices:
        basename = os.path.basename(dev)
        if basename.startswith('video') and basename[5:].isdigit():
            dev_id = int(basename[5:])
            if dev_id % 2 == 0:  # Even = capture, odd = metadata
                return dev
    return '/dev/video0'

def main():
    rospy.init_node('auto_cam_node')
    
    camera_dev = detect_camera()
    rospy.loginfo("Auto-detected camera: %s", camera_dev)
    
    if not os.path.exists(camera_dev):
        rospy.logerr("Camera device %s not found!", camera_dev)
        return
    
    # Get package path for calibration file
    pkg_path = os.path.join(os.path.dirname(__file__), '..')
    calib_url = 'file://' + os.path.join(pkg_path, 'cam_info', 'base_camera.yaml')
    
    # Launch usb_cam_node with detected device
    cmd = [
        'rosrun', 'usb_cam', 'usb_cam_node',
        '_video_device:=' + camera_dev,
        '_image_width:=640',
        '_image_height:=480',
        '_pixel_format:=yuyv',
        '_camera_frame_id:=bottom_camera_optical_link',
        '_io_method:=mmap',
        '_framerate:=60',
        '_camera_name:=base_camera',
        '_camera_info_url:=' + calib_url
    ]
    
    rospy.loginfo("Launching: %s", ' '.join(cmd))
    proc = subprocess.Popen(cmd)
    
    def shutdown(signum, frame):
        proc.terminate()
        proc.wait()
    
    signal.signal(signal.SIGINT, shutdown)
    signal.signal(signal.SIGTERM, shutdown)
    
    proc.wait()

if __name__ == '__main__':
    main()
