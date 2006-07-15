/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmSeparateArgumentsCommand.cxx,v $
  Language:  C++
  Date:      $Date: 2006/05/12 17:53:20 $
  Version:   $Revision: 1.4 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmSeparateArgumentsCommand.h"

// cmSeparateArgumentsCommand
bool cmSeparateArgumentsCommand
::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() != 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  const char* cacheValue = this->Makefile->GetDefinition(args[0].c_str());
  if(!cacheValue)
    {
    return true;
    }
  std::string value = cacheValue;
  cmSystemTools::ReplaceString(value,
                               " ", ";");
  this->Makefile->AddDefinition(args[0].c_str(), value.c_str());
  return true;
}

