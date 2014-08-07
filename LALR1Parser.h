#ifndef _YANJUN_LALR1PASER_H_
#define _YANJUN_LALR1PASER_H_

#include "Rule.h"

#include <set>
#include <hash_map>
#include <list>
#include <stack>
#include <string>
using namespace std;
using namespace stdext;

#include "LALR1Item.h"
#include "LALR1Collection.h"

#include "YYSTYPE.h"

typedef hash_map<string,int>::iterator ValTypeMapIterator;

struct ActionEntry
{
	ActionEntry(char a,int p):action(a),param(p){}
	ActionEntry()
	{
		action=param=0;
	}
	char action;
	int param;
};


class YaccParser;
class LALR1Parser
{
	friend class YaccParser;
private:	
	set<rule,setRuleCMP> Rules;
	RuleIterator rawRules[RULE_MAX_COUNT];
	//int ruleCnt;
	int stateCnt;

	int startSymbol;

	int precTable[NONTERM_BASE];
	int assocTable[NONTERM_BASE];

	YYSTYPE yylval;
	char valTypes[VALTYPE_MAX_COUNT][VALTYPE_MAX_LENGTH];
	//char valTypesType[VALTYPE_MAX_COUNT][50];
	hash_map<string,int> valTypeMap;
	int valTypeCnt;
	int symbolsTypeTable[NONTERM_BASE+NONTERM_MAX_COUNT];

	int termCnt;
	char allTerms[NONTERM_BASE][TERMNAME_MAX_LENGTH];

	int nontermCnt;
	char allNonterms[NONTERM_MAX_COUNT][TERMNAME_MAX_LENGTH];


	int stateSymbolMap[LALR1STATE_MAX_COUNT];

	ActionEntry Action[LALR1STATE_MAX_COUNT][NONTERM_BASE];
	int Goto[LALR1STATE_MAX_COUNT][NONTERM_MAX_COUNT];

	LALR1Collection lalr1collection;

	ActionEntry& LALR1Parser::resolveConflict(ActionEntry& org,ActionEntry& cur,int symbol,LALR1ItemSet* shiftset);

	void parseeror();
	void setToRuleArray();

	void clear();

	void printRule(int rnum,int dot);


/*
	///////////////////////////////////////////////////
	bool nullable[NONTERM_MAX_COUNT];
	set<int> firstset;
	typedef set<int>::iterator FirstIterator;
	//no loops!!!!!!!!!
	void _computeNullable(int nonterm,bool* checked)
	{
		if(checked[nonterm-NONTERM_BASE]) return;
		RuleIterator itrstart,itrend,itr;
		rule cps={nonterm};
		itrstart=Rules.lower_bound(cps);cps.lhs++;
		itrend=Rules.lower_bound(cps);
		for(itr=itrstart;itr!=itrend;itr++)
		{
			int* p=(*itr).body;
			if(*p==TERMEMPTY)
			{
				nullable[nonterm-NONTERM_BASE]=true;
				checked[nonterm-NONTERM_BASE]=true;
				return;
			}

			while(*p&&*p>=NONTERM_BASE&&*p!=nonterm)
			{
				if(!checked[*p-NONTERM_BASE]) _computeNullable(*p,checked);

				if(nullable[*p-NONTERM_BASE]) p++;
				else break;
			}	

			if(*p=='\0')
			{
				nullable[nonterm-NONTERM_BASE]=true;
				checked[nonterm-NONTERM_BASE]=true;
				return;
			}
		}
		checked[nonterm-NONTERM_BASE]=true;
	}

	void computeNullable()
	{
		int noncnt=(*(--(Rules.end()))).lhs-NONTERM_BASE+1;
		bool* checked=new bool[noncnt];
		memset(checked,0,sizeof(bool)*noncnt);
		memset(nullable,0,sizeof(bool)*noncnt);
		for(int i=0;i<noncnt;i++)
		{
			if(!checked[i])
				_computeNullable(i+NONTERM_BASE,checked);
		}
		delete [] checked;
	}

	void first(int* symbols,bool* checked,bool firstiter=false)
	{
		int initnonterm=*symbols;
		while(*symbols>=NONTERM_BASE)
		{
			if(checked[*symbols-NONTERM_BASE])
			{
				if(nullable[*symbols-NONTERM_BASE])
				{
					symbols++;continue;
				}
				else
					return;
			}

			checked[*symbols-NONTERM_BASE]=true;

			RuleIterator itrstart,itrend,itr;
			rule cps={symbols[0]};
			itrstart=Rules.lower_bound(cps);cps.lhs++;
			itrend=Rules.lower_bound(cps);

			for(itr=itrstart;itr!=itrend;itr++)
				first((*itr).body,checked);				

			if(!nullable[*symbols-NONTERM_BASE]) return;
			symbols++;
		}

		if(firstiter&&(*symbols==0||*symbols==TERMEMPTY))
			firstset.insert(TERMEMPTY);
		//if(*symbols==0)
		//if(nullable[*symbols-NONTERM_BASE])
		//firstset.insert(TERMEMPTY);

		//exclude TERMEMPTY
		if(symbols[0]>0&&symbols[0]<NONTERM_BASE&&symbols[0]!=TERMEMPTY)
			firstset.insert(symbols[0]);
	}

	void first(int* symbols)
	{
		int noncnt=(*(--(Rules.end()))).lhs-NONTERM_BASE+1;
		bool* checked=new bool[noncnt];
		memset(checked,0,sizeof(bool)*noncnt);
		firstset.clear();

		first(symbols,checked,true);

		delete [] checked;
	}
*/


public:

	void buildLALRTable();

	void addRule(rule& r)
	{
		int num=Rules.size()+1;
		r.number=num;
		Rules.insert(r);
	}

	void setStartSymbol(int i){startSymbol=i;}

	void addTerminal(const char* term)
	{
		strcpy(allTerms[termCnt++],term);
	}

	void addNonTerminal(const char* nonterm)
	{
		strcpy(allNonterms[nontermCnt++],nonterm);
	}

	void addValType(const char* type);
	//void addValType(const char* typetype,const char* type);

	void setValType(int symbol,const char* type);
	void setPrec(int symbol,int prec){precTable[symbol]=prec;}
	void setAssoc(int symbol,int assoc){assocTable[symbol]=assoc;}
	void readRules(rule* R,int rc);
	LALR1Parser();
	//bool parse(int symbols[]);
	void generate();
};

#endif