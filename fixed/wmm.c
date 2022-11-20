#include <stdio.h>
#include <sys/file.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

#define SIGMA    128
#define BUF_MAX 2048

#include "parse.i"

extern int K;
extern int STAB[], DTAB[];
extern char CTAB[];

static int *TRAN[128];
static int seg, rem, smax;

void setup_search()
{ register int answr;
  register int a, p, i, *b;

  seg = (patlen-1)/K + 1;
  rem = seg*K - patlen;

#ifdef SHOW
  printf("\t(seg,rem) = (%d,%d)\n",seg,rem);
#endif


  smax = 1;
  for (a = 0; a <= K; a++)
    smax *= 3;

  b = (int *) malloc(sizeof(int)*SIGMA*seg);
  for (a = 0; a < SIGMA; a++)
    { TRAN[a] = b;
      for (p = 0; p < patlen; p += K)
        { answr = 0;
          i = p+K;
          if (patlen < i) i = patlen;
          for (i--; i >= p; i--)
            if (patvec[i].type == CHAR)
              answr = 2*answr + (a != *(patvec[i].value));
            else if (patvec[i].value[a>>3] & 1<<a%8)
              answr = 2*answr;
            else
              answr = 2*answr + 1;
          *b++ = answr*smax;
        }
    }
  smax -= 2;
}

#ifdef STATS
static long avedep, avedel;
#endif

void search(ifile,dif) int ifile, dif;
{ int num, i, base, dik;
  register int *s, *a, ix, cr, *stab;
  register char *ctab;
  register int *sd, vi;
  int *S, *SE;
  static char buf[BUF_MAX];


  S  = (int *) malloc(sizeof(int)*(seg+1)) + 1;
  SE = S + (seg-1);
  dik = dif + K;

  sd = S + (dif-1)/K;
  for (s = S-1; s <= sd; s++)
    *s = smax;
  vi = (sd-S+1)*K;

#ifdef STATS
  avedep = avedel = 0;
#endif
  stab = STAB;
  ctab = CTAB;
  for (base = 1-rem; (num = read(ifile,buf,BUF_MAX)) > 0; base += num)
    { for (i = 0; i < num; i++)
        { a  = TRAN[buf[i]];
          cr = 0;
          s  = S;
          while (s <= sd)
            { ix = *a++ + *s + cr;
              cr = ctab[ix];
              *s++  = stab[ix];
            }
#ifdef STATS
          avedep += (sd-S);
#endif
          if (vi == dif && s <= SE)
            { ix = *a + smax + cr;
              *++sd = stab[ix];
              vi += K + ctab[ix];
#ifdef STATS
              avedel += 1;
#endif
            }
          else
            { vi += cr;
              while (vi > dik)
                { vi -= DTAB[*sd--];
#ifdef STATS
                  avedel += 1;
#endif
                }
            }
          if (sd == SE && vi <= dif)
            printf("  Match at %d\n",base+i);
#ifdef SHOW
          printf("%c: %5d(%2d)\n",buf[i],vi,sd-S);
#endif
        }
    }

#ifdef STATS
{ double len;
  len = (base + rem) - 1;
  printf("Ave. Depth = %g, Ave. Change = %g\n",
         avedep/len + 1., avedel/len);
}
#endif
  if (sd == SE)
    { s = sd;
      a = TRAN[0] + (SE-S);
      for (i = 0; i < rem; i++)
        { ix = *a + *s;
          *s = stab[ix];
          vi += ctab[ix];
          if (vi <= dif)
            printf("  Match at %d\n",base+i);
#ifdef SHOW
          printf("#: %5d(%2d)\n",vi,sd-S);
#endif
        }
    }
}

#include "main.i"
