class A {
  attr1: SELF_TYPE <- new A;
  attr2: SELF_TYPE <- new B;
  attr3: A <- self.copy();
  attr4: B <- self.copy();
};

class B inherits A {
  attr5: SELF_TYPE <- new A;
  attr6: SELF_TYPE <- new B;
  attr7: A <- self.copy();
  attr8: B <- self.copy();
};

class Main {
  attr1: A <- new B;
  attr2: B <- new A;
  attr3: Object <- new B;

  main(): Int {
    0
  };
};
