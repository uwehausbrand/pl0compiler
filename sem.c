#include "semrtype.h"
#include "lex.h"
#include <stdarg.h>

int idxProc = 0;
int idxConst = 0;
int LenCode = 0; /* Laenge des Codeausgabebereiches */
char* vCode;     /* Pointer auf dynamischen Bereich für Code */
char* pCode;     /* Pointer auf aktuelle Position            */
tList *pLConst;
tList *pLLabel;  /* Keller für Label               */
tList *pLOp;     /* Operatorenkeller für Ausdruck   */

tMorph Morph;

/*--------------------------------------------------------------*/
/* Stellt den Prozedurkopfblock fuer das Hauptprogramm zusammen */
int initMain(void)
{
  return 0;
};

/*-------------------------------------------------------------------------*/
/* erzeugt einen Bezeichner und traegt ihn in die aktuelle Namensliste ein */
tBez* createBez(char* pName)
{
	tBez* pBez = malloc(sizeof(tBez));

	pBez->Kz      = KzBez;
	pBez->IdxProc = pCurProc->IdxProc;
	pBez->pObj    = NULL;
	
	pBez->pName = malloc(strlen(pName) + 1);
	strcpy(pBez->pName, pName);

	return pBez;
};

/*---------------------------------------------------*/
/* Sucht einen Bezeichner in der lokalen Namensliste */
tBez* searchBez(tProc* pProc, char* pBez)
{
  //  printf("3suche bezeichner lokal in prozedur %d:%s ->\n",pProc->IdxProc,pBez);
	if (pProc != NULL)
	{ 
		// hole Namensliste der Prozedur
		tList* pList = pProc->pLBez;
		if (pList != NULL)
		{
			// durchsuche die Liste nach dem String pBez
			tBez* pItem;
			for (pItem = (tBez*)GetFirst(pList); pItem != NULL; pItem = (tBez*)GetNext(pList))
			{
				if (strcmp(pItem->pName, pBez) == 0)
				{
  //                                  printf("4gefunden\n");
					return pItem;
				}
			}
		}
	}
//	printf("3nicht gefunden\n");
	return NULL;
};

/*---------------------------------------------------*/
/* Sucht einen Bezeichner in den Namenslisten        */
tBez* searchBezGlobal(char* pBez)
{
  //  printf("1suche bezeichner Global in Prozedur %d:%s ->\n",pCurProc->IdxProc,pBez);
	tBez* pTmpBez;
	tProc* pTmpCurProc;
	
	if (pCurProc == NULL)
		printf("Fehler! pCurProc ist NULL\n");
	
	// lokale Suche
	pTmpBez = searchBez(pCurProc, pBez);
        //printf("suche bezeichner lokal (gefunden):%s\n",pTmpBez);
	// wenn Bezeichner lokal gefunden
	if (pTmpBez != NULL)
		return pTmpBez;
	// ansonsten suche global
	else
	{
        //        printf("2suche bezeichner Global in Prozedur %d:%s ->\n", pCurProc->IdxProc, pBez);
		pTmpCurProc = pCurProc;
		while (pTmpCurProc->pParent != NULL)
		{
			pTmpCurProc = pTmpCurProc->pParent;
			// suche dort lokal
			pTmpBez = searchBez(pTmpCurProc, pBez);
			// wenn Bezeichner gefunden
			if (pTmpBez != NULL)
                        {
          //                  printf("2gefunden\n");
				return pTmpBez;
                        }
			// ansonsten weiter in der while-Schleife
		}
	}
//	printf("1nicht gefunden\n");
	return NULL;
}

/*--------------------------------------------------------------*/
/* Erzeugen einer Konstante und Aufnahme in die Konstantenliste */
tConst* createConst(long Val)
{
	tConst* pConst = malloc(sizeof(tConst));
	
	pConst->Kz  = KzConst;
	pConst->Val = Val;
	pConst->Idx = idxConst;
	
	idxConst++;
	pCurConst = pConst;
	return pConst;
};

/*-----------------------------------------------*/
/* Suche einer Konstanten in der Konstantenliste */
tConst* searchConst(long Val)
{
	if (pLConst != NULL)
	{
		// durchsuche die Liste nach der Konstanten Val
		tConst* pItem;
		for (pItem = (tConst*)GetFirst(pLConst); pItem != NULL; pItem = (tConst*)GetNext(pLConst))
		{
			if (pItem->Val == Val)
			{
				return pItem;
			}
		}
	}
	
	return NULL;
};

/* Erzeugt einen Prozedurkopfblock */
tProc* createProc(tProc* pParent)
{
	tProc* pProc = malloc(sizeof(tProc));

	// Startprozedur
	if (pParent == NULL)
	{
		idxProc  = 0;
		idxConst = 0;
		LenCode  = 0;
		vCode = malloc(1024);	// Pointer auf dynamischen Bereich für Code
		pCode = vCode; 		// Pointer auf aktuelle Position
		pLConst = CreateList();
		pLLabel = CreateList();
		pLOp    = CreateList();
		
		pProc->Kz      = KzPrc;
		pProc->IdxProc = idxProc; // = 0
		pProc->pParent = NULL;
		pProc->pLBez   = CreateList();
	}
	// Unterprozeduren
	else
	{
		tBez* pBez = pCurBez;
		
		if (pBez != NULL)
		{
			pProc->Kz      = KzPrc;
			pProc->IdxProc = idxProc;
			pProc->pParent = pParent;
			pProc->pLBez   = CreateList();
			pBez->pObj = pProc;
		}
		else return NULL;
	}

	idxProc++;
	pProc->LenVar = 0;
	pCurProc = pProc;
	return pProc;
};

/*--------------------------*/
/* Erzeugen einer Variablen */
tVar* createVar(void)
{
	tBez* pBez = pCurBez;

	if (pBez != NULL)
	{
		tVar* pVar = malloc(sizeof(tVar));
	
		pVar->Kz = KzVar;
		pVar->Dspl = pCurProc->LenVar; // Relativadresse (4 Byte)
		pBez->pObj = pVar;
	
		pCurProc->LenVar += 4;
		pCurVar = pVar;
		
		return pVar;
	}
	else return NULL;
};


/*--------------------*/
/* Prozedur-Funktion  */
int pr()
{
	// Codegenerierung retProc
	code(retProc);
	
	// Codelänge in den Befehl entryProc als 1. Parameter nachtragen
	CodeOut();
	
	// Namensliste mit allen Konstanten-, Variablen- und Prozedurbeschreibungen auflösen
	DeleteList(pCurProc->pLBez);
	
	// Code aus dem Codepuffer in die Ausgabedatei schreiben (anfügen)
	// Schreiben des Konstantenblocks in das Codefile
	// Schreiben der Anzahl der Prozeduren in das Codefile am Anfang
	closeOFile();
	
	return OK;
}

/*-------------------*/
/* Block-Funktionen  */

int bl1()
{
	// lokale Suche nach dem Bezeichner
	if (searchBez(pCurProc, Morph.Val.pStr) != NULL) 
	{
		// Fehlerbehandlung;
		pCurBez =  NULL;
		printf("%s:%d,%d: Fehler: Neudefinition von `%s'\n", filename, Morph.PosLine, Morph.PosCol, Morph.Val.pStr);
		exit(1);
    } 
	else 
	{
		// Bezeichner anlegen und in Namensliste aufnehmen
		pCurBez = createBez(Morph.Val.pStr);
		if (pCurBez != NULL)
		{
			InsertTail(pCurProc->pLBez, pCurBez);
		}
	}
	
	return OK;
}

int bl3()
{
	if (pCurBez)
	{
		pCurBez->pObj = createConst(Morph.Val.Numb);
		if (pCurBez->pObj != NULL)
		{
			// Suche nach Konstante im Konstantenblock
			if (searchConst(Morph.Val.Numb) == NULL)
			{
				InsertTail(pLConst, pCurBez->pObj);
			}
		}
	}
	
	return OK;
}

int bl9()
{
	// lokale Suche nach dem Bezeichner
	if (searchBez(pCurProc, Morph.Val.pStr) != NULL)
	{
		// Fehlerbehandlung
		pCurBez =  NULL;
		printf("%s:%d,%d: Fehler: Neudefinition von `%s'\n", filename, Morph.PosLine, Morph.PosCol, Morph.Val.pStr);
		exit(1);
	}
	else
	{
		// Bezeichner anlegen und in Namensliste aufnehmen
		pCurBez = createBez(Morph.Val.pStr);
	
		if (pCurBez != NULL)
		{
			InsertTail(pCurProc->pLBez, pCurBez);

			pCurBez->pObj = createVar();
		}
	}
	
	return OK;
}

int bl15()
{
	// lokale Suche nach dem Bezeichner
	if (searchBez(pCurProc, Morph.Val.pStr) != NULL)
	{
		// Fehlerbehandlung
		pCurBez =  NULL;
		printf("%s:%d,%d: Fehler: Neudefinition von `%s'\n", filename, Morph.PosLine, Morph.PosCol, Morph.Val.pStr);
		exit(1);
	}
	else
	{
		// Bezeichner anlegen und in Namensliste aufnehmen
		pCurBez = createBez(Morph.Val.pStr);
		if (pCurBez != NULL)
		{
			InsertTail(pCurProc->pLBez, pCurBez);
				
			// Prozedur anlegen
			pCurBez->pObj = createProc(pCurProc);
		}
	}
	
	return OK;
}

int bl18()
{
	// Codegenerierung retProc
	code(retProc);
	
	// Codelänge in den Befehl entryProc als 1. Parameter nachtragen
	// Code aus dem Codepuffer in die Ausgabedatei schreiben (anfügen)
	CodeOut();
	
	// Namensliste der aktuellen Prozedur löschen
	if (pCurProc)
	{
		DeleteList(pCurProc->pLBez);
		pCurProc = pCurProc->pParent;
	}
	
	return OK;
}

int bl20()
{
	// Codeausgabepuffer initialisieren
	// Codegenerierung entryProc
	code(entryProc, 0, pCurProc->IdxProc, pCurProc->LenVar);
	
	return OK;
}


/*-----------------------*/
/* Statement-Funktionen  */

int st1()
{
	// globale Suche nach dem Bezeichner
	tBez* pBez = searchBezGlobal(Morph.Val.pStr);
	if (pBez == NULL)
	{
		// Fehlerbehandlung
		printf("%s:%d,%d: Fehler: `%s' ist nicht deklariert\n", filename, Morph.PosLine, Morph.PosCol, Morph.Val.pStr);
		exit(1);
	}
	else
	{
		// ist Bezeichner eine Variable?
		if (pBez->pObj)
		{
			if (((tConst*)(pBez->pObj))->Kz == KzConst) 
			{
				// Fehlerbehandlung
				printf("%s:%d,%d: Fehler: Zuweisung an Konstante `%s'\n", 
					   filename, Morph.PosLine, Morph.PosCol, Morph.Val.pStr);
				exit(1);
			}
			else if (((tProc*)(pBez->pObj))->Kz == KzPrc) 
			{
				// Fehlerbehandlung
				printf("%s:%d,%d: Fehler: ungültige lvalue `%s' in Zuweisung\n", 
					   filename, Morph.PosLine, Morph.PosCol, Morph.Val.pStr);
				exit(1);
			}
			else if (((tVar*)(pBez->pObj))->Kz == KzVar) 
			{
				// Codegenerierung
				if ((searchBez(pCurProc, Morph.Val.pStr)) != NULL)
				{
                          //          printf("puAdrVrLocl %d\n",((tVar*)(pBez->pObj))->Dspl);
					code(puAdrVrLocl, ((tVar*)(pBez->pObj))->Dspl);
				}
				else if (searchBez(pStartProc, Morph.Val.pStr) != NULL)
				{
                        //            printf("puAdrVrMain %d\n",((tVar*)(pBez->pObj))->Dspl);
					code(puAdrVrMain, ((tVar*)(pBez->pObj))->Dspl);
				}
				else
				{
                      //              printf("puAdrVrGlob %d\n",((tVar*)(pBez->pObj))->Dspl);
					code(puAdrVrGlob, ((tVar*)(pBez->pObj))->Dspl ,(short)pBez->IdxProc);
				}
			}
			else 
			{
				// Fehlerbehandlung
				printf("%s:%d,%d: Fehler: ungültige Zuweisung `%s'\n", 
					   filename, Morph.PosLine, Morph.PosCol, Morph.Val.pStr);
				exit(1);
			}
		}
		else return FAIL;
	}
	
	return OK;
}

int st2()
{
	// Generieren eines Labels für Rücksprung am Schleifenende
	pushLabel();
	
	return OK;
}

int st9()
{
	// Generieren eines Labels, Codegenerierung jnot
	code(jnot, 0);
	pushLabel();
	
	return OK;
}

int st10()
{
	// Generieren eines Labels, Codegenerierung jnot
	code(jnot, 0);
	pushLabel();
	
	return OK;
}

int st13()
{
	// globale Suche nach dem Bezeichner
	tBez* pBez = searchBezGlobal(Morph.Val.pStr);
	if (pBez == NULL)
	{
		printf("%s:%d,%d: Fehler: `%s' ist nicht deklariert\n", filename, Morph.PosLine, Morph.PosCol, Morph.Val.pStr);
		exit(1);
	}
	else
	{
		// ist Bezeichner eine Prozedur?
		if (pBez->pObj)
		{
			if (((tConst*)(pBez->pObj))->Kz == KzConst) 
			{
				printf("%s:%d,%d: Fehler: aufgerufenes Objekt `%s' ist keine Prozedur\n", filename, Morph.PosLine, Morph.PosCol, Morph.Val.pStr);
				exit(1);
			}
			else if (((tVar*)(pBez->pObj))->Kz == KzVar) 
			{
				printf("%s:%d,%d: Fehler: aufgerufenes Objekt `%s' ist keine Prozedur\n", filename, Morph.PosLine, Morph.PosCol, Morph.Val.pStr);
				exit(1);

			}
			else if (((tProc*)(pBez->pObj))->Kz == KzPrc) 
			{
				// Codegenerierung
				code(call, ((tProc*)(pBez->pObj))->IdxProc);
			}
			else 
			{
				printf("%s:%d,%d: Fehler: aufgerufenes Objekt `%s' ist keine Prozedur\n", filename, Morph.PosLine, Morph.PosCol, Morph.Val.pStr);
				exit(1);
			}
		}
	}
	
	return OK;
}

int st14()
{
	// globale Suche nach dem Bezeichner
	tBez* pBez = searchBezGlobal(Morph.Val.pStr);
	if (pBez == NULL)
	{
		printf("%s:%d,%d: Fehler: `%s' ist nicht deklariert\n", filename, Morph.PosLine, Morph.PosCol, Morph.Val.pStr);
		exit(1);
	}
	else
	{
		// ist Bezeichner eine Variable?
		if (pBez->pObj)
		{
			if (((tConst*)(pBez->pObj))->Kz == KzConst) 
			{
				printf("%s:%d,%d: Fehler: Zuweisung an Konstante `%s'\n", 
					   filename, Morph.PosLine, Morph.PosCol, Morph.Val.pStr);
				exit(1);
			}
			else if (((tProc*)(pBez->pObj))->Kz == KzPrc) 
			{
				printf("%s:%d,%d: Fehler: ungültige Zuweisung an Prozedur `%s'\n", filename, Morph.PosLine, Morph.PosCol, Morph.Val.pStr);
				exit(1);
			}
			else if (((tVar*)(pBez->pObj))->Kz == KzVar) 
			{
				// Codegenerierung
				if ((searchBez(pCurProc, Morph.Val.pStr)) != NULL)
				{
					code(puAdrVrLocl, ((tVar*)(pBez->pObj))->Dspl);
				}
				else if (searchBez(pStartProc, Morph.Val.pStr) != NULL)
				{
					code(puAdrVrMain, ((tVar*)(pBez->pObj))->Dspl);
				}
				else
				{
					code(puAdrVrGlob, ((tVar*)(pBez->pObj))->Dspl ,(int)pBez->IdxProc);
				}
				code(getVal);
			}
			else 
			{
				printf("%s:%d,%d: Fehler: ungültige Zuweisung `%s'\n", filename, Morph.PosLine, Morph.PosCol, Morph.Val.pStr);
				exit(1);
			}
		}
	}
	
	return OK;
}

int st15()
{
	// Ausgabe expression
	code(putVal);
	
	return OK;
}
int st16()
{
	// Ausgabe string
	code(putStrg);
	
	return OK;
}

int st17()
{
	// Codegenerierung storeVal
	code(storeVal);
	
	return OK;
}

/* if */
int st21()
{
	// Label auskellern, Relativadresse berechnen, Relativadresse in jmp-Befehl eintragen
 	popAndInsLabel(2);
	
	return OK;
}

/* while */
int st22()
{
	// Label auskellern, Relativadresse berechnen, 2. Label auskellern und jmp-Befehl generieren
	popAndInsWhile();
	
	return OK;
}

/* if */
int st24()
{
	code(jmp, 0);
	popAndInsLabel(2);

	pushLabel();

	return OK;
}

/* if */
int st26()
{
	// Label auskellern, Relativadresse berechnen, Relativadresse in jmp-Befehl eintragen
 	popAndInsLabel(2);
	
	return OK;
}

/*-----------------------*/
/* Condition-Funktionen  */

/* condition odd */
int co2()
{
	// Codegenerierung odd
	code(odd);
	
	return OK;
}

/* condition = */
int co3()
{
	pushOperator(cmpEQ);
	return OK;
}

/* condition # */
int co4()
{
	pushOperator(cmpNE);
	return OK;
}

/* condition < */
int co5()
{
	pushOperator(cmpLT);
	return OK;
}

/* condition <= */
int co6()
{
	pushOperator(cmpLE);
	return OK;
}

/* condition > */
int co7()
{
	pushOperator(cmpGT);
	return OK;
}

/* condition >= */
int co8()
{
	pushOperator(cmpGE);
	return OK;
}

/* condition rechte Seite des Vergleichs */
int co9()
{
	// Codegenerierung
	code( popOperator() );
	
	return OK;
}

int col5()
{
	code(OpMult);

	return OK;
}

int col6()
{
	code(OpAdd);

	return OK;
}


/*-----------------------*/
/* Expression-Funktionen */

/* negatives Vorzeichen */
int ex3()
{
	// Codegenerierung vzMinus
	code(vzMinus);
	
	return OK;
}

/* Addition */
int ex6()
{
	// Codegenerierung opAdd
	code(OpAdd);
	
	return OK;
}

/* Subtraktion */
int ex8()
{
	// Codegenerierung opSub
	code(OpSub);
	
	return OK;
}


/*------------------*/
/* Term-Funktionen  */

/* Multiplikation */
int te4()
{
	//Codegenerierung opMul
	code(OpMult);
	
	return OK;
}

/* Division */
int te5()
{
	// Codegenerierung opDiv
	code(OpDiv);
	
	return OK;
}


/*-------------------*/
/* Factor-Funktionen */

/* Numeral */
int fa0()
{
	tConst* pConst;
	
	// Konstante in der Konstantenliste suchen
	pConst = searchConst(Morph.Val.Numb);
	if (pConst == NULL)
	{
		//Konstante anlegen
		pConst = createConst(Morph.Val.Numb);
		InsertTail(pLConst, pConst);
	}
	
	// Codegenerierung puConst
	code(puConst, pConst->Idx);

	return OK;
}

/* Ident */
int fa4()
{
	// globale Suche nach dem Bezeichner
	tBez* pBez = searchBezGlobal(Morph.Val.pStr);
	if (pBez == NULL)
	{
		// Fehlerbehandlung
		printf("%s:%d,%d: Fehler: `%s' ist nicht deklariert\n", filename, Morph.PosLine, Morph.PosCol, Morph.Val.pStr);
		exit(1);
	}
	else
	{
		// ist Bezeichner eine Variable oder Konstante ?
		if (pBez->pObj)
		{
			if (((tConst*)(pBez->pObj))->Kz == KzConst)
			{
				code(puConst, ((tConst*)(pBez->pObj))->Idx);
			}
			else if (((tVar*)(pBez->pObj))->Kz == KzVar) 
			{
				// Codegenerierung
				if ((searchBez(pCurProc, Morph.Val.pStr)) != NULL)
				{
					code(puValVrLocl, ((tVar*)(pBez->pObj))->Dspl);
				}
				else if (searchBez(pStartProc, Morph.Val.pStr) != NULL)
				{
					code(puValVrMain, ((tVar*)(pBez->pObj))->Dspl);
				}
				else
				{
					code(puValVrGlob, ((tVar*)(pBez->pObj))->Dspl, (short)pBez->IdxProc);
				}
			}
			else if (((tProc*)(pBez->pObj))->Kz == KzPrc) 
			{
				// Fehlerbehandlung
				printf("%s:%d,%d: Fehler: ungültige Verwendung der Prozedur `%s' als Wert\n", 
					   filename, Morph.PosLine, Morph.PosCol, Morph.Val.pStr);
				exit(1);
			}
			else 
			{
				// Fehlerbehandlung
				printf("%s:%d,%d: Fehler: ungültige Verwendung von `%s'\n", 
					   filename, Morph.PosLine, Morph.PosCol, Morph.Val.pStr);
				exit(1);
			}
		}
	}
	
	return OK;
}
