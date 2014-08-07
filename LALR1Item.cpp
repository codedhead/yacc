#include "LALR1Item.h"

bool setRuleItem1CMP::operator()(const RuleItem1& first, const RuleItem1& second) const 
{   	
	//assume that differt lookaheads share one item
	return first.r.number<second.r.number||(first.r.number==second.r.number&&first.dot<second.dot);
	// 		if(first.r.number!=second.r.number) return first.r.number<second.r.number;
	// 		if(first.dot!=second.dot) return first.dot<second.dot;
	// 		if(first.lacnt!=second.lacnt) return first.lacnt<second.lacnt;
	// 
	// 		for(int i=0;i<first.lacnt;i++)
	// 			if(first.lookaheads[i]!=second.lookaheads[i])
	// 				return first.lookaheads[i]<second.lookaheads[i];
	// 
	// 		return false; 
} 

bool setRuleItem1IteratorCMP::operator()(const RuleItem1Iterator& first,const RuleItem1Iterator& second) const 
{   		
	return *first<*second;
} 

void LALR1ItemSet::first(int* symbols,bool* checked)
{
	while(*symbols)
	{
		if(*symbols>=NONTERM_BASE)
		{
			if(checked[*symbols-NONTERM_BASE])
			{
				if(nullable[*symbols-NONTERM_BASE])
				{
					++symbols;continue;
				}
				else break;
			}

			checked[*symbols-NONTERM_BASE]=true;

			RuleIterator itrstart,itrend,itr;
			rule cps={symbols[0]};
			itrstart=Rules.lower_bound(cps);cps.lhs++;
			itrend=Rules.lower_bound(cps);

			for(itr=itrstart;itr!=itrend;itr++)
				first((*itr).body,checked);

			if(!nullable[*symbols]) break;
		}
		else
		{
			firstset.insert(*symbols);break;
		}
		++symbols;
	}
}

void LALR1ItemSet::first(int* symbols)
{
	int noncnt=(*(--(Rules.end()))).lhs-NONTERM_BASE+1;
	bool* checked=new bool[noncnt];
	memset(checked,0,sizeof(bool)*noncnt);
	firstset.clear();

	bool ep=true;
	while(*symbols)
	{
		if(*symbols>=NONTERM_BASE)
		{
			if(checked[*symbols-NONTERM_BASE])
			{
				if(nullable[*symbols-NONTERM_BASE])
				{
					++symbols;continue;
				}
				else{ep=false;break;}
			}

			checked[*symbols-NONTERM_BASE]=true;

			RuleIterator itrstart,itrend,itr;
			rule cps={symbols[0]};
			itrstart=Rules.lower_bound(cps);cps.lhs++;
			itrend=Rules.lower_bound(cps);

			for(itr=itrstart;itr!=itrend;itr++)
				first((*itr).body,checked);

			if(!nullable[*symbols]) break;
		}
		else
		{
			firstset.insert(*symbols);ep=false;break;
		}
		++symbols;
	}
	if(ep) firstset.insert(TERMEMPTY);

	delete [] checked;
}

// 	void first(int* symbols)
// 	{
// 		int noncnt=(*(--(Rules.end()))).lhs-NONTERM_BASE+1;
// 		bool* checked=new bool[noncnt];
// 		memset(checked,0,sizeof(bool)*noncnt);
// 		firstset.clear();
// 
// 		stack<int*> workStack;
// 		int* thesymbol;
// 		workStack.push(symbols);
// 		while(!workStack.empty())
// 		{
// 			thesymbol=workStack.top();workStack.pop();
// 
// 			if(thesymbol[0]==0)
// 				firstset.insert(TERMEMPTY);
// 			else if(thesymbol[0]==TERMEMPTY)
// 				workStack.push(thesymbol+1);
// 			else if(thesymbol[0]>0&&thesymbol[0]<NONTERM_BASE)
// 				firstset.insert(thesymbol[0]);
// 			else if(thesymbol[0]>=NONTERM_BASE&&!checked[thesymbol[0]-NONTERM_BASE])
// 			{
// 				RuleIterator itrstart,itrend,itr;
// 				itrstart=Rules.lower_bound(rule(thesymbol[0],0));
// 				itrend=Rules.lower_bound(rule(thesymbol[0]+1,0));
// 
// 				for(itr=itrstart;itr!=itrend;itr++)
// 					workStack.push((*itr).body);
// 
// 				checked[thesymbol[0]-NONTERM_BASE]=true;
// 			}	
// 		}	
// 
// 		delete [] checked;
// 	}
// 
	list<gotoEdge> gotoEdges;
	vector<RuleItem1Iterator> reduceItems;

LALR1ItemSet::LALR1ItemSet(set<rule,setRuleCMP>& r,bool* nlb):Rules(r),stateNum(-1),nullable(nlb){}
LALR1ItemSet::LALR1ItemSet(set<rule,setRuleCMP>& r,int sn,bool* nlb):Rules(r),stateNum(sn),nullable(nlb){}

void LALR1ItemSet::clear()
{
	items.clear();
	gotoEdges.clear();
	reduceItems.clear();
}

RuleItem1Iterator LALR1ItemSet::addItem(RuleItem1& ritem)
{
	pair<RuleItem1Iterator,bool> res=items.insert(ritem);
//	bitset.set(ritem.r.number);

	return res.first;
}

// void LALR1ItemSet::closure()
// {
// 	list<RuleItem1Iterator> workList;
// 	//workList.insert(workList.begin(),items.begin(),items.end());
// 
// 	RuleItem1Iterator it;
// 	for(it=items.begin();it!=items.end();it++)
// 		workList.push_back(it);
// 
// 	rule cmpRule;
// 
// 	while(!workList.empty())
// 	{
// 		it=workList.front();workList.pop_front();
// 		int* p=(*it).r.body;
// 		int sb=*(p+(*it).dot);
// 		if(sb>=NONTERM_BASE)
// 		{
// 			first(p+(*it).dot+1);
// 			cmpRule.lhs=sb;cmpRule.body[0]=0;
// 			RuleIterator itr,itrstart,itrend;
// 			itrstart=Rules.lower_bound(cmpRule);
// 			cmpRule.lhs++;
// 			itrend=Rules.lower_bound(cmpRule);
// 			for(itr=itrstart;itr!=itrend;itr++)
// 			{
// 				RuleItem1 newruleitem(*itr,0);
// 
// 				pair<RuleItem1Iterator,bool> rt=items.insert(newruleitem);
// 
// 				for(FirstIterator fit=firstset.begin();fit!=firstset.end();fit++)
// 				{
// 					if(*fit==TERMEMPTY)
// 						(*(rt.first)).lookaheads.insert((*it).lookaheads.begin(),(*it).lookaheads.end());
// 					else
// 						(*(rt.first)).addLA(*fit);
// 				}
// 
// 				if(rt.second)
// 				{
// //					bitset.set((*itr).number);
// 					workList.push_back(rt.first);
// 				}
// 			}
// 		}			
// 	}
// }

void LALR1ItemSet::closure()
{
	list<RuleItem1Iterator> workList;
	//workList.insert(workList.begin(),items.begin(),items.end());

	RuleItem1Iterator it;
	for(it=items.begin();it!=items.end();it++)
		workList.push_back(it);

	rule cmpRule;

	while(!workList.empty())
	{
		it=workList.front();workList.pop_front();
		int* p=(*it).r.body;
		int sb=*(p+(*it).dot);
		if(sb>=NONTERM_BASE)
		{
			first(p+(*it).dot+1);
			cmpRule.lhs=sb;cmpRule.body[0]=0;
			RuleIterator itr,itrstart,itrend;
			itrstart=Rules.lower_bound(cmpRule);
			cmpRule.lhs++;
			itrend=Rules.lower_bound(cmpRule);
			for(itr=itrstart;itr!=itrend;itr++)
			{
				RuleItem1 newruleitem(*itr,0);
				int bupdate=0;

				pair<RuleItem1Iterator,bool> rt=items.insert(newruleitem);

				int lastsize=(*(rt.first)).lookaheads.size();
				for(FirstIterator fit=firstset.begin();fit!=firstset.end();fit++)
				{
					if(*fit==TERMEMPTY)
						(*(rt.first)).lookaheads.insert((*it).lookaheads.begin(),(*it).lookaheads.end());
					else
						(*(rt.first)).addLA(*fit);
				}
				bupdate+=(*(rt.first)).lookaheads.size()>lastsize;

				if(rt.second||bupdate)
				{
					//					bitset.set((*itr).number);
					workList.push_back(rt.first);
				}
			}
		}			
	}
}


void LALR1ItemSet::closure(LALR1ItemSet* dst)
{
	dst->items.clear();
	dst->items.insert(items.begin(),items.end());
	dst->closure();
}



