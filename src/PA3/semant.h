#ifndef SEMANT_H_
#define SEMANT_H_

#include "cool-tree.h"
#include "list.h"
#include "stringtab.h"
#include "symtab.h"
#include <assert.h>
#include <iostream>
#include <map>

#define TRUE 1
#define FALSE 0

class ClassTable;
typedef ClassTable *ClassTableP;

struct TypeEnv {
  ClassTableP class_table;
  SymbolTable<Symbol, Entry> sym_tab;
  Class_ current_class;

  bool conforms(Symbol provided, Symbol expected) const;
  Symbol lub(Symbol a, Symbol b) const;
  ostream &error(tree_node *node) const;
};

// This is a structure that may be used to contain the semantic
// information such as the inheritance graph.  You may use it or not as
// you like: it is only here to provide a container for the supplied
// methods.

struct ClassInfo {
  typedef std::map<Symbol, attr_class *> AttrMap;
  typedef std::map<Symbol, method_class *> MethodMap;

  Class_ class_;

  AttrMap attr_map;
  MethodMap method_map;
};

class ClassTable {
private:
  typedef std::map<Symbol, ClassInfo> ClassMap;
  typedef ClassInfo::AttrMap AttrMap;
  typedef ClassInfo::MethodMap MethodMap;

  enum SelfTypePolicy { ALLOW_SELF_TYPE, FORBID_SELF_TYPE };

  int semant_errors;
  ostream &error_stream;
  ClassMap class_map;
  void install_basic_classes();
  void install_user_classes(Classes classes);
  void insert_class_map(Class_ class_);
  bool check_parents_valid();
  bool check_parents_inheritable();
  bool check_parents_defined();
  bool check_inheritance_cycles();
  void check_main_class();
  bool is_basic_class(Symbol class_name);

  bool is_valid_type(Symbol type, SelfTypePolicy self_type_policy) const;
  void check_features_valid();
  void check_attributes_valid(Class_ current_class, const AttrMap &attr_map);
  void check_methods_valid(Class_ current_class, const MethodMap &method_map);
  void check_method_return_type(Class_, method_class *);
  void check_formals_valid(Class_, method_class *);

  void check_inherited_features_valid();

  void check_inherited_attributes_valid(Class_ current_class,
                                        const AttrMap &attr_map);
  bool has_inherited_attribute(Class_ current_class,
                               Symbol attribute_name) const;

  void check_inherited_methods_valid(Class_ current_class,
                                     const MethodMap &method_map);
  method_class *find_original_inherited_method(Symbol method_name, Symbol class_name) const;
  void check_method_override_valid(Class_, method_class *local_method,
                                   method_class *inherited_method);
  void check_override_formal_types(Class_, method_class *local_method,
                                   method_class *inherited_method);

  Symbol parent_of(Symbol class_name) const;
  int inheritance_depth(Symbol class_name) const;

public:
  ClassTable(Classes);
  bool class_exists(Symbol class_name) const;
  void type_check(Classes classes);
  bool conforms(Symbol provided, Symbol expected, const TypeEnv &te);
  Symbol lub(Symbol a, Symbol b, const TypeEnv &te) const;
  method_class* get_method(Symbol class_name, Symbol method_name) const;
  int errors() { return semant_errors; }
  ostream &semant_error();
  ostream &semant_error(Class_ c);
  ostream &semant_error(Symbol filename, tree_node *t);
};

#endif
