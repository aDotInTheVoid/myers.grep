#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>

#define WORD long

#define SIGMA 128
#define BUF_MAX 2048

#include "parse.i"

static int W;

static unsigned WORD All = -1;
static unsigned WORD Ebit;

static unsigned WORD *TRAN[SIGMA];
static unsigned WORD Pc[SIGMA];
static int seg, rem;

void setup_search() {
  register unsigned WORD *b, bvc, one;
  register int a, p, i, k;

  W = sizeof(unsigned WORD) * 8;

  seg = (patlen - 1) / W + 1;
  rem = seg * W - patlen;

#ifdef SHOW
  printf("\t(seg,rem) = (%d,%d)\n", seg, rem);
#endif

  b = (unsigned WORD *)malloc(sizeof(unsigned WORD) * (SIGMA * seg + 1));
  for (a = 0; a < SIGMA; a++) {
    TRAN[a] = b;
    for (p = 0; p < patlen; p += W) {
      bvc = 0;
      one = 1;
      k = p + W;
      if (patlen < k)
        k = patlen;
      for (i = p; i < k; i++) {
        if (patvec[i].type == CHAR) {
          if (a == *(patvec[i].value))
            bvc |= one;
        } else {
          if (patvec[i].value[a >> 3] & 1 << a % 8)
            bvc |= one;
        }
        one <<= 1;
      }
      k = p + W;
      while (i++ < k) {
        bvc |= one;
        one <<= 1;
      }
      *b++ = bvc;
    }
  }

  for (a = 0; a < SIGMA; a++)
    Pc[a] = TRAN[a][0];

  Ebit = (((long)1) << (W - 1));

#ifdef SHOW
  for (a = 0; a < SIGMA; a++) {
    if (isprint(a))
      printf("  %c: ", a);
    else
      printf("%3d: ", a);
    b = TRAN[a];
    for (p = 0; p < patlen; p += W) {
      for (i = 0; i < W; i++)
        if (*b & (((long)1) << i))
          printf("1");
        else
          printf("0");
      printf(" ");
      b += 1;
    }
    printf("\n");
  }
#endif
}

#ifdef STATS
static long avedep, aveinc, avedec, averes, avezero;
#endif

typedef struct {
  unsigned WORD P;
  unsigned WORD M;
  int V;
} Scell;

void search(ifile, dif) int ifile, dif;
{
  int num, i, base, diw, a, Cscore;
  Scell *s, *sd;
  unsigned WORD pc, mc;
  register unsigned WORD *e;
  register unsigned WORD P, M, U, X, Y;
  Scell *S, *SE;
  static char buf[BUF_MAX];

  S = (Scell *)malloc(sizeof(Scell) * seg);
  SE = S + (seg - 1);

  diw = dif + W;

  sd = S + (dif - 1) / W;
  for (s = S; s <= sd; s++) {
    s->P = All;
    s->M = 0;
    s->V = ((s - S) + 1) * W;
  }

#ifdef STATS
  avedep = aveinc = avedec = averes = avezero = 0;
#endif
  for (base = 1 - rem; (num = read(ifile, buf, BUF_MAX)) > 0; base += num) {
    i = 0;
    if (sd == S) {
      P = S->P;
      M = S->M;
      Cscore = S->V;
      for (; i < num; i++) {
        a = buf[i];

        U = Pc[a];
        X = (((U & P) + P) ^ P) | U;
        U |= M;

        Y = P;
        P = M | ~(X | Y);
        M = Y & X;

        if (P & Ebit)
          Cscore += 1;
        else if (M & Ebit)
          Cscore -= 1;

        Y = P << 1;
        P = (M << 1) | ~(U | Y);
        M = Y & U;

#ifdef STATS
        avezero += 1;
#endif

#ifdef SHOW
        {
          register int k;
          printf("%4d ", i + base);
          if (isprint(buf[i]))
            printf("  %c: ", buf[i]);
          else
            printf("%3d: ", buf[i]);
          printf("Col =");
          for (k = 0; k < W; k++)
            if (P & (((long)1) << k))
              printf(" +1");
            else if (M & (((long)1) << k))
              printf(" -1");
            else
              printf("  0");
          printf(" [%d]\n", Cscore);
        }
#endif

        if (Cscore <= dif)
          break;
      }

      S->P = P;
      S->M = M;
      S->V = Cscore;

      if (i >= num)
        continue;

      if (sd == SE)
        printf("  Match at %d\n", base + i);
#ifdef SHOW
      else
        printf("\n");
#endif

      i += 1;
    }

    for (; i < num; i++) {
      e = TRAN[buf[i]];

#ifdef SHOW
      printf("%4d ", i + base);
      if (isprint(buf[i]))
        printf("  %c: ", buf[i]);
      else
        printf("%3d: ", buf[i]);
      printf("Col =");
#endif

      pc = mc = 0;
      s = S;
      while (s <= sd) {
        U = *e++;
        P = s->P;
        M = s->M;

        Y = U | mc;
        X = (((Y & P) + P) ^ P) | Y;
        U |= M;

        Y = P;
        P = M | ~(X | Y);
        M = Y & X;

        Y = (P << 1) | pc;
        s->P = (M << 1) | mc | ~(U | Y);
        s->M = Y & U;

        U = s->V;
        pc = mc = 0;
        if (P & Ebit) {
          pc = 1;
          s->V = U + 1;
        } else if (M & Ebit) {
          mc = 1;
          s->V = U - 1;
        }

#ifdef SHOW
        {
          register int k;
          for (k = 0; k < W; k++)
            if (s->P & (((long)1) << k))
              printf(" +1");
            else if (s->M & (((long)1) << k))
              printf(" -1");
            else
              printf("  0");
          printf(" [%d]", s->V);
        }
#endif

        s += 1;
      }
#ifdef STATS
      avedep += (sd - S);
#endif

      if (U == dif && (*e & 0x1 | mc) && s <= SE) {
        s->P = All;
        s->M = 0;
        if (pc == 1)
          s->M = 0x1;
        if (mc != 1)
          s->P <<= 1;
        s->V = U = diw - 1;
        sd = s;

#ifdef SHOW
        {
          register int k;
          for (k = 0; k < W; k++)
            if (s->P & (1 << k))
              printf(" +1");
            else if (s->M & (1 << k))
              printf(" -1");
            else
              printf("  0");
          printf(" [%d]", s->V);
        }
#endif
#ifdef STATS
        aveinc += 1;
#endif
      } else {
        U = sd->V;
        while (U > diw) {
          U = (--sd)->V;
#ifdef STATS
          avedec += 1;
#endif
        }
      }
      if (sd == SE && U <= dif)
        printf("  Match at %d\n", base + i);
#ifdef SHOW
      else
        printf("\n");
#endif
    }

    while (sd > S) {
      i = sd->V;
      P = sd->P;
      M = sd->M;

#ifdef TAIL
      {
        register int k;
        printf(" %6d:", base + rem);
        for (k = 0; k < W; k++)
          if (sd->P & (1 << k))
            printf("P");
          else if (sd->M & (1 << k))
            printf("M");
          else
            printf("0");
        printf(" [%d]\n", sd->V);
      }
#endif
      Y = Ebit;
      for (X = 0; X < W; X++) {
        if (P & Y) {
          i -= 1;
          if (i <= dif)
            break;
        } else if (M & Y)
          i += 1;
        Y >>= 1;
      }
      if (i <= dif)
        break;
#ifdef TAIL
      printf("  Dec'd\n");
#endif
#ifdef STATS
      averes += 1;
#endif
      sd -= 1;
    }
  }

#ifdef STATS
  {
    double len;
    len = (base + rem) - 1.;
    printf("Ave. Depth = %g, Ave. Inc = %g, Ave. Dec = %g Ave. Res = %g",
           avedep / len + 1., aveinc / len, avedec / len, averes / len);
    printf(" Ave. Filter = %g\n", avezero / len);
  }
#endif

  if (sd == SE) {
    P = sd->P;
    M = sd->M;
    U = sd->V;
    for (i = 0; i < rem; i++) {
      if (P & Ebit)
        U -= 1;
      else if (M & Ebit)
        U += 1;
      P <<= 1;
      M <<= 1;
      if (U <= dif)
        printf("  Match at %d\n", base + i);
#ifdef SHOW
      printf("#: %5d(%2d)\n", U, sd - S);
#endif
    }
  }
}

#include "main.i"
