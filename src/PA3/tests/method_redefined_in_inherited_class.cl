class A {
  meth(): Int {
    0
  };
};

class B inherits A {
  meth(): Bool {
    true
  };
};

class C inherits A {
  meth(attr: Int): Int {
    0
  };
};

class D inherits C {
  meth(attr: Int): Int {
    1
  };
};

class F {
  meth(attr: Int, another_attr: Int): Int {
    0
  };
};

class G inherits F {
  meth(attr: Bool, another_attr: String): Int {
    0
  };
};

class Main {
  main(): Int {
    0
  };
};
