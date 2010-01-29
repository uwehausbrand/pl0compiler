#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "code.h"
#include "lex.h"
#include "semrtype.h"

FILE* pOFile;
char* pCode;

extern int idxProc;		/* Zaehler fuer Prozeduren                   */
extern int idxConst;
extern tProc *pCurProc;
extern tMorph Morph;
char*  vCode;    /* Pointer auf dynamischen Bereich fuer Code */
char*  pCode;    /* Pointer auf aktuelle Position             */

extern int    LenCode;  /* Laenge des Codeausgabebereiches           */
extern tList *pLConst;
extern tList *pLOp;  /* Operatorenkeller für Ausdruck   */
extern tList* pLLabel;

/*----------------------------------------------*/
/* Schreiben von Code in den Codeausgabebereich */
int code(tCode Code,...)
{
	va_list ap;
	short sarg;
	
	if (pCode-vCode+MAX_LEN_OF_CODE >= LenCode)
	{
		char* xCode = realloc(vCode, (LenCode+=1024));
		if (xCode == NULL)
			Error(ENoMem);
		pCode = xCode + (pCode-vCode);
		vCode = xCode;
	}
	*pCode++ = (char)Code;
	va_start(ap, Code);
	switch (Code)
	{
		/* Befehle mit 3 Parametern */
		case entryProc:
			sarg =va_arg(ap,int);
			wr2ToCode(sarg);
		/* Befehle mit 2 Parametern */
		case puValVrGlob:
		case puAdrVrGlob:
			sarg = va_arg(ap,int);
			wr2ToCode(sarg);
		/* Befehle mit 1 Parameter */
		case puValVrMain:
		case puAdrVrMain:
		case puValVrLocl:
		case puAdrVrLocl:
		case puConst:
		case jmp :
		case jnot:
		case call:
			sarg = va_arg(ap,int); /* Prozedurnummer               */
			wr2ToCode(sarg);
			break;
	
		/* keine Parameter */
		case putStrg:
		  //printf("Stringausgabe: %s length:%d Startadresse:%d Endadresse:",Morph.Val.pStr,strlen(Morph.Val.pStr),pCode);
					if ((int)(pCode-vCode+strlen(Morph.Val.pStr)+1) >= LenCode)
					{
						char* xCode = realloc(vCode, (LenCode+=1024));
							if (xCode==NULL) Error(ENoMem);
						pCode = xCode+(pCode-vCode);
						vCode = xCode;
					}
					strcpy(pCode, Morph.Val.pStr);
					pCode += strlen(pCode)+1;
					//printf("%d\n",pCode);
					break;
		default     : break;
	}
	va_end(ap);
	return OK;
}

/*----------------------------------------------*/
/* Schreiben von Code in den Codeausgabebereich */
int CodeOut(void)
{
	unsigned short Len = (unsigned short)(pCode-vCode);
	wr2ToCodeAtP((short)Len, vCode+1);
	wr2ToCodeAtP((short)pCurProc->LenVar, vCode+5);
	if (Len == fwrite(vCode, sizeof(char), Len, pOFile)) 
	{
		pCode = vCode = NULL;
		LenCode = 0;
		return OK;
	}
	else
		return FAIL;
}

/*-----------------------------------------*/
/* Oeffnen/Schliessen des Codeausgabefiles */
int openOFile(char* filename)
{
	long  i=0L;
	char  vName[128+1];
	
	strcpy(vName, filename);
	if (strstr(vName, ".pl0") == NULL)
			strcat(vName, ".cl0");
	else
			*(strrchr(vName, '.')+1) = 'c';
	
	if ((pOFile=fopen(vName,"wb")) != NULL)
	{
		fwrite(&i, sizeof(long), 1, pOFile);
		return OK;
	}
	else
		return FAIL;
}

int closeOFile(void)
{
	char vBuf2[2];
	tConst* pConst = (tConst*)GetFirst(pLConst);
	
	if (pOFile != NULL)
	{
		fseek(pOFile, 0, SEEK_END);     // Dateizeiger auf Ende


		// Konstanten noch an das Ende des Codes hängen
		while(pConst && idxConst > 0)
		{
				wr2ToCodeAtP((pConst->Val), vBuf2);     // Const Value auf 2 Byte im Code schreiben
				fwrite(vBuf2, 2, 1, pOFile);            // Zwei Byte aus Buf 2 schreiben

				wr2ToCodeAtP((pConst->Val >> 16), vBuf2);       // Value 2 byte nach rechts verschieben
				fwrite(vBuf2, 2, 1, pOFile);                    // 00 00 = 2 Byte in Datei Schreiben

				//printf("debug %d\n",pConst->Val);
				pConst = (tConst*)GetNext(pLConst);
				idxConst--;
		}


		fseek(pOFile, 0, SEEK_SET);       /*Dateizeiger auf Anfang */
		wr2ToCodeAtP(idxProc, vBuf2);    /*Prozedurenanzahl an CodeAnfang schreiben */

		if ((fwrite(vBuf2,2,1,pOFile)) == 1) /* Code in Datei schreiben */
			return OK;

	}

	return FAIL;
}

void wr2ToCode(short x)
{
	*pCode++ = (unsigned char)(x & 0xff);
	*pCode++ = (unsigned char)(x >> 8);
}

void wr2ToCodeAtP(short x, char* pD)
{
	* pD   = (unsigned char)(x & 0xff); 
	*(pD+1)= (unsigned char)(x >> 8);
}


/*--------------------------------------------------*/
/* Kellern und Auskellern einer Sprungmarke bei if  */
int pushLabel(void)
{
	tLabl* pLabl = (tLabl*)malloc(sizeof(tLabl));
	pLabl->Kz   = KzLabl;
	pLabl->iJmp = (long)pCode;
	
	InsertTail(pLLabel, (tLabl*)pLabl);
	return pLabl->iJmp;
}

int popAndInsLabel(int x)
{
	tLabl* pLabl = NULL;
	short addr;
	char* pAddr;
	
	if ( (pLabl=(tLabl*)GetLast(pLLabel)) )
	{
		pAddr = (char*)pLabl->iJmp;
		RemoveItem(pLLabel);

		addr  = (int)pCode - (int)pAddr; /* Relativadresse berechnen */
		printf("pCode: %X, pAddr: %X, rel: %d\n", (int)pCode, (int)pAddr, addr);
		wr2ToCodeAtP(addr, pAddr-x);
	}
	else
		return 0;
	
	return pLabl->iJmp;
}

int popLabel(void)
{
	int iJmp;
	tLabl* pLabl = NULL;
	if ( (pLabl=(tLabl*)GetLast(pLLabel)) )
	{
		iJmp = pLabl->iJmp;
		RemoveItem(pLLabel);
	}
	else
	{
		return 0;
	}
	
	return pLabl->iJmp;
}

void pushOperator(tCode c)
{
	tOp* pOp;
	pOp = (tOp*)malloc(sizeof(tOp));
	pOp->Kz = KzOp; 
	pOp->Code = c; 
	InsertTail(pLOp, (tOp*)pOp);
}

tCode popOperator(void)
{ 
	tCode c; 
	tOp* pOp = (tOp*)GetLast(pLOp); 
	RemoveItem(pLOp); 
	c = pOp->Code; 
	return c;
}

int popAndInsWhile()
{
	short addr; 
	char *pAddr; 

	code(jmp, 0); 

	/* condition */ 
	// Adresse hinter "jnot 00 00" auskellern
	// pCode zeigt auf code hinter jmp 00 00
	pAddr = (char*)popLabel(); 
	
	// Relative addr ermitteln, aktuelle Adresse - Adrese hinter jnot 00 00
	addr = (int)pCode - (int)pAddr; 
	
	wr2ToCodeAtP(addr, pAddr-2); 

	/* while */ 
	// Adresse von jnot auskellern
	// pCode zeigt auf code hinter jmp 00 00	
	pAddr = (char*)popLabel(); 
	
	// Relative addr ermitteln 
	addr = (int)pAddr - (int)pCode; 
	
	// zwei bytes vorher den relativen wert für Rücksprung zu jnot schreiben
	wr2ToCodeAtP(addr, pCode-2); 

	return OK;
}
