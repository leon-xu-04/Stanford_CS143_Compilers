

#include "semant.h"
#include "cool-tree.h"
#include "cool-tree.handcode.h"
#include "utilities.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <regex>
#include <set>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <typeindex>
#include <vector>

extern int semant_debug;
extern char *curr_filename;

//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
static Symbol arg, arg2, Bool, concat, cool_abort, copy, Int, in_int, in_string,
    IO, length, Main, main_meth, No_class, No_type, Object, out_int, out_string,
    prim_slot, self, SELF_TYPE, Str, str_field, substr, type_name, val;
//
// Initializing the predefined symbols.
//
static void initialize_constants(void) {
  arg = idtable.add_string("arg");
  arg2 = idtable.add_string("arg2");
  Bool = idtable.add_string("Bool");
  concat = idtable.add_string("concat");
  cool_abort = idtable.add_string("abort");
  copy = idtable.add_string("copy");
  Int = idtable.add_string("Int");
  in_int = idtable.add_string("in_int");
  in_string = idtable.add_string("in_string");
  IO = idtable.add_string("IO");
  length = idtable.add_string("length");
  Main = idtable.add_string("Main");
  main_meth = idtable.add_string("main");
  //   _no_class is a symbol that can't be the name of any
  //   user-defined class.
  No_class = idtable.add_string("_no_class");
  No_type = idtable.add_string("_no_type");
  Object = idtable.add_string("Object");
  out_int = idtable.add_string("out_int");
  out_string = idtable.add_string("out_string");
  prim_slot = idtable.add_string("_prim_slot");
  self = idtable.add_string("self");
  SELF_TYPE = idtable.add_string("SELF_TYPE");
  Str = idtable.add_string("String");
  str_field = idtable.add_string("_str_field");
  substr = idtable.add_string("substr");
  type_name = idtable.add_string("type_name");
  val = idtable.add_string("_val");
}

ClassTable::ClassTable(Classes classes) : semant_errors(0), error_stream(cerr) {
  install_basic_classes();

  install_user_classes(classes);

  if (!check_parents_valid()) {
    return;
  }

  if (!check_inheritance_cycles()) {
    return;
  }

  check_main_class();

  check_features_valid();

  check_inherited_features_valid();
}

void ClassTable::install_basic_classes() {

  // The tree package uses these globals to annotate the classes built below.
  // curr_lineno  = 0;
  Symbol filename = stringtable.add_string("<basic class>");

  // The following demonstrates how to create dummy parse trees to
  // refer to basic Cool classes.  There's no need for method
  // bodies -- these are already built into the runtime system

  // IMPORTANT: The results of the following expressions are
  // stored in local variables.  You will want to do something
  // with those variables at the end of this method to make this
  // code meaningful.

  //
  // The Object class has no parent class. Its methods are
  //        abort() : Object    aborts the program
  //        type_name() : Str   returns a string representation of class name
  //        copy() : SELF_TYPE  returns a copy of the object
  //
  // There is no need for method bodies in the basic classes---these
  // are already built in to the runtime system.

  Class_ Object_class = class_(
      Object, No_class,
      append_Features(
          append_Features(single_Features(method(cool_abort, nil_Formals(),
                                                 Object, no_expr())),
                          single_Features(method(type_name, nil_Formals(), Str,
                                                 no_expr()))),
          single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
      filename);

  //
  // The IO class inherits from Object. Its methods are
  //        out_string(Str) : SELF_TYPE       writes a string to the output
  //        out_int(Int) : SELF_TYPE            "    an int    "  "     "
  //        in_string() : Str                 reads a string from the input
  //        in_int() : Int                      "   an int     "  "     "
  //
  Class_ IO_class = class_(
      IO, Object,
      append_Features(
          append_Features(
              append_Features(single_Features(method(
                                  out_string, single_Formals(formal(arg, Str)),
                                  SELF_TYPE, no_expr())),
                              single_Features(method(
                                  out_int, single_Formals(formal(arg, Int)),
                                  SELF_TYPE, no_expr()))),
              single_Features(
                  method(in_string, nil_Formals(), Str, no_expr()))),
          single_Features(method(in_int, nil_Formals(), Int, no_expr()))),
      filename);

  //
  // The Int class has no methods and only a single attribute, the
  // "val" for the integer.
  //
  Class_ Int_class = class_(
      Int, Object, single_Features(attr(val, prim_slot, no_expr())), filename);

  //
  // Bool also has only the "val" slot.
  //
  Class_ Bool_class = class_(
      Bool, Object, single_Features(attr(val, prim_slot, no_expr())), filename);

  //
  // The class Str has a number of slots and operations:
  //       val                                  the length of the string
  //       str_field                            the string itself
  //       length() : Int                       returns length of the string
  //       concat(arg: Str) : Str               performs string concatenation
  //       substr(arg: Int, arg2: Int): Str     substring selection
  //
  Class_ Str_class = class_(
      Str, Object,
      append_Features(
          append_Features(
              append_Features(
                  append_Features(
                      single_Features(attr(val, Int, no_expr())),
                      single_Features(attr(str_field, prim_slot, no_expr()))),
                  single_Features(
                      method(length, nil_Formals(), Int, no_expr()))),
              single_Features(method(concat, single_Formals(formal(arg, Str)),
                                     Str, no_expr()))),
          single_Features(
              method(substr,
                     append_Formals(single_Formals(formal(arg, Int)),
                                    single_Formals(formal(arg2, Int))),
                     Str, no_expr()))),
      filename);

  insert_class_map(Object_class);
  insert_class_map(IO_class);
  insert_class_map(Int_class);
  insert_class_map(Bool_class);
  insert_class_map(Str_class);
}

void ClassTable::install_user_classes(Classes classes) {
  for (int i = classes->first(); classes->more(i); i = classes->next(i)) {
    Class_ c = classes->nth(i);
    Symbol n = c->get_name();

    if (is_basic_class(n)) {
      semant_error(c) << "Redefinition of basic class " << n << ".\n";
      continue;
    }

    if (class_map.find(n) != class_map.end()) {
      semant_error(c) << "Class " << n << " was previously defined.\n";
      continue;
    }

    insert_class_map(c);
  }
}

void ClassTable::insert_class_map(Class_ class_) {
  AttrMap attr_map;
  MethodMap method_map;
  Features features = class_->get_features();

  for (int i = features->first(); features->more(i); i = features->next(i)) {
    Feature f = features->nth(i);
    Symbol n = f->get_name();
    if (attr_class *attr = dynamic_cast<attr_class *>(f)) {
      if (n == self) {
        semant_error(class_) << "'self' cannot be the name of an attribute.\n";
        continue;
      }

      if (attr_map.find(n) != attr_map.end()) {
        semant_error(class_)
            << "Attribute " << n << " is multiply defined in class "
            << class_->get_name() << ".\n";
        continue;
      }

      attr_map[n] = attr;
    } else if (method_class *method = dynamic_cast<method_class *>(f)) {
      if (method_map.find(n) != method_map.end()) {
        semant_error(class_)
            << "Method " << n << " is multiply defined in class "
            << class_->get_name() << ".\n";
        continue;
      }

      method_map[n] = method;
    }
  }

  ClassInfo info = {class_, attr_map, method_map};
  class_map.insert(std::make_pair(class_->get_name(), info));
}

bool ClassTable::check_parents_valid() {
  bool valid = true;

  for (ClassMap::iterator it = class_map.begin(); it != class_map.end(); ++it) {
    Class_ c = it->second.class_;
    Symbol n = c->get_name();
    Symbol p = c->get_parent_name();

    if (p == Int || p == Bool || p == Str || p == SELF_TYPE) {
      semant_error(c) << "Class " << n << " cannot inherit class " << p
                      << ".\n";
      valid = false;
    } else if (p != No_class && !class_exists(p)) {
      semant_error(c) << "Class " << n << " inherits from an undefined class "
                      << p << ".\n";
      valid = false;
    }
  }

  return valid;
}

bool ClassTable::check_inheritance_cycles() {
  bool valid = true;

  for (ClassMap::iterator it = class_map.begin(); it != class_map.end(); ++it) {
    Class_ c = it->second.class_;

    Class_ curr = c;
    Symbol curr_n = curr->get_name();
    std::set<Symbol> seen;
    while (curr_n != No_class) {
      if (seen.find(curr_n) != seen.end()) {
        semant_error(c) << "Class " << c->get_name() << ", or an ancestor of "
                        << c->get_name()
                        << ", is involved in an inheritance cycle.\n";
        valid = false;
        break;
      }
      seen.insert(curr_n);

      // TODO: Consider extracting this strict lookup into a helper. We use
      // find() instead of [] because ClassInfo has no default constructor and
      // a lookup should not insert into class_map. No_class is the endpoint of
      // a parent chain rather than an entry in the map, so handle it first.
      curr_n = curr->get_parent_name();
      if (curr_n == No_class) {
        break;
      }

      ClassMap::iterator parent_it = class_map.find(curr_n);
      // Parent validation succeeded before this pass, so a missing entry here
      // indicates an internal ClassTable invariant violation.
      assert(parent_it != class_map.end());
      curr = parent_it->second.class_;
    }
  }

  return valid;
}

void ClassTable::check_main_class() {
  ClassMap::iterator class_it = class_map.find(Main);

  if (class_it == class_map.end()) {
    semant_error() << "Class Main is not defined.\n";
    return;
  }
  Class_ class_ = class_it->second.class_;

  MethodMap method_map = class_it->second.method_map;
  MethodMap::iterator method_it = method_map.find(main_meth);

  if (method_it == method_map.end()) {
    semant_error(class_) << "No 'main' method in class Main.\n";
    return;
  }

  method_class *main_method = method_it->second;
  if (main_method->get_formals()->len() > 0) {
    semant_error(class_->get_filename(), main_method)
        << "'main' method in class Main should have no arguments.\n";
    return;
  }
}

bool ClassTable::is_basic_class(Symbol class_name) {
  return (class_name == Object || class_name == IO || class_name == Int ||
          class_name == Bool || class_name == Str || class_name == SELF_TYPE);
}

bool ClassTable::class_exists(Symbol class_name) const {
  return class_map.find(class_name) != class_map.end();
}

bool ClassTable::is_valid_type(Symbol type,
                               SelfTypePolicy self_type_policy) const {
  return class_exists(type) ||
         (self_type_policy == ALLOW_SELF_TYPE && type == SELF_TYPE);
}

void ClassTable::check_features_valid() {
  for (ClassMap::iterator it = class_map.begin(); it != class_map.end(); ++it) {
    ClassInfo &class_info = it->second;
    Class_ current_class = class_info.class_;

    if (is_basic_class(current_class->get_name())) {
      continue;
    }

    check_attributes_valid(current_class, class_info.attr_map);
    check_methods_valid(current_class, class_info.method_map);
  }
}

void ClassTable::check_attributes_valid(Class_ current_class,
                                        const AttrMap &attr_map) {
  for (AttrMap::const_iterator it = attr_map.begin(); it != attr_map.end();
       ++it) {
    attr_class *attr = it->second;
    Symbol declared_type = attr->get_type_decl();

    if (!is_valid_type(declared_type, ALLOW_SELF_TYPE)) {
      semant_error(current_class->get_filename(), attr)
          << "Class " << declared_type << " of attribute " << attr->get_name()
          << " is undefined.\n";
    }
  }
}

void ClassTable::check_methods_valid(Class_ current_class,
                                     const MethodMap &method_map) {
  for (MethodMap::const_iterator it = method_map.begin();
       it != method_map.end(); ++it) {
    method_class *method = it->second;
    check_formals_valid(current_class, method);
    check_method_return_type(current_class, method);
  }
}

void ClassTable::check_method_return_type(Class_ current_class,
                                          method_class *method) {
  Symbol return_type = method->get_return_type();

  if (!is_valid_type(return_type, ALLOW_SELF_TYPE)) {
    semant_error(current_class->get_filename(), method)
        << "Undefined return type " << return_type << " in method "
        << method->get_name() << ".\n";
  }
}

void ClassTable::check_formals_valid(Class_ current_class,
                                     method_class *method) {
  Formals formals = method->get_formals();

  std::set<Symbol> seen;

  for (int i = formals->first(); formals->more(i); i = formals->next(i)) {
    Formal formal = formals->nth(i);
    Symbol formal_type = formal->get_type_decl();
    Symbol formal_name = formal->get_name();

    if (!is_valid_type(formal_type, FORBID_SELF_TYPE)) {
      if (formal_type == SELF_TYPE) {
        semant_error(current_class->get_filename(), formal)
            << "Class SELF_TYPE of formal parameter " << formal->get_name()
            << " is undefined.\n";
      } else {
        semant_error(current_class->get_filename(), formal)
            << "Class " << formal_type << " of formal parameter "
            << formal->get_name() << " is undefined.\n";
      }
    }

    if (formal->get_name() == self) {
      semant_error(current_class->get_filename(), formal)
          << "'self' cannot be the name of an formal.\n";
      continue;
    }

    if (seen.find(formal_name) != seen.end()) {
      semant_error(current_class->get_filename(), formal)
          << "Formal parameter " << formal_name << " is multiply defined.\n";
    }
    seen.insert(formal_name);
  }
}

void ClassTable::check_inherited_features_valid() {
  for (ClassMap::iterator it = class_map.begin(); it != class_map.end(); ++it) {
    ClassInfo &class_info = it->second;
    Class_ current_class = class_info.class_;

    if (is_basic_class(current_class->get_name())) {
      continue;
    }

    check_inherited_attributes_valid(current_class, class_info.attr_map);
    check_inherited_methods_valid(current_class, class_info.method_map);
  }
}

void ClassTable::check_inherited_attributes_valid(Class_ current_class,
                                                  const AttrMap &attr_map) {
  for (AttrMap::const_iterator it = attr_map.begin(); it != attr_map.end();
       ++it) {
    attr_class *attr = it->second;

    if (has_inherited_attribute(current_class, attr->get_name())) {
      semant_error(current_class->get_filename(), attr)
          << "Attribute " << attr->get_name()
          << " is an attribute of an inherited class.\n";
    }
  }
}

bool ClassTable::has_inherited_attribute(Class_ current_class,
                                         Symbol attribute_name) const {
  Symbol parent_name = current_class->get_parent_name();
  if (parent_name == No_class) {
    return false;
  }

  ClassMap::const_iterator parent_it = class_map.find(parent_name);
  assert(parent_it != class_map.end());

  const ClassInfo &parent_info = parent_it->second;
  const AttrMap &parent_attributes = parent_info.attr_map;
  if (parent_attributes.find(attribute_name) != parent_attributes.end()) {
    return true;
  }

  return has_inherited_attribute(parent_info.class_, attribute_name);
}

void ClassTable::check_inherited_methods_valid(Class_ current_class,
                                               const MethodMap &method_map) {
  for (MethodMap::const_iterator it = method_map.begin();
       it != method_map.end(); ++it) {
    method_class *method = it->second;

    method_class *original = find_original_inherited_method(
        method->get_name(), current_class->get_parent_name());

    if (original) {
      check_method_override_valid(current_class, method, original);
    }
  }
}

method_class *
ClassTable::find_original_inherited_method(Symbol method_name,
                                           Symbol ancestor_name) const {
  if (ancestor_name == No_class) {
    return NULL;
  }

  ClassMap::const_iterator it = class_map.find(ancestor_name);
  assert(it != class_map.end());
  const ClassInfo &class_info = it->second;
  method_class *original = find_original_inherited_method(
      method_name, class_info.class_->get_parent_name());

  if (original) {
    return original;
  }

  const MethodMap &methods = class_info.method_map;
  MethodMap::const_iterator meth_it = methods.find(method_name);
  if (meth_it != methods.end()) {
    return meth_it->second;
  }

  return NULL;
}

void ClassTable::check_method_override_valid(Class_ current_class,
                                             method_class *local_method,
                                             method_class *inherited_method) {
  Symbol file_name = current_class->get_filename();
  if (local_method->get_return_type() != inherited_method->get_return_type()) {
    semant_error(file_name, local_method)
        << "In redefined method " << local_method->get_name()
        << ", return type " << local_method->get_return_type()
        << " is different from original return type "
        << inherited_method->get_return_type() << "\n";
  } else if (local_method->get_formals()->len() !=
             inherited_method->get_formals()->len()) {
    semant_error(file_name, local_method)
        << "Incompatible number of formal parameters in redefined method "
        << local_method->get_name() << ".\n";
  } else {
    check_override_formal_types(current_class, local_method, inherited_method);
  }
}

void ClassTable::check_override_formal_types(Class_ current_class,
                                             method_class *local_method,
                                             method_class *inherited_method) {
  Symbol file_name = current_class->get_filename();
  Symbol method_name = local_method->get_name();
  Formals local_formals = local_method->get_formals();
  Formals inherited_formals = inherited_method->get_formals();

  int local_it = local_formals->first();
  int inherited_it = inherited_formals->first();

  while (local_formals->more(local_it) &&
         inherited_formals->more(inherited_it)) {
    Formal local_formal = local_formals->nth(local_it);
    Formal inherited_formal = inherited_formals->nth(inherited_it);

    Symbol local_type = local_formal->get_type_decl();
    Symbol inherited_type = inherited_formal->get_type_decl();
    if (local_type != inherited_type) {
      semant_error(current_class->get_filename(), local_method)
          << "In redefined method " << local_method->get_name()
          << ", parameter type " << local_type
          << " is different from original type " << inherited_type << ".\n";
      return;
    }

    local_it = local_formals->next(local_it);
    inherited_it = inherited_formals->next(inherited_it);
  }
}

void ClassTable::type_check(Classes classes) {
  SymbolTable<Symbol, Entry> sym_tab;
  sym_tab.enterscope();
  sym_tab.addid(self, SELF_TYPE);

  for (int i = classes->first(); classes->more(i); i = classes->next(i)) {
    Class_ c = classes->nth(i);

    sym_tab.enterscope();
    for (Symbol cls = c->get_name(); cls != No_class;) {
      const ClassInfo &info = class_map.find(cls)->second;
      const AttrMap &attr_map = info.attr_map;
      for (AttrMap::const_iterator it = attr_map.begin(); it != attr_map.end();
           ++it) {
        sym_tab.addid(it->first, it->second->get_type_decl());
      }
      cls = info.class_->get_parent_name();
    }

    TypeEnv te = {this, sym_tab, c};
    Features features = c->get_features();

    for (int j = features->first(); features->more(j); j = features->next(j)) {
      Feature f = features->nth(j);
      f->type_check(te);
    }

    sym_tab.exitscope();
  }
}

bool ClassTable::conforms(Symbol provided, Symbol expected, const TypeEnv &te) {
  Symbol current_class = te.current_class->get_name();
  if (provided == SELF_TYPE && expected == SELF_TYPE) {
    return true;
  } else if (expected == SELF_TYPE) {
    return false;
  } else if (provided == SELF_TYPE) {
    provided = current_class;
  }

  while (provided != No_class && provided != expected) {
    Class_ c = class_map.find(provided)->second.class_;
    provided = c->get_parent_name();
  }

  return provided == expected;
}

Symbol ClassTable::parent_of(Symbol class_name) const {
  ClassMap::const_iterator it = class_map.find(class_name);
  assert(it != class_map.end() && "parent_of: symbol not in class_map");
  return it->second.class_->get_parent_name();
}

int ClassTable::inheritance_depth(Symbol class_name) const {
  int level = 0;
  while (class_name != No_class) {
    class_name = parent_of(class_name);
    ++level;
  }
  return level;
}

Symbol ClassTable::lub(Symbol a, Symbol b, const TypeEnv &te) const {
  if (a == SELF_TYPE && b == SELF_TYPE) {
    return SELF_TYPE;
  } else if (a == SELF_TYPE) {
    return lub(te.current_class->get_name(), b, te);
  } else if (b == SELF_TYPE) {
    return lub(te.current_class->get_name(), a, te);
  }

  int depth_a = inheritance_depth(a), depth_b = inheritance_depth(b);
  if (depth_a > depth_b) {
    std::swap(a, b);
    std::swap(depth_a, depth_b);
  }

  while (depth_b > depth_a) {
    b = parent_of(b);
    --depth_b;
  }

  while (a != b) {
    a = parent_of(a);
    b = parent_of(b);
  }

  return a;
}

bool TypeEnv::conforms(Symbol provided, Symbol expected) const {
  return class_table->conforms(provided, expected, *this);
}

Symbol TypeEnv::lub(Symbol a, Symbol b) const {
  return class_table->lub(a, b, *this);
}

void method_class::type_check(TypeEnv &te) {
  te.sym_tab.enterscope();
  for (int i = formals->first(); formals->more(i); i = formals->next(i)) {
    Formal f = formals->nth(i);
    te.sym_tab.addid(f->get_name(), f->get_type_decl());
  }

  Symbol inferred_return_type = expr->type_check(te);
  if (!te.conforms(inferred_return_type, return_type)) {
    te.error(this) << "Inferred return type " << inferred_return_type
                   << " of method " << get_name()
                   << " does not conform to declared return type "
                   << return_type << ".\n";
  }

  te.sym_tab.exitscope();
}

void attr_class::type_check(TypeEnv &te) {
  Symbol type_inferred = init->type_check(te);
  if (type_inferred == No_type) {
    return;
  }

  if (!te.conforms(type_inferred, type_decl)) {
    te.error(this) << "Inferred type " << type_inferred
                   << " of initialization of attribute " << get_name()
                   << " does not conform to declared type " << type_decl
                   << ".\n";
  }
}

Symbol assign_class::type_check(TypeEnv &te) {
  if (name == self) {
    te.error(this) << "Cannot assign to 'self'.\n";
  }

  Symbol type_decl = te.sym_tab.lookup(name);
  if (type_decl == NULL) {
    te.error(this) << "Assignment to undeclared variable " << name << ".\n";
    type_decl = Object;
  }

  Symbol type_inferred = expr->type_check(te);

  if (!te.conforms(type_inferred, type_decl)) {
    te.error(this)
        << "Type " << type_inferred
        << " of assigned expression does not conform to declared type "
        << type_decl << " of identifier " << name << ".\n";
    type_inferred = Object;
  }

  set_type(type_inferred);
  return type_inferred;
}

method_class* ClassTable::get_method(Symbol class_name, Symbol method_name) const {
  for (Symbol cls = class_name; cls != No_class; cls = parent_of(cls)) {
    const MethodMap &map = class_map.find(cls)->second.method_map;
    MethodMap::const_iterator it = map.find(method_name);
    if (it != map.end()) {
      return it->second;
    }
  }
  return NULL;
}

static Symbol type_check_call(Symbol lookup_class, Symbol receiver_type,
                              Symbol method_name, Expressions actual,
                              Expression node, TypeEnv &te) {
  std::vector<Symbol> arg_types;
  arg_types.reserve(actual->len());
  for (int i = actual->first(); actual->more(i); i = actual->next(i)) {
    arg_types.push_back(actual->nth(i)->type_check(te));
  }

  method_class *method = te.class_table->get_method(lookup_class, method_name);
  if (method == NULL) {
    te.error(node) << "Dispatch to undefined method " << method_name << ".\n";
    node->set_type(Object);
    return Object;
  }

  Formals formals = method->get_formals();
  if (formals->len() != actual->len()) {
    te.error(node) << "Method " << method_name
                   << " called with wrong number of arguments.\n";
  } else {
    for (int i = formals->first(); formals->more(i); i = formals->next(i)) {
      Formal formal = formals->nth(i);
      if (!te.conforms(arg_types[i], formal->get_type_decl())) {
        te.error(node) << "In call of method " << method_name << ", type "
                       << arg_types[i] << " of parameter " << formal->get_name()
                       << " does not conform to declared type "
                       << formal->get_type_decl() << ".\n";
      }
    }
  }

  Symbol return_type = method->get_return_type();
  if (return_type == SELF_TYPE) {
    return_type = receiver_type;
  }
  node->set_type(return_type);
  return return_type;
}

Symbol static_dispatch_class::type_check(TypeEnv &te) {
  Symbol receiver_type = expr->type_check(te);
  if (!te.class_table->class_exists(type_name)) {
    te.error(this) << "Static dispatch to undefined class " << type_name
                   << ".\n";
    set_type(Object);
    return Object;
  }
  if (!te.conforms(receiver_type, type_name)) {
    te.error(this) << "Expression type " << receiver_type
                   << " does not conform to declared static dispatch type "
                   << type_name << ".\n";
  }
  return type_check_call(type_name, receiver_type, name, actual, this, te);
}

Symbol dispatch_class::type_check(TypeEnv &te) {
  Symbol receiver_type = expr->type_check(te);
  Symbol lookup_class = (receiver_type == SELF_TYPE)
                            ? te.current_class->get_name()
                            : receiver_type;
  return type_check_call(lookup_class, receiver_type, name, actual, this, te);
}

Symbol cond_class::type_check(TypeEnv &te) {
  Symbol type_pred = pred->type_check(te);
  if (type_pred != Bool) {
    te.error(pred) << "Predicate of 'if' does not have type Bool.\n";
  }

  Symbol type_then = then_exp->type_check(te);
  Symbol type_else = else_exp->type_check(te);
  Symbol combined_type = te.lub(type_then, type_else);
  set_type(combined_type);
  return combined_type;
}

Symbol loop_class::type_check(TypeEnv &te) {
  Symbol t1 = pred->type_check(te);
  if (t1 != Bool) {
    te.error(pred) << "Loop condition does not have type Bool.\n";
  }

  body->type_check(te);

  set_type(Object);
  return Object;
}

Symbol typcase_class::type_check(TypeEnv &te) {
  expr->type_check(te);
  Symbol t_ret = NULL;
  std::set<Symbol> seen;

  for (int i = cases->first(); cases->more(i); i = cases->next(i)) {
    Case c = cases->nth(i);

    if (seen.find(c->get_type_decl()) != seen.end()) {
      te.error(c) << "Duplicate branch " << c->get_type_decl()
                  << " in case statement.\n";
    } else {
      seen.insert(c->get_type_decl());
    }

    bool type_decl_exists = te.class_table->class_exists(c->get_type_decl());
    if (c->get_type_decl() != SELF_TYPE && ! type_decl_exists) {
      te.error(c) << "Class " << c->get_type_decl()
        << " of case branch is undefined.\n";
    }

    if (c->get_name() == self) {
      te.error(c) << "'self' bound in 'case'.\n";
    }

    if (c->get_type_decl() == SELF_TYPE) {
      te.error(c) << "Identifier " << c->get_name()
                  << " declared with type SELF_TYPE in case branch.\n";
    }

    te.sym_tab.enterscope();
    if (type_decl_exists) {
      te.sym_tab.addid(c->get_name(), c->get_type_decl());
    } else {
      te.sym_tab.addid(c->get_name(), Object);
    }

    Symbol t = c->get_expr()->type_check(te);
    te.sym_tab.exitscope();

    t_ret = (t_ret == NULL) ? t : te.lub(t, t_ret);
  }

  set_type(t_ret);
  return t_ret;
}

Symbol block_class::type_check(TypeEnv &te) {
  Symbol t = Object;
  for (int i = body->first(); body->more(i); i = body->next(i)) {
    t = body->nth(i)->type_check(te);
  }
  set_type(t);
  return t;
}

Symbol let_class::type_check(TypeEnv &te) {
  if (identifier == self) {
    te.error(this) << "'self' cannot be bound in a 'let' expression.\n";
  }
  
  bool type_decl_valid = te.class_table->class_exists(type_decl);
  if (! type_decl_valid) {
    te.error(init) << "Class " << type_decl
      << " of let-bound identifier " << identifier << " is undefined.\n";
  }

  Symbol t1 = init->type_check(te);
  if (t1 != No_type && !te.conforms(t1, type_decl)) {
    te.error(init) << "Inferred type " << t1 << " of initialization of "
                   << identifier
                   << " does not conform to identifier's declared type "
                   << type_decl << ".\n";
  }

  te.sym_tab.enterscope();
  if (identifier != self) {
    te.sym_tab.addid(identifier, type_decl);
  }
  Symbol t2 = body->type_check(te);
  te.sym_tab.exitscope();

  set_type(t2);
  return t2;
}

static Symbol type_check_int_binop(Expression e1, Expression e2, const char *op,
                                   Symbol result_type, Expression node,
                                   TypeEnv &te) {
  Symbol t1 = e1->type_check(te);
  Symbol t2 = e2->type_check(te);
  if (t1 != Int || t2 != Int) {
    te.error(node) << "non-Int arguments: " << t1 << " " << op << " " << t2
                   << ".\n";
  }
  node->set_type(result_type);
  return result_type;
}

Symbol plus_class::type_check(TypeEnv &te) {
  return type_check_int_binop(e1, e2, "+", Int, this, te);
}

Symbol sub_class::type_check(TypeEnv &te) {
  return type_check_int_binop(e1, e2, "-", Int, this, te);
}

Symbol mul_class::type_check(TypeEnv &te) {
  return type_check_int_binop(e1, e2, "*", Int, this, te);
}

Symbol divide_class::type_check(TypeEnv &te) {
  return type_check_int_binop(e1, e2, "/", Int, this, te);
}

Symbol neg_class::type_check(TypeEnv &te) {
  Symbol t = e1->type_check(te);
  if (t != Int) {
    te.error(this) << "Argument of '~' has type " << t << " instead of Int.\n";
  }
  set_type(Int);
  return Int;
}

Symbol lt_class::type_check(TypeEnv &te) {
  return type_check_int_binop(e1, e2, "<", Bool, this, te);
}

static bool is_primitive(Symbol t) { return t == Int || t == Str || t == Bool; }

Symbol eq_class::type_check(TypeEnv &te) {
  Symbol t1 = e1->type_check(te);
  Symbol t2 = e2->type_check(te);

  if ((is_primitive(t1) || is_primitive(t2)) && t1 != t2) {
    te.error(this) << "Illegal comparison with a basic type.\n";
  }

  set_type(Bool);
  return Bool;
}

Symbol leq_class::type_check(TypeEnv &te) {
  return type_check_int_binop(e1, e2, "<=", Bool, this, te);
}

Symbol comp_class::type_check(TypeEnv &te) {
  Symbol t = e1->type_check(te);
  if (t != Bool) {
    te.error(this) << "Argument of 'not' has type " << t
                   << " instead of Bool.\n";
  }
  set_type(Bool);
  return Bool;
}

Symbol int_const_class::type_check(TypeEnv &te) {
  set_type(Int);
  return Int;
}

Symbol bool_const_class::type_check(TypeEnv &te) {
  set_type(Bool);
  return Bool;
}

Symbol string_const_class::type_check(TypeEnv &te) {
  set_type(Str);
  return Str;
}

Symbol new__class::type_check(TypeEnv &te) {
  Symbol t = type_name;
  if (t != SELF_TYPE && !te.class_table->class_exists(t)) {
    te.error(this) << "'new' used with undefined class " << t << ".\n";
    t = Object;
  }
  set_type(t);
  return t;
}

Symbol isvoid_class::type_check(TypeEnv &te) {
  e1->type_check(te);
  set_type(Bool);
  return Bool;
}

Symbol no_expr_class::type_check(TypeEnv &te) {
  set_type(No_type);
  return No_type;
}

Symbol object_class::type_check(TypeEnv &te) {
  Symbol t = te.sym_tab.lookup(name);
  if (t == NULL) {
    te.error(this) << "Undeclared identifier " << name << ".\n";
    t = Object;
  }
  set_type(t);
  return t;
}
////////////////////////////////////////////////////////////////////
//
// semant_error is an overloaded function for reporting errors
// during semantic analysis.  There are three versions:
//
//    ostream& ClassTable::semant_error()
//
//    ostream& ClassTable::semant_error(Class_ c)
//       print line number and filename for `c'
//
//    ostream& ClassTable::semant_error(Symbol filename, tree_node *t)
//       print a line number and filename
//
///////////////////////////////////////////////////////////////////

ostream &ClassTable::semant_error(Class_ c) {
  return semant_error(c->get_filename(), c);
}

ostream &ClassTable::semant_error(Symbol filename, tree_node *t) {
  error_stream << filename << ":" << t->get_line_number() << ": ";
  return semant_error();
}

ostream &ClassTable::semant_error() {
  semant_errors++;
  return error_stream;
}

ostream &TypeEnv::error(tree_node *node) const {
  return class_table->semant_error(current_class->get_filename(), node);
}

/*   This is the entry point to the semantic checker.

     Your checker should do the following two things:

     1) Check that the program is semantically correct
     2) Decorate the abstract syntax tree with type information
        by setting the `type' field in each Expression node.
        (see `tree.h')

     You are free to first do 1), make sure you catch all semantic
     errors. Part 2) can be done in a second stage, when you want
     to build mycoolc.
 */
void program_class::semant() {
  initialize_constants();

  /* ClassTable constructor may do some semantic analysis */
  ClassTableP classtable = new ClassTable(classes);

  /* some semantic analysis code may go here */
  if (! classtable->errors()) {
    classtable->type_check(classes);
  }

  if (classtable->errors()) {
    cerr << "Compilation halted due to static semantic errors." << endl;
    exit(1);
  }
}
