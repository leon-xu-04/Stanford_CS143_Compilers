
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

    no_expression() : Int {
        
    };

    ok() : Int { 1 };

    problematic_separator_for_formals(a : Int,) : Int {
        3
    };

    Type_error_in_id() : Int {
        3
    };

    formal_wrong_type_id(a : int) : Int {
        3
    };

    Another_type_error_in_id : Int;

    missing_expression : Int <-;

    missing_semicolon : Int <- 4
};

(* error:  let binding error *)
Class G inherits A {
    function() : Int { {
        let x : wrongType <- 3, y : Int <- 4, z : Int in x;
        let x : Int <- 3, y : anotherWrongType <- 4, z : Int in x;
        let x : Int <- 3, y : Int <- 4, z : yetAnotherOne in x;

        Seeing_This_Means_Bad_Let_Binding_Tests_Pass;
    } };
};

(* error:  expression error in block *)
Class G inherits A {
    function() : Int {
        {
            a <- 3;
            b <- True;
            c <- 5;
            d <- False;
        }
    };
};

(* error:  missing expression for assignment *)
class H {
    function() : Int {
        a <- 
    };
};

(* error: dispatch *)
class I {
    missing_method_name() : Int {
        obj.(1)
    };

    missing_type_id() : Int {
        obj@.function(1)
    };

    incomplete_argument() : Int {
        obj.function(a, )
    };

    type_lower() : Int {
        obj@type.function()
    };

    function_call_upper() : Int {
        Function()
    };

    complete_function() : Int {
        3
    };

    Seeing_This_Means_Dispatch_Tests_Pass
};

(*  error:  if then else fi *)
class J {
    if_then_else_fi() : Int {
        if true then 3 else 4
    };
}

(* error: while loop *)
class K {
    ok1() : Int { 1 };
    (* error: missing pool *)
    missing_pool() : Int { while true loop 3 };
    ok2() : Int { 2 };
    (* error: missing loop *)
    missing_loop() : Int { while true 3 pool };
    ok3() : Int { 3 };
};

(* error: case *)
class L {
    ok1() : Int { 1 };
    (* error: missing esac *)
    missing_esac() : Int { case x of y : Int => 3; };
    ok2() : Int { 2 };
    (* error: branch missing => *)
    bad_branch() : Int { case x of y : Int 3; esac };
    ok3() : Int { 3 };
};

(* error: new / isvoid / paren / operators *)
class M {
    ok1() : Int { 1 };
    (* error: new with lowercase (object) id, not a type *)
    new_lower() : Int { new foo };
    ok2() : Int { 2 };
    (* error: isvoid missing operand *)
    isvoid_empty() : Int { isvoid };
    ok3() : Int { 3 };
    (* error: missing closing paren *)
    missing_paren() : Int { (3 };
    ok4() : Int { 4 };
    (* error: binary operator missing right operand *)
    incomplete_plus() : Int { 3 + };
    ok5() : Int { 5 };
    (* error: not missing operand *)
    not_empty() : Bool { not };
    ok6() : Int { 6 };
};
