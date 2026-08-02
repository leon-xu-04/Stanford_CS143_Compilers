class Main {
  self: This_is_Missing;

  attr_with_a_nonexistent_type_decl: Missing;

  method_with_a_nonexistent_return_type(): Missing { 0 };

  method_with_a_nonexistent_formal(arg: Missing): Also_Missing { 0 };

  main(): Int {
    0
  };
};
