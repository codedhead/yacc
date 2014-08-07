#ifndef _YANJUN_PARSER_H_
#define _YANJUN_PARSER_H_

#include <cstdio>
using namespace std;

#include "YYSTYPE.h"
#include "LALR1Parser.h"
#include "YaccScanner.h"

struct StackEntry
{
	StackEntry(){state=-1;memset(&val,0,sizeof(YYSTYPE));}
	StackEntry(int s):state(s){memset(&val,0,sizeof(YYSTYPE));}
	StackEntry(int s,YYSTYPE v):state(s),val(v){}
	int state;
	YYSTYPE val;
};

class RuleManager
{
public:
	RuleManager()
	{
		clear();
	}
	void clear()
	{
		//r.assoc=r.prec=r.number=r.lhs=r.body[0]=r.action[0]=r.blen=0;
		memset(&r,0,sizeof(rule));
		precSet=false;lastTerm=0;
	}
	void setLHS(int l)
	{
		r.lhs=l;
	}
	void setPrec(int p)
	{
		precSet=true;
		r.prec=p;
	}
	void setAssoc(int a)
	{
		r.assoc=a;
	}
	void setAction(char* a)
	{
		strcpy(r.action,a);
	}
	void addSymbol(int s)
	{
		r.body[r.blen++]=s;
		if(s<NONTERM_BASE) lastTerm=s;
	}

	bool precSet;
	int lastTerm;

	rule r;
};

class Stack
{
public:
	Stack();
	~Stack();
	bool isEmpty();
	bool isFull();
	void init();
	StackEntry& getAt(int n);
	int top();//top position, not value
	void pop();
	bool push(const StackEntry& x);
	const StackEntry& peek();
private:
	StackEntry element[PARSESTACK_SIZE];
	int _top;
};

class YaccParser
{
private:
	Stack sstack;
	YYSTYPE curval;

	RuleManager addingRule;
	bool startSymbolSet; 
	bool firstNonTerm;

	int curTerm,curNonTerm;
	int curPrec;
	hash_map<string,int> symbolsMap;

	YaccScanner scanner;
	LALR1Parser& parser;
	void handleSematicAction(int rnum);

public:	
	YYSTYPE yylval;	

	YaccParser(LALR1Parser& p):parser(p),scanner(yylval){reset();}
	void parseerror(char*);
	bool yyparse(char* symbols);

	/* add other methods here */
	void reset()
	{
		curTerm=TERM_BASE;curNonTerm=NONTERM_BASE;
		curPrec=1;firstNonTerm=true;startSymbolSet=false;
		symbolsMap.clear();
		addingRule.clear();
	}
};

#endif