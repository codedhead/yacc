#include "YaccParser.h"
#include "LALR1Parser.h"

LALR1Parser parser;
YaccParser reader(parser);

int main(int argc, char* argv[])
{
	if(argc!=2)
	{
		printf("usage: Yacc filename\n");
		return -1;
	}

	char* buffer=NULL;
	
	FILE* inf=fopen(argv[1],"r");

	if(!inf)
	{
		printf("cannot open the file\n");
		return -1;
	}

	fseek(inf,0,SEEK_END);
	unsigned long filesize=ftell(inf);
	fseek(inf,0,SEEK_SET);
	unsigned long MAXBUFFER=filesize+20;
	buffer=new char[MAXBUFFER];

	int cnt=fread(buffer,sizeof(char),MAXBUFFER,inf);
	fclose(inf);
	buffer[cnt]='\0';

	reader.yyparse(buffer);

	parser.buildLALRTable();

	for(int i=0;i<7;++i)
	{
		i=i;
	}
	parser.generate();

	delete [] buffer;

	return 0;
}