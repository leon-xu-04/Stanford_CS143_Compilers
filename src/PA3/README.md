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

<!-- TODO: install basic+user classes -> duplicate/reserved-name checks ->
parent validity (inheritable + defined) -> cycle detection -> Main check.
Explain WHY the order is forced: each pass's safety depends on the previous
(can't walk parent links until every parent is proven to exist). -->

### class_map as the Class-Name Symbol Table

<!-- TODO: why a map (name -> Class_ node) and not a set; why the value is the
node pointer (cycle walk needs parents, errors need line numbers, later phases
need features); why the 5 basic classes live in the same map as user classes. -->

### Cycle Detection

<!-- TODO: the naive walk-to-root bug (infinite loop when a class dangles off a
cycle: A -> B -> C -> B), and the fix. State the invariant: no cycles iff every
class reaches Object. -->

### TypeEnv and the ClassTable Facade

<!-- TODO: bundling O/M/C into one struct passed by reference; SymbolTable held
by value and what its persistent-list copy semantics mean (snapshot, nothing
ever freed); the te.conforms/te.lub/te.error facade and why the logic stays on
ClassTable (state ownership). Honest trade-off: conforms/lub taking TypeEnv
couples the layers; a plain self-class Symbol parameter would decouple them. -->

### SELF_TYPE: Lazy Resolution

<!-- TODO: self is bound to the literal SELF_TYPE symbol; the subscript C is
never stored - it is reconstructed from te.current_class at each use site
(conforms, lub, dispatch). Why eager resolution to C is unsound: the choose()
example, lub(SELF_TYPE, SELF_TYPE) = SELF_TYPE, and why nothing concrete
conforms to SELF_TYPE (inheritance makes it a moving target). -->

### Error Recovery Conventions

<!-- TODO: report, recover, keep checking. Object as the fallback type;
No_type strictly for absent expressions (attr/let without init); "erroneous
declarations produce no bindings" (self attr, self in let/case) so the real
self binding is never shadowed. Halt gates: after the constructor and after
type checking - why type checking must not run on a broken hierarchy. -->

### Virtual type_check on AST Nodes

<!-- TODO: one method per node kind mirrors the manual's one rule per form;
declared through Expression_EXTRAS (= 0) and Expression_SHARED_EXTRAS in
cool-tree.handcode.h, bodies in semant.cc. Contrast with the rejected
alternative (dynamic_cast chain in ClassTable). Note the set_type + return
pair every rule must maintain and why (return feeds the parent rule, set_type
feeds dump_with_types). -->

### Shared Helpers Instead of Repeated Rules

<!-- TODO: type_check_int_binop with an explicit result_type parameter (the
op-string-inspection bug: control coupling, pointer == on string literals);
type_check_call shared by dispatch and static dispatch (lookup_class vs
receiver_type as the two axes of variation, SELF_TYPE return tracks the
receiver). -->

## Why the Code Is Correct

<!-- TODO: each expression rule is a transcription of the manual's typing rule;
conforms and lub are the only two type-algebra operations and both are simple
parent-chain walks over class_map; methods are checked once in their defining
class and remain sound for all subclasses (monotonicity: the check establishes
C <= T, any D <= C gives D <= T by transitivity). -->

## Tests

<!-- TODO: tests/ organized per construct (class checks, each expression form,
dispatch/static dispatch, SELF_TYPE cases); methodology: diff against the
reference compiler for message wording and ordering; PA2-inherited idea of
injecting a second error to prove recovery continues; pa3-grading.pl result. -->
