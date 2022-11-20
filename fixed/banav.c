#include <stdio.h>
#include <sys/file.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define WORD long

#define SIGMA    128
#define BUF_MAX 2048

#include "parse.i"

static int W;

void setup_search()
{ W = 8*sizeof(unsigned WORD); }

static int J, lr, lc;

static unsigned WORD M1, M2, M3, Din, GL, GI;

static unsigned WORD *Tm2[SIGMA];

static unsigned WORD Tm[SIGMA];

void dynamic_setup(int dif)
{ register unsigned WORD t, f, *b, one, con;
  unsigned WORD Pc[SIGMA];
  register int j, i, a, p, k;
  int  lrp1;
  void error();

  if (dif+1 >= W)
    error("Pattern diagonal does not fit in a word\n");

  one = 1;

  lr   = dif + 1;
  lrp1 = lr+1;
  lc   = W/lrp1;
  J    = ((patlen-dif) - 1)/lc;

  M3 = 1;
  for (i = 0; i < dif; i++)
    M3 = (M3 << 1) | one;

  Din = M3;
  for (i = 1; i < lc; i++)
    Din |= (Din << lrp1);

  M1 = 1;
  for (i = 1; i < lc; i++)
    M1 |= (M1 << lrp1);

  M2 = M1 | M3;

  GI = (one << dif);
  j  = (lc - (patlen-dif) % lc) %lc;
  GL = (one << (lrp1*j + dif));

  b = (unsigned WORD *) malloc(sizeof(unsigned WORD)*SIGMA*(J+1));

  for (a = 0; a < SIGMA; a++)
    { Tm2[a] = b;
      b += (J+1);
    }

  for (j = 0; j <= J; j++)
    { for (a = 0; a < SIGMA; a++)
        Pc[a] = -1;

      k = lc*j + lr + lc - 1;
      if (k > patlen) k = patlen;
      one = 1;
      for (p = lc*j; p < k; p++)
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
         for (i = 0; i < lc; i++)
           { t = (t << lrp1) | (f & M3);
             f >>= 1;
           }
         Tm2[a][j] = t;
       }
    }

  for (a = 0; a < SIGMA; a++)
    Tm[a] = Tm2[a][0];

#ifdef SHOW
  printf("J = %d  lc = %d  lr = %d\n",J,lc,lr);
  one = 1;
  printf(" M1: ");
  for (p = lc*lrp1-1; p >= 0; p--)
    if (M1 & (one<<p))
      printf("1");
    else
      printf("0");
  printf("\n");

  printf(" M2: ");
  for (p = lc*lrp1-1; p >= 0; p--)
    if (M2 & (one<<p))
      printf("1");
    else
      printf("0");
  printf("\n");

  printf(" M3: ");
  for (p = lc*lrp1-1; p >= 0; p--)
    if (M3 & (one<<p))
      printf("1");
    else
      printf("0");
  printf("\n");

  printf("Din: ");
  for (p = lc*lrp1-1; p >= 0; p--)
    if (Din & (one<<p))
      printf("1");
    else
      printf("0");
  printf("\n");

  printf(" GL: ");
  for (p = lc*lrp1-1; p >= 0; p--)
    if (GL & (one<<p))
      printf("1");
    else
      printf("0");
  printf("\n");

  printf(" GI: ");
  for (p = lc*lrp1-1; p >= 0; p--)
    if (GI & (one<<p))
      printf("1");
    else
      printf("0");
  printf("\n");

/*
  for (a = 0; a < SIGMA; a++)
    { if (isprint(a))
        printf("  %c: ",a);
      else
        printf("%3d: ",a);
      for (j = 0; j <= J; j++)
        { printf("[%d] = ",j);
          for (p = lc*lrp1-1; p >= 0; p--)
            if (Tm2[a][j] & (one<<p))
              printf("1");
            else
              printf("0");
          printf("\n     ");
        }
      printf("\n");
    }
*/

  fflush(stdout);
#endif
}

#ifdef STATS
static long avedep, aveinc, avedec, avezero;
#endif

static int cnt = 0;

void report(int pos)
{ cnt += 1; }

void search(ifile,dif) int ifile, dif;
{ register unsigned WORD *D, X, C, *e;
  unsigned WORD one;
  int lrp1, lrp2, ld, ldm1;
  int i, j, num, base, active; 
  static char buf[BUF_MAX];
  static void search1(int,int);

  dynamic_setup(dif);

  if (lc == 1)
    { search1(ifile,dif);
      return;
    }

  one  = 1;
  lrp1 = lr+1;
  lrp2 = lr+2;
  ld   = lrp1*(lc-1);
  ldm1 = ld - 1;

  D  = (unsigned WORD *) malloc(sizeof(unsigned WORD)*(J+2));

  for (j = 0; j <= J+1; j++)
    D[j] = Din;

  C = D[0];
  active = 0;

#ifdef STATS
  avedep = aveinc = avedec = avezero = 0;
#endif
  for (base = 1; (num = read(ifile,buf,BUF_MAX)) > 0; base += num)
    { i = 0;
      while (i < num)
        if (active == 0)
          { while (i < num)
              { X = (C >> lrp1) | Tm[buf[i++]];
                C = ( (C << 1) | M1) 
                  & ( (C << lrp2) | M2 )
                  & (((X + M1) ^ X) >> 1) & Din;
#ifdef STATS
                avezero += 1;
#endif
#ifdef SHOW
                { int p;
                  if (isprint(buf[i-1]))
                    printf("  %c: ",buf[i-1]);
                  else
                    printf("%3d: ",buf[i-1]);
                  printf("D[0] =");
                  for (p = lc*lrp1-1; p >= 0; p--)
                    if (C & (one << p))
                      printf("1");
                    else
                      printf("0");
                  printf("\n\n");
                }
#endif
                if ((C & GI) == 0)
                  { active = 1; D[0] = C;
                    break;
                  }
              }
          }

        else
          { while (i < num)
              { e = Tm2[buf[i++]];
                C = 0;
                for (j = 0; j <= active; j++)
                  { X = (D[j] >> lrp1) | (C << ld) | *e++;
                    C = D[j];
                    D[j] = ((C<<1) & ((C<<lrp2) | (D[j+1]>>ldm1)) | M1)
                         & (((X + M1) ^ X) >> 1) & Din;
                  }
#ifdef STATS
                avedep += active;
#endif

                X = (active == J) ? GL : GI;
                if (D[active] & X)
                  { while ((~ D[active] & Din) == 0)
                      if (D[active-1] & GI)
#ifdef STATS
                        { avedec += 1;
                          if (--active == 0) break;
                        }
#else
                        { if (--active == 0) break; }
#endif
                      else
                        break;
                  }
                else if (active == J)
                  report(base+i);
                else
#ifdef STATS
                  { aveinc += 1;
                    active += 1;
                  }
#else
                  active += 1;
#endif
#ifdef SHOW
                { int p;
                  if (isprint(buf[i-1]))
                    printf("  %c: ",buf[i-1]);
                  else
                    printf("%3d: ",buf[i-1]);
        
                  for (j = 0; j <= J; j++)
                    { printf("D[%d] =",j);
                      X = D[j];
                      for (p = lc*lrp1-1; p >= 0; p--)
                        if (X & (one << p))
                          printf("1");
                        else
                          printf("0");
                      printf("\n     ");
                    }
                  printf("  active = %d\n",active);
                }
#endif
                if (active == 0)
                  { C = D[0]; break; }
              }
          }
    }

#ifdef STATS
{ double len;
  len = base - 1.;
  printf("Ave. Depth = %g, Ave. Inc = %g, Ave. Dec = %g Ave. Filter = %g\n",
         avedep/len + 1., aveinc/len, avedec/len, avezero/len);
}
#endif
}

static void search1(ifile,dif) int ifile, dif;
{ register unsigned WORD *D, X, C, *e;
  unsigned WORD one;
  int i, j, num, base, active; 
  static char buf[BUF_MAX];

  one  = 1;

  D  = (unsigned WORD *) malloc(sizeof(unsigned WORD)*(J+2));

  for (j = 0; j <= J+1; j++)
    D[j] = Din;

  C = D[0];
  active = 0;

#ifdef STATS
  avedep = aveinc = avedec = avezero = 0;
#endif
  for (base = 1; (num = read(ifile,buf,BUF_MAX)) > 0; base += num)
    { i = 0;
      while (i < num)
        if (active == 0)
          { while (i < num)
              { X = Tm[buf[i++]];
                C = ((C << 1) | 1) 
                  & (((X + 1) ^ X) >> 1) & Din;
#ifdef STATS
                avezero += 1;
#endif
#ifdef SHOW
                { int p;
                  if (isprint(buf[i-1]))
                    printf("  %c: ",buf[i-1]);
                  else
                    printf("%3d: ",buf[i-1]);
                  printf("D[0] =");
                  for (p = lr; p >= 0; p--)
                    if (C & (one << p))
                      printf("1");
                    else
                      printf("0");
                  printf("\n\n");
                }
#endif
                if ((C & GI) == 0)
                  { active = 1; D[0] = C;
                    break;
                  }
              }
          }

        else
          { while (i < num)
              { e = Tm2[buf[i++]];
                C = 0;
                for (j = 0; j <= active; j++)
                  { X = C | *e++;
                    C = D[j];
                    D[j] = (((C & D[j+1]) <<1) | 1)
                         & (((X + 1) ^ X) >> 1) & Din;
                  }
#ifdef STATS
                avedep += active;
#endif

                X = (active == J) ? GL : GI;
                if (D[active] & X)
                  { while ((~ D[active] & Din) == 0)
                      if (D[active-1] & GI)
#ifdef STATS
                        { avedec += 1;
                          if (--active == 0) break;
                        }
#else
                        { if (--active == 0) break; }
#endif
                      else
                        break;
                  }
                else if (active == J)
                  report(base+i);
                else
#ifdef STATS
                  { aveinc += 1;
                    active += 1;
                  }
#else
                  active += 1;
#endif
#ifdef SHOW
                { int p;
                  if (isprint(buf[i-1]))
                    printf("  %c: ",buf[i-1]);
                  else
                    printf("%3d: ",buf[i-1]);
        
                  for (j = 0; j <= J; j++)
                    { printf("D[%d] =",j);
                      X = D[j];
                      for (p = lr; p >= 0; p--)
                        if (X & (one << p))
                          printf("1");
                        else
                          printf("0");
                      printf("\n     ");
                    }
                  printf("  active = %d\n",active);
                }
#endif
                if (active == 0)
                  { C = D[0]; break; }
              }
          }
    }

#ifdef STATS
{ double len;
  len = base - 1.;
  printf("Ave. Depth = %g, Ave. Inc = %g, Ave. Dec = %g Ave. Filter = %g\n",
         avedep/len + 1., aveinc/len, avedec/len, avezero/len);
}
#endif
}

#include "main.i"
