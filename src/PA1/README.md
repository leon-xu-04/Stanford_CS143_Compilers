# Write-up for PA1

## Project Description

This project defines regular expression rules for flex, which converts
them into an NFA, then a DFA, and finally a DFA transition table. The
result is a lexer that tokenizes Cool source files into (token, value)
pairs.

## Design Decisions

### Deferred String Error Strategy

I initially had separate booleans for tracking each potential string
error, but this doesn't scale. The solution is a single `const char *`
pointer that stores the error message when one is encountered and stays
NULL otherwise. This cleanly satisfies two requirements:
- report only the first string error encountered
- keep scanning until the string ends

### Escape Sequence Handling

The `\\([^\0])` escape rule consolidates all escape handling into one
rule while excluding the NUL byte from the match. This avoids
duplicating the null-character error logic in both the escape rule and
the bare `\0` rule (DRY principle).

### String Literals vs strdup

Most error messages in this lexer are string literals, which live in
static memory and persist for the lifetime of the program — no
allocation needed. However, `strdup(yytext)` is needed for invalid
characters (`cool.flex:206`) because `yytext` is a flex-managed buffer
that gets overwritten on the next token match. `strdup` copies it to
the heap so the error message remains valid.

### Start Conditions and State Management

The states can get quite intricate. One example: `BEGIN(INITIAL)` must
be called before returning `ERROR` when `<<EOF>>` is encountered inside
a comment or string. Without it, the lexer stays in the STR/BLOCK_COMMENT
state and never reaches `yyterminate()`, causing an infinite loop. It also
ensures a clean state when scanning the next file.

Lesson learned: always reset state during initialization. I forgot to
clear `string_error` when entering a new string, which caused errors
from one string to poison later ones.

For start conditions, the `%x` (exclusive) syntax is neat — when active,
all rules without an explicit `<STATE>` prefix are ignored.

## Flex Concepts Applied

### Rule Ordering and Conflicts

Two flex rules are key to getting correct tokenization:
- **Longest match**: flex picks the rule that matches the most characters.
  For example, `IfThisIsATypeID` matches as a single TYPEID rather than
  the keyword IF followed by something else.
- **Priority (first match)**: when two rules match the same length, the one
  appearing first in the file wins. For example, `if` matches both the IF
  keyword rule and the OBJECTID regex, but the keyword rule comes first,
  so it gets the correct token.

## Tests

Tests are in the `tests/` folder, split into separate files:
- **basic**: keywords, identifiers, operators, and integers — straightforward cases.
- **comments**: line comments and nested block comments, verifying that
  keywords inside comments are ignored and nesting depth is tracked correctly.
- **strings**: basic strings plus all error cases except NUL byte.
- **null_in_string**: separated out because `.cl` files containing NUL
  bytes can't be created or read as normal text.

> **Note on creating NUL byte tests:** `printf '\\\0'` (single quotes)
> passes all four characters verbatim to printf, which interprets
> `\\` → `\` and `\0` → NUL byte. But `printf "\\\0"` (double quotes)
> lets bash process the string first: `\\` → `\`, then printf only sees
> `\\0`, producing `\` + `0` (the digit) instead of `\` + NUL.
