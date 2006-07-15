/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmLinkDirectoriesCommand.cxx,v $
  Language:  C++
  Date:      $Date: 2006/05/12 15:56:08 $
  Version:   $Revision: 1.12 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmLinkDirectoriesCommand.h"

// cmLinkDirectoriesCommand
bool cmLinkDirectoriesCommand
::InitialPass(std::vector<std::string> const& args)
{
 if(args.size() < 1 )
    {
    return true;
    }

  for(std::vector<std::string>::const_iterator i = args.begin();
      i != args.end(); ++i)
    {
    std::string unixPath = *i;
    cmSystemTools::ConvertToUnixSlashes(unixPath);
    this->Makefile->AddLinkDirectory(unixPath.c_str());
    }
  return true;
}

