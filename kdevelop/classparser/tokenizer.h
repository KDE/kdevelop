/********************************************************************
* Name    :                                                         *
* ------------------------------------------------------------------*
* File    : tokenizer                                            *
* Author  :                                         *
* Date    : ~dy-~dn-~dd                                             *
*                                                                   *
* ------------------------------------------------------------------*
* Purpose :                                                         *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
* Usage   :                                                         *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
* Macros:                                                           *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
* Types:                                                            *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
* Functions:                                                        *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
* Modifications:                                                    *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
*********************************************************************/
#ifndef _TOKENIZER_H_INCLUDED
#define _TOKENIZER_H_INCLUDED

#define CLASS          258
#define PUBLIC         259
#define PROTECTED      260
#define PRIVATE        261
#define ID             262
#define CLCL           263
#define CONST          264
#define SIGNALSLOT_MAP 265
#define RA             266
#define SIGNALTEXT_MAP 267
#define STRING         268
#define STATIC         269
#define VIRTUAL        270
#define QTSIGNAL       271
#define QTSLOT         272
#define CPGLOBAL       273
#define CPENUM         274
#define CPTYPEDEF      275
typedef const char * YYSTYPE;

extern YYSTYPE yylval;

#endif
