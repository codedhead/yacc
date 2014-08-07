#include "LALR1Parser.h"
#include <fstream>
#include <string>
using namespace std;
#define NONASSOC   0
#define LEFTASSOC  1
#define RIGHTASSOC 2

#include "yacctokens.h"

void LALR1Parser::parseeror()
{
	printf("lr parse error\n");
}

void LALR1Parser::clear()
{	
	//YYSTYPE yylval;	
	stateCnt=0;
	valTypeMap.clear();
	valTypeCnt=0;
	termCnt=0;
	startSymbol=NONTERM_BASE;

	memset(stateSymbolMap,0,sizeof(stateSymbolMap));

	memset(rawRules,0,sizeof(rawRules));

	memset(precTable,0,sizeof(precTable));
	memset(assocTable,0,sizeof(assocTable));	

	memset(valTypes,0,sizeof(valTypes));	
	memset(symbolsTypeTable,0,sizeof(symbolsTypeTable));

	memset(Action,0,sizeof(Action));
	memset(Goto,0,sizeof(Goto));
}

void LALR1Parser::addValType(const char* type)
{
	ValTypeMapIterator it=valTypeMap.find(type);
	if(it==valTypeMap.end())
	{
		valTypeMap.insert(pair<string,int>(type,valTypeCnt));
		strcpy(valTypes[valTypeCnt],type);valTypeCnt++;
		//strcpy(valTypesType[valTypeCnt++],typetype);
	}
}

void LALR1Parser::setValType(int symbol,const char* type)
{	
	int t=0;
	ValTypeMapIterator it=valTypeMap.find(type);
	if(it!=valTypeMap.end())
		t=(*it).second;

	symbolsTypeTable[symbol]=t;
}

void LALR1Parser::setToRuleArray()
{
	RuleIterator it=Rules.begin();
	int i=0;

	//memset(rawRules,0,sizeo

	for(;it!=Rules.end();it++)
		rawRules[(*it).number]=it;
	//rawRules[i].lhs=0;
	//ruleCnt=i;
}

void LALR1Parser::printRule(int rnum,int dot)
{
	RuleIterator rit=rawRules[rnum];
	printf("%s ->",allNonterms[rit->lhs-NONTERM_BASE]);
	if(rit->body[0]==TERMEMPTY)
	{
		printf(" .\n");			
	}
	else
	{
		if(dot==-1) dot=rit->blen;
		int i,*p=rit->body;
		for(i=0;i<dot;++i,++p)
		{
			if(*p<TERMEOF)
				printf(" %c",*p);
			else if(*p<NONTERM_BASE)
				printf(" %s",allTerms[*p-TERM_BASE]);
			else
				printf(" %s",allNonterms[*p-NONTERM_BASE]);
		}
		printf(" .");
		while(*p)
		{
			if(*p<TERMEOF)
				printf(" %c",*p);
			else if(*p<NONTERM_BASE)
				printf(" %s",allTerms[*p-TERM_BASE]);
			else
				printf(" %s",allNonterms[*p-NONTERM_BASE]);
			++p;
		}
		printf("\n");
	}
}
ActionEntry& LALR1Parser::resolveConflict(ActionEntry& org,ActionEntry& cur,int symbol,LALR1ItemSet* shiftset)
{
	if(org.action=='r')
	{
		if(cur.action=='r')
		{
#define PREFER_ORG {printf("prefer 1.\n\n");return org;}
#define PREFER_CUR {printf("prefer 2.\n\n");return cur;}

			printf("reduce reduce conflict on symbol: ");
			if(symbol==TERMEOF) puts("$");
			else if(symbol<TERMEOF) printf("%c\n",(char)symbol);
			else printf("%s\n",allTerms[symbol-TERM_BASE]);
			printf("1) ");printRule(org.param,-1);
			printf("2) ");printRule(cur.param,-1);
			if((*rawRules[org.param]).prec<(*rawRules[cur.param]).prec)
				PREFER_CUR
			else if((*rawRules[cur.param]).prec<(*rawRules[org.param]).prec)
				PREFER_ORG
			else if(org.param<cur.param)
				PREFER_ORG
			else if(cur.param<org.param)
				PREFER_CUR
			else PREFER_CUR
		}
		else if(cur.action=='s')
		{
			printf("reduce reduce conflict on symbol: ");
			if(symbol==TERMEOF) puts("$");
			else if(symbol<TERMEOF) printf("%c\n",(char)symbol);
			else printf("%s\n",allTerms[symbol-TERM_BASE]);

			int i=1;
			printf("%d) ",i++);printRule(org.param,-1);
			
			for(RuleItem1Iterator riit=shiftset->items.begin();riit!=shiftset->items.end();++riit)
			{printf("%d) ",i++);printRule(riit->r.number,riit->dot-1);}

#define PREFER_SHIFT {printf("prefer shift.\n\n");return cur;}
#define PREFER_REDUCE {printf("prefer reduce.\n\n");return org;}

			if((*rawRules[org.param]).prec<precTable[symbol])
				PREFER_SHIFT
			else if((*rawRules[org.param]).prec>precTable[symbol])
				PREFER_REDUCE
			else if((*rawRules[org.param]).assoc==LEFTASSOC)//or assocTable[symbol]?
				PREFER_REDUCE
			else if((*rawRules[org.param]).assoc==RIGHTASSOC)
				PREFER_SHIFT
			else PREFER_SHIFT
		}
	}
	else if(org.action=='s')
	{
		//impossible
		if(cur.action=='r')
		{
			puts("sr conflic!");
			if((*rawRules[cur.param]).prec<precTable[symbol])
				return org;
			else if((*rawRules[cur.param]).prec>precTable[symbol])
				return cur;
			else if((*rawRules[cur.param]).assoc==LEFTASSOC)
				return cur;
			else if((*rawRules[cur.param]).assoc==RIGHTASSOC)
				return org;
		}
		return org;//prefer shift
	}

	return cur;
}

void LALR1Parser::buildLALRTable()
{
	memset(Action,0,sizeof(Action));
	memset(Goto,0,sizeof(Goto));

	setToRuleArray();

	stateCnt=lalr1collection.buildKernel(startSymbol);
	lalr1collection.attachLookAhead();

	for(LALR1ItemSetIterator it=lalr1collection.itemsets.begin();it!=lalr1collection.itemsets.end();it++)
	{
		for(vector<RuleItem1Iterator>::iterator ri1it=(*it)->reduceItems.begin();ri1it!=(*it)->reduceItems.end();ri1it++)
		{				
			if((**ri1it).r.number==0)
				Action[(*it)->stateNum][TERMEOF].action='a';
			else
			{
				for(set<int>::iterator itw=(**ri1it).lookaheads.begin();itw!=(**ri1it).lookaheads.end();itw++)
				{			
					Action[(*it)->stateNum][*itw]=resolveConflict(Action[(*it)->stateNum][*itw],ActionEntry('r',(**ri1it).r.number),*itw,NULL);
// 					Action[(*it)->stateNum][*itw].action='r';
// 					Action[(*it)->stateNum][*itw].param=(**ri1it).r.number;
				}		
			}					
		}

		for(list<gotoEdge>::iterator itge=(*it)->gotoEdges.begin();itge!=(*it)->gotoEdges.end();itge++)
		{
			stateSymbolMap[(*it)->stateNum]=itge->symbol;//[0]?

			if(itge->symbol>=NONTERM_BASE)
				Goto[(*it)->stateNum][itge->symbol-NONTERM_BASE]=itge->next->stateNum;
			else
			{
// 				Action[(*it)->stateNum][itge->symbol].action='s';
// 				Action[(*it)->stateNum][itge->symbol].param=itge->next->stateNum;
				Action[(*it)->stateNum][itge->symbol]=resolveConflict(Action[(*it)->stateNum][itge->symbol],ActionEntry('s',itge->next->stateNum),itge->symbol,itge->next);
			}
		}

	}
}

LALR1Parser::LALR1Parser():lalr1collection(Rules)
{		
	clear();
}
void LALR1Parser::readRules(rule* R,int rc)
{		
	int lastTerm=0;
	for(int i=0;i<rc;i++)
	{		
		R[i].number=i+1;
		R[i].blen=0;
		R[i].assoc=R[i].prec=0;
		int* p=R[i].body;
		while(*p)
		{
			if(*p==YACC_PREC)
			{
				R[i].prec=precTable[*p];
				R[i].assoc=assocTable[*p];
				break;
			}
			else if(*p<NONTERM_BASE)
				lastTerm=*p;
			p++;R[i].blen++;//termempty
		}
		if(*p==0)
		{
			R[i].prec=precTable[lastTerm];
			R[i].assoc=assocTable[lastTerm];
		}
		Rules.insert(R[i]);
	}

	setToRuleArray();

	//////////////////////////////////////////////////////////////////////////
	// 	computeNullable();
	// 	first((*rawRules[18]).body);

	buildLALRTable();
}

void LALR1Parser::generate()
{
	ofstream outf("_parser.h");
	ifstream inf("PARSER_H_TEMPLATE");
	string buffer;
	if(inf.is_open())
	{
		while(!inf.eof())
		{
			getline(inf,buffer);
			outf<<buffer<<endl;
		}		
	}
	else puts("missing file: PARSER_H_TEMPLATE");

	inf.close();inf.clear();
	outf.close();

	//output terminals
	if(termCnt)
	{
		outf.open("_tokens.h");
		outf<<"#ifndef _YANJUN_TOKEN_H_"<<endl;
		outf<<"#define _YANJUN_TOKEN_H_"<<endl;
		outf<<"#include \"limits.h\""<<endl;
		outf<<"enum TERMINALS{"<<endl;
		outf<<allTerms[0]<<"=TERM_BASE,";
		for(int i=1;i<termCnt;i++)
			outf<<allTerms[i]<<',';
		outf<<"\n};\n";
		outf<<"#endif";
		outf.close();
	}
/*
	//output nonterminals
	if(nontermCnt)
	{
		outf.open("_nttokens.h");
		outf<<"#ifndef _YANJUN_NTTOKEN_H_"<<endl;
		outf<<"#define _YANJUN_NTTOKEN_H_"<<endl;
		outf<<"#include \"yacclimits.h\""<<endl;
		outf<<"enum NONTERMINALS{"<<endl;
		outf<<allNonterms[0]<<"=NONTERM_BASE,";
		for(int i=1;i<nontermCnt;i++)
			outf<<allNonterms[i]<<',';
		outf<<"\n};\n";
		outf<<"#endif";
		outf.close();
	}

*/
	outf.open("_parser.cpp");

	outf<<"#include \"_parser.h\""<<endl;
	outf<<"#include <fstream>"<<endl;
	outf<<"using namespace std;"<<endl;
	outf<<endl;

	inf.open("_yblock1");
	if(inf.is_open())
	{
		while(!inf.eof())
		{
			getline(inf,buffer);
			outf<<buffer<<endl;
		}
		inf.close();inf.clear();
		remove("_yblock1");
	}	
	else{inf.close();inf.clear();}

	outf<<"struct Action"<<endl;
	outf<<'{'<<endl;
	outf<<"\tchar action;"<<endl;
	outf<<"\tint param;"<<endl;
	outf<<"};"<<endl;
	outf<<"struct srule"<<endl;
	outf<<'{'<<endl;
	outf<<"\tint lhs;"<<endl;
	outf<<"\tint body[RULEBODY_MAX_LENGTH];"<<endl;
	outf<<"\tint blen;"<<endl;
	outf<<"};"<<endl;

	outf<<"Action parseAction[][NONTERM_BASE]={"<<endl;
	for(int i=0;i<stateCnt;i++)
	{
		outf<<"\t{";
		for(int j=0;j<NONTERM_BASE;j++)
		{
			if(Action[i][j].action=='r'||Action[i][j].action=='s'||Action[i][j].action=='a')
				outf<<"{'"<<Action[i][j].action<<"',"<<Action[i][j].param<<"},";
			else outf<<"{0,0},";
		}
		outf<<"},"<<endl;
	}
	outf<<"};"<<endl;

	outf<<"int parseGoto[][NONTERM_MAX_COUNT]={"<<endl;
	for(int i=0;i<stateCnt;i++)
	{
		outf<<"\t";
		for(int j=0;j<NONTERM_MAX_COUNT;j++)
		{
			outf<<Goto[i][j]<<',';
		}
		outf<<endl;
	}
	outf<<"};"<<endl;

	int ruleCnt=Rules.size();
	char posbuffer[10],typebuffer[50];

	outf<<"srule Rules[]={{0,{0}},"<<endl;
	for(int i=1;i<=ruleCnt;i++)
	{
		outf<<"\t{"<<(*rawRules[i]).lhs-NONTERM_BASE<<",{";
		int* p=(*rawRules[i]).body;
		while(*p)
			outf<<*(p++)<<',';
		outf<<"},"<<(*rawRules[i]).blen<<"},"<<endl;
	}
	outf<<"};"<<endl;

	//outf<<"void Parser::handleSematicAction(int rnum)"<<endl;
	

	/* help print */
	//output terminals
	if(termCnt)
	{		
		outf<<"char termString[][50]={\n";
		for(int i=0;i<termCnt;i++)
			outf<<'"'<<allTerms[i]<<'"'<<',';
		outf<<"\n};\n";
	}

	//output nonterminals
	if(nontermCnt)
	{		
		outf<<"char nontermString[][50]={\n";
		for(int i=0;i<nontermCnt;i++)
			outf<<'"'<<allNonterms[i]<<'"'<<',';
		outf<<"\n};\n";
	}


	outf<<"bool Parser::yyparse(char* symbols)\n{\n";

	inf.open("_yblock2");
	if(inf.is_open())
	{
		while(!inf.eof())
		{
			getline(inf,buffer);
			outf<<buffer<<endl;
		}
		inf.close();inf.clear();
		remove("_yblock2");
	}	
	else{inf.close();inf.clear();}

	inf.open("PARSER_CPP_TEMPLATE");
	if(inf.is_open())
	{
		while(!inf.eof())
		{
			getline(inf,buffer);
			if(buffer[0]=='$') break;
			outf<<buffer<<endl;
		}

		outf<<"\t\t\t\t{\n";
		outf<<"\t\t\t\t\tint stackstartpos=sstack.top()-Rules[rnum].blen;"<<endl;
		outf<<"\t\t\t\t\tswitch(rnum)"<<endl;
		outf<<"\t\t\t\t\t{"<<endl;

		for(RuleIterator rit=Rules.begin();rit!=Rules.end();rit++)
		{
			char* p=(*rit).action;	

			if((*rit).body[0]==TERMEMPTY&&*p==0) continue;

			if(*p==0)
			{
				//default action
				if(symbolsTypeTable[(*rit).lhs]||symbolsTypeTable[(*rit).body[0]])
				{
					outf<<"\t\t\t\t\tcase "<<(*rit).number<<":\n\t\t\t\t\t\t";
					outf<<"curval."<<valTypes[symbolsTypeTable[(*rit).lhs]]
					<<"=sstack.getAt(stackstartpos+"<<1<<").val."
						<<valTypes[symbolsTypeTable[(*rit).body[0]]]<<";";
					outf<<"\n\t\tbreak;"<<endl;
				}
				//$$=$1;
			}
			else
			{
				outf<<"\t\t\t\t\tcase "<<(*rit).number<<":\n\t\t\t\t\t\t";
				while(*p)
				{
					if(*p=='"'||*p=='\'')
					{
						char c=*p;
						outf<<*(p++);
						while(true)
						{
							while(*p!=c)
								outf<<*(p++);
							outf<<*(p++);
							if(*(p-2)!='\\') break;
						}
					}
					else if(*p=='$')
					{
						//must follow the format, or the result is undefined
						char* q;p++;
						typebuffer[0]='\0';

						if(*p=='<')
						{
							q=typebuffer;
							p++;
							while(*p!='>')
								*(q++)=*(p++);
							p++;*q='\0';
							//no checking here
						}
						q=posbuffer;

						if(*p=='$')
						{
							if(typebuffer[0])
								outf<<"curval."<<typebuffer;
							else
								outf<<"curval."<<valTypes[symbolsTypeTable[(*rit).lhs]];
							p++;
						}
						else
						{
							bool bnegate=false;
							if(*p=='-'){bnegate=true;p++;}
							while(isdigit(*p))
								*(q++)=*(p++);
							//if(q==posbuffer){ERROR,expect for a non-negate number}
							*q='\0';
							int pos=atoi(posbuffer);if(bnegate) pos=-pos;

							//if(pos==0||pos>(*rit).blen){ERROR,exceed stack}

							if(typebuffer[0])
								outf<<"sstack.getAt(stackstartpos+"<<pos<<").val."<<typebuffer;
							else if(pos>0)
								outf<<"sstack.getAt(stackstartpos+"<<pos<<").val."<<valTypes[symbolsTypeTable[(*rit).body[pos-1]]];
							else//if pos<0 and type is not defined, use default, should report it
								outf<<"sstack.getAt(stackstartpos+"<<pos<<").val."<<valTypes[0];
						}
					}
					else
						outf<<*(p++);
				}
				outf<<"\n\t\t\t\t\t\tbreak;"<<endl;
			}		

		}
		outf<<"\t\t\t\t\tdefault:\n";
		outf<<"\t\t\t\t\t\tcurval=sstack.getAt(stackstartpos+1).val;\n\t\t\t\t\t\tbreak;\n";	
		outf<<"\t\t\t\t\t}"<<endl;
		outf<<"\t\t\t\t}"<<endl;

		while(!inf.eof())
		{
			getline(inf,buffer);
			outf<<buffer<<endl;
		}
	}
	else puts("missing file: PARSER_CPP_TEMPLATE");

	inf.close();inf.clear();

	inf.open("_yblock3");
	if(inf.is_open())
	{
		while(!inf.eof())
		{
			getline(inf,buffer);
			outf<<buffer<<endl;
		}
		inf.close();inf.clear();
		remove("_yblock3");
	}	
	else{inf.close();inf.clear();}

	outf.close();
}
