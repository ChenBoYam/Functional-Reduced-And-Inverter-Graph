/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include <list>
#include "cirDef.h"
#include "sat.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGate
{
public:
   friend class CirMgr;
   CirGate(enum GateType _gateType, unsigned int _line): gateType(_gateType), lineNo(_line), dfsOrderWithUNDEF_flag(-1), dfsOrderWith_out_UNDEF_flag(-1){}
   virtual ~CirGate() {}

   // Basic access methods
   string getTypeStr() const;
   unsigned getLineNo() const { return 0; }
   virtual unsigned int getID() const = 0;
   bool isInvert(CirGate*) const; // for fanout only, because fanin save inverted information in
   virtual bool isAig() const { return false; }

   // Printing functions
   virtual void printGate() const {}
   void reportGate() const;
   void reportFanin(int level) const;
   void reportFanout(int level) const;
   void reportFaninInternal(int level, int indent, bool invert, list<const CirGate*> *reported) const;
   void reportFanoutInternal(int level, int indent, bool invert, list<const CirGate*> *reported) const;
private:
   CirGate(){}; 
protected:
   vector<unsigned int> fanin;
   vector<unsigned int> fanout;
   enum GateType gateType;
   unsigned int lineNo;
   int dfsOrderWithUNDEF_flag;
   int dfsOrderWith_out_UNDEF_flag;
};
class CirAndGate: public CirGate
{
public:
   friend class CirMgr;
   friend class CirGate;
   CirAndGate(unsigned int, unsigned int, unsigned int, unsigned int);
   ~CirAndGate();
   virtual unsigned int getID() const;
protected:
   CirAndGate();
   unsigned int pin[3]; // pins: o, i1, i2 respectively
   bool inv[3];
};

class CirIOGate: public CirGate
{
public:
   friend class CirMgr;
   friend class CirGate;
   CirIOGate(unsigned int, unsigned int); // for PI
   CirIOGate(unsigned int, int, unsigned int); // for PO
   ~CirIOGate();
   void setName(const string&);
   virtual unsigned int getID() const;
protected:
   unsigned int id;
   bool inverted;
   string name;
   int n; // id for PO
};

class CirConstGate: public CirGate
{
public:
   friend class CirMgr;
   friend class CirGate;
   CirConstGate(bool _value): CirGate(CONST_GATE, 0), value(_value){}
   CirConstGate():CirGate(CONST_GATE, 0){};
   ~CirConstGate(){};
   virtual unsigned int getID() const;
private:
   
   bool value;
};

class CirUndefGate: public CirGate
{
public:
   friend class CirMgr;
   friend class CirGate;
   CirUndefGate(unsigned int);
   ~CirUndefGate(){};
   virtual unsigned int getID() const;
private:
   CirUndefGate();
   unsigned int id;
};

#endif // CIR_GATE_H
