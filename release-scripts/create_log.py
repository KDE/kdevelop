#!/usr/bin/env python3

# stolen from releaseme/plasma, which was stolen from release-tools Applications/15.04 branch
# ported to Python 3 and fixed the worst issues + removed Plasma-related bits --Kevin

from __future__ import print_function

import argparse
import os
import subprocess
import sys
import cgi


def createLog(workingDir, fromVersion, toVersion, repositoryName=None, showInterestingChangesOnly=True):
    if not repositoryName:
        # use cwd name as repository name
        repositoryName = os.path.split(workingDir)[1]

    p = subprocess.Popen('git fetch', shell=True, cwd=workingDir,
                         stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if p.wait() != 0:
        raise NameError('git fetch failed')

    p = subprocess.Popen('git rev-parse ' + fromVersion + ' ' + toVersion, shell=True, cwd=workingDir,
                         stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if p.wait() != 0:
        raise NameError("git rev-parse failed -- correct to/from version?")

    p = subprocess.Popen('git log ' + fromVersion + '...' + toVersion, shell=True, cwd=workingDir,
                         stdout=subprocess.PIPE, universal_newlines=True)
    commit = []
    commits = []
    for line in p.stdout:
        if line.startswith("commit"):
            if len(commit) > 1 and not ignoreCommit:
                commits.append(commit)
            commitHash = line[7:].strip()
            ignoreCommit = False
            commit = [commitHash]
        elif line.startswith("Author"):
            pass
        elif line.startswith("Date"):
            pass
        elif line.startswith("Merge"):
            pass
        else:
            line = line.strip()
            if line.startswith("Merge remote-tracking branch"):
                ignoreCommit = True
            elif line.startswith("SVN_SILENT"):
                ignoreCommit = True
            elif line.startswith("GIT_SILENT"):
                ignoreCommit = True
            elif line.startswith("Merge branch"):
                ignoreCommit = True
            elif line.startswith("Update version number for"):
                ignoreCommit = True
            elif line:
                commit.append(line)
    # Add the last commit
    if len(commit) > 1 and not ignoreCommit:
        commits.append(commit)

    commitLogEntries = []
    if len(commits) > 0:
        for commit in commits:
            extra = ""
            changelog = commit[1]

            for line in commit:
                line = cgi.escape(line)
                if line.startswith("BUGS:"):
                    bugNumbers = line[line.find(":") + 1:].strip()
                    for bugNumber in bugNumbers.split(","):
                        if bugNumber.isdigit():
                            if extra:
                                extra += ". "
                            extra += "fixes bug <a href='https://bugs.kde.org/" + bugNumber + "'>#" + bugNumber + "</a>"
                elif line.startswith("BUG:"):
                    bugNumber = line[line.find(":") + 1:].strip()
                    if bugNumber.isdigit():
                        if extra:
                            extra += ". "
                        extra += "fixes bug <a href='https://bugs.kde.org/" + bugNumber + "'>#" + bugNumber + "</a>"
                elif line.startswith("REVIEW:"):
                    if extra:
                        extra += ". "
                    reviewNumber = line[line.find(":") + 1:].strip()
                    extra += "code review <a href='https://git.reviewboard.kde.org/r/" + reviewNumber + "'>#" + reviewNumber + "</a>"
                    # jr addition 2017-02 phab link
                elif line.startswith("Differential Revision:"):
                    if extra:
                        extra += ". "
                    reviewNumber = line[line.find("org/") + 4:].strip()
                    extra += "code review <a href='https://phabricator.kde.org/" + reviewNumber + "'>" + reviewNumber + "</a>"
                elif line.startswith("CCBUG:"):
                    if extra:
                        extra += ". "
                    bugNumber = line[line.find(":") + 1:].strip()
                    extra += "See bug <a href='https://bugs.kde.org/" + bugNumber + "'>#" + bugNumber + "</a>"
                elif line.startswith("FEATURE:"):
                    feature = line[line.find(":") + 1:].strip()
                    if feature.isdigit():
                        if extra:
                            extra += ". "
                        extra += "Implements feature <a href='https://bugs.kde.org/" + feature + "'>#" + feature + "</a>"
                    else:
                        if feature:
                            changelog = feature

                elif line.startswith("CHANGELOG:"):
                    changelog = line[11:]  # remove word "CHANGELOG: "
                elif line.startswith("Merge Plasma"):
                    pass

            if showInterestingChangesOnly and not extra:
                continue

            commitHash = commit[0]
            if not changelog.endswith("."):
                changelog = changelog + "."

            capitalizedChangelog = changelog[0].capitalize() + changelog[1:]
            commitLogEntries.append("<li>" + capitalizedChangelog + " (<a href='https://commits.kde.org/" + repositoryName + "/" + commitHash + "'>commit.</a> " + extra + ")</li>")

    # Print result to stdout
    print("<h3><a name='" + repositoryName + "' href='https://commits.kde.org/" + repositoryName + "'>" + repositoryName + "</a></h3>")
    if len(commitLogEntries) > 0:
        print("<ul id='ul" + repositoryName + "' style='display: block'>")
        for commitLogEntry in commitLogEntries:
            print(commitLogEntry)
        print("</ul>\n\n")
    else:
        print("<em>No changes</em>")


    if p.wait() != 0:
        raise NameError('git log failed', repositoryName, fromVersion, toVersion)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Create HTML log based on Git history in the current working directory')
    parser.add_argument('--repositoryName', type=str, help='The path to the Git repositoryNamesitory (default: name of current working dir', default=None)
    parser.add_argument('from_version', type=str, help='The start of the revision range (e.g. "v5.0.0")')
    parser.add_argument('to_version', type=str, help='The end of the revision range (e.g. "v5.0.1"')
    args = parser.parse_args()

    createLog(os.getcwd(), args.repositoryName, args.from_version, args.to_version)
