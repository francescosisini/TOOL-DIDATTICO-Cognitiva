#include "rnss_libpercettrone.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
  rnss_rete * r =  rnss_Crea_rete(
			   2,
			   1,
			   0,
			   0);

  rnss_parametri par;
  par.fattore_apprendimento = 0.2;
  
  double d[]={0,1};
  double c[1] = {0};

  printf("----Addestra----\n");
  for(int i=0;i<1000;i++)
    {
      d[0]=(double)rand()/(double)RAND_MAX;
      d[1]=(double)rand()/(double)RAND_MAX;
      c[0]=1;
      if(d[0]<d[1]) c[0]=0;
  
      r = rnss_Addestra(r,&par,d,c);
    }

  printf("----Test----\n");
   for(int i=0;i<10;i++)
    {
      d[0]=(double)rand()/(double)RAND_MAX;
      d[1]=(double)rand()/(double)RAND_MAX;
      rnss_Classifica(r, d);
      printf("(%lf,%lf)->%d\n",d[0],d[1],r->strato_uscita[0]>0.5);
	
    }
}
