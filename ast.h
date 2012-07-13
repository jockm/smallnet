#pragma once

#include <vector>
#include <string>
#include <iostream>
#include "ir.h"

class Environment;
class Declarations;

namespace AST {
    class Statement;
    class Namespace;
    class Class;
    class Feature;

    typedef std::vector<Statement*> StatementList;
    typedef std::vector<Namespace*> NamespaceList;
    typedef std::vector<Class*> ClassList;
    typedef std::vector<Feature*> FeatureList;

    typedef std::vector<Statement*>::iterator StatementIterator;
    typedef std::vector<Namespace*>::iterator NamespaceIterator;
    typedef std::vector<Class*>::iterator ClassIterator;
    typedef std::vector<Feature*>::iterator FeatureIterator;

    class Node
    {
        public:
            int linenum;

            Node(int linenum) : linenum(linenum) { }
            virtual ~Node() { }
            virtual void dump(std::ostream &o) = 0;
            virtual void semant(Environment *e) = 0;
    };

    class Program : public Node
    {
        public:
            NamespaceList *nsl;

            Program(int linenum, NamespaceList *nsl) : Node(linenum), nsl(nsl) { }
            Program(int linenum) : Node(linenum), nsl(new NamespaceList()) { }
            ~Program();
            void dump(std::ostream &o);
            void declare(Declarations *decl);
            void semant(Environment *e);
            void codegen_start(IR::Prog *irrog, Environment *env);
            IR::Prog *codegen(Environment *env);
    };

    class Expr : public Node {
        public:
            std::string type;

            Expr(int linenum) : Node(linenum) { }
            IR::Type get_irtype();
            virtual int codegen(IR::Block *b, Environment *env) = 0;
    };

    class Statement : public Node {
        public:
            virtual void codegen(IR::Block *b, Environment *env) = 0;
    };

    class Integer : public Expr {
        public:
            int value;

            Integer(int linenum, int value) : Expr(linenum), value(value) { }
            void dump(std::ostream &o) { o << value; }
            void semant(Environment *e);
            int codegen(IR::Block *b, Environment *env);
    };

    class Object : public Expr {
        public:
            std::string id;

            Object(int linenum, std::string id) : Expr(linenum), id(id) { }
            void dump(std::ostream &o) { o << id; }
            void semant(Environment *e);
            int codegen(IR::Block *b, Environment *env);
    };

    class Block : public Node {
        public:
            StatementList *sl;

            Block(int linenum, StatementList *sl) : Node(linenum), sl(sl) { }
            Block(int linenum) : Node(linenum), sl(new StatementList()) { }
            ~Block();
            void dump(std::ostream &o);
            void semant(Environment *e);
            void codegen(IR::Block *b, Environment *env);
    };

    class Feature : public Node {
        public:
            bool is_static;

            Feature(int linenum, bool is_static) : Node(linenum), is_static(is_static) { }
            virtual void declare(Declarations *decl) = 0;
            virtual void codegen(IR::Prog *ir, Environment *env) = 0;
    };

    class FieldFeature : public Feature {
        public:
            std::string id;
            std::string decl_type;
            std::string type;
            Expr *expr;

            FieldFeature(int linenum, std::string id, std::string decl_type, Expr *expr, bool is_static)
                : Feature(linenum, is_static), id(id), decl_type(decl_type), expr(expr) { }
            ~FieldFeature() { if(expr) delete expr; }
            void dump(std::ostream &o);
            void declare(Declarations *decl);
            void semant(Environment *e);
            void codegen(IR::Prog *ir, Environment *env);
    };

    class MethodFeature : public Feature {
        public:
            std::string id;
            std::string ret_type;
            std::string type;
            Block *block;
            // TODO args
            
            void dump(std::ostream &o);
            void declare(Declarations *decl);
            void semant(Environment *e);
            void codegen(IR::Prog *ir, Environment *env);
    };

    class Namespace : public Node {
        public:
            std::string id;
            ClassList *csl;

            Namespace(int linenum, std::string id, ClassList *csl) : Node(linenum), id(id), csl(csl) { }
            Namespace(int linenum, std::string id) : Node(linenum), id(id), csl(new ClassList()) { }
            ~Namespace();
            void dump(std::ostream &o);
            void declare(Declarations *decl);
            void semant(Environment *e);
            void codegen(IR::Prog *irprog, Environment *env);
    };

    class Class : public Node {
        public:
            std::string id;
            FeatureList *fl;
            bool is_static;

            Class(int linenum, std::string id, FeatureList *fl, bool is_static) : Node(linenum), id(id), fl(fl), is_static(is_static) { }
            Class(int linenum, std::string id, bool is_static) : Node(linenum), id(id), fl(new FeatureList()), is_static(is_static) { }
            ~Class();
            void dump(std::ostream &o);
            void declare(Declarations *decl);
            void semant(Environment *e);
            void codegen_static_initializer(IR::Prog *irprog, Environment *env);
            void codegen_initializer(IR::Prog *irprog, Environment *env);
            void codegen(IR::Prog *irprog, Environment *env);
    };

    class BinOp : public Expr {
        public:
            Expr* e1;
            Expr* e2;

            BinOp(int linenum, Expr *e1, Expr *e2) : Expr(linenum), e1(e1), e2(e2) { }
            ~BinOp() { delete e1; delete e2; }
            void semant(Environment *e);
    };

    class Plus : public BinOp {
        public:
            Plus(int linenum, Expr *e1, Expr *e2) : BinOp(linenum, e1, e2) { }
            ~Plus() { delete e1; delete e2; }
            void dump(std::ostream &o) { o << "("; e1->dump(o); o << ")+("; e2->dump(o); o << ")"; }
            int codegen(IR::Block *b, Environment *env);
    };

    class Sub : public BinOp {
        public:
            Sub(int linenum, Expr *e1, Expr *e2) : BinOp(linenum, e1,  e2) { }
            ~Sub() { delete e1; delete e2; }
            void dump(std::ostream &o) { o << "("; e1->dump(o); o << ")-("; e2->dump(o); o << ")"; }
            int codegen(IR::Block *b, Environment *env);
    };

    class Mult : public BinOp {
        public:
            Mult(int linenum, Expr *e1, Expr *e2) : BinOp(linenum, e1,  e2) { }
            ~Mult() { delete e1; delete e2; }
            void dump(std::ostream &o) { o << "("; e1->dump(o); o << ")*("; e2->dump(o); o << ")"; }
            int codegen(IR::Block *b, Environment *env);
    };

    class Div : public BinOp {
        public:
            Div(int linenum, Expr *e1, Expr *e2) : BinOp(linenum, e1,  e2) { }
            ~Div() { delete e1; delete e2; }
            void dump(std::ostream &o) { o << "("; e1->dump(o); o << ")/("; e2->dump(o); o << ")"; }
            int codegen(IR::Block *b, Environment *env);
    };
}

