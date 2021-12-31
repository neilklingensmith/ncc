





Grammar
=======

    program       -> [declaration]* [function]*
    function      -> type identifier `(' [type identifier,]* `)' block
    block         -> `{' [declaration]* [statement]* `}'
    declaration   -> (int | short | char) identifier ;
    statement     -> identifier `=' expression
                   | return identifier;   
    expression    -> term [(`+' | `-') term]*
    term          -> signedfactor [(`*' | `/') factor]*
    signedfactor  -> [(`+' | `-')] factor
    factor        -> constant | identifier | `(' expression `)'



