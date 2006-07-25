// Mostly as an exercise to the lexer, but contains some difficulties
// (e.g. variable declarations) that can be tricky for the parser too.

class Literals
{
    void integers()
    {
        // decimal
        int x = 0;
        x = 1;
        x = -1;
        x = 101;
        x = 230;
        x = 459;
        x = 6l;
        x = 7L;

        // octal
        x = 00;
        x = 07;
        x = 012345670;
        x = 02l;
        x = 004L;

        // hexadecimal
        x = 0x0;
        x = 0X9;
        x = 0xF;
        x = 0Xa;
        x = 0x1A0F;
        x = 0X00F;
        x = 0x1aBf;
        x = 0x3l;
        x = 0X600L;

        // disabled because the lexer doesn't recognize unicode in normal code
        // TODO: FIXME in java-lexer.ll
        //x = \u0031; // which is '1' in unicode. valid, because unicode is preprocessed
    }

    void floats()
    {
        // decimal
        float y = 0f;
        y = 0.;
        y = -1.;
        y = 9.f;
        y = 1.e10;
        y = 029.;
        y = 30.e-1D;
        y = .2F;
        y = .4e+0d;
        y = .0000000000005;
        y = .6000000000000;
        y = 0.9;
        y = 28.403f;
        y = 0e1;
        y = 7e-9999f;

        // hexadecimal
        y = 0x3Ep-1;
        y = 0X00.P+70f;
        y = 0x.afp0f;
        y = 0X9.50cP-99;
    }

    void stringsAndChars()
    {
        String s = "";
        s = "bla";
        s = "39 wärmere weißwürste";
        s = "And God said, \"Now this is really pimpin'!\"\n";
        s = "\u0066\uu0F3b is unicode";
        s = "So he finished off that #$!*§ file and got to bed.";

        char c = 'a';
        c = '\u0007';
        c = '\n';
        c = '\'';
        // c = '\';  // invalid!
        c = '%';
        c = 'Ö';
    }
}
