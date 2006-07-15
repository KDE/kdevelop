/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmSubdirDependsCommand.cxx,v $
  Language:  C++
  Date:      $Date: 2002/10/23 22:03:26 $
  Version:   $Revision: 1.6 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmSubdirDependsCommand.h"

// cmSubdirDependsCommand
bool cmSubdirDependsCommand::InitialPass(std::vector<std::string> const& )
{
  return true;
}

