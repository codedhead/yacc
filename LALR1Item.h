#ifndef _YANJUN_LALR1ITEM_H_
#define _YANJUN_LALR1ITEM_H_

#include "Rule.h"
#include "Limits.h"
#include <set>
#include <hash_map>
#include <list>
#include <stack>
using namespace std;
using namespace stdext;

class LALR1ItemSet;
struct gotoEdge
{
	gotoEdge(int sb,LALR1ItemSet* n):symbol(sb),next(n){}
	int symbol;
	LALR1ItemSet* next;
};

struct RuleItem1;
struct setRuleItem1CMP;
struct setRuleItem1CMP
{
	bool operator()(const RuleItem1& first, const RuleItem1& second) const ;
};
typedef set<RuleItem1,setRuleItem1CMP>::iterator RuleItem1Iterator;

struct setRuleItem1IteratorCMP
{
	bool operator()(const RuleItem1Iterator& first,const RuleItem1Iterator& second) const ;
};
typedef set<RuleItem1Iterator,setRuleItem1IteratorCMP>::iterator RuleItem1IteratorIterator;
struct RuleItem1
{
	RuleItem1(rule& rr,int d,const set<int>& initset,RuleItem1Iterator from){r=rr;dot=d;lookaheads=initset;propagateFrom=from;}
	RuleItem1(rule& rr,int d,const set<int>& initset){r=rr;dot=d;lookaheads=initset;}
	RuleItem1(rule& rr,int d,int initla){r=rr;dot=d;lookaheads.insert(initla);}
	RuleItem1(rule& rr,int d){r=rr;dot=d;}
	RuleItem1(){dot=0;}
	rule r;
	int dot;

	
	RuleItem1Iterator propagateFrom;
	set<RuleItem1Iterator,setRuleItem1IteratorCMP> propagateTo;
	
	
	set<int> lookaheads;
	bool addLA(int la)
	{
		pair<set<int>::iterator,bool> res=lookaheads.insert(la);
		return res.second;
	}
	bool addPropagateTo(RuleItem1Iterator p)
	{
		pair<RuleItem1IteratorIterator,bool> res=propagateTo.insert(p);
		return res.second;
	}

	bool operator<(const RuleItem1& b)
	{
		return r.number<b.r.number||(r.number==b.r.number&&dot<b.dot);
	}
	bool operator==(const RuleItem1& b)
	{
		return r.number==b.r.number&&dot==b.dot;
	}
};


struct SpontaneousItem
{
	SpontaneousItem():lookahead(-1){}
	SpontaneousItem(RuleItem1Iterator rit,int la):item(rit),lookahead(la){}
	RuleItem1Iterator item;
	int lookahead;
};
typedef vector<SpontaneousItem>::iterator SpontaneousItemIterator;

class LALR1ItemSet
{
public:
	//BitSet bitset;
	set<rule,setRuleCMP>& Rules;
	int stateNum;
	set<RuleItem1,setRuleItem1CMP> items;

	///////////////////////////////////////////////////
	//bool nullable[NONTERM_MAX_COUNT];
	bool* nullable;
	set<int> firstset;
	typedef set<int>::iterator FirstIterator;
	//no loops!!!!!!!!!
// 	void _computeNullable(int nonterm);
// 	void computeNullable();
	void first(int* symbols,bool* checked);
	void first(int* symbols);
	///////////////////////////////////////////////////

	list<gotoEdge> gotoEdges;
	vector<RuleItem1Iterator> reduceItems;

	LALR1ItemSet(set<rule,setRuleCMP>& r,bool* nlb);
	LALR1ItemSet(set<rule,setRuleCMP>& r,int sn,bool* nlb);

	void clear();
	RuleItem1Iterator addItem(RuleItem1& ritem);
	void closure();
	void closure(LALR1ItemSet* dst);
};

struct setLALR1ItemSetCMP
{
	bool operator()(LALR1ItemSet* first,LALR1ItemSet* second) const 
	{   
		if(first->items.size()!=second->items.size())
			return first->items.size()<second->items.size();

		for(RuleItem1Iterator it1=first->items.begin(),it2=second->items.begin();
			it1!=first->items.end();it1++,it2++)
		{
			if((*it1).r.number<(*it2).r.number)
				return true;
			else if((*it1).r.number>(*it2).r.number)
				return false;
			else if((*it1).dot<(*it2).dot)
				return true;
			else if((*it1).dot>(*it2).dot)
				return false;
		}

		return false; 
	} 
};

typedef set<LALR1ItemSet*,setLALR1ItemSetCMP>::iterator LALR1ItemSetIterator;

#endif