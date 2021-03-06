/*______________________________________________
 | rnss_libpercettrone
 | 
 | Francesco e Valentina Sisini (c) 2020
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "libcogni.h"
#include "rele_libpercettrone.h"

#define MAX_PESO 0.1
#define MIN_PESO -0.1


rele_rete *  rele_Addestra(rele_rete * rn,
			   rele_parametri * par,
			   double * dati,
			   double * classi)
{ 
  int l1_nd = rn->N_neuroni_ingresso;
  int l1_np = rn->N_neuroni_primo_strato_interno;
  int l2_nd = l1_np;
  int l2_np;
  /* preparazione degli ingressi e delle label delle classi*/
  rn->v_x0[0] = 1;
  
  memcpy(rn->v_x0+1,dati,rn->N_neuroni_ingresso*sizeof(double));

  /** UNO STRATO **/
  if(rn->N_strati_computazionali == 1)
    {
      memcpy(rn->v_Dt,classi,rn->N_neuroni_uscita*sizeof(double));
      /* Feed Forward: Input->L1->output to L2 */
      layer_feed_forward(
			 rn->v_s1,
			 rn->v_y1,
			 rn->v_t,
			 rn->v_x0,
			 l1_np,
			 l1_nd);

      /** Propagazione inversa dell'errore in L1  (v_t  <- v_y1) */
      for(int i=0;i<l1_np;i++)
	{
	 
	  /* correzione dei pesi (v_t) del percettrone i-esimo */
	  perc_correzione(
			   rn->v_t+i*(l1_nd+1),
			   rn->v_x0,
			   rn->v_s1[i],
			   rn->v_Dt[i]- rn->v_y1[i],
			   par->fattore_apprendimento,
			   l1_nd);
	}
      
      return rn;
    }

  /** DUE STRATI **/
   if(rn->N_strati_computazionali == 2)
     {
       l2_np = rn->N_neuroni_uscita;
       memcpy(rn->v_Du,classi,rn->N_neuroni_uscita*sizeof(double));
       
       /* Feed Forward: Input->L1->output to L2 */
       layer_feed_forward( rn->v_s1, rn->v_y1, rn->v_t,rn->v_x0,l1_np,l1_nd);
       
       /* Mappa y1 in x1 aggiungendo l'elemento x1_0=1*/
       layer_map_out_in(rn->v_x1, rn->v_y1,l2_nd);
       
       /* Feed Forward: L2->output */
       layer_feed_forward(rn->v_s2,rn->v_y2,rn->v_u,rn->v_x1,l2_np,l2_nd);
       
       
       /** Propagazione inversa dell'errore in L2  (v_u  <- v_y1) */
       for(int i=0;i<l2_np;i++)
	 {
	   /* correzione dei pesi (v_t) del percettrone i-esimo */
	   perc_correzione(
			   rn->v_u+i*(l1_nd+1),
			   rn->v_x1,
			   rn->v_s2[i],
			   rn->v_Du[i]- rn->v_y2[i],
			   par->fattore_apprendimento,
			   l2_nd);
	 }
       
       /** Propagazione inversa dell'errore in L1  (v_t <- v_y2)*/
       for(int i=0;i<l1_np;i++)
	 {
	   double dd = 0;
	   for(int j=0;j<l2_np;j++)
	     {
	       /* w: peso del i-esimo dendrite del j-esimo percettrone dello strato più esterno */
	       double w = rn->v_u_tmp[j*(l2_nd+1)+i];
	       /* correzione   */
	       dd=dd+w*(rn->v_Dt[j]-rn->v_y2[j])*Dactiv_function(rn->v_s2[j]);
	     }
	   
	   /* correzione del percettrone i-esimo*/
	   perc_correzione( rn->v_t+i*(l1_nd+1),rn->v_x0,rn->v_s1[i],dd, par->fattore_apprendimento,l1_nd);
	 }

       /* Aggiornamento dello strato L2 */
       memcpy(rn->v_u_tmp,rn->v_u,sizeof(double)*l2_np*(l1_nd+1));
       return rn;
     }
   
  /** TRE STRATI **/
   if(rn->N_strati_computazionali == 3)
     {
  
       int l3_np = rn->N_neuroni_uscita;
       int l2_np = rn->N_neuroni_secondo_strato_interno;
       int l2_nd = l1_np;
       int l3_nd = l2_np;
       memcpy(rn->v_Dv,classi,rn->N_neuroni_uscita*sizeof(double));
       
       /* Feed Forward: x0->L1->y1 */
       layer_feed_forward(rn->v_s1,rn->v_y1,rn->v_t,rn->v_x0,l1_np,l1_nd);
       
       /* Mappa y1 in x1 aggiungendo l'elemento x1_0=1*/
       layer_map_out_in(rn->v_x1, rn->v_y1,l2_nd);
       
       /* Feed Forward: x1->L2->y2 */
       layer_feed_forward(rn->v_s2,rn->v_y2,rn->v_u,rn->v_x1,l2_np,l2_nd);

        /* Mappa y2 in x2 aggiungendo l'elemento x3_0=1*/
       layer_map_out_in(rn->v_x2, rn->v_y2,l3_nd);

        /* Feed Forward: x2->L3->y3 */
       layer_feed_forward(rn->v_s3,rn->v_y3,rn->v_v,rn->v_x2,l3_np,l3_nd);

       
       
       /** Propagazione inversa dell'errore in L3  (v_v  <- v_y3) */
       for(int i=0;i<l3_np;i++)
	 {
	   
	   /* correzione dei pesi (v_t) del percettrone i-esimo */
	   perc_correzione(
			   rn->v_v+i*(l2_nd+1),
			   rn->v_x2,
			   rn->v_s3[i],
			   rn->v_Dv[i]- rn->v_y3[i],
			   par->fattore_apprendimento,
			   l3_nd);
	 }
       
       /** Propagazione inversa dell'errore in L2  (v_u <- v_y3)*/
       for(int i=0;i<l2_np;i++)
	 {
	   double dd=0;
	   for(int j=0;j<l3_np;j++)
	     {
	       /* w: peso del i-esimo dendrite del j-esimo percettrone dello strato più esterno */
	       double w = rn->v_v[j*(l3_nd+1)+i];
	       /* correzione   */
	       dd=dd+w*(rn->v_Du[j]-rn->v_y3[j])*Dactiv_function(rn->v_s3[j]);
	     }
	   
	   /* correzione del percettrone i-esimo*/
	   perc_correzione( rn->v_u+i*(l2_nd+1),rn->v_x1,rn->v_s2[i],dd, par->fattore_apprendimento,l2_nd);
	 }
       
        /** Propagazione inversa dell'errore in L2  (v_t <- v_y2)*/
       for(int i=0;i<l1_np;i++)
	 {
	   double dd=0;
	   for(int j=0;j<l2_np;j++)
	     {
	       /* w: peso del i-esimo dendrite del j-esimo percettrone dello strato più esterno */
	       double w=rn->v_u[j*(l2_nd+1)+i];
	       /* correzione   */
	       dd=dd+w*(rn->v_Dt[j]-rn->v_y2[j])*Dactiv_function(rn->v_s2[j]);
	     }
	   
	   /* correzione del percettrone i-esimo*/
	   perc_correzione( rn->v_t+i*(l1_nd+1),rn->v_x0,rn->v_s1[i],dd, par->fattore_apprendimento,l1_nd);
	 }
       
       
       return rn;
     }
}



void rele_Classifica(rele_rete * rn, double * dati)
{
  int l1_nd = rn->N_neuroni_ingresso;
  int l1_np = rn->N_neuroni_primo_strato_interno;
  int l2_nd;
  int l2_np;
  /* preparazione degli ingressi e delle label delle classi*/
  rn->v_x0[0] = 1;
  memcpy(rn->v_x0+1,dati,rn->N_neuroni_ingresso*sizeof(double));
  /* UNO STRATO */
  if(rn->N_strati_computazionali == 1)
    {
      layer_feed_forward(
			 rn->v_s1,
			 rn->v_y1,
			 rn->v_t,
			 rn->v_x0,
			 l1_np,
			 l1_nd);
      rn->strato_uscita = rn->v_y1;
      
    }

  /* DUE STRATI */
  if(rn->N_strati_computazionali == 2)
    {
      l2_np = rn->N_neuroni_uscita;
      l2_nd = l1_np;
      layer_feed_forward(
			 rn->v_s1,
			 rn->v_y1,
			 rn->v_t,
			 rn->v_x0,
			 l1_np,
			 l1_nd);
      
       /** Mappa y1 in x1 aggiungendo l'elemento x1_0=1*/
      layer_map_out_in(rn->v_x1,
		       rn->v_y1,
		       l2_nd);
      
      /** Feed Forward: L1->L2->output*/
      layer_feed_forward(rn->v_s2,
			 rn->v_y2,
			 rn->v_u,
			 rn->v_x1,
			 l2_np,
			 l2_nd);
      
      rn->strato_uscita = rn->v_y2;   
    }
/* TRE STRATI */
  if(rn->N_strati_computazionali == 3)
    {
      l2_np = rn->N_neuroni_secondo_strato_interno;
      l2_nd = l1_np;
      int l3_np = rn->N_neuroni_uscita;
      int l3_nd = l2_np;
      layer_feed_forward(
			 rn->v_s1,
			 rn->v_y1,
			 rn->v_t,
			 rn->v_x0,
			 l1_np,
			 l1_nd);
      
       /** Mappa y1 in x1 aggiungendo l'elemento x1_0=1*/
      layer_map_out_in(rn->v_x1,
		       rn->v_y1,
		       l2_nd);
      
      /** Feed Forward: L1->L2->output*/
      layer_feed_forward(rn->v_s2,
			 rn->v_y2,
			 rn->v_u,
			 rn->v_x1,
			 l2_np,
			 l2_nd);

       /** Mappa y2 in x2 aggiungendo l'elemento x2_0=1*/
      layer_map_out_in(rn->v_x2,
		       rn->v_y2,
		       l3_nd);
      
      /** Feed Forward: L1->L2->output*/
      layer_feed_forward(rn->v_s3,
			 rn->v_y3,
			 rn->v_v,
			 rn->v_x2,
			 l3_np,
			 l3_nd);

      
      rn->strato_uscita = rn->v_y3;   
    }

  
  
}


void  rele_Libera_rete(rele_rete * rete)
{
  free(rete->v_x0);
  free(rete->v_t);
  free(rete->v_Dt);
  free(rete->v_s1);
  free(rete->v_y1);
  
  if( rete->N_strati_computazionali >=2)
    {
      free(rete->v_x1);
      free(rete->v_u);
      free(rete->v_Du);
      free(rete->v_s2);
      free(rete->v_y2);
    }
 
   if( rete->N_strati_computazionali >=3)
    {
      free(rete->v_x2);
      free(rete->v_v);
      free(rete->v_Dv);
      free(rete->v_s3);
      free(rete->v_y3);
    }

   printf("Libero: %p\n", rete);
     free(rete);
  return;
}

rele_rete * rele_Crea_rete(
			   int N_neuroni_ingresso,
			   int N_neuroni_uscita,
			   int N_neuroni_primo_strato_interno,
			   int N_neuroni_secondo_strato_interno)
{
  
  rele_rete * rn = malloc(sizeof(rele_rete));
  printf("Creo rete: %p\n",rn);
  rn->N_neuroni_uscita = N_neuroni_uscita;
  rn->N_neuroni_ingresso = N_neuroni_ingresso;
  rn-> N_neuroni_primo_strato_interno =  N_neuroni_primo_strato_interno;
  rn-> N_neuroni_secondo_strato_interno =  N_neuroni_secondo_strato_interno;
  rn->N_strati_computazionali = 3;
  
  if(rn == 0) exit (1);
   
  /*
    STRATO 1: Questo è il primo strato computazionale della rete
    se si è creata una rete ad un solo strato (percettrone classico)
    questo rappresenta anche l'unico strato della rete
   */
  
  /* neuroni del primo strato (computazionale) interno */
  int l1_np =  N_neuroni_primo_strato_interno;
  if(l1_np == 0)
    {
      /* la rete ha un solo strato, quindi il numero di neuroni di 
       uscita deve essere uguale al numero di neuroni 
      dello strato computazionale
      */
      l1_np = N_neuroni_uscita;
      rn->N_strati_computazionali = 1;
    }
  /* dendriti del primo strato iinterno */
  int l1_nd = rn->N_neuroni_ingresso;
  rn->N_neuroni_ingresso = l1_nd;
  /* input dei percettroni dello strato 1*/
  rn->v_x0 = malloc((l1_nd+1)*sizeof(double));
  /* un vettore di l1_nd dendriti per ogniuno dei l1_np neuron1*/
  rn->v_t = malloc((l1_nd+1)*l1_np*sizeof(double));
  /* variazione v_t */
  rn->v_Dt = malloc((l1_nd+1)*l1_np*sizeof(double));
  /*vettore della somma pesata dei canali dendritici per ogni neurone*/
  rn->v_s1 = malloc(l1_np*sizeof(double));
  /* vettore dell'output di ogni neurone*/
  rn->v_y1 = malloc(l1_np*sizeof(double));

  /* inizializzazione pseudocasuale delle connessioni */
   for(int i=0;i<(l1_nd+1)*l1_np;i++)
      rn->v_t[i]=MAX_PESO*(double)rand()/(double)RAND_MAX-MIN_PESO;
   
  /* RETURN si tratta si un percettrone a singolo strato*/
  if(rn->N_strati_computazionali == 1) return rn;

  /*
    STRATO 2: Questo è il secondo strato computazionale della rete
    Questo strato è necessario se si vogliono classificare dei dati
    che non sono linearmente separabili
   */
  
  /* neuroni del secondo strato (computazionale) interno */
  int l2_np =  N_neuroni_secondo_strato_interno;
  if(l2_np == 0)
    {
       /* la rete ha un solo due strati, quindi il numero di neuroni di 
       uscita deve essere uguale al numero di neuroni 
      del secondo strato computazionale
      */
      l2_np = N_neuroni_uscita;
      rn->N_strati_computazionali = 2;

    }
  /* dendriti del secondo strato iinterno */
  int l2_nd = N_neuroni_ingresso;
  
  /* input dei percettroni dello strato 1*/
  rn->v_x1 = malloc((l2_nd+1)*sizeof(double));
  /* un vettore di l1_nd dendriti per ogniuno dei l1_np neuron1*/
  rn->v_u = malloc((l2_nd+1)*l1_np*sizeof(double));
  /* vettore temporaneo per l'update*/
  rn->v_u_tmp = malloc((l2_nd+1)*l1_np*sizeof(double));
  /* variazione v_t */
  rn->v_Du = malloc((l2_nd+1)*l1_np*sizeof(double));
  /*vettore della somma pesata dei canali dendritici per ogni neurone*/
  rn->v_s2 = malloc(l2_np*sizeof(double));
  /* vettore dell'output di ogni neurone*/
  rn->v_y2 = malloc(l2_np*sizeof(double));

  /* inizializzazione pseudocasuale delle connessioni */
   for(int i=0;i<(l2_nd+1)*l2_np;i++)
      rn->v_u[i]=MAX_PESO*(double)rand()/(double)RAND_MAX-MIN_PESO;
   /* copia i pesi inizializzati nel vettore temporaneo */
   memcpy(rn->v_u,rn->v_u_tmp,sizeof(double)*l2_np*(l1_nd+1));
   
  /* RETURN si tratta si un percettrone a due strati*/
  if(rn->N_strati_computazionali == 2) return rn;

   /*
    STRATO 3: Questo è il terzo strato computazionale della rete
    Questo strato è necessario se si vogliono classificare dei dati
    distribuiti in regioni geometriche non convesse
   */
  
  /* neuroni del terzo strato (computazionale) interno */
  int l3_np =  N_neuroni_uscita;
  rn->N_strati_computazionali = 3;

  /* dendriti del terzo strato iinterno */
  int l3_nd = l2_np;
  
  /* input dei percettroni dello strato 1*/
  rn->v_x2 = malloc((l3_nd+1)*sizeof(double));
  /* un vettore di l1_nd dendriti per ogniuno dei l1_np neuron1*/
  rn->v_v = malloc((l2_nd+1)*l1_np*sizeof(double));
  /* variazione v_t */
  rn->v_Dv = malloc((l2_nd+1)*l1_np*sizeof(double));
  /*vettore della somma pesata dei canali dendritici per ogni neurone*/
  rn->v_s3 = malloc(l2_np*sizeof(double));
  /* vettore dell'output di ogni neurone*/
  rn->v_y3 = malloc(l2_np*sizeof(double));

  /* inizializzazione pseudocasuale delle connessioni */
   for(int i=0;i<(l3_nd+1)*l3_np;i++)
      rn->v_v[i]=MAX_PESO*(double)rand()/(double)RAND_MAX-MIN_PESO;

  /* RETURN */
  return rn;
  

}

