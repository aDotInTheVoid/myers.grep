#include <stdio.h>
#include <sys/file.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define WORD long

#define SIGMA    128
#define BUF_MAX 2048

#include "parse.i"

unsigned WORD Pc[SIGMA];

void setup_search()
{ unsigned WORD One;
  register int a, p;
  void error();

  if (patlen > 8*sizeof(unsigned WORD))
    error("Pattern is longer than %d\n elements",8*sizeof(unsigned WORD));
  for (a = 0; a < SIGMA; a++)
    Pc[a] = 0;
  One = 1;
  for (p = 0; p < patlen; p++)
    { if (patvec[p].type == CHAR)
        Pc[*(patvec[p].value)] |= One;
      else
        for (a = 0; a < SIGMA; a++)
          if (patvec[p].value[a>>3] & 1<<a%8)
            Pc[a] |= One;
      One <<= 1;
    }

#ifdef SHOW
  One = 1;
  for (a = 0; a < SIGMA; a++)
    { if (isprint(a))
        printf("  %c: ",a);
      else
        printf("%3d: ",a);
      for (p = 0; p < patlen; p++)
        if (Pc[a] & (One<<p))
          printf("1");
        else
          printf("0");
      printf("\n");
    }
#endif
}

static int cnt = 0;

void report(int pos)
{ cnt += 1; }

void search(ifile,dif) int ifile, dif;
{ register unsigned WORD P, M, X, U, Y;
  unsigned WORD Ebit, One;
  int a, i, p, num, base, Cscore; 
  static char buf[BUF_MAX];

  One  = 1;
  Ebit = (One << (patlen-1));
  P    = -1;
  M    = 0;

  Cscore = patlen;
  for (base = 1; (num = read(ifile,buf,BUF_MAX)) > 0; base += num)
    { for (i = 0; i < num; i++)
        { a  = buf[i];

          U  = Pc[a];
          X  = (((U & P) + P) ^ P) | U;
          U |= M;

          Y = P;
          P = M | ~ (X | Y);
          M = Y & X;

          if (P & Ebit)
            Cscore += 1;
          else if (M & Ebit)
            Cscore -= 1;

          Y = P << 1;
          P = (M << 1) | ~ (U | Y);
          M = Y & U;

          if (Cscore <= dif)
            report(base+i);

#ifdef SHOW
          if (isprint(a))
            printf("  %c: ",a);
          else
            printf("%3d: ",a);

          printf("U   =");
          for (p = 0; p < patlen; p++)
            if (U & (One << p))
              printf("  1");
            else
              printf("  0");
          printf("\n     ");

          printf("X   =");
          for (p = 0; p < patlen; p++)
            if (X & (One << p))
              printf("  1");
            else
              printf("  0");
          printf("\n     ");

          printf("Col =");
          for (p = 0; p < patlen; p++)
            if (P & (One << p))
              printf(" +1");
            else if (M & (1 << p))
              printf(" -1");
            else
              printf("  0");
          printf("\n\n");
#endif
        }
    }

}

#include "main.i"
