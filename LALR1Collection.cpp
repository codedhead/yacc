#include "LALR1Collection.h"

void LALR1Collection::clear()
{
	for(LALR1ItemSetIterator it=itemsets.begin();it!=itemsets.end();it++)
		delete *it;

	itemsets.clear();
	spontaneousSet.clear();
}
int* nontermsbody[100];
static bool checked[NONTERM_MAX_COUNT];
bool LALR1Collection::_computeNullable(int nonterm)
{
	if(checked[nonterm]) return false;
	// no circle!!!!!!!!!!!!!
	RuleIterator itrstart,itrend,itr;
	rule cps={nonterm+NONTERM_BASE};
	itrstart=Rules.lower_bound(cps);cps.lhs++;
	itrend=Rules.lower_bound(cps);

	int nontermsbodycnt=0;
	for(itr=itrstart;itr!=itrend;itr++)
	{
		int* p=itr->body;
		if(*p==TERMEMPTY)
		{
			nullable[nonterm]=true;
			checked[nonterm]=true;
			return true;
		}

		while(*p)
		{
			if(*p<NONTERM_BASE) break;
			++p;
		}
		if(!*p) nontermsbody[nontermsbodycnt++]=(*itr).body;
	}	
	
	if(!nontermsbodycnt)
		nullable[nonterm]=false;
	else
	{			
		nullable[nonterm]=false;
		while(nontermsbodycnt--)
		{
			int* p=nontermsbody[nontermsbodycnt];

			//if *p==nonterm can't determine, skip it
			while(*p&&*p>=NONTERM_BASE&&*p!=nonterm+NONTERM_BASE)
			{
				if(!checked[*p-NONTERM_BASE]) _computeNullable(*p-NONTERM_BASE);

				if(nullable[*p-NONTERM_BASE]) ++p;
				else break;
			}	

			if(!*p)
			{
				nullable[nonterm]=true;
				break;
			}
		}		
	}		
	
	checked[nonterm]=true;
	return true;
}
bool LALR1Collection::computeNullable()
{
	int noncnt=(*(--(Rules.end()))).lhs-NONTERM_BASE+1;
	// 	bool* checked=new bool[noncnt];
	memset(checked,0,sizeof(checked));
	memset(nullable,0,sizeof(bool)*noncnt);
	for(int i=0;i<noncnt;i++)
	{
		if(!checked[i]) _computeNullable(i);
	}
	return true;
}

int LALR1Collection::buildKernel(int stsymbol)
{
	clear();

	if(!computeNullable()) return -1;

	int stNum=0;

	LALR1ItemSet tmpClosure(Rules,nullable);

	LALR1ItemSet* initialset=new LALR1ItemSet(Rules,stNum++,nullable);
	rule augmentS={NONTERM_STARTSYMBOL,{stsymbol}};augmentS.number=0;

	spontaneousSet.push_back(SpontaneousItem(initialset->addItem(RuleItem1(augmentS,0)),TERMEOF));

	itemsets.insert(initialset);

	list<LALR1ItemSet*> workList;
	workList.push_back(initialset);
	LALR1ItemSet* theSet;

	hash_map<int,LALR1ItemSet*> hashGOTO;
	typedef hash_map<int,LALR1ItemSet*>::iterator hashGOTOIterator;

	while(!workList.empty())
	{
		theSet=workList.front();workList.pop_front();

		for(RuleItem1Iterator kernelit=theSet->items.begin();kernelit!=theSet->items.end();kernelit++)
		{
			tmpClosure.clear();tmpClosure.items.insert(RuleItem1((*kernelit).r,(*kernelit).dot,TERMVIRTUAL));
			tmpClosure.closure();

			for(RuleItem1Iterator itr=tmpClosure.items.begin();itr!=tmpClosure.items.end();itr++)
			{
				int thesb=*((*itr).r.body+(*itr).dot);

				if(thesb==0) continue;////reduce??
				else if(thesb==TERMEMPTY)
				{
					//add to itemset?
					//RuleItem1((*itr).r,(*itr).dot+1,(*itr).lookaheads)
					RuleItem1Iterator newitr=theSet->addItem(RuleItem1((*itr).r,(*itr).dot+1,(*itr).lookaheads));						

					for(set<int>::iterator lait=(*newitr).lookaheads.begin();lait!=(*newitr).lookaheads.end();lait++)
					{
						if(*lait==TERMVIRTUAL)
							(*kernelit).addPropagateTo(newitr);
						else
							spontaneousSet.push_back(SpontaneousItem(newitr,*lait));//duplicated? newadditem freed
					}

					theSet->reduceItems.push_back(newitr);
					(*newitr).lookaheads.clear();

					continue;
				}

				if(hashGOTO.find(thesb)==hashGOTO.end())
					hashGOTO.insert(pair<int,LALR1ItemSet*>(thesb,new LALR1ItemSet(Rules,nullable)));

				hashGOTO[thesb]->addItem(RuleItem1((*itr).r,(*itr).dot+1,(*itr).lookaheads,kernelit));

			}


		}	

		for(hashGOTOIterator hit=hashGOTO.begin();hit!=hashGOTO.end();hit++)
		{
			pair<LALR1ItemSetIterator,bool> res=itemsets.insert((*hit).second);
			if(res.second)
			{						
				(*(res.first))->stateNum=stNum++;
				workList.push_back(*(res.first));

				for(RuleItem1Iterator itr=(*(res.first))->items.begin();itr!=(*(res.first))->items.end();itr++)
				{
					for(set<int>::iterator lait=(*itr).lookaheads.begin();lait!=(*itr).lookaheads.end();lait++)
					{
						if(*lait==TERMVIRTUAL)
							(*((*itr).propagateFrom)).addPropagateTo(itr);
						else
							spontaneousSet.push_back(SpontaneousItem(itr,*lait));//duplicated? newadditem freed								
					}

					if(*((*itr).r.body+(*itr).dot)==0)
						(*(res.first))->reduceItems.push_back(itr);
					(*itr).lookaheads.clear();
				}		
			}	
			else
			{
				//base on the fact that two set have same order
				for(RuleItem1Iterator itrla=((*hit).second)->items.begin(),itr=(*(res.first))->items.begin();
					itrla!=((*hit).second)->items.end();itrla++,itr++)
				{
					for(set<int>::iterator lait=(*itrla).lookaheads.begin();lait!=(*itrla).lookaheads.end();lait++)
					{
						if(*lait==TERMVIRTUAL)
							(*((*itrla).propagateFrom)).addPropagateTo(itr);
						else
							spontaneousSet.push_back(SpontaneousItem(itr,*lait));//duplicated? newadditem freed

					}
// 					if(*((*itr).r.body+(*itr).dot)==0)
// 						(*(res.first))->reduceItems.push_back(itr);
					//no need,would be deleted later
					//(*itr).lookaheads.clear();
				}		

				delete (*hit).second;///////////////////////////////////////////
			}
			theSet->gotoEdges.push_back(gotoEdge((*hit).first,*(res.first)));
		}
		hashGOTO.clear();
	}

	return stNum;
}

void LALR1Collection::attachLookAhead()
{
	typedef SpontaneousItem PropagateToItem;
	typedef SpontaneousItemIterator PropagateToItemIterator;

	list<PropagateToItem> workList;

	for(SpontaneousItemIterator it=spontaneousSet.begin();it!=spontaneousSet.end();it++)
	{
		(*((*it).item)).addLA((*it).lookahead);
		workList.push_back((*it));
	}

	PropagateToItem theItem;

	while(!workList.empty())
	{
		theItem=workList.front();workList.pop_front();

		for(RuleItem1IteratorIterator itr=(*(theItem.item)).propagateTo.begin();
			itr!=(*(theItem.item)).propagateTo.end();itr++)
			if((**itr).addLA(theItem.lookahead))
				workList.push_back(PropagateToItem(*itr,theItem.lookahead));
	}
}
