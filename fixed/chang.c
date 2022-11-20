#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>

#define SIGMA 128
#define BUF_MAX 2048

#include "parse.i"

static int *TRAN[128];

void setup_search() {
  register int a, p, *b;

  b = (int *)malloc(sizeof(int) * SIGMA * (patlen + 2));
  for (a = 0; a < SIGMA; a++) {
    TRAN[a] = b;
    b[patlen] = b[patlen + 1] = patlen;
    for (p = patlen - 1; p >= 0; p -= 1)
      if (patvec[p].type == CHAR)
        if (a == *(patvec[p].value))
          b[p] = p;
        else
          b[p] = b[p + 1];
      else if (patvec[p].value[a >> 3] & 1 << a % 8)
        b[p] = p;
      else
        b[p] = b[p + 1];
    b += patlen + 2;
  }
#ifdef SHOW
  for (a = 0; a < SIGMA; a++)
    if (TRAN[a][0] < patlen) {
      printf("%c: ", a);
      for (p = TRAN[a][0]; p < patlen; p = TRAN[a][p + 1])
        printf(" %2d", p);
      printf("\n");
    }
#endif
}

#ifdef STATS
static int avedep;
#endif

void search(ifile, dif) int ifile, dif;
{
  register int next_match;
  register int next_dr;
  register int *dr_ptr;
  register int kmax;
  register int e;
  register int *l, cd;
  int i, num, base, *del;
  static char buf[BUF_MAX];

  kmax = dif + 1;
  del = (int *)malloc(sizeof(int) * patlen);
  del[0] = 0;
  del[1] = patlen + 1;
#ifdef STATS
  avedep = 1;
#endif

  for (base = 1; (num = read(ifile, buf, BUF_MAX)) > 0; base += num) {
    for (i = 0; i < num; i++) {
      l = TRAN[buf[i]];

      next_match = l[0];
      next_dr = *(dr_ptr = del + 1);

      while (1) {
        if (next_match >= next_dr) {
          if (next_dr == dr_ptr[1])
            *dr_ptr++ = next_dr;
          *dr_ptr++ = next_dr + 1;
        } else {
          *dr_ptr++ = next_match + 1;
          if ((next_match = l[next_dr]) >= kmax) {
            if (next_dr >= kmax)
              break;
            next_dr = *dr_ptr;
            while (next_dr < kmax) {
              if (next_dr == dr_ptr[1])
                *dr_ptr++ = next_dr;
              *dr_ptr++ = next_dr + 1;
              next_dr = *dr_ptr;
            }
            break;
          }
        }
        next_dr = *dr_ptr;
      }

      while (dr_ptr[-1] > patlen)
        dr_ptr -= 1;
      *dr_ptr = patlen + 1;

      e = patlen - (cd = (dr_ptr - del) - 1);
      if (e <= dif)
        printf("  Match at %d\n", base + i);

      while (*--dr_ptr - cd > dif)
        cd--;
      if ((kmax = dif + cd + 1) > patlen)
        kmax = patlen;
#ifdef STATS
      avedep += (dr_ptr - del);
#endif

#ifdef SHOW
      printf("col %2d(%c): ", base + i, buf[i]);
      for (dr_ptr = del; *dr_ptr <= patlen; dr_ptr++)
        printf("%d ", *dr_ptr);
      printf("\n");
      fflush(stdout);
#endif
    }
  }
#ifdef STATS
  printf("Average Depth = %g\n", (1. * avedep) / base);
#endif
}

#include "main.i"
