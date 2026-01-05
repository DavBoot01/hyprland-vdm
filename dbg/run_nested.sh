#!/bin/bash
#
# Script Name: run_nested.sh
# Description: Launches a nested Hyprland instance for plugin testing.
#
# Author:      Davide Cardillo <cardillo.davide85@gmail.com>
# Owner:       No Company
# Created:     2025-12-28
# Version:     1.0.0
# License:     MIT
#
# Usage:       source ./run_nested.sh
#
# ==============================================================================
# REVISION HISTORY:
# Date        Author            Description
# 2025-12-28  Your Name         Initial creation and sourcing check
# 2025-12-29  Your Name         Useful script for multi-monitor nested Hyprland instances
# ==============================================================================

function close_current_instance {
    if [ -z "$HYPR_NESTED_PID" ]; then
        echo "No existing nested Hyprland instance detected."
    else
        echo "Closing existing nested Hyprland instance with PID: $HYPR_NESTED_PID"
        kill $HYPR_NESTED_PID
        unset HYPR_NESTED_PID
        unset HYPRLAND_INSTANCE_SIGNATURE
        sleep 1
    fi
}

function close_all_instances {
    echo "Closing all existing nested Hyprland instances..."
    pgrep Hyprland | grep -v $(pgrep -u $USER -x Hyprland | head -n 1) | xargs kill
}

function list_existing_instances {
    echo "Listing all existing nested Hyprland instances..."
    pgrep Hyprland | grep -v $(pgrep -u $USER -x Hyprland | head -n 1)
}

function help_message {
    echo "Usage: source ./run_nested.sh [-m <number_of_monitors>] [-d] [-D] [-l]"
    echo "  -m : number of monitors (default: 1)"
    echo "  -d : close existing nested instance"
    echo "  -D : close all existing nested instances"
    echo "  -l : list all existing nested instances"
}


# >> Detect script file (works in both bash and zsh)
if [ -n "$BASH_SOURCE" ]; then
    SCRIPT_FILE="${BASH_SOURCE[0]}"
elif [ -n "$ZSH_VERSION" ]; then
    SCRIPT_FILE="${(%):-%x}"
else
    SCRIPT_FILE="$0"
fi

# >> Check if the script is being sourced
if [[ "$SCRIPT_FILE" == "$0" ]] && [ -z "$ZSH_VERSION" ]; then
    echo "ERROR: This script must be sourced to export variables to your current shell."
    echo "Usage: source ./run_nested.sh"
    exit 1
fi

# >> Detect the script's directory
# This works even if you source it from a different folder
export SCRIPT_DIR=$(dirname "$(realpath "$SCRIPT_FILE")")
export SCRIPT_PATH=$(realpath "$SCRIPT_FILE")

# Note WLR_WL_OUTPUTS=${MONITORS} doesn't work for Hyprland >= 0.52
MONITORS=1

# >> Parse options manually (more reliable with source)
DELETE_INSTANCE=false
DELETE_ALL_INSTANCES=false
LIST_INSTANCES=false
SHOW_HELP=false

while [[ $# -gt 0 ]]; do
    case "$1" in
        -m)
            shift
            MONITORS="$1"
            shift
            ;;
        -d)
            DELETE_INSTANCE=true
            shift
            ;;
        -D)
            DELETE_ALL_INSTANCES=true
            shift
            ;;
        -l)
            LIST_INSTANCES=true
            shift
            ;;
        -h)
            SHOW_HELP=true
            shift
            ;;
        *)
            echo "Invalid option: $1"
            help_message
            return 1
            ;;
    esac
done

if [ "$DELETE_INSTANCE" = true ]; then
    close_current_instance
    return 0
fi
if [ "$DELETE_ALL_INSTANCES" = true ]; then
    close_all_instances
    return 0
fi
if [ "$LIST_INSTANCES" = true ]; then
    list_existing_instances
    return 0
fi
if [ "$SHOW_HELP" = true ]; then
    help_message
    return 0
fi

# >> Clear previous signatures to avoid confusion
PREVIOUS_SESSIONS=$(ls -1 -t $XDG_RUNTIME_DIR/hypr/ 2>/dev/null)

# >> Launch Hyprland in the background (nested mode)
echo "Launching nested Hyprland instance..."
# We use a temporary log file for the nested session
 start-hyprland -- --config ${SCRIPT_DIR}/hyprland_test.conf > ${SCRIPT_DIR}/hyprland_nested.log 2>&1 &
HYPR_PID=$!

# >> Wait for the new socket to be created
sleep 1.5

# >> Identify the NEW instance signature
ALL_SESSIONS=$(ls -1 -t $XDG_RUNTIME_DIR/hypr/ 2>/dev/null)
NEW_SIGNATURE=""

echo ${ALL_SESSIONS} | while read -r session; do
    if ! echo "$PREVIOUS_SESSIONS" | grep -q "$session"; then
        NEW_SIGNATURE=$session
        break
    fi
done

if [ -z "$NEW_SIGNATURE" ]; then
    echo "Error: Could not identify the new Hyprland instance signature."
    kill $HYPR_PID
    return 1
else
    echo "New Hyprland instance signature: $NEW_SIGNATURE"
fi

# >> Export the variable to the CURRENT shell
export HYPRLAND_INSTANCE_SIGNATURE=$NEW_SIGNATURE
export HYPR_NESTED_PID=$HYPR_PID

# >> Add optional monitors configuration
echo "Configuring monitors... Requested: $MONITORS"
if [ $MONITORS -gt 1 ]; then
    echo "Configuring $MONITORS monitors for the nested instance..."
    for ((i=1; i<MONITORS; i++)); do
        hyprctl output create wayland
    done
fi    


echo "-----------------------------------------------------"
echo "Nested instance started with PID: $HYPR_PID"
echo "INSTANCE_SIGNATURE exported: $HYPRLAND_INSTANCE_SIGNATURE"
echo "-----------------------------------------------------"
echo "Now you can run 'hyprctl' commands directly."
echo "To stop the nested instance, run: kill \$HYPR_NESTED_PID"
echo "To check logs: tail -f \$XDG_RUNTIME_DIR/hypr/\$HYPRLAND_INSTANCE_SIGNATURE/hyprland.log"
echo "-----------------------------------------------------"
