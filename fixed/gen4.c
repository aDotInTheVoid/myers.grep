/* Generate Four-Russians tables */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int K;

static int  *STAB, *DTAB;
static char *CTAB;

static int pow3[50], pow2[50];

static void STATE(k,idx,ost,car) register int k, idx, ost, car;
{ register int rdx, p3;

  if (k < K)
    { rdx = idx + pow2[k];
      k  += 1;
      p3  = pow3[k];
      if (car < 0)
        { ost += 2*p3;
          STATE(k,idx      ,ost, 1);
          STATE(k,rdx      ,ost, 1);
          STATE(k,idx += p3,ost, 0);
          STATE(k,rdx += p3,ost, 0);
          STATE(k,idx +  p3,ost,-1);
          STATE(k,rdx +  p3,ost,-1);
        }
      else if (car > 0)
        { STATE(k,idx      ,ost   , 1);
          STATE(k,rdx      ,ost   , 1);
          STATE(k,idx += p3,ost   , 0);
          STATE(k,rdx += p3,ost+p3, 1);
          STATE(k,idx +  p3,ost   ,-1);
          STATE(k,rdx +  p3,ost+p3, 0);
        }
      else
        { ost += p3;
          STATE(k,idx      ,ost   , 1);
          STATE(k,rdx      ,ost   , 1);
          STATE(k,idx += p3,ost   , 0);
          STATE(k,rdx += p3,ost+p3, 1);
          STATE(k,idx +  p3,ost   ,-1);
          STATE(k,rdx +  p3,ost+p3, 0);
        }
    }
  else
    { STAB[idx] = ost;
      CTAB[idx] = car;
    }
}

void DELTA(k,idx,sum) register int k, idx, sum;
{ register int p3;
  if (k < K)
    { k += 1;
      p3 = pow3[k];
      DELTA(k,idx      ,sum-1);
      DELTA(k,idx += p3,sum  );
      DELTA(k,idx +  p3,sum+1);
    }
  else
    DTAB[idx] = DTAB[idx+1] = DTAB[idx+2] = sum;
}

int main(argc,argv) int argc; char *argv[];
{ register int p, c, ix;

  if (argc < 2)
    { fprintf(stderr,"%s: usage is '%s <tuple size> [ -y -n -t ]'\n",
                     argv[0],argv[0]);
      exit (1);
    }

  K = atoi(argv[1]);

  if (K <= 0)
    { fprintf(stderr,"%s: tuple size, %d, must be positive\n",argv[0]);
      exit (1);
    }
  if (K > 11)
    { fprintf(stderr,"%s: tuple size, %d, is too large\n",argv[0]);
      exit (1);
    }

  pow3[0] = 1;
  for (c = 1; c <= K+1; c++)
    pow3[c] = 3*pow3[c-1];
  pow2[0] = pow3[K+1];
  for (c = 1; c <= K; c++)
    pow2[c] = 2*pow2[c-1];

  if (argc < 3 || strcmp(argv[2],"-y") != 0)
    { fprintf(stderr,"Tables will occupy %d bytes.\n",5*pow2[K]+4*pow3[K+1]);
      if (argc >= 3 && strcmp(argv[2],"-n") == 0) exit (0);
      fprintf(stderr,"Do you wish to proceed (y/n): ");
      c = getchar();
      if (c != '\n' && c != 'y') exit (0);
    }

  STAB = (int *)  malloc(sizeof(int)*pow2[K]);
  CTAB = (char *) malloc(sizeof(char)*pow2[K]);
  DTAB = (int *)  malloc(sizeof(int)*pow3[K+1]);

  for (c = 0; c <= 2; c++)
    STATE(0,c,1,c-1);
  DELTA(0,0,0);

  if (argc >= 3 && strcmp(argv[2],"-t") == 0) exit (0);

  printf("int K = %d;\n\n",K);

  ix = pow2[K];
  printf("int STAB[] = {\n");
#ifdef DEBUG
  for (p = 0; p < ix; p += 3)
    { printf("\t\t");
      for (c = 0; c <= 2; c++)
        printf("%5d, ",STAB[p+c]/3);
      printf("\n");
    }
#else
  for (p = 0; p < ix; p++)
    printf("%d,",STAB[p]);
#endif
  printf("             };\n\n");

  printf("char CTAB[] = {\n");
#ifdef DEBUG
  for (p = 0; p < ix; p += 3)
    { printf("\t\t");
      for (c = 0; c <= 2; c++)
        printf("%2d, ",CTAB[p+c]);
      printf("\n");
    }
#else
  for (p = 0; p < ix; p++)
    printf("%d,",CTAB[p]);
#endif
  printf("             };\n\n");

  printf("int DTAB[] = {\n");
  ix = pow3[K+1];
#ifdef DEBUG
  for (p = 0; p < ix; p += 3)
    { printf("\t\t");
      for (c = 0; c <= 2; c++)
        printf("%2d, ",DTAB[p+c]);
      printf("\n");
    }
#else
  for (p = 0; p < ix; p++)
    printf("%d,",DTAB[p]);
#endif
  printf("             };\n");

  exit (0);
}
