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
      (new B).missing_method();
      (new B).simple_func();
      (new B).complex_func(0, true, "str", new A);
      (new B).complex_func(0, true, "str");
      (new B).complex_func(true, "str", new A, 0);
      (new B).self_func().func_in_D();
      (new D).self_func().func_in_D();
      0;
    }
  };
};
