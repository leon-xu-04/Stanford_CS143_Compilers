-- line comment: everything after here is ignored class if then
-- line comment with special chars: (* *) " \n
(* simple block comment *)
(* block comment
   spanning multiple lines *)
(* nested (* comment *) still in outer *)
(* deeply (* nested (* three *) levels *) deep *)
-- unmatched close outside comment should be ERROR
*)
-- EOF in comment:
(* this comment is never closed