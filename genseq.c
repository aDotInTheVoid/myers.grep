/* genseq "#l,#a":
     Places a random string of length #l over alphabet #a on stdout.
*/

#include <stdio.h>
#include <math.h>

#define SIG 70

char *cset =
   "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*(";

int len, alpha;
double drand48();
void   srand48();

main(argc,argv) int argc; char *argv[];
{ register int i;

  if (argc <= 1)
    error(1,"usage: #l,#a");
  if (sscanf(argv[1],"%d,%d",&len,&alpha) != 2)
    error(1,"usage: #l,#a");
  if (len < 0 || alpha < 0)
    error(0,"positive arguments only");
  if (alpha > SIG)
    error(0,"alphabet too large");

  for (i = 1; i <= len; i++)
    fputc(cset[(int) (drand48()*alpha)],stdout);
  exit (0);
}

error(usage,txt) int usage; char *txt;
{ if (usage)
    fprintf(stderr,"%s\n",txt);
  else
    fprintf(stderr,"#l = %d\n#a = %d\n\n%s\n",len,alpha,txt);
  exit (1);
}
