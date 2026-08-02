class A { };
class B inherits A { };
class C inherits A { };

class D {
  func(): A {
    case 1 of
      x_1: A => x_1;
      x_2: B => x_2;
      x_3: C => x_3;
    esac
  };

  another_func(): B {
    case 1 of
      x_1: A => x_1;
      x_2: B => x_2;
      x_3: C => x_3;
    esac
  };
};

class Main {
  main(): Int {
    case 1 of
      x_1: Int => 1;
      x_2: Int => x_2 + "str2";
      x_3: SELF_TYPE => 3;
      x_4: SELF_TYPE => 4;
    esac
  };
};
