# Write-up for PA3

## Project Description

Semant is the last stage of the Cool compiler's frontend. It checks the
program's semantic validity — flagging forbidden identifier names, resolving
every name used in type declarations, method invocations, and variable
references, and type-checking every expression — then annotates the AST with
the inferred types for the code generator.

The work happens in two phases. Phase 1 (the `ClassTable` constructor) builds
the class hierarchy and validates its shape: duplicate and reserved class
names, illegal or undefined parents, inheritance cycles, and feature-level
rules like method overrides. Phase 2 walks each class's expressions and
applies the manual's typing rules. The split is forced by dependency: type
checking constantly walks parent links, so it can only run on a hierarchy
already proven well-formed.

The code is organized around the judgment `O, M, C |- e : T`: `O` is a scoped
symbol table of object identifiers, `M` is the per-class method map inside
`ClassTable`, and `C` is the class currently being checked. The three travel
together as a `TypeEnv` passed through every typing rule, and each rule
computes its node's `T`.

## Design Decisions

### Multi-Pass Class Validation and Forced Ordering

Starting this project was painful: there are so many specifications, and I had
no idea what to check first. Breaking the requirements down by dependency is
what helped. Within phase 1:

  - collect all class names first (inheritance can't be checked against names
    we haven't seen), catching duplicates and reserved names while collecting
  - once the names are in, check every parent is defined and inheritable
  - only then run cycle detection, which walks parent links and would crash or
    loop on an unvalidated hierarchy

Each step is only safe because the previous one ran. The same logic forces the
phase 1 / phase 2 split itself: phase 2 calls `parent_of()` constantly, so it
needs the whole hierarchy proven first.

### class_map as the Class-Name Symbol Table

The AST stores a class's parent as a `Symbol` (a name), not a pointer — the
parser couldn't link nodes that might not exist yet. So the core operation of
this whole assignment is name -> node resolution, and that is exactly a
`Symbol -> Class_` map. A set of `Class_` wouldn't work: to find a class in it
you'd already need the pointer you're looking for.

### Cycle Detection

For each class, we repeatedly step to its parent. If the walk reaches `Object`,
the class is not on a cycle; if it revisits a class, it is.

This is not optimally efficient: when a walk reaches `Object`, every class on
that path is also proven cycle-free, but we throw that information away and
re-walk from each of them. We kept the naive version because clarity and
correctness are the goals here, and class hierarchies are tiny.

### TypeEnv and the ClassTable Facade

I bundled O, M, C into a `TypeEnv` struct so every typing rule takes one
argument instead of three. Since the rules invoke the same few ClassTable
queries constantly, I added facade methods (`te.conforms`, `te.lub`,
`te.error`) to cut the call-site noise; the logic stays on `ClassTable`, which
owns the class hierarchy.

Reading `symtab.h` surprised me: nothing is ever freed. Scope nodes are shared
between table copies (that's what makes copying a table a cheap snapshot), so
no single owner could safely delete them — and since a compiler is a one-shot
program rather than a persistent server, leaking is an acceptable price.

### SELF_TYPE: Lazy Resolution

The lazy resolution of SELF_TYPE is the most interesting idea in the project,
and I didn't appreciate it while first implementing it. We keep the type as
the literal `SELF_TYPE` symbol until an operation forces us to resolve it to
the current class — for example, `lub(SELF_TYPE, SELF_TYPE) = SELF_TYPE`.
Resolving early throws away precision that can never be recovered.

What felt unintuitive at first: nothing concrete conforms to SELF_TYPE. The
reason is inheritance — SELF_TYPE can stand for any subclass of the current
class, including ones not written yet, so no fixed type can be guaranteed to
conform to it.

### Error Recovery Conventions

Phase 1 errors are not recovered from: if the hierarchy is broken, everything
downstream (`parent_of`, conformance, lub) is meaningless, so we halt before
phase 2.

Within phase 2 we report and keep going: an undefined type becomes `Object`
and checking proceeds. The cost is cascading errors — a body recovered to
`Object` then fails its declared return type, producing a second message for
one mistake. A better design would be a dedicated error type that conforms to
everything, so errors derived from an already-reported one stay silent.

### Virtual type_check on AST Nodes

The AST is a good exemplification of virtual functions: an abstract class per
node category (Expression, Feature, Formal, ...) with one concrete class per
form, so each typing rule is a `type_check` override and dynamic dispatch
picks the right one — no long `dynamic_cast` chains.

The mechanism for adding these methods is unusual: `cool-tree.h` is
auto-generated, so instead of editing it we define the declarations as macros
in `cool-tree.handcode.h`, which the generated file expands at `#ifdef` hook
points, and put the bodies in `semant.cc`.

### Shared Helpers Instead of Repeated Rules

We tried to maximize reuse: one helper covers all six Int-operand operators
(parameterized by result type), dispatch and static dispatch share their
argument-checking core, and `parent_of` / `conforms` / `lub` back every
hierarchy question.

## Why the Code Is Correct

My development flow was roughly TDD after mapping out a checklist per phase.
The phase 1 rules were the hard part to extract from the manual; for phase 2,
each expression's typing rule transcribes almost mechanically into its
`type_check` body.

One question I kept returning to: with inheritance, why is a method checked
once in its defining class still valid in every subclass, especially with
`self` and SELF_TYPE involved? The answer is the lazy resolution above — the
check is performed against SELF_TYPE's most flexible reading, so anything that
passes it stays sound as the hierarchy grows downward.

## Tests

The suite has 30+ files, roughly one per language construct, so clear naming
matters. If I had more time I would build an automatic runner that diffs every
test against saved expected output; currently tests are run file by file.

## Known Issues and Future Work

From a code review; the crash bugs it found (undefined declared types in
`let`/`case` reaching `conforms`) are fixed, the rest remains open:

- **Error emission order.** Several checks emit errors while iterating
  `class_map`/`attr_map`/`method_map`, whose Symbol-pointer keys give intern
  order, not source order — the reference emits in source order. Fix: drive
  error loops from the class list and `get_features()`; keep maps for lookup
  only.
- **Anchors and wording.** Some errors anchor to a different node than the
  reference (`if`/`while` predicate, `let` init, duplicate features) and a few
  message texts differ (trailing periods, formal-parameter wording).
- **Hardening.** `class_map.find(...)->second` appears in seven places with
  inconsistent failure behavior (assert vs unchecked UB). Extract one strict
  `info_of(Symbol)` accessor with an assert so future gaps fail loudly.
- **Smaller:** unused C++11 includes (`<regex>`, `<typeindex>`) that break
  strict C++98 builds; `conforms`/`lub` need only a class name, not the whole
  `TypeEnv`; misleading names (`is_basic_class` matches SELF_TYPE, `get_method`
  walks the ancestor chain); some dead getters and locals.
