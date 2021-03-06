#include "rele_libreteneurale.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{

  double eqm = 0;
  
  rele_rete * r =  rele_Crea_rete(
			   2,
			   1,
			   0,
			   0);

  rele_parametri par;
  par.fattore_apprendimento = 0.05;
  
  double d[]={0,1};
  double c[1] = {0};
  
  printf("Neuroni primo strato %d\n", r->N_neuroni_primo_strato_intercalare);
  printf("Strati computazionali %d\n", r->N_strati_computazionali);
  printf("----Addestra N1----\n");
  int iterazioni = 0;
  for(int i=0;i<30000;i++)
    {
      iterazioni++;
      d[0]=(double)rand()/(double)RAND_MAX;
      d[1]=(double)rand()/(double)RAND_MAX;
      c[0]=1;
      if(d[0]<d[1]) c[0]=0;
  
      r = rele_Addestra(r,&par,d,c);
      //printf("EQM %lf \n",r->EQM);
      if(r->EQM<0.001) break;
    }

  printf("----Test N1----\n");
  printf("Terminato in %d iterazioni\n",iterazioni);
   for(int i=0;i<10;i++)
    {
      d[0]=(double)rand()/(double)RAND_MAX;
      d[1]=(double)rand()/(double)RAND_MAX;
      rele_Classifica(r, d);
      printf("(%lf,%lf)->%d\n",d[0],d[1],r->strato_uscita[0]>0.5);
	
    }

   printf("----Addestra N2----\n");
   
   rele_Libera_rete(r);

    
   /*
   rele_rete *  r2 =  rele_Crea_rete( 2,1,30,0);
   par.fattore_apprendimento = 0.1;
   iterazioni = 0;
   eqm = 0;
   
   do
     {
       eqm = 0;
       for(int i = 0; i<100; i++)
	 {
	   iterazioni++;
	   d[0]=4.*(double)rand()/(double)RAND_MAX-2;
	   d[1]=5*(double)rand()/(double)RAND_MAX;
	   c[0]=1;
	   if(-d[0]*d[0]+4 < d[1]) c[0]=0;
	   r2 = rele_Addestra(r2,&par,d,c);
	   eqm+=1./100.*r2->EQM;
	 }
       //printf("EQM %lf\n",eqm);
     }
   while(eqm>0.005);
   
   printf("----Test N2----\n");
   printf("Terminato in %d iterazioni\n",iterazioni);
   for(int i=0;i<2500;i++)
     {
       d[0]=4.*(double)rand()/(double)RAND_MAX-2;
       d[1]=5*(double)rand()/(double)RAND_MAX;
       rele_Classifica(r2, d);
       
       printf("%lf %lf %d\n",d[0],d[1],r2->strato_uscita[0]>0.5);
       
    }
   */
   //rele_Libera_rete(r2);
   
   rele_rete *  r3 =  rele_Crea_rete( 2,1,30,5);
   
   printf("Neuroni primo strato %d\n", r3->N_neuroni_primo_strato_intercalare);
   printf("Strati computazionali %d\n", r3->N_strati_computazionali);
   
   par.fattore_apprendimento = 0.1;
   printf("----Addestra N3----\n");
   iterazioni = 0;
   do{
     eqm = 0;
     for(int i=0;i<100;i++)
       {
	 iterazioni++;
	 d[0]=4.*(double)rand()/(double)RAND_MAX-2.;
	 d[1]=5.*(double)rand()/(double)RAND_MAX;
	 c[0]=1;
	 //if((0.5-d[0])*(0.5-d[0])+(0.5-d[1])*(0.5-d[1])>0.15) c[0]=0;
	 //if(d[0]*d[0]+d[1]*d[1]>1.5) c[0]=0;
	 if(-d[0]*d[0]+4 < d[1]) c[0]=0;
	 r3 = rele_Addestra(r3,&par,d,c);
	 eqm+=1./100.*r3->EQM;
	 //printf("R: %lf\n",r3->v_y3[0]);
       }
     //printf("EQM %lf\n",eqm);
   } while(eqm>0.005);
   
   printf("----Test N3----\n");
   printf("Terminato in %d iterazioni\n",iterazioni);
   for(int i=0;i<500;i++)
     {
       d[0]=4.*(double)rand()/(double)RAND_MAX-2;
       d[1]=5.*(double)rand()/(double)RAND_MAX;
       rele_Classifica(r3, d);
       
       printf("%lf %lf %d\n",d[0],d[1],r3->strato_uscita[0]>0.5);
       
    }

   
}
