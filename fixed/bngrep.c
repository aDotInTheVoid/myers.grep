#include <stdio.h>
#include <sys/file.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>

#define WORD long

#define SIGMA    128
#define BUF_MAX 2048

#include "parse.i"

void setup_search()
{ return; }

static unsigned WORD M1, M2, M3, Din, G;

static unsigned WORD Tm[SIGMA];

static void dynamic_setup(int dif)
{ register unsigned WORD t, f, one, con;
  unsigned WORD Pc[SIGMA];
  register int i, a, p, k2;
  void error();

  k2 = dif+2;

  if ((patlen-dif)*k2 > 8*sizeof(unsigned WORD))
    error("Pattern does not fit in a word\n");

  one = 1;

  M3 = 1;
  for (i = 0; i < dif; i++)
    M3 = (M3 << 1) | one;

  Din = M3;
  for (i = 1; i < patlen-dif; i++)
    Din |= (Din << k2);

  M1 = 1;
  for (i = 1; i < patlen-dif; i++)
    M1 |= (M1 << k2);

  M2 = M1 | M3;

  G = (one << dif);

  for (a = 0; a < SIGMA; a++)
    Pc[a] = -1;

  one = 1;
  for (p = 0; p < patlen; p++)
    { con = ~one;
      if (patvec[p].type == CHAR)
        Pc[*(patvec[p].value)] &= con;
      else
        for (a = 0; a < SIGMA; a++)
          if (patvec[p].value[a>>3] & 1<<a%8)
            Pc[a] &= con;
      one <<= 1;
    }

  for (a = 0; a < SIGMA; a++)
    { f = Pc[a];
      t = 0;
      for (i = 0; i < patlen-dif; i++)
        { t = (t << k2) | (f & M3);
          f >>= 1;
        }
      Tm[a] = t;
    }

#ifdef SHOW
  one = 1;
  printf(" M1: ");
  for (p = (patlen-dif)*k2-1; p >= 0; p--)
    if (M1 & (one<<p))
      printf("1");
    else
      printf("0");
  printf("\n");

  printf(" M2: ");
  for (p = (patlen-dif)*k2-1; p >= 0; p--)
    if (M2 & (one<<p))
      printf("1");
    else
      printf("0");
  printf("\n");

  printf(" M3: ");
  for (p = (patlen-dif)*k2-1; p >= 0; p--)
    if (M3 & (one<<p))
      printf("1");
    else
      printf("0");
  printf("\n");

  printf("Din: ");
  for (p = (patlen-dif)*k2-1; p >= 0; p--)
    if (Din & (one<<p))
      printf("1");
    else
      printf("0");
  printf("\n");

  printf("  G: ");
  for (p = (patlen-dif)*k2-1; p >= 0; p--)
    if (G & (one<<p))
      printf("1");
    else
      printf("0");
  printf("\n");

  for (a = 0; a < SIGMA; a++)
    { if (isprint(a))
        printf("  %c: ",a);
      else
        printf("%3d: ",a);
      for (p = (patlen-dif)*k2-1; p >= 0; p--)
        if (Tm[a] & (one<<p))
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
{ register unsigned WORD D, X;
  register unsigned WORD m1, m2, g, din;
  unsigned WORD one;
  int k2, k3;
  int a, i, num, base; 
  static char buf[BUF_MAX];

  one = 1;
  k2 = dif+2;
  k3 = dif+3;

  dynamic_setup(dif);

  m1 = M1;
  m2 = M2;
  g  = G;
  D  = din = Din;
  for (base = 1; (num = read(ifile,buf,BUF_MAX)) > 0; base += num)
    { for (i = 0; i < num; i++)
        { a  = buf[i];

          X = (D >> k2) | Tm[a];
          D = ((D << 1) | m1) & ((D << k3) | m2) &
              (((X + m1) ^ X) >> 1) & din;

          if ((D & g) == 0)
            report(base+i);
#ifdef SHOW
        { int p;
          if (isprint(a))
            printf("  %c: ",a);
          else
            printf("%3d: ",a);

          printf("D  =");
          for (p = (patlen-dif)*k2-1; p >= 0; p--)
            if (D & (one << p))
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
