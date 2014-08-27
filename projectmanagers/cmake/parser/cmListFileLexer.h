/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmListFileLexer.h,v $
  Language:  C++
  Date:      $Date: 2004/08/31 22:39:42 $
  Version:   $Revision: 1.3 $

  Copyright 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmListFileLexer_h
#define cmListFileLexer_h

#include "cmakecommonexport.h"

typedef enum cmListFileLexer_Type_e
{
  cmListFileLexer_Token_None,
  cmListFileLexer_Token_Newline,
  cmListFileLexer_Token_Identifier,
  cmListFileLexer_Token_ParenLeft,
  cmListFileLexer_Token_ParenRight,
  cmListFileLexer_Token_ArgumentUnquoted,
  cmListFileLexer_Token_ArgumentQuoted,
  cmListFileLexer_Token_BadCharacter,
  cmListFileLexer_Token_BadString
} cmListFileLexer_Type;

typedef struct cmListFileLexer_Token_s cmListFileLexer_Token;
struct cmListFileLexer_Token_s
{
  cmListFileLexer_Type type;
  char* text;
  int length;
  int line;
  int column;
};

typedef struct cmListFileLexer_s cmListFileLexer;

#ifdef __cplusplus
extern "C"
{
#endif

KDEVCMAKECOMMON_EXPORT cmListFileLexer* cmListFileLexer_New();
KDEVCMAKECOMMON_EXPORT int cmListFileLexer_SetFileName(cmListFileLexer*, const char*);
KDEVCMAKECOMMON_EXPORT int cmListFileLexer_SetString(cmListFileLexer*, const char*);
KDEVCMAKECOMMON_EXPORT cmListFileLexer_Token* cmListFileLexer_Scan(cmListFileLexer*);
KDEVCMAKECOMMON_EXPORT long cmListFileLexer_GetCurrentLine(cmListFileLexer*);
KDEVCMAKECOMMON_EXPORT long cmListFileLexer_GetCurrentColumn(cmListFileLexer*);
KDEVCMAKECOMMON_EXPORT const char* cmListFileLexer_GetTypeAsString(cmListFileLexer*,
                                            cmListFileLexer_Type);
KDEVCMAKECOMMON_EXPORT void cmListFileLexer_Delete(cmListFileLexer*);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
