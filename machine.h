#pragma once

#include "ir.h"
#include <iostream>
#include <set>
#include <map>
#include <list>

typedef int RealReg;

class RegGraph;

class RegConstraints {
    public:
        bool collapse_rdst_and_rsrc1;
        std::set<IR::VirtualReg> rdsts;
        std::set<IR::VirtualReg> rsrcs1;
        std::set<IR::VirtualReg> rsrcs2;
};

class BaseMachine {
    protected:
        typedef std::map<IR::VirtualReg, std::set<RealReg> > DefaultRegs;
        typedef std::map<IR::VirtualReg, RealReg> RealRegMap;
        typedef std::map<IR::Opcode, RegConstraints> OpcodeRegConstraints;

        OpcodeRegConstraints constraints;
        DefaultRegs default_regs;

        void add_graph_nodes(RegGraph &g, IR::Block *b);
        void add_graph_collapses(RegGraph &g, IR::Block *b);
        void add_graph_edges(RegGraph &g, IR::Block *b, IR::BlockInfo &binfo);
        void add_graph_constraints(RegGraph &g, IR::Block *b, IR::BlockInfo &binfo);
        RealRegMap regallocator(IR::Block *b);
        void codegen(IR::Block *b, std::ostream &o);

        void asmgen(RealRegMap &hardregs, IR::Block *b, std::ostream &o);
        virtual void asmgen(RealRegMap &hardregs, IR::Inst *inst, std::ostream &o) = 0;

    public:
        std::set<RealReg> get_regs_by_mask(RealReg reg_mask);
        void codegen(IR::Prog *irprog, std::ostream &o);
};

