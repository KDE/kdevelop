#!/usr/bin/env python3

import argparse
import os
import subprocess

# local imports
import create_log

DIR_PATH = os.path.dirname(os.path.realpath(__file__))

def run(fromVersion, toVersion):
    repositories = []
    with open(os.path.join(DIR_PATH, "REPOSITORIES.inc"), 'r') as f:
        repositories = f.read().split()

    for repository in repositories:
        workingDir = os.path.join("..", repository)
        create_log.createLog(workingDir, fromVersion, toVersion)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Create HTML log based on Git history for released KDevelop projects')
    parser.add_argument('from_version', type=str, help='The start of the revision range (e.g. "v5.0.0")')
    parser.add_argument('to_version', type=str, help='The end of the revision range (e.g. "v5.0.1"')
    args = parser.parse_args()

    run(args.from_version, args.to_version)
