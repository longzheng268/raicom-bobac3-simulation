# AGENTS.md — raicom-bobac3-simulation

## Project

ROS 1 catkin workspace (Melodic/Noetic) for BOBAC3 service robot: autonomous navigation, voice interaction (iFlytek AIUI), face recognition, AR marker tracking, auto-charging. Developed by Lanzhou University of Technology Intelligent Vehicle Association (2023). GPL-3.0 licensed.

## Build

```bash
catkin_make -j$(nproc)
source devel/setup.bash
```

Requires: `ros-${ROS_DISTRO}-{amcl,map-server,move-base,gmapping,cartographer-ros,gazebo-ros,usb-cam,ydlidar,robot-state-publisher,joint-state-publisher}`.

## Simulation (Gazebo)

Copy Gazebo models before first use:
```bash
mkdir -p ~/.gazebo/models
cp -r src/reinovo_raicom_map src/small_marker_charge_pile src/obs_block ~/.gazebo/models/
cp -r src/bobac3_description/world/* ~/.gazebo/models/
```

Launch simulation:
```bash
roslaunch bobac3_navigation demo_nav_2d.launch [map_file_name:=demo]
```

Available maps: `demo`, `demo1`, `map`, `0123`, `aa`, `test`, `test1`, `reicom`, `rtab_map`.

## Key Launch Files

| Feature | Simulation | Real robot |
|---------|-----------|------------|
| Navigation | `bobac3_navigation demo_nav_2d.launch` | `bobac3_navigation bobac3_nav_2d.launch` |
| Voice interaction | `bobac3_audio nav.launch` | same |
| Face recognition | `face_rec face_rec_service.launch` | same |
| AR tracking | `ar_pose ar_base.launch` | same |
| Auto-charging | `auto_charging auto_charging.launch` | same |

Each terminal must `source devel/setup.bash`. Multiple terminals needed for concurrent nodes.

## Package Structure

| Package | Role |
|---------|------|
| `bobac3_navigation` | Nav stack: maps, AMCL/Cartographer, move_base+TEB, RViz configs |
| `robot_audio` | Voice: VAD, wake word ("yuanbao"), ASR/NLU/TTS via iFlytek AIUI |
| `bobac3_audio` | High-level C++ voice clients |
| `face_rec` | Face recognition (Python, dlib/face_recognition). Database in `face_data/` (gitignored) |
| `face_rec_ex` | Face recognition + voice integration demo |
| `ar_pose` | AR marker pose adjustment state machine |
| `ar_track_alvar` | Vendored Alvar AR detection (v0.7.1) |
| `relative_move` | PID-controlled relative displacement service |
| `pid_lib` | Header-only C++ PID controller |
| `auto_charging` | AR-guided charging dock approach |
| `secondary_localization` | AR marker fine localization |
| `bobac3_description` | Robot URDF + Gazebo world files |
| `bobac3_msgs` | Custom ROS message definitions |

## Gotchas

- **Python scripts lose execute bit on Windows→Linux sync.** Fix: `chmod +x src/face_rec/scripts/*.py` then `catkin_make`.
- **Gazebo SDF version**: must be 1.6, not 1.7 (Gazebo 9 / Melodic). Don't upgrade SDF models.
- **Real robot** needs `rei_robot_base` and `rei_ydlidar_nodelet` from Reinovo. Use `demo_nav_2d.launch` for simulation-only.
- **iFlytek SDK**: requires registration + AppID; place SDK files in `src/robot_audio/AIUI/`.
- **Face data is gitignored**: `face_rec/face_data/` not in repo. Add folders named per-person with photos.

## Code Conventions

- C++ follows ROS conventions; Python follows PEP 8.
- Commit must pass `catkin_make`. No face photos/recordings in commits.
