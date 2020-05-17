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

  printf("----Addestra N1----\n");
  for(int i=0;i<1000;i++)
    {
      d[0]=(double)rand()/(double)RAND_MAX;
      d[1]=(double)rand()/(double)RAND_MAX;
      c[0]=1;
      if(d[0]<d[1]) c[0]=0;
  
      r = rnss_Addestra(r,&par,d,c);
    }

  printf("----Test N1----\n");
   for(int i=0;i<10;i++)
    {
      d[0]=(double)rand()/(double)RAND_MAX;
      d[1]=(double)rand()/(double)RAND_MAX;
      rnss_Classifica(r, d);
      printf("(%lf,%lf)->%d\n",d[0],d[1],r->strato_uscita[0]>0.5);
	
    }

   rnss_Libera_rete(r);
   rnss_rete *  r2 =  rnss_Crea_rete( 2,1,20,0);
   
   printf("Neuroni primo strato %d\n", r2->N_neuroni_primo_strato_interno);
   printf("Strati computazionali %d\n", r2->N_strati_computazionali);
   
   par.fattore_apprendimento = 0.6;
   printf("----Addestra N2----\n");
   
   for(int i=0;i<100000;i++)
     {
       d[0]=(double)rand()/(double)RAND_MAX;
       d[1]=(double)rand()/(double)RAND_MAX;
       c[0]=1;
       if(d[0]*d[0]<d[1]) c[0]=0;
       
       r2 = rnss_Addestra(r2,&par,d,c);
     }
   
   printf("----Test N2----\n");
   for(int i=0;i<100;i++)
     {
       d[0]=(double)rand()/(double)RAND_MAX;
       d[1]=(double)rand()/(double)RAND_MAX;
       rnss_Classifica(r2, d);
       
       printf("(%lf,%lf)>%d\n",d[0],d[1],r2->strato_uscita[0]>=0.5);
       
    }
   
   //rnss_Libera_rete(r2);
   
   rnss_rete *  r3 =  rnss_Crea_rete( 2,1,5,5);
   
   printf("Neuroni primo strato %d\n", r3->N_neuroni_primo_strato_interno);
   printf("Strati computazionali %d\n", r3->N_strati_computazionali);
   
   par.fattore_apprendimento = 0.3;
   printf("----Addestra N3----\n");
   
   for(int i=0;i<1000000;i++)
     {
       d[0]=(double)rand()/(double)RAND_MAX;
       d[1]=(double)rand()/(double)RAND_MAX;
       c[0]=1;
       //if((0.5-d[0])*(0.5-d[0])+(0.5-d[1])*(0.5-d[1])>0.15) c[0]=0;
       if(d[0]*d[0]+d[1]*d[1]>0.5) c[0]=0;
       r3 = rnss_Addestra(r3,&par,d,c);
       //printf("R: %lf\n",r3->v_y3[0]);
     }
   
   printf("----Test N3----\n");
   for(int i=0;i<100;i++)
     {
       d[0]=(double)rand()/(double)RAND_MAX;
       d[1]=(double)rand()/(double)RAND_MAX;
       rnss_Classifica(r3, d);
       
       printf("(%lf,%lf)>%lf\n",d[0],d[1],r3->strato_uscita[0]);
       
    }

   
}
