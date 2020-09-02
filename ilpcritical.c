/*
 * Copyright: Tatsuya Akutsu, Kyoto University 
 * This program can be used only for acamemic purposes.
 * The author does not have any responsibility on problems caused by this program.
 * */

#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include        <sys/time.h>
#include        <math.h>

double  drand48();
double getendtime();

#define TRUE    1
#define FALSE   0
 
#define MAXNODES	1501

#define	NODENAMELEN	20

int	NUMNODE;
int	NUMTOTALNODE;

int	ADJ[MAXNODES][MAXNODES];
char	NODENAMES[MAXNODES][NODENAMELEN];

#define	INTERMITTENT	0
#define	CRITICAL	1
#define	REDUNDANT	-1
int	NODETYPE[MAXNODES];


int	MDSTBL[MAXNODES];
int	FVSTBL[MAXNODES];
int	CRTTBL[MAXNODES];
int	INTTBL[MAXNODES];
int	NODEATTR[MAXNODES];

int ident_node(char str[]);
int fvs_to_ip2(int n,int mode,int nodei);
int get_cplex_fvs2(int n,int fvstbl[]);
int output_list(int n,int tbl[],char fname1[],char fname2[]);
int output_list2(int n,int tbl[],char fname2[]);

char	STRBUF[5001];

int main(int argc,char *argv[])
{
	int	n,m,m2,i,j,i1,i2,ii1,ii2,deg,degcnt;
	char	str1[100],str2[100],str3[100];
	double	w;
	FILE	*fp;

	if (argc != 1) {
		fprintf(stderr,"Usage: iprmdsneuro\non");
		exit(1);
	}


	system("rm fvs.batch >& err");
	system("rm fvs.lp >& err");
	system("rm fvs.sol >& err");
	if ((fp = fopen("fvs.batch","w"))==NULL) {
		printf("cannot open fvs.batch\n");
		exit(1);
	}
	fprintf(fp,"read fvs.lp\n");
	fprintf(fp,"optimize\n");
	fprintf(fp,"write fvs.sol\n");
	fclose(fp);

	if ((fp = fopen("neuronodes.txt","r"))==NULL) {
		printf("cannot open neuronodes.txt\n");
		exit(0);
	}
	NUMTOTALNODE = 0;
	while(fgets(STRBUF,5000,fp)!=NULL) {
		NUMTOTALNODE++;
	}
	fclose(fp);
	NUMTOTALNODE--;

	if ((fp = fopen("neuroedges.txt","r"))==NULL) {
		printf("cannot open neuroedges.txt\n");
		exit(1);
	}

	NUMNODE = 0;
	if (fgets(STRBUF,5000,fp)==NULL || sscanf(STRBUF,"%s %s %s",str1,str3,str2)!=3 || strcmp(str1,"source")!=0 || strcmp(str3,"weight")!=0 || strcmp(str2,"target")!=0) {
		printf("Error at the first line: %s\n",STRBUF);
		exit(0);
	}
	m = 0;
	while(fgets(STRBUF,5000,fp)!=NULL && sscanf(STRBUF,"%s %lf %s",str1,&w,str2)==3) {
		i1 = ident_node(str1);
		i2 = ident_node(str2);
		m++;
	}

	fclose(fp);

	n = NUMNODE;

	for(i = 0;i < n;i++) {
		for(j = 0;j < n;j++) {
			ADJ[i][j] = 0;
		}
	}

	if ((fp = fopen("neuroedges.txt","r"))==NULL) {
		printf("cannot open neuroedges.txt\n");
		exit(1);
	}
	if (fscanf(fp,"%s %s %s",str1,str3,str2)!=3) {
		printf("Error1: %s %s %s\n",str1,str2,str3);
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

	degcnt = 0;
	for(j = 0;j < n;j++) {
		deg = 0;
		for(i = 0;i < n;i++) {
			if (ADJ[i][j] == 1) {
				deg++;
			}
		}
		if (deg > 0) { 
			degcnt++;
		}
	}


	fvs_to_ip2(n,0,0);

	system("cplex < fvs.batch > fvs.err");
	m = get_cplex_fvs2(n,FVSTBL);

	printf("#FVS = %d\n",m);
	output_list(n,FVSTBL,"fvslist.txt","fvs01list.txt");

	
	for(i = 0;i < n;i++) {
		NODETYPE[i] = INTERMITTENT;
	}
	for(i = 0;i < n;i++) {
		system("rm fvs.sol >& err");
		fvs_to_ip2(n,-1,i);
		system("cplex < fvs.batch > fvs.err");
		m2 = get_cplex_fvs2(n,FVSTBL);
		if (m2 < 0 || m2 > m) {
			NODETYPE[i] = REDUNDANT;
			continue;
		}

		system("rm fvs.sol >& err");
		fvs_to_ip2(n,1,i);
		system("cplex < fvs.batch > fvs.err");
		m2 = get_cplex_fvs2(n,FVSTBL);

		if (m2 < 0 || m2 > m) {
			NODETYPE[i] = CRITICAL;
		}
	}

	for(i = 0;i < n;i++) {
		NODEATTR[i] = 0;
	}
		
	printf("CRITICAL = \{");
	for(i = 0;i < n;i++) {
		if (NODETYPE[i] == CRITICAL) {
			NODEATTR[i] = 2;
			CRTTBL[i] = 1;
			printf("%s  ",NODENAMES[i]);
			if ((j % 10)==9) {
				printf("\n");
			}
		}
		else {
			CRTTBL[i] = 0;
		}
	}
	printf("}\n");

	printf("INTERMITTENT = \{");
	for(i = 0;i < n;i++) {
		if (NODETYPE[i] == INTERMITTENT) {
			NODEATTR[i] = 1;
			INTTBL[i] = 1;
			printf("%s  ",NODENAMES[i]);
			if ((j % 10)==9) {
				printf("\n");
			}
		}
		else {
			INTTBL[i] = 0;
		}
	}
	printf("}\n");

	output_list(n,CRTTBL,"crtlist.txt","crt01list.txt");
	output_list(n,INTTBL,"intlist.txt","int01list.txt");
	output_list2(n,NODEATTR,"nodetypelist.txt");
}

int ident_node(char str[])
{
	int	i;

	if (strlen(str) >= NODENAMELEN-1) {
		printf("Too long node name: %s\n",str);
		exit(0);
	}
	for(i = 0;i < NUMNODE;i++) {
		if (strcmp(NODENAMES[i],str)==0) {
			return i;
		}
	}
	strcpy(NODENAMES[NUMNODE],str);
	NUMNODE++;
	if (NUMNODE > MAXNODES) {
		printf("Too many nodes (> %d) \n",MAXNODES);
		exit(0);
	}
}

int get_cplex_fvs2(int n,int fvstbl[])
{
	FILE	*fp;
	int	i,j,h,m,flag;
	char	buf[1000],buf2[1000],vname[1000];
	double act;

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
		printf("cannot open fvs.sol\n"); exit(0);
	}
	for(i = 0;i < n;i++) {
		fvstbl[i] = 0;
	}
	m = 0;
	while(fgets(buf,500,fp)!=NULL) {
		if (sscanf(buf," <variable name=\"x%d\" index=\"%d\" value=\"%lf\"/>",&i,&h,&act)!=3) {
			continue;
		}
		if (act >= 0.9999) {
			fvstbl[i-1] = 1;
			m++;
		}
	}
	return m;
}


int fvs_to_ip2(int n,int mode,int nodei)
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
		/* redundant */
	if (mode == -1) {
		fprintf(fp,"  x%d >= 1\n",nodei+1);
	}
		/* critical */
	else if (mode == 1) {
		fprintf(fp,"  x%d <= 0\n",nodei+1);
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

int	MDSFVSTBL[MAXNODES];

int output_list(int n,int tbl[],char fname1[],char fname2[])
{
	int	i,nid;
	FILE	*fp;

	for(i = 0;i < NUMTOTALNODE;i++) {
		MDSFVSTBL[i] = 0;
	}
		

	if ((fp = fopen(fname1,"w"))==NULL) {
		printf("Cannot open %s\n",fname1);
		exit(0);
	}
	for(i = 0;i < n;i++) {
		if (tbl[i]==1) {
			fprintf(fp,"%s\n",NODENAMES[i]);
		}
	}
	fclose(fp);

	if ((fp = fopen(fname2,"w"))==NULL) {
		printf("Cannot open %s\n",fname2);
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
			MDSFVSTBL[nid-1] = 1;
		}
	}
	for(i = 0;i < NUMTOTALNODE;i++) {
		fprintf(fp,"%d\n",MDSFVSTBL[i]);
	}
	fclose(fp);
}

int output_list2(int n,int tbl[],char fname2[])
{
	int	i,nid;
	FILE	*fp;

	for(i = 0;i < NUMTOTALNODE;i++) {
		MDSFVSTBL[i] = 0;
	}

	if ((fp = fopen(fname2,"w"))==NULL) {
		printf("Cannot open %s\n",fname2);
		exit(0);
	}
	for(i = 0;i < n;i++) {
		if (tbl[i]==1 || tbl[i]==2) {
			if (strncmp(NODENAMES[i],"Node",4)!=0) {
				printf("Error in node name: %s\n",NODENAMES[i]);
				exit(0);
			}
			if (sscanf(&(NODENAMES[i][4]),"%d",&nid)!=1) {
				printf("Error in node name2: %s\n",NODENAMES[i]);
				exit(0);
			}
			MDSFVSTBL[nid-1] = tbl[i];
		}
	}
	for(i = 0;i < NUMTOTALNODE;i++) {
		if (MDSFVSTBL[i]==2) {
			fprintf(fp,"3\n",MDSFVSTBL[i]);
		}
		else if (MDSFVSTBL[i]==1) {
			fprintf(fp,"2\n",MDSFVSTBL[i]);
		}
		else {
			fprintf(fp,"0\n",MDSFVSTBL[i]);
		}
	}
	fclose(fp);
}





