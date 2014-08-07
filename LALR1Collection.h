#ifndef _YANJUN_LALR1COLLECTION_H_
#define _YANJUN_LALR1COLLECTION_H_
#include "LALR1Item.h"

class LALR1Collection
{
private:
	bool nullable[NONTERM_MAX_COUNT];
public:
	set<rule,setRuleCMP>& Rules;
	set<LALR1ItemSet*,setLALR1ItemSetCMP> itemsets;

	vector<SpontaneousItem> spontaneousSet;//[LALR1STATE_MAX_COUNT];

	LALR1Collection(set<rule,setRuleCMP>& R):Rules(R){}

	void clear();
	bool _computeNullable(int nonterm);
	bool computeNullable();
	int buildKernel(int stsymbol);
	void attachLookAhead();
};


#endif