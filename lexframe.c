/****************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lex.h"

static FILE* pIF;			/* Quellfile 				*/
static tMorph MorphInit;		/* Morphem   				*/
static signed char X;			/* Aktuelles Eingabezeichen 		*/
static int    Z;			/* Aktueller Zustand des Automaten 	*/
static char   vBuf[128+1],*pBuf;	/* Ausgabepuffer */
static int    Ende;			/* Flag 				*/
static int cur_column;
static int cur_line;
tMorph Morph;

/*---- Initialisierung der lexikalischen Analyse ----*/
int initLex(char* fname)
{
	pIF=fopen(fname,"r+t");
	if (pIF)
	{
		X=fgetc(pIF); //lies erstes zeichen und lege es unter X ab
		cur_column=0;
		cur_line=0;
	}
	Morph.MC=mcEmpty;
	return (int)pIF;
}

/* Zeichenklassenvektor */

/*Klassen:
 * 0: Sonderzeichen
 * 1:Ziffer
 * 2:Buchstabe
 * 3:':'
 * 4:'='
 * 5:'<'
 * 6:'>'
 * 7:Sonstige
 * 8:'('
 * 9:'*'
 * 10:')'
 * 11:"
 */
 
 /*Erweiterung HEX-Zahlen
 * 0: Sonderzeichen
 * 1: Ziffer: 0
 * 2: Ziffer: 1-9
 * 3: Hexbuchstabe (A-F)
 * 4: Buchstaben G-Z (ohne X)
 * 5: Buchstabe x
 * 6:':'
 * 7:'='
 * 8:'<'
 * 9:'>'
 * 10:Sonstige
 * 11:'('
 * 12:'*'
 * 13:')'
 * 14:"
 * 15:'/'
 */
 
static char vZKl[128]=
{
	/*	0    1   2   3   4   5   6   7   8   9   A   B   C   D   E  F*/
		10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,/* 0*/
		10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,/*10*/
		10, 0 , 14, 0 , 0 , 0 , 0 , 0 , 11, 13, 12, 0 , 0 , 0 , 0 , 15,/*20*/
		1 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 6 , 0 , 8 , 7 , 9 , 0 ,/*30*/
		0 , 3 , 3 , 3 , 3 , 3 , 3 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 ,/*40*/
		4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 0 , 0 , 0 , 0 , 0 ,/*50*/
		0 , 3 , 3 , 3 , 3 , 3 , 3 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 ,/*60*/
		4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 5 , 4 , 4 , 0 , 0 , 0 , 0 , 16 /*70*/
};

/* Automatentabelle */

static char vSMatrix[13][17]=
/*         So      Zi(0) Zi(1-9)  HexBu  Bu(G-Z) Bu(x)   ':'     '='     '<'     '>'    Space    '('      '*'       ')'     '"'      /    EOT(EOF) */
/* 0 */{{0+ifslb,7+ifsl ,1+ifsl ,2+ifgl ,2+ifgl ,2+ifgl ,3+ifsl ,0+ifslb,4+ifsl ,5+ifsl ,0+ifl  ,0+ifslb ,0+ifslb ,0+ifslb ,6+ifl  ,9+ifsl ,0+ifb}, /*Startzustand*/
/* 1 */ {0+ifb  ,1+ifsl ,1+ifsl ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb   ,0+ifb   ,0+ifb   ,0+ifb  ,0+ifb  ,0+ifb}, /*Zahl*/
/* 2 */ {0+ifb  ,2+ifsl ,2+ifsl ,2+ifgl ,2+ifgl ,2+ifgl ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb   ,0+ifb   ,0+ifb   ,0+ifb  ,0+ifb  ,0+ifb}, /*Bezeichner*/
/* 3 */ {0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifslb,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb   ,0+ifb   ,0+ifb   ,0+ifb  ,0+ifb  ,0+ifb}, /*:*/
/* 4 */ {0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifslb,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb   ,0+ifb   ,0+ifb   ,0+ifb  ,0+ifb  ,0+ifb}, /*<*/
/* 5 */ {0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifslb,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb   ,0+ifb   ,0+ifb   ,0+ifb  ,0+ifb  ,0+ifb}, /*>*/
/* 6 */ {6+ifsl ,6+ifsl ,6+ifsl ,6+ifsl ,6+ifsl ,6+ifsl ,6+ifsl ,6+ifsl ,6+ifsl ,6+ifsl ,6+ifsl ,6+ifsl  ,6+ifsl  ,6+ifsl  ,0+iflb ,6+ifsl ,0+ifb}, /*String*/
/* 7 */ {0+ifb  ,1+ifsl ,1+ifsl ,0+ifb  ,0+ifb  ,12+ifsl,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb   ,0+ifb   ,0+ifb   ,0+ifb  ,0+ifb  ,0+ifb}, /*pot. Hexzahl(immernoch Zahl)*/
/* 8 */ {0+ifb  ,8+ifsl ,8+ifsl ,8+ifgl ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb   ,0+ifb   ,0+ifb   ,0+ifb  ,0+ifb  ,0+ifb}, /*Hexzahl*/
/* 9 */ {0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb   ,10+ifsl ,0+ifb   ,0+ifb  ,0+ifb  ,0+ifb}, /*pot. kommentaranfang*/
/* 10 */{10+ifsl,10+ifsl,10+ifsl,10+ifsl,10+ifsl,10+ifsl,10+ifsl,10+ifsl,10+ifsl,10+ifsl,10+ifsl,10+ifsl ,11+ifsl ,10+ifsl ,10+ifsl,10+ifsl,0+ifb}, /*kommentare*/
/* 11 */{10+ifsl,10+ifsl,10+ifsl,10+ifsl,10+ifsl,10+ifsl,10+ifsl,10+ifsl,10+ifsl,10+ifsl,10+ifsl,10+ifsl ,11+ifsl ,10+ifsl ,10+ifsl,0+ifrl ,0+ifb}, /*pot. kommentarende*/
/* 12 */{0+ifb  ,8+ifsl ,8+ifsl ,8+ifgl ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb  ,0+ifb   ,0+ifb   ,0+ifb   ,0+ifb  ,0+ifb  ,0+ifb} /*Hexzahl,bei abbruch fehler*/};
/* problem x entfaellt bei falscher hexzahl*/

tKeyWordTab mSclW['Z'-'A'+1][8]=
{
/*Len        2          3          4          5          6          7          8          9    */
/* A */ {{0L ,zNIL},{"ND", zAND},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL}},
/* B */ {{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{"EGIN",zBGN},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL}},
/* C */ {{0L ,zNIL},{0L, zNIL},{"ALL",zCLL},{"ONST",zCST},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL}},
/* D */ {{"O",zDO},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL}},
/* E */ {{0L ,zNIL},{"ND",zEND},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL}},
/* F */ {{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL}},
/* G */ {{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL}},
/* H */ {{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL}},
/* I */ {{"F",zIF},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL}},
/* J */ {{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL}},
/* K */ {{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL}},
/* L */ {{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL}},
/* M */ {{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L,        zNIL}},
/* N */ {{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L,        zNIL}},
/* O */ {{"R",zOR},{"DD",zODD},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L,       zNIL}},
/* P */ {{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{"ROCEDURE",zPRC}},
/* Q */ {{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL}},
/* R */ {{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL}},
/* S */ {{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL}},
/* T */ {{0L ,zNIL},{0L, zNIL},{"HEN",zTHN},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL}},
/* U */ {{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL}},
/* V */ {{0L ,zNIL},{"AR",zVAR},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL}},
/* W */ {{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{"HILE",zWHL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL}},
/* X */ {{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL}},
/* Y */ {{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL}},
/* Z */ {{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL},{0L ,zNIL},{0L, zNIL}}
};

/* Ausgabefunktionen des Automaten */
static void fl  (void);
static void fb  (void);
static void fgl (void);
static void fsl (void);
static void fslb(void);
static void flb(void);
static void fr(void);
static void frl(void);

typedef void (*FX)(void);

static FX vfx[]={fl,fb,fgl,fsl,fslb,flb,fr,frl};

/*---- Lexikalische Analyse ----*/
tMorph* Lex(void)
{
	int Zn;
	Morph=MorphInit;
	pBuf=vBuf;
	Ende=0;
	do
	{
//		printf("Berechnung neuer Zustand: [ %d ]+ < %c > =>",Z,X);
		/* Berechnung des Folgezustands */
		Zn=   vSMatrix[Z][vZKl[X&0x7f]]&0xF;
//		printf("[ %d ]\n",Zn);
		/* Ausfuehrung der Aktion (Ausgabefunktion */
		vfx[((vSMatrix[Z][vZKl[X&0x7f]]))>>4]();
		//printf("Aktion ausgefuehrt\n");
		/* Automat schaltet */
		Z=Zn;
	}while (Ende==0);
	//printf("return Lex\n");
	return &Morph;
}


/*---- Testprogramm fuer lexikalische Analyse ----*/
#ifdef _LEXER_
int main(int argc, char*argv[])
{
    if(argc>1)
    {
	initLex(argv[1]);
	while (X!=EOF)
	{
		Lex();
		switch(Morph.MC)
		{
			case mcSymb :
				if (Morph.Val.Symb==zErg)   printf("Symbol,:=\n");    else
				if (Morph.Val.Symb==zle )   printf("Symbol,<=\n");    else
				if (Morph.Val.Symb==zge )   printf("Symbol,>=\n");    else
				if (Morph.Val.Symb==zBGN)   printf("Symbol,_BEGIN\n");else
				if (Morph.Val.Symb==zCLL)   printf("Symbol,_CALL\n"); else
				if (Morph.Val.Symb==zCST)   printf("Symbol,_CONST\n");else
				if (Morph.Val.Symb==zDO )   printf("Symbol,_DO\n");   else
				if (Morph.Val.Symb==zEND)   printf("Symbol,_END\n");  else
				if (Morph.Val.Symb==zIF )   printf("Symbol,_IF\n");   else
				if (Morph.Val.Symb==zODD)   printf("Symbol,_ODD\n");  else
				if (Morph.Val.Symb==zPRC)   printf("Symbol,_PROCEDURE\n");else
				if (Morph.Val.Symb==zTHN)   printf("Symbol,_THEN\n"); else
				if (Morph.Val.Symb==zVAR)   printf("Symbol,_VAR\n");  else
				if (Morph.Val.Symb==zWHL)   printf("Symbol,_WHILE\n");else
				if (Morph.Val.Symb==zOR)   printf("Symbol,_OR\n"); else 
				if (Morph.Val.Symb==zAND)   printf("Symbol,_AND\n");
	//			printf("case mcSymb printable\n");
				if (isprint(Morph.Val.Symb))printf("Symbol,%c\n",(char)Morph.Val.Symb);
				break;
			case mcNumb :
				printf("Zahl  ,%ld\n",Morph.Val.Numb);
				break;
			case mcIdent:
				printf("Ident ,%s\n",(char*)Morph.Val.pStr);
				break;
			case mcStrng:
				printf("String ,%s\n",(char*)Morph.Val.pStr);
				break;
            case mcEmpty:
                printf("mcEmpty\n");// ,%s\n",(char*)Morph.Val.pStr);
                break;
		}
	}
    }
	return 0;
}
#endif
static void fl(void)
{
  
	//X=fgetc(pIF); //read next char and save it under X
	
	cur_column=cur_column+1;
	if(X=='\n')
	{
		cur_line=cur_line+1;
		cur_column=0;
	}
	X=fgetc(pIF); //read next char and save it under X
//	printf("[read] line:%d col:%d %c\n",cur_line+1,cur_column+1,X);
}

static void fs(void)
{
	int len = strlen(vBuf);
	//printf("[write] line:%d col:%d writepos:%d %c\n",cur_line+1,cur_column+1,len,X);
	vBuf[len] = X;
	vBuf[len+1] = '\0';
}

static void fg(void)
{
	int len = strlen(vBuf);
	vBuf[len] = toupper(X);
	vBuf[len+1] = '\0';
}


static void fb(void)
{
	int len=strlen(vBuf);
	//printf("[morphemstop] line:%d col:%d length:%d\n",cur_line+1,cur_column+1,len);
//	printf("Zustand begin fb(): %d len=%d\n",Z,len);
	switch(Z)
	{
		case 0://Symbol
			Morph.MC=mcSymb;
			Morph.PosLine=cur_line+1;
			Morph.PosCol=cur_column+1-len;
			Morph.Val.Symb=vBuf[0];
			Morph.MLen=len;
			break;
		case 1://Zahl
			Morph.MC=mcNumb;
			Morph.PosLine=cur_line+1;
			Morph.PosCol=cur_column+1-len;
			Morph.Val.Numb=atoi(vBuf);
			Morph.MLen=len;
			break;
		case 2://Bezeichner/Wortsymbol
//		printf("fb(): %s boolean:%d == %d (%d,%d)\n",vBuf,mSclW[vBuf[0]-'A'][len-2].special_code,zNIL,zErg,zAND);

			if((len-2)<0 || mSclW[vBuf[0]-'A'][len-2].special_code==zNIL)
			{
				Morph.MC=mcIdent;
				Morph.Val.pStr=malloc(len+1);
				strcpy(Morph.Val.pStr,vBuf);
			}
			else
			{
				Morph.MC=mcSymb;
				Morph.Val.Symb=mSclW[vBuf[0]-'A'][len-2].special_code;
			}
			Morph.PosLine=cur_line+1;
			Morph.PosCol=cur_column+1-len;
			Morph.MLen=len;
			break;
		case 3://:=
			Morph.MC=mcSymb;
			Morph.PosLine=cur_line+1;
			Morph.PosCol=cur_column+1-len;
			if(len==2)
				Morph.Val.Symb=zErg;
			else
				Morph.Val.Symb=vBuf[0];
			Morph.MLen=len;
			break;
		case 4://<=
			Morph.MC=mcSymb;
			Morph.PosLine=cur_line+1;
			Morph.PosCol=cur_column+1-len;
			if(len==2)
				Morph.Val.Symb=zle;
			else
				Morph.Val.Symb=vBuf[0];
			Morph.MLen=len;
			break;
		case 5://>=
			Morph.MC=mcSymb;
			Morph.PosLine=cur_line+1;
			Morph.PosCol=cur_column+1-len;
			if(len==2)
				Morph.Val.Symb=zge;
			else
				Morph.Val.Symb=vBuf[0];
			Morph.MLen=len;
			break;
		case 6://"..."
			Morph.MC=mcStrng;
			Morph.PosLine=cur_line+1;
			Morph.PosCol=cur_column+1-len;
			Morph.Val.pStr=malloc(len+1);
			strcpy(Morph.Val.pStr,vBuf);
			Morph.MLen=len;
			break;
		case 7://pot. Hexzahl (immer noch 0)
			Morph.MC=mcNumb;
			Morph.PosLine=cur_line+1;
			Morph.PosCol=cur_column+1-len;
			Morph.Val.Numb=atoi(vBuf);
			Morph.MLen=len;
			break;
		case 8://Hexzahl
			Morph.MC=mcNumb;
			Morph.PosLine=cur_line+1;
			Morph.PosCol=cur_column+1-len;
			Morph.Val.Numb=strtol(vBuf,NULL,16);
			Morph.MLen=len;
			break;
	case 9://pot. kommentaranfang
			Morph.MC=mcSymb;
			Morph.PosLine=cur_line+1;
			Morph.PosCol=cur_column+1-len;
			Morph.Val.Symb=vBuf[0];
			Morph.MLen=len;
			break;
	case 10://Kommentar
			//printf("ungueltiger Zustand! Lexer akzeptiert eingabe nicht!");
			//exit(1);
			//break;
	case 11://pot. Kommentarende
			printf("[%d,%d] ungueltiger Zustand! Lexer akzeptiert eingabe nicht!\n",cur_line+1,cur_column+1);
			exit(1);
			break;
	case 12://falsche Hexzahl (0x)
			printf("[%d,%d] ungueltiger Zustand! Lexer akzeptiert eingabe nicht!\n",cur_line+1,cur_column+1-len);
			exit(1);
			break;
	}
	Ende=1;
	vBuf[0]='\0';
}

static void fsl(void)
{
	fs();
	fl();
}

static void fgl(void)
{
	fg();
	fl();
}

static void fslb(void)
{
	fs();
	fl();
	fb();
}

static void flb(void)
{
	fl();
	fb();
}

static void fr(void)
{
	vBuf[0]='\0';
}

static void frl(void)
{
	fr();
	fl();
}

