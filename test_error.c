int a = 10;
char b = 'c';
if (a == 10 && b ! 'x') {  // Error: missing '=' after '!'
    a = a ++ 1;            // Error: invalid use of '++' with '+'
    b == 'y';              // Error: trying to assign with '==' instead of '='
    @invalid_symbol;       // Error: '@' is an unrecognized symbol
}

/* Unterminated comment:
int c = 20;                // Error: this part is inside an unterminated comment

return 0;

