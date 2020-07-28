
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define	MAXNODES	5000
double	NODEX[MAXNODES];
double	NODEY[MAXNODES];
#define	MAXEDGES	2000000
int	EDGES[MAXEDGES][2];

int	CONNECTEDID[MAXNODES];
int	CONNECTEDSIZE[MAXNODES];

char	ADJ[MAXNODES][MAXNODES];

int delcomma(char infile[], char outfile[]);

#define	BIGBUFSIZE	1000000
char	BIGBUF[BIGBUFSIZE];

int	main(int argc, char *argv[])
{
	FILE	*fp;
	int	i,j,k,m,n,cnt,zcnt,errcnt;
	double	x,tmp;
	char	buf[100];

	if (argc != 2) {
		printf("Usage: convfile pdfXXXXX.txt\n");
		exit(0);
	}

	delcomma(argv[1],"pdftempfile.txt");


	if ((fp = fopen("pdftempfile.txt","r"))==NULL) {
		printf("Cannnot open pdftempfile.txt\n");
		exit(0);
	}
	n = 0;
	while(fgets(BIGBUF,BIGBUFSIZE-2,fp)!=NULL) {
		n++;
		if (n >= MAXNODES) {
			printf("Too many nodes\n");
			exit(0);
		}
	}
	fclose(fp);
			
	if ((fp = fopen("pdftempfile.txt","r"))==NULL) {
		printf("Cannnot open pdftempfile.txt\n");
		exit(0);
	}
	m = 0;
	for(i = 0;i < n;i++) {
		for(j = 0;j < n;j++) {
			if (fscanf(fp,"%d",&k)!=1) {
				printf("Error5: %d %d\n",i,j);
				exit(0);
			}
			ADJ[i][j] = k;
			if (k == 1) {
				EDGES[m][0] = i;
				EDGES[m][1] = j;
				m++;
				if (m >= MAXEDGES) {
					printf("Too many edges\n");
					exit(0);
				}
			}
		}
	}
	fclose(fp);

	printf("#nodes = %d    #edges = %d\n",n,m);

	fp = fopen("neuroedges.txt","w");
	fprintf(fp,"source\tweight\ttarget\n");
	for(i = 0;i < n;i++) {
		for(j = 0;j < n;j++) {
			if (ADJ[i][j]==1) {
				fprintf(fp,"Node%d\t1.0\tNode%d\n",i+1,j+1);
			}
		}
	}
	fclose(fp);

	fp = fopen("neuronodes.txt","w");
	fprintf(fp,"NodeID\tAttributes\n");
	for(i = 0;i < n;i++) {
		fprintf(fp,"Node%d\t1\n",i+1,j);
	}
	fclose(fp);

}

int delcomma(char infile[], char outfile[])
{
	FILE	*fp1,*fp2;
	char	ch;
	int	cnt;

	if ((fp1 = fopen(infile,"r"))==NULL) {
		printf("Cannot open %s\n",infile);
		exit(0);
	}
	if ((fp2 = fopen(outfile,"w"))==NULL) {
		printf("Cannot open %s\n",outfile);
		exit(0);
	}
	cnt = 0;
	while((ch=fgetc(fp1)) != EOF) {
		if (ch == ',') {
			ch = ' ';
			cnt++;
		}
		fputc(ch,fp2);
	}

	fclose(fp1);
	fclose(fp2);
}
 

