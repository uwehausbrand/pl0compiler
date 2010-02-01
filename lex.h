typedef enum T_Fx {
	ifl=0x0,
	ifb=0x10,
	ifgl=0x20,
	ifsl=0x30,
	ifslb=0x40,
	iflb=0x50,
	ifr=0x60,
	ifrl=0x70
} tFx;

/* Morphemcodes */
typedef enum T_MC {
	mcEmpty, 
	mcSymb, 
	mcNumb, 
	mcIdent, 
	mcStrng
} tMC;

typedef enum T_ZS {
	zNIL,
	zErg=128,
	zle,
	zge,
 	zBGN,zCLL,zCST,zDO,zEND,zIF,zODD,zPRC,zTHN,zVAR,zWHL,zOR,zAND,zELSE,zRET
} tZS;

typedef struct
{
  tMC  MC;		/* Morphemcode */
  int  PosLine; /* Zeile       */
  int  PosCol;	/* Spalte      */
  union VAL
  {
    long Numb;
    char*pStr;
    int  Symb;
  } Val;
  int  MLen;   /* Morphemlaenge*/
} tMorph;

typedef struct
{
	char* special_word;
	tZS special_code;
}tKeyWordTab;

extern tMorph Morph;

int initLex(char* fname);

tMorph* Lex(void);

