-- ===== KEYWORDS (case-insensitive, except true/false) =====
class Class CLASS cLaSs
else ELSE eLsE
fi FI fI
if IF iF
in IN iN
inherits INHERITS iNhErItS
let LET lEt
loop LOOP lOoP
pool POOL pOoL
then THEN tHeN
while WHILE wHiLe
case CASE cAsE
esac ESAC eSaC
of OF oF
new NEW nEw
isvoid ISVOID iSvOiD
not NOT nOt
true tRuE
false fAlSe
-- true/false must start lowercase; uppercase start = TYPEID
True True TRUE
False False FALSE

-- ===== IDENTIFIERS =====
MyClass Object Int Bool String SELF_TYPE
foo self x bar_baz a1_2B

-- ===== OPERATORS =====
-- single-char tokens
+ - * / ~ < = . , : ; { } ( ) @
-- multi-char tokens
<- <= =>

-- ===== INVALID CHARACTERS =====
-- each should produce ERROR with that character
> [ ] ! # ^ & % \ `

-- ===== INTEGERS =====
0 42 007 123456789