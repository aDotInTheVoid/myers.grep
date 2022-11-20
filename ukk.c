#include <stdio.h>
#include <sys/file.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define SIGMA    128
#define BUF_MAX 2048

#include "parse.i"

static int *TRAN[128];

void setup_search()
{ register int a, p, *b;

  b = (int *) malloc(sizeof(int)*SIGMA*patlen);
  for (a = 0; a < SIGMA; a++)
    { TRAN[a] = b;
      for (p = 0; p < patlen; p += 1)
        if (patvec[p].type == CHAR)
          *b++ = (a != *(patvec[p].value));
        else if (patvec[p].value[a>>3] & 1<<a%8)
          *b++ = 0;
        else
          *b++ = 1;
    }
}

#ifdef STATS
static long avedep;
#endif

void search(ifile,dif) int ifile, dif;
{ int num, i, base;
  register int *s, *sd, *a;
  register int  c, d, e;
  int *S, *SE;
  static char buf[BUF_MAX];

  S  = (int *) malloc(sizeof(int)*(patlen+1));
  SE = S + patlen;

  sd = S+dif;
  for (s = S; s <= sd; s++)
    *s = s-S;

#ifdef STATS
  avedep = 0;
#endif

  for (base = 1; (num = read(ifile,buf,BUF_MAX)) > 0; base += num)
    { for (i = 0; i < num; i++)
        { a = TRAN[buf[i]];
          s = S;
          *s++ = e = c = 0;
          while (s <= sd)
            { c += *a++;
              e += 1;
              if (c < e) e = c;
              d = (c = *s) + 1;
              if (d < e) e = d;
              *s++ = e;
            }
#ifdef STATS
          avedep += (sd-S);
#endif
          if (s <= SE)
            { c += *a++;
              e += 1;
              if (c < e) e = c;
              *(sd = s) = e;
            }
          while (*sd > dif)
            sd -= 1;
          if (sd == SE)
            printf("  Match at %d\n",base+i);
#ifdef SHOW
          printf("%c: %2d\n",buf[i],sd-S);
#endif
        }
    }

#ifdef STATS
{ double len;
  len = base - 1.;
  printf("Ave. Depth = %g\n",avedep/len + 1.);
}
#endif
}

#include "main.i"
