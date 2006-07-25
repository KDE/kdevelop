// some line comment
/* some block comment */

/* some common
   multiline comment */


//*       'switch' comment, part 1 (to switch, delete the first slash)
class EitherValid {};
/*/    // 'switch' comment, part 2 (switching machine: closing xor opening the block)
invalid OrInvalid {};
// */  // 'switch' comment, part 3 (closing the block comment)


/*       'switch' comment, other way round
invalid EitherInvalid {};
/*/
class OrValid {};
// */


/* /* the second block opener is ignored */

/* some block comment
// where this line comment is ignored */

// /* some line comment where the block starter is ignored
//    note: encountering EOF when the block comment is open yet makes for an error
