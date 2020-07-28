/*
Copyright: Tatsuya Akutsu, Kyoto University 
This program can be used only for acamemic purposes.
The author does not have any responsibility on problems caused by this program.
*/

#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include        <sys/time.h>
#include        <math.h>

double  drand48();
double getendtime();

#define MAXNODES	3000

#define	NODENAMELEN	20

int	NUMNODES;
int	NUMEDGES;
int	NUMORIGNODES;

char	ADJ[MAXNODES][MAXNODES];
char	NODENAMES[MAXNODES][NODENAMELEN];

int	FVSTBL[MAXNODES];

int	FVSORIGTBL[MAXNODES];

int ident_node(char str[]);
int fvs_to_ilp(int n);
int ilp_to_fvs(int n,int fvstbl[]);
int output_01_list(int n,int tbl[],char outfile[]);

char	STRBUF[5001];

int main(int argc,char *argv[])
{
	int	n,m,f,i,j,i1,i2;
	char	str1[100],str2[100],str3[100];
	double	w;
	FILE	*fp;

	if (argc != 2) {
		printf("Usage: ilpfvs outputfilename\n");
		exit(0);
	}


	system("rm fvs.batch >& err");
	system("rm fvs.lp >& err");
	system("rm fvs.sol >& err");
	if ((fp = fopen("fvs.batch","w"))==NULL) {
		printf("Cannot open fvs.batch\n");
		exit(1);
	}
	fprintf(fp,"read fvs.lp\n");
	fprintf(fp,"optimize\n");
	fprintf(fp,"write fvs.sol\n");
	fclose(fp);

	if ((fp = fopen("neuronodes.txt","r"))==NULL) {
		printf("Cannot open neuronodes.txt\n");
		exit(0);
	}
	NUMORIGNODES = 0;
	while(fgets(STRBUF,5000,fp)!=NULL) {
		NUMORIGNODES++;
	}
	fclose(fp);
	NUMORIGNODES--;

	if ((fp = fopen("neuroedges.txt","r"))==NULL) {
		printf("Cannot open neuroedges.txt\n");
		exit(0);
	}

	NUMNODES = 0;
	if (fgets(STRBUF,5000,fp)==NULL || sscanf(STRBUF,"%s %s %s",str1,str3,str2)!=3 || strcmp(str1,"source")!=0 || strcmp(str3,"weight")!=0 || strcmp(str2,"target")!=0) {
		printf("Error in neuroedges.txt: %s\n",STRBUF);
		exit(0);
	}
	m = 0;
	while(fgets(STRBUF,5000,fp)!=NULL && sscanf(STRBUF,"%s %lf %s",str1,&w,str2)==3) {
		i1 = ident_node(str1);
		i2 = ident_node(str2);
		m++;
	}
	fclose(fp);

	NUMEDGES = m;

	n = NUMNODES;

	printf("#nodes = %d    #relevant nodes = %d    #edges = %d\n",NUMORIGNODES,n,m);

	for(i = 0;i < n;i++) {
		for(j = 0;j < n;j++) {
			ADJ[i][j] = 0;
		}
	}

	if ((fp = fopen("neuroedges.txt","r"))==NULL) {
		printf("Cannot open neuroedges.txt\n");
		exit(0);
	}
	if (fscanf(fp,"%s %s %s",str1,str3,str2)!=3) {
		printf("Error in neuroedges.txt: %s %s %s\n",str1,str2,str3);
		exit(0);
	}
	m = 0;
	while(fscanf(fp,"%s %lf %s",str1,&w,str2)==3) {
		i1 = ident_node(str1);
		i2 = ident_node(str2);
		ADJ[i1][i2] = 1;
		m++;
	}
	fclose(fp);

	if (m != NUMEDGES) {
		printf("Error in edges\n");
		exit(0);
	}

	fvs_to_ilp(n);

	system("cplex < fvs.batch > fvs.err");
	f = ilp_to_fvs(n,FVSTBL);

	printf("#FvsNodes = %d\n",f);

	output_01_list(n,FVSTBL,argv[1]);
}

int ident_node(char str[])
{
	int	i;

	if (strlen(str) >= NODENAMELEN-1) {
		printf("Too long node name: %s\n",str);
		exit(0);
	}
	for(i = 0;i < NUMNODES;i++) {
		if (strcmp(NODENAMES[i],str)==0) {
			return i;
		}
	}
	strcpy(NODENAMES[NUMNODES],str);
	NUMNODES++;
	if (NUMNODES >= MAXNODES) {
		printf("Too many nodes\n");
		exit(0);
	}

	return NUMNODES-1;
}

int ilp_to_fvs(int n,int fvstbl[])
{
	FILE	*fp;
	int	i,j,h,m,flag;
	char	buf[1000];
	double	val;

	if ((fp = fopen("fvs.err","r"))==NULL) {
		printf("cannot open fvs.err\n"); exit(0);
	}
	flag = 0;
	while (fgets(buf,1000,fp)!=NULL) {
		if (strncmp("MIP - Integer optimal",buf,21)==0) {
			flag = 1;
			break;
		}
		if (strncmp("MIP - Integer infeasible",buf,24)==0) {
			flag = -1;
			break;
		}
	}
	fclose(fp);

	if (flag == -1) {
		return -1;
	}

	if (flag == 0) {
		printf("Error in cplex\n");
		exit(0);
	}

	if ((fp = fopen("fvs.sol","r"))==NULL) {
		printf("Cannot open fvs.sol\n");
		exit(0);
	}
	for(i = 0;i < n;i++) {
		fvstbl[i] = 0;
	}
	m = 0;
	while(fgets(buf,500,fp)!=NULL) {
		if (sscanf(buf," <variable name=\"x%d\" index=\"%d\" value=\"%lf\"/>",&i,&h,&val)!=3) {
			continue;
		}
		if (val >= 0.9999) {
			fvstbl[i-1] = 1;
			m++;
		}
	}
	return m;
}

int fvs_to_ilp(int n)
{
	FILE	*fp;
	int	i,j,flag,jj1,jj2,deg;
	double	lnp,p;
	char	fname[100];


	sprintf(fname,"fvs.lp");
	if ((fp = fopen(fname,"w"))==NULL) {
		printf("cannot open fvs.lp\n"); exit(0);
	}

	fprintf(fp,"\\Problem name: fvs.lp\n");
	fprintf(fp,"Minimize\n");
	for(i = 0;i < n-1;i++) {
		fprintf(fp,"  x%d + ",i+1);
	}
	fprintf(fp,"  x%d\n",n);
	fprintf(fp,"Subject To\n");
	for(i = 0;i < n;i++) {
		if (ADJ[i][i] == 1) {
			fprintf(fp,"  x%d >= 1\n",i+1);
			fprintf(fp,"  x%d <= 1\n",i+1);
		}
	}
			
	for(j = 0;j < n;j++) {
		for(i = 0;i < n;i++) {
			if (i == j) {
				continue;
			}
			if (ADJ[i][j] == 1) {
				fprintf(fp,"  w%d - w%d + %d x%d >= 1\n",i+1,j+1,n,i+1);
			}
		}
	}
	fprintf(fp,"Bounds\n");
	for(i = 0;i < n;i++) {
		fprintf(fp,"  0 <= w%d <= %d\n",i+1,n-1);
	}
	fprintf(fp,"Binary\n");
	for(i = 0;i < n;i++) {
		fprintf(fp,"  x%d\n",i+1);
	}
	fprintf(fp,"General\n");
	for(i = 0;i < n;i++) {
		fprintf(fp,"  w%d\n",i+1);
	}

	fprintf(fp,"END\n");

	fclose(fp);
}


int output_01_list(int n,int tbl[],char outfile[])
{
	int	i,nid;
	FILE	*fp;

	for(i = 0;i < NUMORIGNODES;i++) {
		FVSORIGTBL[i] = 0;
	}
		
	if ((fp = fopen(outfile,"w"))==NULL) {
		printf("Cannot open %s\n",outfile);
		exit(0);
	}
	for(i = 0;i < n;i++) {
		if (tbl[i]==1) {
			if (strncmp(NODENAMES[i],"Node",4)!=0) {
				printf("Error in node name: %s\n",NODENAMES[i]);
				exit(0);
			}
			if (sscanf(&(NODENAMES[i][4]),"%d",&nid)!=1) {
				printf("Error in node name2: %s\n",NODENAMES[i]);
				exit(0);
			}
			FVSORIGTBL[nid-1] = 1;
		}
	}
	for(i = 0;i < NUMORIGNODES;i++) {
		fprintf(fp,"%d\n",FVSORIGTBL[i]);
	}
	fclose(fp);
}

