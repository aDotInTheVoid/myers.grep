CC = gcc
CFLAGS = -O

all : ukk chang wmm4 wmm5 myers banav mygrep agrep bngrep \
      ukkS changS wmm4S wmm5S myersS banavS mygrepS agrepS bngrepS

data :
	gcc -O genseq.c
	a.out "1000000,2" >DATA.2
	a.out "1000000,4" >DATA.4
	a.out "1000000,8" >DATA.8
	a.out "1000000,16" >DATA.16
	a.out "1000000,32" >DATA.32
	a.out "1000000,64" >DATA.64
	rm a.out

ukk : ukk.c
	$(CC) $(CFLAGS) -o ukk ukk.c
ukkS : ukk.c
	$(CC) $(CFLAGS) -DSTATS -o ukkS ukk.c

chang : chang.c
	$(CC) $(CFLAGS) -o chang chang.c
changS : chang.c
	$(CC) $(CFLAGS) -DSTATS -o changS chang.c

gen4 : gen4.c
	$(CC) $(CFLAGS) -o gen4 gen4.c

wmm4 : wmm.c gen4
	gen4 4 -y >xxx.c
	$(CC) $(CFLAGS) -o wmm4 wmm.c xxx.c
	rm xxx.c
wmm4S : wmm.c gen4
	gen4 4 -y >xxx.c
	$(CC) $(CFLAGS) -DSTATS -o wmm4S wmm.c xxx.c
	rm xxx.c

wmm5 : wmm.c gen4
	gen4 5 -y >xxx.c
	$(CC) $(CFLAGS) -o wmm5 wmm.c xxx.c
	rm xxx.c
wmm5S : wmm.c gen4
	gen4 5 -y >xxx.c
	$(CC) $(CFLAGS) -DSTATS -o wmm5S wmm.c xxx.c
	rm xxx.c

myers : myers.c
	$(CC) $(CFLAGS) -o myers myers.c
myersS : myers.c
	$(CC) $(CFLAGS) -DSTATS -o myersS myers.c

banav : banav.c
	$(CC) $(CFLAGS) -o banav banav.c
banavS : banav.c
	$(CC) $(CFLAGS) -DSTATS -o banavS banav.c

mygrep : mygrep.c
	$(CC) $(CFLAGS) -o mygrep mygrep.c
mygrepS : mygrep.c
	$(CC) $(CFLAGS) -DSTATS -o mygrepS mygrep.c

agrep : agrep.c
	$(CC) $(CFLAGS) -o agrep agrep.c
agrepS : agrep.c
	$(CC) $(CFLAGS) -DSTATS -o agrepS agrep.c

bngrep : bngrep.c
	$(CC) $(CFLAGS) -o bngrep bngrep.c
bngrepS : bngrep.c
	$(CC) $(CFLAGS) -DSTATS -o bngrepS bngrep.c

clear:
	rm -f ukk chang wmm4 wmm5 gen4 myers banav mygrep agrep bngrep
	rm -f ukkS changS wmm4S wmm5S myersS banavS mygrepS agrepS bngrepS
	rm -f DATA.2 DATA.4 DATA.8 DATA.16 DATA.32 DATA.64
	rm -f xxx.c
