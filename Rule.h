#ifndef _YANJUN_RULE_H
#define _YANJUN_RULE_H

#include "Limits.h"
#include <set>
using namespace std;

struct rule
{
	int lhs;
	int body[RULEBODY_MAX_LENGTH];
	char action[RULEACTION_MAX_LENGTH];
	int number;		
	int prec;
	int assoc;
	int blen;	
};

struct setRuleCMP
{
	bool operator()(const rule& first, const rule& second) const 
	{   
		if(first.lhs!=second.lhs) return first.lhs<second.lhs;

		int* p1=(int*)(first.body),*p2=(int*)(second.body);
		while(*p1&&*p2&&*p1==*p2) p1++,p2++;

		if(*p1==0&&*p2==0) return false;

		return *p1<*p2; 
	} 
};

typedef set<rule,setRuleCMP>::iterator RuleIterator;

#endif