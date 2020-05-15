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
#include "rnss_libpercettrone.h"

#define MAX_PESO 0.1
#define MIN_PESO -0.1


rnss_rete *  rnss_Addestra(rnss_rete * rn,
			   rnss_parametri * par,
			   double * dati,
			   double * classi)
{ 
  int l1_nd = rn->N_neuroni_ingresso;
  int l1_np = rn->N_neuroni_primo_strato_interno;
  /* preparazione degli ingressi e delle label delle classi*/
  rn->v_x0[0] = 1;
  memcpy(rn->v_x0+1,dati,rn->N_neuroni_ingresso);
  if(rn->N_starti_computazionali == 1)
    {
      memcpy(rn->v_Dt,classi,rn->N_neuroni_ingresso);
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
    }
}

/* 
   Usa una rete già addestrata per classificare i dati 
   il risultato è puntato da rete->strato_uscita
*/
void  rnss_Classifica(rnss_rete * rete, double * dati);


void rnss_Classifica(rnss_rete * rn, double * dati)
{
  int l1_nd = rn->N_neuroni_ingresso;
  int l1_np = rn->N_neuroni_primo_strato_interno;
  /* preparazione degli ingressi e delle label delle classi*/
  rn->v_x0[0] = 1;
  memcpy(rn->v_x0+1,dati,rn->N_neuroni_ingresso);
  if(rn->N_starti_computazionali == 1)
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
}


void * rnss_Libera_rete(rnss_rete * rete)
{
  free(rete->v_x0);
  free(rete->v_t);
  free(rete->v_Dt);
  free(rete->v_s1);
  free(rete->v_y1);

  if( rete->N_starti_computazionali >=2)
    {
      free(rete->v_x1);
      free(rete->v_u);
      free(rete->v_Du);
      free(rete->v_s2);
      free(rete->v_y2);
    }

   if( rete->N_starti_computazionali >=3)
    {
      free(rete->v_x2);
      free(rete->v_v);
      free(rete->v_Dv);
      free(rete->v_s3);
      free(rete->v_y3);
    }
  
  free(rete);
}

rnss_rete * rnss_Crea_rete(
			   int N_neuroni_ingresso,
			   int N_neuroni_uscita,
			   int N_neuroni_primo_strato_interno,
			   int N_neuroni_secondo_strato_interno)
{
  rnss_rete * rn = malloc(sizeof(rnss_rete));
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
      rn->N_starti_computazionali = 1;
    }
  rn-> N_neuroni_primo_strato_interno = l1_np;
  /* dendriti del primo strato iinterno */
  int l1_nd = N_neuroni_ingresso;
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
  if(rn->N_starti_computazionali == 1) return rn;

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
      rn->N_starti_computazionali = 2;

    }
  /* dendriti del secondo strato iinterno */
  int l2_nd = N_neuroni_ingresso;
  
  /* input dei percettroni dello strato 1*/
  rn->v_x1 = malloc((l2_nd+1)*sizeof(double));
  /* un vettore di l1_nd dendriti per ogniuno dei l1_np neuron1*/
  rn->v_u = malloc((l2_nd+1)*l1_np*sizeof(double));
  /* variazione v_t */
  rn->v_Du = malloc((l2_nd+1)*l1_np*sizeof(double));
  /*vettore della somma pesata dei canali dendritici per ogni neurone*/
  rn->v_s2 = malloc(l2_np*sizeof(double));
  /* vettore dell'output di ogni neurone*/
  rn->v_y2 = malloc(l2_np*sizeof(double));

  /* inizializzazione pseudocasuale delle connessioni */
   for(int i=0;i<(l2_nd+1)*l2_np;i++)
      rn->v_u[i]=MAX_PESO*(double)rand()/(double)RAND_MAX-MIN_PESO;

  /* RETURN si tratta si un percettrone a due strati*/
  if(rn->N_starti_computazionali == 2) return rn;

   /*
    STRATO 3: Questo è il terzo strato computazionale della rete
    Questo strato è necessario se si vogliono classificare dei dati
    distribuiti in regioni geometriche non convesse
   */
  
  /* neuroni del terzo strato (computazionale) interno */
  int l3_np =  N_neuroni_uscita;
  rn->N_starti_computazionali = 3;

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

