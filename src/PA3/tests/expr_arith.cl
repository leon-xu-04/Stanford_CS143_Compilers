class A { };

class B inherits A { };

class Main {
  add_error(): Int {
    3 + "Str"
  };

  sub_error(): Int {
    true - 3
  };

  mul_error(): Int {
    3 * (new A)
  };

  div_error(): Int {
    3 / self
  };

  neg_error(): Int {
    ~self
  };

  lt_error(): Bool {
    1 < true
  };

  leq_error(): Bool {
    true <= 1
  };

  eq_error(): Bool {
    {
      1 = 2;
      1 = true;
      false = "str";
      1 = (new A);
      (new A) = (new B);
    }
  };

  main(): Int {
    3 + 5
  };
};
