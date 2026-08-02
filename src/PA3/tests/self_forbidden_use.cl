class A {
  func1(): Int {
    let self: Int <- 0 in 3
  };

  func2(): SELF_TYPE {
    self <- (new A).copy()
  };
};

class Main {
  main(): Int {
    (new A).func()
  };
};
