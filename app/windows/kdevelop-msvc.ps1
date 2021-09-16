<###########################################################################
 #   Copyright 2021 Simon Redman <simon@ergotech.com>                      #
 #                                                                         #
 #   This program is free software; you can redistribute it and/or modify  #
 #   it under the terms of the GNU General Public License as published by  #
 #   the Free Software Foundation; either version 2 of the License, or     #
 #   (at your option) any later version.                                   #
 #                                                                         #
 #   This program is distributed in the hope that it will be useful,       #
 #   but WITHOUT ANY WARRANTY; without even the implied warranty of        #
 #   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
 #   GNU General Public License for more details.                          #
 #                                                                         #
 #   You should have received a copy of the GNU General Public License     #
 #   along with this program; if not, write to the                         #
 #   Free Software Foundation, Inc.,                                       #
 #   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         #
 ###########################################################################>

 <#
.SYNOPSIS
    Assists in starting KDevelop with MSVC (Microsoft Visual C++ Compiler)

.DESCRIPTION
    Assists in starting KDevelop with MSVC (Microsoft Visual C++ Compiler)

    This uses vswhere.exe to find all installations of Visual Studio, downloading
    it if necessary. vswhere is provided by Microsoft for this purpose under the
    MIT license. More information can be found here:
    https://www.github.com/microsoft/vswhere

.PARAMETER VsWherePath
    Custom path to vswhere.exe

.PARAMETER SelectedVersion
    Pass the version of MSVC to use. Note that this is the version number, not the release year.
    For instance, to pick Visual Studio 2019, pass "16".

.PARAMETER ToolchainArchitecture
    Which toolchain to initialize. The valid values are determined by what you have installed and
    what your version of vsvarsall.bat supports.
    Common values are:
    "x86", which initializes an x86 host toolchain which builds for x86
    "x64", which initializes an amd_64 host toolchain which builds for amd_64
    "x86_amd64", which initializes an x86 host toolchain which builds for amd_64
    "amd64_arm64", which initializes an amd_64 host toolchain which builds for arm_64
    The default value is "x86", which is the default if you were to open a Visual Studio developer
    command prompt.

.INPUTS
    None

.OUTPUTS
    None

.NOTES
    Version:        1.0

.EXAMPLE
    kdevelop-msvc.ps1
    Starts KDevelop using the highest installed version of Visual Studio and the default toolchain

.EXAMPLE
    kdevelop-msvc.ps1 -SelectedVersion 15
    Starts KDevelop using version 15 (Visual Studio 2017) and the default toolchain

.EXAMPLE
    kdevelop-msvc.ps1 -ToolchainArchitecture amd64_x86
    Starts KDevelop using the highest installed version of Visual Studio and a toolchain which runs on
    amd64 and builds for x86.

.EXAMPLE
    kdevelop-msvc.ps1 -VsWherePath "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    Starts KDevelop using the version of vswhere included with the Visual Studio (2017+) installer.
#>

param(
    [Parameter(
        Mandatory = $false,
        HelpMessage = "Custom path to vswhere.exe."
    )]
    $VsWherePath,
    [Parameter(
        Mandatory = $false,
        HelpMessage = "Select this version of MSVC. Will otherwise auto-pick the latest version."
    )]
    [int]$SelectedVersion,
    [Parameter(
        Mandatory = $false,
        HelpMessage = "Toolchain to use. One of the values supported by vsvarsall, like 'x86', 'x64', or 'amd64_x86'."
    )]
    [string]$ToolchainArchitecture = "x86" # Picking the same default as we would get if we ran vcvarsall without specifying an argument
)

$ErrorActionPreference = "Stop"
$sScriptVersion = "1.0"

# Check if the user provided a particular path
if ($VsWherePath -eq $null) {
    # Use the directory from which this script is being run
    if ($PSScriptRoot -eq $null) {
        Write-Error "This script requires Powershell version 3+. Please try passing -VsWherePath X:\path\to\vswhere.exe to bypass this requirement."
    }

    $VsWherePath = $(Join-Path $PSScriptRoot "vswhere.exe")
}
else {
    # If the user provided a path but vswhere.exe isn't there, die
    $customFileExists = Test-Path $VsWherePath -PathType Leaf

    if (!$customFileExists) {
        Write-Error "VsWherePath provided but path does not exist."
    }
}

# Download vswhere.exe if not found
$vsWhereExists = Test-Path $VsWherePath -PathType Leaf

if (!$vsWhereExists) {
    Invoke-WebRequest -Method GET -Uri "https://github.com/microsoft/vswhere/releases/latest/download/vswhere.exe" -OutFile $VsWherePath
}

$visualStudioVersions = Invoke-Expression "$VsWherePath -format json -legacy" | ConvertFrom-Json

[array]$validVSVersions = $visualStudioVersions |
    Select-Object -Property *, @{ Name = "majorVersion"; Expression = { [int]$_.installationVersion.Split(".")[0] }} |
    Select-Object -Property *, @{ Name = "humanReadableName"; Expression = {
        if ($_.majorVersion -ge 15) {
            # Version 15 (VS 2017 and later) have this property defined
            $_.displayName
        }
        else {
            # For "legacy" versions, a mostly human-readable name is in the instanceId field instead
            $_.instanceId
        }
    }} |
    # Add a property which points to the vsvarsall.bat for this version
    Select-Object -Property *, @{ Name= "vsvarsallPath"; Expression = {
            if ($_.majorVersion -ge 15) {
            # Version 15 (VS 2017 and later) use this path as described here:
            # https://docs.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=msvc-160#developer_command_file_locations
            Join-Path $_.installationPath "VC" | Join-Path -ChildPath "Auxiliary" | Join-Path -ChildPath "Build" | Join-Path -ChildPath "vcvarsall.bat"
        }
        else {
            # "legacy" versions, use this path as described here:
            # https://docs.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=msvc-140#developer_command_file_locations
            Join-Path $_.installationPath "VC" | Join-Path -ChildPath "vcvarsall.bat"
        }
    }} |
    # Filter any options where vsvarsallPath does not exist
    Where-Object {
        $vsvarsallExists = Test-Path $_.vsvarsallPath
        if(!$vsvarsallExists) {
            Write-Warning "vsvarsall.bat does not exist for $($_.humanReadableName) at $($_.vsvarsallPath). If this is unexpected, please ensure that the Visual C++ payload was installed for this version."
        }
        $vsvarsallExists
    } |
    Select-Object -Property humanReadableName, majorVersion, vsvarsallPath

if ($validVSVersions.Length -eq 0) {
    Write-Error "Did not find any versions of Visual Studio installed. Please visit https://visualstudio.microsoft.com/ and install one."
}

$versionToUse = $null
if ($SelectedVersion -ne 0) {
    # If the user requested a version, take that
    $versionToUse = $validVSVersions | Where-Object { $_.majorVersion -eq $SelectedVersion } | Select-Object -First 1

    if ($versionToUse -eq $null) {
        [int[]]$availableVSVersions = $validVSVersions.majorVersion
        Write-Error "Unable to match requested version $SelectedVersion. Available versions are $($availableVSVersions -join ", ")"
    }
}
else {
    # Otherwise, take the latest version
    $versionToUse = $validVSVersions | Sort-Object -Property majorVersion -Descending | Select-Object -First 1
}

# Run the script, grab all the variables into this Powershell session
# Based on the example on the vswhere website: https://github.com/microsoft/vswhere/wiki/Start-Developer-Command-Prompt#using-powershell
& "${env:COMSPEC}" /s /c "`"$($versionToUse.vsvarsallPath)`" $ToolchainArchitecture && set" |
Where-Object { !$_.StartsWith("*") } | # Filter out all the VS Command Prompt logo which are all outlined with *
Where-Object { $_.Contains("=") } | # Filter out anything which doesn't look like we are getting the results of `set`
Foreach-Object {
    $name, $value = $_ -split '=', 2
    Set-Content env:\"$name" $value
}

# Now we just need to launch kdevelop... Easier said than done!
# Read the registry, which might be in any of several different keys
$kdevelopInstallDir = @(
    "Registry::HKEY_LOCAL_MACHINE\Software\KDE e.V.\KDevelop",
    "Registry::HKEY_LOCAL_MACHINE\Software\KDE\KDevelop",
    "Registry::HKEY_LOCAL_MACHINE\Software\Wow6432Node\KDE e.V.\KDevelop",
    "Registry::HKEY_LOCAL_MACHINE\Software\Wow6432Node\KDE\KDevelop"
) |
Select-Object -Property @{ Name="Install_Dir"; Expression = { (Get-ItemProperty -Path $_ -Name "Install_Dir")."Install_Dir" } } |
Where-Object { $_."Install_Dir" -ne $null } |
Select-Object -First 1

$kdevelopExe = Join-Path "$($kdevelopInstallDir."Install_Dir")" "bin" | Join-Path -ChildPath "kdevelop.exe"

Start-Process "$kdevelopExe"
