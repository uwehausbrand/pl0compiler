#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lex.h"
#include "semrtype.h"

#define OK 1
#define FAIL 0

tProc* pStartProc;
tProc* pCurProc;
tConst* pCurConst;
tVar* pCurVar;
tBez* pCurBez;
char* filename;
tList *pLConst; // Liste aller Konstanten

typedef enum BOGEN_DESC {
  BgNl =  0,   /* NIL     */
  BgSy =  1,   /* Symbol  */
  BgMo =  2,   /* Morphem */
  BgGr =  4,   /* Graph   */
  BgEn =  8,   /* Graphende */
  BgAl = 16   /* Alternative */
}tBg;

/* Ein Graph wird durch einen Vektor von Boegen beschrieben,
   Jeder Bogen enthaelt den Index des Folgebogens und ggf.
   den Index eines Alternativbogens                           */

/* Beschreibung eines Bogens eines Syntaxgraphen */
typedef struct BOGEN
{
  tBg BgD;             /* Kurzbeschreibung des Bogens */
  union BGX            /* Detaillierte Beschreibung   */
  {
    unsigned long X;   /* fuer Initialisierung */
    int           S;   /* Symbol      */
    tMC           M;   /* Morphemcode */
    struct BOGEN* G;   /* Graph       */
  }BgX;
  int (*fx)(void);     /* Action/NULL                 */
  int iNext;           /* Index Folgebogen / 0        */
  int iAlt;            /* Index Alternativbogen / 0   */
}tBog;


const tBog gBlock[], gProg[], gState[], gExpr[], gCond[];

////////////////////////
//  factor
////////////////////////
const tBog gFactor[] =
{
/*index  Typ  Bogenbewertung           Action  FB AB */
/* 0*/ {BgMo, {(unsigned long)mcNumb }, fa0 , 5, 1},
/* 1*/ {BgSy, {(unsigned long)'('    }, NULL, 2, 4},
/* 2*/ {BgGr, {(unsigned long)gExpr  }, NULL, 3, 0},
/* 3*/ {BgSy, {(unsigned long)')'    }, NULL, 5, 0},
/* 4*/ {BgMo, {(unsigned long)mcIdent}, fa4 , 5, 6},
/* 5*/ {BgEn, {(unsigned long)0      }, NULL, 0, 0},
/* 6*/ {BgSy, {(unsigned long)zCLL   }, fa6 , 7, 0},
/* 7*/ {BgMo, {(unsigned long)mcIdent}, st13, 5, 0}
};


////////////////////////
//  term
////////////////////////
const tBog gTerm[] =
{
/*index  Typ  Bogenbewertung           Action  FB AB */
/* 0*/ {BgGr, {(unsigned long)gFactor}, NULL, 1, 0},
/* 1*/ {BgNl, {(unsigned long)0      }, NULL, 2, 0},
/* 2*/ {BgSy, {(unsigned long)'*'    }, NULL, 4, 3},
/* 3*/ {BgSy, {(unsigned long)'/'    }, NULL, 5, 6},
/* 4*/ {BgGr, {(unsigned long)gFactor}, te4 , 1, 0},
/* 5*/ {BgGr, {(unsigned long)gFactor}, te5 , 1, 0},
/* 6*/ {BgEn, {(unsigned long)0      }, NULL, 0, 0}
};

////////////////////////
//  conditionlist
////////////////////////
const tBog gCondList[] =
{
/*index  Typ  Bogenbewertung           Action  FB AB */
/* 0*/ {BgGr, {(unsigned long)gCond   }, NULL, 1, 0},
/* 1*/ {BgSy, {(unsigned long)zAND    }, NULL, 5, 2},
/* 2*/ {BgSy, {(unsigned long)zOR     }, NULL, 6, 3},
/* 3*/ {BgNl, {(unsigned long)0       }, NULL, 8, 0},
/* 4*/ {BgSy, {(unsigned long)zAND    }, NULL, 5, 7},
/* 5*/ {BgGr, {(unsigned long)gCond   }, col5, 4, 0},
/* 6*/ {BgGr, {(unsigned long)gCond   }, col6, 4, 0},
/* 7*/ {BgSy, {(unsigned long)zOR     }, NULL, 6, 8},
/* 8*/ {BgEn, {(unsigned long)0       }, NULL, 0, 0}
};

////////////////////////
//  condition
////////////////////////
const tBog gCond[] =
{
/*index  Typ  Bogenbewertung           Action  FB AB */
/* 0*/ {BgSy, {(unsigned long)zODD   }, NULL, 2, 1},
/* 1*/ {BgGr, {(unsigned long)gExpr  }, NULL, 3, 0},
/* 2*/ {BgGr, {(unsigned long)gExpr  }, co2 ,10, 0},
/* 3*/ {BgSy, {(unsigned long)'='    }, co3 , 9, 4},
/* 4*/ {BgSy, {(unsigned long)'#'    }, co4 , 9, 5},
/* 5*/ {BgSy, {(unsigned long)'<'    }, co5 , 9, 6},
/* 6*/ {BgSy, {(unsigned long)zle    }, co6 , 9, 7},
/* 7*/ {BgSy, {(unsigned long)'>'    }, co7 , 9, 8},
/* 8*/ {BgSy, {(unsigned long)zge    }, co8 , 9, 0},
/* 9*/ {BgGr, {(unsigned long)gExpr  }, co9 ,10, 0},
/* 10*/ {BgGr, {(unsigned long)gExpr  }, co9 ,0, 11},
/* 11*/ {BgGr, {(unsigned long)gExpr  }, co9 ,0, 12},
/* 12*/ {BgEn, {(unsigned long)0      }, NULL, 0, 0}
};


////////////////////////
//  expression
////////////////////////
const tBog gExpr[] =
{
/*index  Typ  Bogenbewertung           Action  FB AB */
/* 0*/ {BgSy, {(unsigned long)'-'    }, NULL, 3, 1},
/* 1*/ {BgSy, {(unsigned long)'+'    }, NULL, 4, 2},
/* 2*/ {BgNl, {(unsigned long)0      }, NULL, 4, 0},
/* 3*/ {BgGr, {(unsigned long)gTerm  }, ex3 , 5, 0},
/* 4*/ {BgGr, {(unsigned long)gTerm  }, NULL, 5, 0},
/* 5*/ {BgSy, {(unsigned long)'+'    }, NULL, 6, 7},
/* 6*/ {BgGr, {(unsigned long)gTerm  }, ex6 , 5, 0},
/* 7*/ {BgSy, {(unsigned long)'-'    }, NULL, 8, 9},
/* 8*/ {BgGr, {(unsigned long)gTerm  }, ex8 , 5, 0},
/* 9*/ {BgEn, {(unsigned long)0      }, NULL, 0, 0}
};


////////////////////////
//  statement
////////////////////////
const tBog gState[] =
{
/*index  Typ  Bogenbewertung           Action  FB AB */
/* 0*/ {BgMo, {(unsigned long)mcIdent}, st1 , 8, 1},
/* 1*/ {BgSy, {(unsigned long)zIF    }, NULL, 9, 2},
/* 2*/ {BgSy, {(unsigned long)zWHL   }, st2 ,10, 3},
/* 3*/ {BgSy, {(unsigned long)zBGN   }, NULL,11, 4},
/* 4*/ {BgSy, {(unsigned long)zCLL   }, NULL,13, 5},
/* 5*/ {BgSy, {(unsigned long)'?'    }, NULL,14, 6},
/* 6*/ {BgSy, {(unsigned long)'!'    }, NULL,15,27},
/* 7*/ {BgNl, {(unsigned long)0      }, NULL,23, 0},
/* 8*/ {BgSy, {(unsigned long)zErg   }, NULL,17, 0},
/* 9*/ {BgGr, {(unsigned long)gCondList }, st9 ,18, 0},//and/or-erweiterung
/*10*/ {BgGr, {(unsigned long)gCondList }, st10,19, 0},//and/or-erweiterung
/*11*/ {BgGr, {(unsigned long)gState }, NULL,12, 0},
/*12*/ {BgSy, {(unsigned long)';'    }, NULL,11,20},
/*13*/ {BgMo, {(unsigned long)mcIdent}, st13,23, 0},
/*14*/ {BgMo, {(unsigned long)mcIdent}, st14,23, 0},
/*15*/ {BgGr, {(unsigned long)gExpr  }, st15,23,16},
/*16*/ {BgMo, {(unsigned long)mcStrng}, st16,23, 0},//stringausgabe
/*17*/ {BgGr, {(unsigned long)gExpr  }, st17,23, 0},
/*18*/ {BgSy, {(unsigned long)zTHN   }, NULL,21, 0},
/*19*/ {BgSy, {(unsigned long)zDO    }, NULL,22, 0},
/*20*/ {BgSy, {(unsigned long)zEND   }, NULL,23, 0},
/*21*/ {BgGr, {(unsigned long)gState }, NULL,24, 0},
/*22*/ {BgGr, {(unsigned long)gState }, st22,23, 0},
/*23*/ {BgEn, {(unsigned long)0      }, NULL, 0, 0},
/*24*/ {BgSy, {(unsigned long)zELSE  }, st24,26,25},//else-erweiterung
/*25*/ {BgNl, {(unsigned long)0      }, st21,23, 0}, //else-erweiterung
/*26*/ {BgGr, {(unsigned long)gState }, st26,23, 0}, //else-erweiterung
/*27*/ {BgSy, {(unsigned long)zRET   }, st27,28, 7}, //return-erweiterung
/*28*/ {BgGr, {(unsigned long)gExpr  }, st28,23, 0} //return-erweiterung
};


////////////////////////
//  block
////////////////////////
const tBog gBlock[] = 
{
/*index Typ    Bogenbewertung          Action  FB AB */
/* 0*/ {BgSy, {(unsigned long)zCST   }, NULL, 1, 8},
/* 1*/ {BgMo, {(unsigned long)mcIdent}, bl1,  2, 0},
/* 2*/ {BgSy, {(unsigned long)'='    }, NULL, 3, 0},
/* 3*/ {BgMo, {(unsigned long)mcNumb }, bl3,  4, 0},
/* 4*/ {BgSy, {(unsigned long)','    }, NULL, 1, 5},
/* 5*/ {BgSy, {(unsigned long)';'    }, NULL, 8, 0},
/* 6*/ {BgNl, {(unsigned long)0      }, NULL, 8, 0},
/* 7*/ {BgNl, {(unsigned long)0      }, NULL, 8, 0},
/* 8*/ {BgSy, {(unsigned long)zVAR   }, NULL, 9,14},
/* 9*/ {BgMo, {(unsigned long)mcIdent}, bl9 ,10, 0},
/*10*/ {BgSy, {(unsigned long)','    }, NULL, 9,11},
/*11*/ {BgSy, {(unsigned long)';'    }, NULL,14, 0},
/*12*/ {BgNl, {(unsigned long)0      }, NULL,14, 0},
/*13*/ {BgNl, {(unsigned long)0      }, NULL,14, 0},
/*14*/ {BgSy, {(unsigned long)zPRC   }, NULL,15,20},
/*15*/ {BgMo, {(unsigned long)mcIdent}, bl15,16, 0},
/*16*/ {BgSy, {(unsigned long)';'    }, NULL,17, 0},
/*17*/ {BgGr, {(unsigned long)gBlock }, NULL,18, 0},
/*18*/ {BgSy, {(unsigned long)';'    }, bl18,14, 0},
/*19*/ {BgNl, {(unsigned long)0      }, NULL,14, 0},
/*20*/ {BgNl, {(unsigned long)0      }, bl20,21, 0},
/*21*/ {BgGr, {(unsigned long)gState }, NULL,22, 0},
/*22*/ {BgEn, {(unsigned long)0      }, NULL, 0, 0},
};


////////////////////////
//  program
////////////////////////
const tBog gProg[] =
{
/*index  Typ  Bogenbewertung           Action  FB AB */
/* 0*/ {BgGr, {(unsigned long)gBlock }, NULL, 1, 0},
/* 1*/ {BgSy, {(unsigned long)'.'    }, pr  , 2, 0},
/* 2*/ {BgEn, {(unsigned long)0,     }, NULL, 0, 0}
};


int parse(const tBog* pGraph)
{
  const tBog* pBog=pGraph;
  int succ;

  if (Morph.MC == mcEmpty) Lex();

if (pBog == gProg  ) printf("Program\n");
if (pBog == gBlock ) printf("Block\n");
if (pBog == gState ) printf("Statement\n");
if (pBog == gExpr  ) printf("Expression\n");
if (pBog == gCond  ) printf("Condition\n");
if (pBog == gTerm  ) printf("Term\n");
if (pBog == gFactor) printf("Factor\n");
  while(1)
  {
    switch(pBog->BgD)
    {
      case BgNl: succ = 1;                             break;
      case BgSy: succ = (Morph.Val.Symb==pBog->BgX.S); break;
      case BgMo: succ = (Morph.MC==(tMC)pBog->BgX.M);  break;
      case BgGr: succ = parse(pBog->BgX.G);            break;
      case BgEn: return 1;   /* Ende erreichet - Erfolg */
      case BgAl:;
    }
    if (succ && pBog->fx!=NULL) succ=pBog->fx();
    if (!succ)
    /* Alternativbogen probieren */
    {
      if (pBog->iAlt != 0)
      {
        pBog=pGraph+pBog->iAlt;
      }
      else
        return FAIL;
    }
    else /* Morphem formal akzeptiert */
    {
      if (pBog->BgD & BgSy || pBog->BgD & BgMo)
      {
		Lex();
      }
      pBog = pGraph + pBog->iNext;
    }
  }/* while */
}

void Error(int e)
{
	switch (e)
	{
	case ESyntx: 
			fprintf(stderr, "%s:%d,%d: Syntaxfehler\n", 
				filename, Morph.PosLine, Morph.PosCol);
			break;
	default:
		fprintf(stderr, "Fehler #%i\n", e);
		break;
	}
	
}

int main(int argc, char*argv[])
{
	if (argc > 1)               /* PL/0-Datei angegeben ? */
	{
		filename = argv[1];
		if (openOFile(filename) == OK)
		{
			if (initLex(argv[1]))
			{
				if ( (pStartProc = createProc(NULL)) == NULL) /* Hauptprozedur erzeugen */
					printf("Hauptprozedur kann nicht erzeugt werden.\n");
			
				if (parse(gProg) == OK)   /* Start des Parsers      */
				{
					printf("\n OK\n");
				}
				else Error(ESyntx);     /* Ausgabe eines Syntaxfehlers */
			}
			else printf("Dateifehler\n");
		}
	}
	else
	{
		printf("Bitte eine PL/0-Datei angeben!\n");
	}
	return 0;
}
