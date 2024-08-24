```
program    = definition*
definition = type ident "(" ( type ident ("," type ident)* )? ")" "{" stmt* "}"
stmt       = expr ";"
           | "if" "(" expr ")" stmt ("else" stmt)?
           | "while" "(" expr ")" stmt
           | "for" "(" expr? ";" expr? ";" expr? ")" stmt
           | "return" expr ";"
           | "{" stmt* "}"
           | type ident ";"
expr       = assign
assign     = equality ("=" assign)?
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary)*
unary      = ("+" | "-")? primary
           | ("*" | "&")? unary
primary    = num 
           | ident ( "(" ( expr ("," expr)* )? ")" )? 
           | "(" expr ")"
type       = ("int" | "char") "*"*
```
