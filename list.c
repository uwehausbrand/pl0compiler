#include "list.h"

/* Listenfunktion:	CreateList */

tList* CreateList(void)
{
	tList *ptmp = malloc(sizeof(tList));

	if (ptmp)
		ptmp->pFirst = ptmp->pLast = ptmp->pCurr = NULL;
	else return NULL;

	return ptmp;
}

/* Listenfunktion:	DeleteList */

int DeleteList(tList* pList) {
	
	GetFirst(pList);
	int i=0;
	while (pList->pCurr) {
		RemoveItem(pList);
		i++;
	}

	if (pList->pFirst == pList->pLast && pList->pLast == pList->pCurr && pList->pCurr == NULL)
	/*Wenn Liste leer ist*/
		free(pList);
	else return 0;

	return 1;
}

/* Listenfunktion:	InsertBehind */

int InsertBehind(tList* pList, void *pItem) {

	tCnct *ptmp;

	if (pList->pCurr) { /* existiert ein Datensatz */

		if (pList->pCurr->pNxt) { /* existiert Nachfolger */

  			ptmp = malloc(sizeof(tCnct));
			if(ptmp) {

				ptmp->pBez = pItem;
				ptmp->pNxt = pList->pCurr->pNxt;
				pList->pCurr->pNxt = ptmp;
				ptmp->pPrv = pList->pCurr;
				ptmp->pNxt->pPrv = ptmp;
				pList->pCurr = ptmp;
			}
			else return 0;

			return(int)ptmp;
		}
		else { /* existiert kein Nachfolger */
			return InsertTail(pList, pItem);
		}
	}
	else {	/* existiert kein Datensatz */
		return InsertHead(pList, pItem);
	}
}

/* Listenfunktion:	InsertBefore */

int InsertBefore(tList*pList, void *pItem) {

	tCnct *ptmp;

	if (pList->pCurr) {	/* existiert ein Datensatz */
 		if (pList->pCurr->pPrv)	{ /* existiert ein Vorgänger */

			ptmp = malloc(sizeof(tCnct));
            if (ptmp) {

				ptmp->pBez = pItem;
				ptmp->pPrv = pList->pCurr->pPrv;
				pList->pCurr->pPrv = ptmp;
				ptmp->pNxt = pList->pCurr;
				ptmp->pPrv->pNxt = ptmp;
				pList->pCurr = ptmp;
			}
			else
				return 0;

			return(int)ptmp;
		}
		else	/* existiert kein Vorgänger */
  			return InsertHead(pList, pItem);
  	}
	else	/*Existiert kein Datensatz*/
 		return InsertHead(pList, pItem);
 }

/* Listenfunktion:	InsertHead */

int InsertHead(tList* pList, void *pItem) {

	tCnct *ptmp = malloc(sizeof(tCnct));

	if (ptmp) {

		ptmp->pBez = pItem;
		ptmp->pNxt = pList->pFirst;
		pList->pFirst = ptmp;
		if (ptmp->pNxt)	/* existiert ein Nachfolger */
  			ptmp->pNxt->pPrv=ptmp;

  		else	/* existiert kein weiterer Datensatz */
 		 	pList->pLast = ptmp;

		pList->pCurr = ptmp;
		ptmp->pPrv = NULL;
	}
	else return 0;

	return (int)ptmp;
}

/* Listenfunktion:	InsertTail */

int InsertTail(tList* pList, void *pItem) {

	tCnct *ptmp = malloc(sizeof(tCnct));

	if (ptmp) {

		ptmp->pBez = pItem;
		ptmp->pPrv = pList->pLast;
		pList->pLast = ptmp;
		if (ptmp->pPrv)	/* existiert ein Vorgänger */
  			ptmp->pPrv->pNxt = ptmp;

		else	/* existiert kein weiterer Datensatz */
  			pList->pFirst = ptmp;
  		pList->pCurr = ptmp;
		ptmp->pNxt = NULL;
	}
	else return 0;

	return OK;
}

/* Listenfunktion:	RemoveItem
	loescht aktuellen Datensatz, setzt pCurr auf Vorgaenger */

int RemoveItem(tList* pList) {

	if (pList->pFirst == pList->pLast && pList->pLast == pList->pCurr) {
	/* wenn keine weiteren Elemente in Liste ausser einem */

		free(pList->pCurr->pBez);
		free(pList->pCurr);
		pList->pFirst = pList->pLast = pList->pCurr = NULL;
		return 0;
	}
	if (pList->pFirst == pList->pCurr) {	/* wenn erstes Element in Liste */

		pList->pFirst = pList->pCurr->pNxt;
		free(pList->pCurr->pBez);
		free(pList->pCurr);
		pList->pCurr = pList->pFirst;
		pList->pCurr->pPrv = NULL;
	}
	else {

		if (pList->pLast == pList->pCurr) {	/* wenn letztes Element in Liste */
 			pList->pLast = pList->pCurr->pPrv;
			free(pList->pCurr->pBez);
			free(pList->pCurr);
			pList->pCurr = pList->pLast;
			pList->pCurr->pNxt = NULL;
		}
		else {	/* wenn Element in der Mitte der Liste */
 			pList->pCurr->pNxt->pPrv = pList->pCurr->pPrv;
			pList->pCurr = pList->pCurr->pNxt;
			free(pList->pCurr->pPrv->pNxt->pBez);
			free(pList->pCurr->pPrv->pNxt);
			pList->pCurr->pPrv->pNxt = pList->pCurr;
		}
	}
	return 1;
}

/* Listenfunktion:	chgItemInList */

void* chgItemInList (tList* pList, void*pItem) {

	free(pList->pCurr->pBez);
	pList->pCurr->pBez = pItem;		/* neue Daten einhaengen */
	return GetSelected(pList);
}

/* Listenfunktion:	GetSelected */

void* GetSelected(tList* pList) {

	if (pList->pCurr)
         return pList->pCurr->pBez;
 	else
         return NULL;
}

/* Listenfunktion:	GetFirst */

void* GetFirst(tList* pList) {

	pList->pCurr = pList->pFirst;
	return GetSelected(pList);
}

/* Listenfunktion:	GetLast */

void* GetLast(tList* pList) {

	pList->pCurr = pList->pLast;
	return GetSelected(pList);
}

/* Listenfunktion:	GetNext */

void* GetNext(tList* pList) {

	pList->pCurr = pList->pCurr->pNxt;
	return GetSelected(pList);
}

/* Listenfunktion:	GetPrev */

void* GetPrev(tList* pList) {

	pList->pCurr = pList->pCurr->pPrv;
	return GetSelected(pList);
}

/* Listenfunktion:	GetIndexed */

void* GetIndexed(tList* pList, int Idx) {

	int i;

	pList->pCurr = pList->pFirst;
	for (i=1; i<Idx; i++) {
 		pList->pCurr = pList->pCurr->pNxt;
	}
	return GetSelected(pList);
}
