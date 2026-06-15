
(*
 *  execute "coolc bad.cl" to see the error messages that the coolc parser
 *  generates
 *
 *  execute "myparser bad.cl" to see the error messages that your parser
 *  generates
 *)

(* no error *)
class A {
};

(* error:  b is not a type identifier *)
Class b inherits A {
};

(* error:  a is not a type identifier *)
Class C inherits a {
};

(* error:  keyword inherits is misspelled *)
Class D inherts A {
};

(* error:  closing brace is missing *)
Class E inherits A {
;

(* error:  feature error *)
Class F inherits A {
    feature_a: Int <- 3;
    feature_b: wrongType;
    feature_c: Bool <- true;
};

(* error:  let binding error *)
Class G inherits A {
    function() : Int {
        let x : wrongType <- 3, y : Int <- 4 in y
    };
};

(* error:  expression error in block *)
Class G inherits A {
    function() : Int {
        {
            a <- 3;
            b <- True;
            c <- 5;
        }
    };
};
