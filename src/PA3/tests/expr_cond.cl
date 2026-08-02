class A { };

class B inherits A { };

class C inherits A { };

class D {
  func(): A {
    if true then new B else new A fi
  };
};

class E {
  func(): A {
    if true then new B else new C fi
  };
};

class F {
  func(): SELF_TYPE {
    if true then self else new F fi
  };
};

class G {
  func(): SELF_TYPE {
    if true then new F else self fi
  };
};

class H {
  func(): SELF_TYPE {
    if true then self else self fi
  };
};

class Main {
  main(): Int {
    if 1 then 1 else true fi
  };
};
