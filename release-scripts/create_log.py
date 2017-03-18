#!/usr/bin/env python3

# stolen from releaseme/plasma, which was stolen from release-tools Applications/15.04 branch
# ported to Python 3 and fixed the worst issues + removed Plasma-related bits --Kevin

from __future__ import print_function

import configparser
import os
import subprocess
import sys
import cgi

THIS_DIR = os.path.dirname(os.path.realpath(__file__))

f = open(os.path.join(THIS_DIR, 'REPOSITORIES.inc'))
srcdir = os.getcwd()
repos = f.read().rstrip().split(" ")

for repo in repos:
    config = configparser.ConfigParser()
    config.read(os.path.join(THIS_DIR, "VERSIONS.inc"))
    fromVersion = config['default']['OLD_SHA1']
    toVersion = config['default']['NEW_SHA1']

    os.chdir(os.path.join(srcdir, repo))

    p = subprocess.Popen('git fetch', shell=True,
                         stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if p.wait() != 0:
        raise NameError('git fetch failed')

    p = subprocess.Popen('git rev-parse ' + fromVersion + ' ' + toVersion, shell=True,
                         stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if p.wait() != 0:
        raise NameError("git rev-parse failed -- correct to/from version?")

    p = subprocess.Popen('git log ' + fromVersion + '...' + toVersion, shell=True,
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

    if len(commits):
        print("<h3><a name='" + repo + "' href='https://commits.kde.org/"+repo+"'>" + repo + "</a></h3>")
        print("<ul id='ul" + repo + "' style='display: block'>")
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
                            extra += "Fixes bug <a href='https://bugs.kde.org/" + bugNumber + "'>#" + bugNumber + "</a>"
                elif line.startswith("BUG:"):
                    bugNumber = line[line.find(":") + 1:].strip()
                    if bugNumber.isdigit():
                        if extra:
                            extra += ". "
                        extra += "Fixes bug <a href='https://bugs.kde.org/" + bugNumber + "'>#" + bugNumber + "</a>"
                elif line.startswith("REVIEW:"):
                    if extra:
                        extra += ". "
                    reviewNumber = line[line.find(":") + 1:].strip()
                    extra += "Code review <a href='https://git.reviewboard.kde.org/r/" + reviewNumber + "'>#" + reviewNumber + "</a>"
                    # jr addition 2017-02 phab link
                elif line.startswith("Differential Revision:"):
                    if extra:
                        extra += ". "
                    reviewNumber = line[line.find("org/") + 4:].strip()
                    extra += "Phabricator Code review <a href='https://phabricator.kde.org/" + reviewNumber + "'>" + reviewNumber + "</a>"
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
                    changelog = line[11:] # remove word "CHANGELOG: "
                elif line.startswith("Merge Plasma"):
                    pass

            commitHash = commit[0]
            if not changelog.endswith("."):
                changelog = changelog + "."

            # NOTE: Only showing interesting changes
            if extra:
                capitalizedChangelog = changelog[0].capitalize() + changelog[1:]
                print("<li>" + capitalizedChangelog + " <a href='https://commits.kde.org/"+repo+"/"+commitHash+"'>Commit.</a> " + extra + "</li>")
        print("</ul>\n\n")

    if p.wait() != 0:
        raise NameError('git log failed', repo, fromVersion, toVersion)
