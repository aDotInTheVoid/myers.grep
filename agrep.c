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
{ register int d;
  register unsigned WORD W, X, Y, P, Q, A;
  register unsigned WORD *V;
  unsigned WORD *VA, Ebit, One;
  int i, p, num, base; 
  static char buf[BUF_MAX];

  VA = (unsigned WORD *) malloc((dif+1)*sizeof(unsigned WORD));

  One = 1;

  for (d = 0; d <= dif; d++)
    VA[d] = 0;
  Ebit = (One << (patlen-1));

  for (base = 1; (num = read(ifile,buf,BUF_MAX)) > 0; base += num)
    { for (i = 0; i < num; i++)
        { A  = Pc[buf[i]];
          P  = *(V = VA);
          X  = (P << 1 | One);
          *V = W = X & A;
          for (d = 1; d <= dif; d++)
            { Q = *++V;
              Y = (Q << 1 | One);
              *V = W = P | (W<<1) | X | (Y & A); 
              P = Q;
              X = Y;
            }

          if (W & Ebit)
            report(base+i);

#ifdef SHOW
          if (isprint(buf[i]))
            printf("  %c: ",buf[i]);
          else
            printf("%3d: ",buf[i]);
          printf("V[0] =");
          for (p = 0; p < patlen; p++)
            if (VA[0] & (One<<p))
              printf("1");
            else
              printf("0");
          printf("\n");
          for (d = 1; d <= dif; d++)
            { printf("     V[%d] =",d);
              for (p = 0; p < patlen; p++)
                if (VA[d] & (One<<p))
                  printf("1");
                else
                  printf("0");
              printf("\n");
            }
#endif
        }
    }
}

#include "main.i"
