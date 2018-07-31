# kdev-clang-tidy

A plugin for [KDevelop](https://www.kdevelop.org) to support [clang-tidy](http://clang.llvm.org/extra/clang-tidy/) 
static analysis.
It uses the [KDevelop5 Problem Checker Framework](https://techbase.kde.org/KDevelop5/Problem_Checker_Framework), parsing 
clang-tidy's output and showing the issues on the Problems Viewer, allowing easy review of the code.

## Build and install
In order to build this plugin you must have the libraries for KDevPlatform and KDevelop installed into your system.
Get the source code from this repository ( https://commits.kde.org/kdev-clang-tidy ), tweak it if you want, run 
cmake to configure the project, build it and have fun.

## Screenshots
![clang-tidy output on Problem Viewer](/doc/cltd-1-default.png?raw=true)
![Per-project configuration page](/doc/cltd-2-projconfig.png?raw=true)
![Tooltip/hint details clang-tidy check result](/doc/cltd-3-w-tooltip.png?raw=true)
![Global configuration](/doc/cltd-4-globalconfig.png?raw=true)

## TO-DO List
- Refine the code:
 - Exclude unnecessary includes from all sources;
 - Review the encapsulation to extend/shrink number of members inside the classes;
- Implement the interactive fix feature:
 - [OK] Add output to yaml option into command line;
 - [OK] Parse yaml output to display on a suggested correction widget;
 - Create a widget to allow user to choose which correction to use and which ignore;
 - Create a way to exclude some corrections per user wish from the original yaml output;
 - Apply the remained fixes from the yaml file.


## License

  This program is free software; you can redistribute it and/or                    
  modify it under the terms of the GNU General Public License                      
  as published by the Free Software Foundation; either version 2                   
  of the License, or (at your option) any later version.                           
                                                                                    
  This program is distributed in the hope that it will be useful,                  
  but WITHOUT ANY WARRANTY; without even the implied warranty of                   
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    
  GNU General Public License for more details.                                     
                                                                                  
  You should have received a copy of the GNU General Public License                
  along with this program; if not, write to the Free Software Foundation, 
  Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
  
  Copyright (C) 2016 by Carlos Nihelton <carlosnsoliveira@gmail.com>  
