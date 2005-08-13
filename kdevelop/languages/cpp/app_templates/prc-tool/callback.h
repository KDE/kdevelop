#ifndef __CALLBACK_H__
#define __CALLBACK_H__

/* This is a workaround for a bug in the current version of gcc:

   gcc assumes that no one will touch %a4 after it is set up in crt0.o.
   This isn't true if a function is called as a callback by something
   that wasn't compiled by gcc (like FrmCloseAllForms()).  It may also
   not be true if it is used as a callback by something in a different
   shared library.

   We really want a function attribute "callback" which will insert this
   progloue and epilogoue automatically.
   
      - Ian */

register void *reg_a4 asm("%a4");

#define CALLBACK_PROLOGUE \
    void *save_a4 = reg_a4; asm("move.l %%a5,%%a4; sub.l #edata,%%a4" : :);

#define CALLBACK_EPILOGUE reg_a4 = save_a4;

#endif
