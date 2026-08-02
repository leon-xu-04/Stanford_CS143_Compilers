class A { };

class B {
  simple_func(): Int {
    0
  };

  complex_func(arg1: Int, arg2: Bool, arg3: String, arg4: A): Int {
    0
  };

  self_func(): SELF_TYPE {
    self.copy()
  };
};

class C inherits B { };

class D inherits C {
  func_in_D(): Int {
    0
  };
};

class Main {
  main(): Int {
    {
      (new D)@B.simple_func();
      (new D)@C.simple_func();
      (new D)@B.complex_func(0, true, "str", new A);

      (new B)@D.func_in_D();
      (new B)@Missing.simple_func();
      (new D)@SELF_TYPE.simple_func();
      (new D)@B.missing_method();
      (new D)@B.complex_func(0, true, "str");
      (new D)@B.complex_func(true, "str", new A, 0);

      (new D)@B.self_func().func_in_D();
      (new B)@B.self_func().func_in_D();
      0;
    }
  };
};
