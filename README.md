





Grammar
=======

    program       -> [declaration]* [function]*
    function      -> type identifier `(' [type identifier,]* `)' block
    block         -> `{' [declaration]* [statement]* `}'
    declaration   -> (int | short | char) identifier ;
    statement     -> identifier `=' expression
                   | return expression;
                   | if ( bexpression ) block [else block]
    expression    -> term [(`+' | `-') term]*
    term          -> signedfactor [(`*' | `/') factor]*
    signedfactor  -> [(`+' | `-')] factor
    factor        -> constant | identifier | `(' expression `)'
                   | funcname ( [identifier ,]* ) ;



