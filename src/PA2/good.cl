Class Person {
    age: Int <- 18;

    getAge(): Int {
        18
    };
};

class Student inherits Person {
    grade: Int <- 4;
    gender: String;
    getGrade(): Int {
        grade
    };
    setGender(aGender: String): Int {
        3
    };

    multiple_formals(a: Int, b: String, c: Bool): Int {
        3
    };
};

(* dispatch *)
class Teacher inherits Person {
    dispatch1(): Int {
        getAge()
    };

    dispatch2(p: Teacher): Int {
        p.dispatch1()
    };

    dispatch3(p: Teacher): Int {
        p@Person.getAge()
    };

    dispatch_with_args(p: Teacher): Int {
        p.dispatch2(1 + 2, 3, 4)
    };

    self_dispatch_with_args(): Int {
        getAge(1, 2)
    };
};

class Expression_Test {
    if_then_else(): Int {
        if true then 2 else 3 fi
    };

    while_loop(): Int {
        while true loop 3 pool
    };

    block(): Int {
        {
            2;
            3;
        }
    };

    let_test(): Int {
        let x : Int <- 3, y : Int <- 4 in x
    };

    let_no_init(): Int {
        let x : Int in x
    };

    case_test(): Int {
        {
            case x of
                y : Int => 3;
            esac;

            case x of
                y : Int => 4;
                z : Bool => 5;
            esac;
        }
    };

    new_test() : Int {
        new TypeName
    };

    plus_test() : Int {
        3 + 4
    };

    sub_test() : Int {
        4 - 3
    };

    mul_test() : Int {
        3 * 4
    };

    divide_test() : Int {
        4 / 3
    };

    left_assoc_test() : Int {
        3 + 4 + 5
    };

    add_mul_precedence_test() : Int {
        3 + 4 * 5
    };

    neg_test() : Int {
        ~3
    };

    compare_test() : Bool {
        {
            3 < 4;
            3 <= 4;
            3 = 4;
        }
    };

    complement_test() : Bool {
        not true
    };

    parenthesis_test() : Int {
        2 - (3 - 4)
    };

    string_test() : Str {
        "Test"
    };

    assign_test(a: Int) : Int {
        a <- 5
    };

    false_test() : Bool {
        false
    };
};

class Create_Void {
    this_is_void : Create_Void;

    is_void_test() : Bool {
        isvoid this_is_void
    };
};
