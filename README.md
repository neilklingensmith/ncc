





Grammar
=======

    program       -> [<declaration>]* [<function>]*
    function      -> type <identifier> `(' [type <identifier>,]* `)' <block>
    block         -> `{' [<declaration>]* [<statement>]* `}'
    declaration   -> (int | short | char) <identifier> ;
    statement     -> identifier `=' <expression>
                   | return <expression>;
                   | if ( <bexpression> ) <block> [else <block>]
    bexpression   -> <bterm> [`||' <bterm>]*
    bterm         -> <not-factor> [`&&' <not-factor>]*
    not-factor    -> [~] <b-factor>
    b-factor      -> <b-literal> | <relation>
    relation      -> <expression> [(`>' | `<' | `==' | `>=' | `<=' | `!=') <expression>]
    expression    -> <term> [(`+' | `-') <term>]*
    term          -> <signedfactor> [(`*' | `/') <factor>]*
    signedfactor  -> [(`+' | `-')] <factor>
    factor        -> constant | `*' data
                   | identifier ( [identifier ,]* ) ;
    data          -> identifier | `(' <expression> `)' | `*' pointer

NOTE: Productions for logical expressions are on page 402 of the dragon book.
