/***************************************************************************
                          tokenizer.l  -  description
                             -------------------
    begin                : Mon Mar 15 1999
    copyright            : (C) 1999 by Jonas Nordin
    email                : jonas.nordin@syncom.se

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/* N.B
 * When adding new tokens, use the CP prefix to avoid nameclashes
 * with the compiler and other packages.
 *
 * /Jonas
 */

#ifndef _TOKENIZER_CC_H_INCLUDED
#define _TOKENIZER_CC_H_INCLUDED

#define CPCLASS        258
#define CPPUBLIC       259
#define CPPROTECTED    260
#define CPPRIVATE      261
#define ID             262
#define CLCL           263
#define CPCONST        264
#define CPVOLATILE     265
#define CPUNION        267
#define STRING         268
#define CPSTATIC       269
#define CPVIRTUAL      270
#define QTSIGNAL       271
#define QTSLOT         272
#define CPGLOBAL       273
#define CPENUM         274
#define CPTYPEDEF      275
#define CPFRIEND       276
#define CPSTRUCT       277
#define CPOPERATOR     278
#define CPTEMPLATE     279
#define NUM            280
#define CPTHROW        281
#define CPNAMESPACE    282
#define CPOBJCCLASS      283
#define CPOBJCINTERFACE  284
#define CPOBJCPROTOCOL   285
#define CPOBJCIMPLEMENTATION 286
#define CPOBJCEND        287
#define CPOBJCPRIVATE    288
#define CPOBJCPROTECTED  289
#define CPOBJCPUBLIC     290

/* added by Daniel */
#define CPEMIT         291
#define CPREGISTER     292

#define CPVOID         300
#define CPINT          301
#define CPFLOAT        302
#define CPCHAR         303
#define CPBOOL         304
#define CPFOR          305
#define CPWHILE        306
#define CPDCAST        307
#define CPSCAST        308
#define CPIF           309
#define CPELSE         310
#define CPDO           311
#define CPSWITCH       312
#define CPCASE         313
#define CPBREAK        314
#define CPDEFAULT      315
#define CPRETURN       316

#define CPUNKNOWN      400
#define CPPOINTER      401
#define CPREFERENCE    402
#define CPNONE         403

#define CPPIPE         500

typedef const char* YYSTYPE;

extern YYSTYPE yylval;

#endif
