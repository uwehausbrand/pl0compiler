#ifndef SEMRTYPE_H
#define SEMRTYPE_H

#include "list.h"
#include "code.h"

typedef enum tKZ {
	KzBez,
	KzPrc,
	KzConst,
	KzVar,
	KzLabl,
    KzOp
}tKz;

typedef struct tBEZ
{
  tKz   Kz;			/* Kennzeichen */
  short IdxProc;	/* Prozedurindex */
  void* pObj;		/* Zeiger auf Beschreibung */
  char* pName;		/* Bezeichner */
}tBez;

typedef struct tPROC
{
  tKz    Kz;		/* Kennzeichen */
  short  IdxProc;	/* Prozedurindex */
  struct tPROC*pParent;	/* Zeiger auf umgebende Prozedur */
  tList *pLBez;    	/* Namensliste     */
  short  LenVar;	/* Länge der Variablen */
}tProc;

typedef struct tCONST
{
  tKz  Kz;		/* Kennzeichen */
  long Val;		/* Wert der Konstanten*/
  int  Idx;		/* Index im Konstantenblock */
}tConst;

typedef struct tVAR
{
  tKz Kz;		/* Kennzeichen */
  int Dspl;		/* Relativadresse der Variablen */
}tVar;

typedef struct tLABL
{
  tKz Kz;		/* Kennzeichen */
  long iJmp;	/* Pointer in Codeausgabebereich */
}tLabl;

typedef struct tOPRTR
{ 
  tKz Kz; 
  tCode Code;
}tOp;

enum Errors { ESyntx, ENoMem };


extern tProc* pCurProc;
extern tProc* pStartProc;
extern tConst* pCurConst;
extern tVar* pCurVar;
extern tBez* pCurBez;
extern char* filename;

/*---------------------------------*/
/* Erzeugt einen Prozedurkopfblock */
tProc* createProc(tProc* pParent);

/*--------------------------------------------------------------*/
/* Stellt den Prozedurkopfblock fuer das Hauptprogramm zusammen */
int initMain(void);

/*---------------------------------------------------*/
/* Sucht einen Bezeichner in der lokalen Namensliste */
tBez* searchBez(tProc* pProc, char* pBez);

/*----------------------------------------------------*/
/* Sucht einen Bezeichner in der globalen Namensliste */
tBez* searchBezGlobal(char* pBez);

/*-------------------------------------------------------------------------*/
/* erzeugt einen Bezeichner und traegt ihn in die aktuelle Namensliste ein */
tBez* createBez(char* pName);

/*--------------------------------------*/
/* Ausgabe der Fehlernachricht und exit */
void Error(int ENr);

/*---------------------------------------------------------------*/
/* Erzeugen einer Konstante und Aufnahme in die Konmstantenliste */
tConst* createConst(long Val);

/*-----------------------------------------------*/
/* Suche einer Konstanten in der Konstantenliste */
tConst* searchConst(long Val);

/*--------------------------*/
/* Erzeugen einer Variablen */
tVar* createVar(void);

/*----------------------------------------------*/
/* Schreiben von Code in den Codeausgabebereich */
int code(tCode Code,...);

int CodeOut(void);

void wr2ToCode(short x);

void wr2ToCodeAtP(short x, char* pD);

int openOFile(char* filename);

int closeOFile(void);

int pushLabel(void);

int popAndInsLabel(int x);

int popLabel(void);

void pushOperator(tCode c);

tCode popOperator(void);

int popAndInsWhile();

/* proc . */
int pr();

/* const Bezeichner */
int bl1();

/* const Wert */
int bl3();

/* var Bezeichner */
int bl9();

/* proc Bezeichner */
int bl15();

/* proc Ende */
int bl18();

/* state Beginn */
int bl20();

/* linke Seite der Zuweisung */
int st1();

/* rechte Seite der Zuweisung */
int st17();

/* Ausgabe String */
int st16();

/* if condition */
int st9();

/* if statement */
int st21();

/* if else */
int st24();

/* else statement */
int st26();

/* while */
int st2();

/* while condition */
int st10();

/* while statement */
int st22();

/* call Prozeduraufruf */
int st13();

/* ? Eingabe */
int st14();

/* ! Ausgabe */
int st15();

/* condition odd */
int co2();

/* condition = */
int co3();

/* condition # */
int co4();

/* condition < */
int co5();

/* condition <= */
int co6();

/* condition > */
int co7();

/* condition >= */
int co8();

/* condition rechte Seite des Vergleichs */
int co9();

int col5();

/* condition rechte Seite des Vergleichs */
int col6();

/* negatives Vorzeichen */
int ex3();

/* Addition */
int ex6();

/* Subtraktion */
int ex8();

/* Multiplikation */
int te4();

/* Division */
int te5();

/* Numeral */
int fa0();

/* Ident */
int fa4();

#endif
