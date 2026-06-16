# Write-up for PA2

## Project Description

This project defines a bison grammar that turns the token sequence from
the flex lexer into an abstract syntax tree (AST) for later semantic
analysis. Apart from the standard parsing features for all context-free
grammar listed in the Cool manual, the generated parser also has a
certain level of support for error recovery.

This project relies heavily on the tree package predefined in the
`cool-tree.h`. Tree nodes are created with the constructors in that
library.


## Design Decisions

### AST Normalization (implicit constructs made explicit)

There are many "syntactic sugars" the parser handles implicitly that
make the AST more uniform and easier for later stages to process. A few
examples:
 - no `inherits` clause -> parent defaulted to `Object`
 - attribute / let binding with no initializer -> no_expr()
 - bare `f(...)` dispatch -> dispatch on object(self)
 - curr_filename stored (interned) on each class node

This demonstrates the idea of abstraction in systems: the grammar of a
language shouldn't require the user to understand the inner workings of
how it's represented in the AST. The parser fills these in so the
interface for users stays clean.

### Let Desugaring and Right Recursion

The single hardest grammar to work on is the let statement. This is
because we need to desugar the let grammar originally written in a list
format to a recursive format. So this involves a right-recursive parsing
as the final expression is in the end.

This takes up more stack space than left recursion because at each
step, we can't be sure whether the let statement terminates unless the
next token doesn't fit into the recursion, so we need to keep all the
previous tokens on the stack, reserved for consecutive reducing later.

Contrast this with the left recursive grammar for class / feature / 
formal lists. After each node is constructed, since the list is on the
left, we can immediately reduce to a new list, so we don't keep the node
on the stack.

Again, the syntactic sugar simplifies the grammar for users.

### Precedence Declarations

Though there are ways to distinguish precedence through grammar
declarations like having two layers where addition / subtraction are
handled at the upper layer then multiplication / division handled the
lower layer would allow us to achieve the de facto effect of
arithmetic precedence, we chose to achieve this effect with the
precedence declarations functionality of bison, which makes the grammar
much cleaner. Bison achieves this precedence effect with conflict-
resolution metadata on the LR parse table instead of rewriting the
grammar. When there is a shift/reduce conflict, bison compares the
precedence of the rule to reduce (taken from its last terminal) with
that of the lookahead token, then chooses to shift if the token has
higher precedence, reduce if the rule has a higher one, and consults
the associativity when the precedences are equal. This is pretty cool,
as rewriting grammars to achieve the same precedence effect seems rather
complicated.

The precedence declarations in bison also allows us to resolve another
shift/reduce conflict for the `let ... in expression.dispatch` when
the parser couldn't decide whether it should continue shifting to
reach dispatch or first reduce the existing let expression. It turns out
this is indeed a grammar ambiguity. The behavior we want — per the
manual, a let body extends as far right as possible — is achieved by
giving the let rule the lowest precedence. Since the dispatch operators
`.` and `@` are declared at the highest precedence, the token-to-shift
out-ranks the rule-to-reduce, so bison shifts — the body keeps
extending — and the conflict is resolved.


### Error Recovery

We supported error recovery at class / feature / block expression / let
binding / and formal list level. It seems bison have an intricate error
recovery mechanism. It first keeps popping the existing states until
error is in a valid state. Then it keeps discarding input tokens until
the next token can follow the error token as specified by the grammar.
So if the next token of error is `;`, just having the rule ... : error
`;` is enough. A special case is the let binding error. Since the let statement
is not finished at `,` we need to add the recursive structure
`error ',' let_binding` to consume the rest of the let statement, which
would leave the tokens in a clean, consistent state for the next
grammar.

### Line Numbers

Line numbers are set automatically with the `YYLLOC_DEFAULT` macro. One
issue we faced with line number is when there's no right hand side in
production for `no_expr()`, we would get a random number. The solution
we picked is to refactor productions so that when these `no_expr()` show
up, we have non-empty right hand sides.


## Tests

### good.cl

We added incremental tests in `good.cl` as we implement new grammars to
ensure they work. In the end, we have a complete grammatical coverage —
both class forms, methods, attrs (with/without init), formals
(multiple), all three dispatch forms (with and without args),
if/while/block/let (multi-binding and no-init)/case
(single/multi-branch)/new/isvoid, every operator, precedence +
associativity combos, parens, all constants. Note good.cl need only be
grammatically valid, not semantically valid.

### bad.cl

For `bad.cl`, the most important goal is to ensure error recovery works.
So we have extensive tests coverage for all the cases we expect recovery
to happen. We verify recovery works by injecting another error after
the recovery and ensure we can see it reported, because otherwise, valid
syntaxes just get discarded when an error occurs (`omerrs > 0`). We also
design many non-recoverable errors to ensure our grammar correctly
reflects what's specified in the manual.
