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
#include "rele_libreteneurale.h"
#define MAX_PESO 0.1
#define MIN_PESO -0.1


rele_rete *  rele_Addestra(rele_rete * rn,
			 rele_parametri * par,
			 double * dati,
			 double * classi)
{ 
  int l1_np = rn->l1_np;
  int l2_np = rn->l2_np;
  int l3_np = rn->l3_np;
  int l1_nd = rn->l1_nd;
  int l2_nd = rn->l2_nd;
  int l3_nd = rn->l3_nd;

  /* preparazione degli ingressi e delle label delle classi */
  rn->v_x0[0] = 1;
  memcpy(rn->v_x0+1,dati,rn->N_neuroni_sensitivi*sizeof(double));
  /* Carica il target-output (l'output voluto) in memoria */
  memcpy(rn->v_d,classi,rn->N_neuroni_afferenti*sizeof(double));

  /** RETE A UN SOLO STRATO **/
  if(rn->N_strati_computazionali == 1)
    {
      /* Feed Forward: Input->L1->output to L2 */
      layer_feed_forward(rn->v_s1,rn->v_y1,rn->v_t,rn->v_x0,l1_np,l1_nd);
      
      rn->EQM = 0;
      /* Propagazione inversa dell'errore in L1  (v_t  <- v_y1) */
      for(int i=0;i<l1_np;i++)
	{
	  double errore = rn->v_d[i]- rn->v_y1[i];
	  /* aggiornamento EQM */
	  rn->EQM += (1./(double)l1_np)*errore*errore; 
	  /* correzione dei pesi (v_t) del percettrone i-esimo */
	  perc_correzione(rn->v_t+i*(l1_nd+1),rn->v_x0,
			  rn->v_s1[i],errore,
			  par->fattore_apprendimento,l1_nd);
	}
      return rn;
    }

  /** RETE A DUE STRATI **/
   if(rn->N_strati_computazionali == 2)
     {
       /* Feed Forward: Input->L1->output to L2 */
       layer_feed_forward( rn->v_s1, rn->v_y1, rn->v_t,rn->v_x0,l1_np,l1_nd);
       
       /* Mappa y1 in x1 aggiungendo l'elemento x1_0=1*/
       layer_map_out_in(rn->v_x1, rn->v_y1,l2_nd);
       
       /* Feed Forward: L2->output */
       layer_feed_forward(rn->v_s2,rn->v_y2,rn->v_u,rn->v_x1,l2_np,l2_nd);

       rn->EQM = 0;
       /** Propagazione inversa dell'errore in L2  (v_u  <- v_y1) */
       for(int i=0;i<l2_np;i++)
	 {
	   double errore = rn->v_d[i]- rn->v_y2[i];
	   rn->v_dy2[i] = errore;
	   rn->EQM += (1./(double)l2_np)*errore*errore; 
	   /* correzione dei pesi (v_u) del percettrone i-esimo 
	      La correzione viene memorizzata su un array temporaneo
	      per completare la backpropagation
	    */
	   perc_correzione(
			   rn->v_u_tmp+i*(l2_nd+1),
			   rn->v_x1,
			   rn->v_s2[i],
			   errore,
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
	       double w = rn->v_u[j*(l2_nd+1)+i];
	       /* correzione   */
	       //dd=dd+w*(rn->v_d[j]-rn->v_y2[j])*Dactiv_function(rn->v_s2[j]);
	       dd=dd+w*(rn->v_dy2[j])*Dactiv_function(rn->v_s2[j]);
	     }
	   
	   /* correzione del percettrone i-esimo dello strato t */
	   perc_correzione( rn->v_t+i*(l1_nd+1),rn->v_x0,rn->v_s1[i],dd, par->fattore_apprendimento,l1_nd);
	 }

       /* 
	  Aggiornamento dello strato L2: i pesi della rete dello strato u temporaneo
	  vengono copiati sullo strato u
       */
       memcpy(rn->v_u,rn->v_u_tmp,sizeof(double)*l2_np*(l2_nd+1));
       return rn;
     }
   
  /** TRE STRATI **/
   if(rn->N_strati_computazionali == 3)
     {
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

       rn->EQM = 0;
       
	/** Propagazione inversa dell'errore in L3  (v_v  <- v_y3) */
       for(int i=0;i<l3_np;i++)
	 {
	   double errore = rn->v_d[i]- rn->v_y3[i];
	   rn->v_dy3[i] = errore;
	   rn->EQM += (1./(double)l3_np)*errore*errore; 
	   /* correzione dei pesi (v_v_tmp) del percettrone i-esimo */
	   perc_correzione(
			   rn->v_v_tmp+i*(l3_nd+1),
			   rn->v_x2,
			   rn->v_s3[i],
			   errore,
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
	       //dd=dd+w*(rn->v_Dv[j]-rn->v_y3[j])*Dactiv_function(rn->v_s3[j]);
	       dd=dd+w*(rn->v_dy3[j])*Dactiv_function(rn->v_s3[j]);
	     }
	   rn->v_dy2[i] = dd;
	   /* correzione del percettrone i-esimo*/
	   perc_correzione( rn->v_u_tmp+i*(l2_nd+1),rn->v_x1,rn->v_s2[i],dd, par->fattore_apprendimento,l2_nd);
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
	       dd=dd+w*(rn->v_dy2[j])*Dactiv_function(rn->v_s2[j]);
	     }
	   
	   /* correzione del percettrone i-esimo*/
	   perc_correzione( rn->v_t+i*(l1_nd+1),rn->v_x0,rn->v_s1[i],dd, par->fattore_apprendimento,l1_nd);
	 }
       /* Aggiornamento rete */
       memcpy(rn->v_v,rn->v_v_tmp,sizeof(double)*l3_np*(l3_nd+1));
       memcpy(rn->v_u,rn->v_u_tmp,sizeof(double)*l2_np*(l2_nd+1));
              
       return rn;
     }
}



void rele_Classifica(rele_rete * rn, double * dati)
{

  int l1_np = rn->l1_np;
  int l2_np = rn->l2_np;
  int l3_np = rn->l3_np;
  int l1_nd = rn->l1_nd;
  int l2_nd = rn->l2_nd;
  int l3_nd = rn->l3_nd;

    
  /* preparazione degli ingressi e delle label delle classi*/
  rn->v_x0[0] = 1;
  memcpy(rn->v_x0+1,dati,rn->N_neuroni_sensitivi*sizeof(double));
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

/*** rele_crea_rete ***/
rele_rete * rele_Crea_rete(
			   int N_neuroni_sensitivi,
			   int N_neuroni_afferenti,
			   int N_neuroni_primo_strato_intercalare,
			   int N_neuroni_secondo_strato_intercalare)
{
  
  rele_rete * rn = malloc(sizeof(rele_rete));
  printf("Creo rete: %p\n",rn);
  rn->N_neuroni_afferenti = N_neuroni_afferenti;
  rn->N_neuroni_sensitivi = N_neuroni_sensitivi;
  rn-> N_neuroni_primo_strato_intercalare =  N_neuroni_primo_strato_intercalare;
  rn-> N_neuroni_secondo_strato_intercalare =  N_neuroni_secondo_strato_intercalare;
    
  if(rn == 0) exit (1);

  /* Errore quadratico medio */
  rn->EQM = 0;

  /* target output */
  rn->v_d = malloc(N_neuroni_afferenti*sizeof(double));
    
  int l1_np = 0, l1_nd = 0;
  int l2_np = 0, l2_nd = 0;
  int l3_np = 0, l3_nd = 0;
  
  /* UN SOLO STRATO COMPUTAZIONALE */
  if(N_neuroni_primo_strato_intercalare == 0 && N_neuroni_secondo_strato_intercalare == 0)
    {
      rn->N_strati_computazionali = 1;
      /* percettroni del primo strato */
      l1_np = N_neuroni_afferenti;
      /* dendriti del primo strato iintercalare */
      l1_nd = rn->N_neuroni_sensitivi;
    }
  
  /* DUE STRATI COMPUTAZIONALI */
  if(N_neuroni_primo_strato_intercalare !=0 && N_neuroni_secondo_strato_intercalare == 0)
    {
      rn->N_strati_computazionali = 2;
      /* percettroni del primo strato */
      l1_np =  rn-> N_neuroni_primo_strato_intercalare;
      /* dendriti del primo strato iintercalare */
      l1_nd = rn->N_neuroni_sensitivi;
      /* percettroni del secondo strato */
      l2_np = N_neuroni_afferenti;
      /* dendriti secondo strato */
      l2_nd = l1_np;
    }

  /* TRE STRATI COMPUTAZIONALI */
  if(N_neuroni_primo_strato_intercalare != 0 && N_neuroni_secondo_strato_intercalare != 0)
    {
      rn->N_strati_computazionali = 3;
      /* percettroni del primo strato */
      l1_np =  rn-> N_neuroni_primo_strato_intercalare;
      /* dendriti del primo strato iintercalare */
      l1_nd = rn->N_neuroni_sensitivi;
      /* percettroni del secondo strato */
      l2_np = N_neuroni_secondo_strato_intercalare;
      /* dendriti secondo strato */
      l2_nd = l1_np;
      /* neuroni del terzo strato (computazionale) intercalare */
      l3_np =  N_neuroni_afferenti;
      /* dendriti del terzo strato iintercalare */
      l3_nd = l2_np;
    }

  rn->l1_np = l1_np;
  rn->l2_np = l2_np;
  rn->l3_np = l3_np;
  rn->l1_nd = l1_nd;
  rn->l2_nd = l2_nd;
  rn->l3_nd = l3_nd;
  
  /* input dei percettroni dello strato 0*/
  rn->v_x0 = malloc((l1_nd+1)*sizeof(double));
      /* un vettore di l1_nd dendriti per ogniuno dei l1_np neuron1*/
  rn->v_t = malloc((l1_nd+1)*l1_np*sizeof(double));
  /* output atteso v_t */
  rn->v_Dt = malloc(l1_np*sizeof(double));
  /*vettore della somma pesata dei canali dendritici per ogni neurone*/
  rn->v_s1 = malloc(l1_np*sizeof(double));
  /* vettore dell'output di ogni neurone*/
  rn->v_y1 = malloc(l1_np*sizeof(double));
  
  /* inizializzazione pseudocasuale delle connessioni 
     da -MAX_PESO a + MAX_PESO */
  for(int i=0;i<(l1_nd+1)*l1_np;i++)
    rn->v_t[i]=2*MAX_PESO*(double)rand()/(double)RAND_MAX-MIN_PESO;
  
  /* RETURN si tratta si un percettrone a singolo strato*/
  if(rn->N_strati_computazionali == 1) return rn;
  
  /* input dei percettroni dallo strato 1*/
  rn->v_x1 = malloc((l2_nd+1)*sizeof(double));
  /* un vettore di l1_nd dendriti per ogniuno dei l1_np neuron1*/
  rn->v_u = malloc((l2_nd+1)*l2_np*sizeof(double));
  /* vettore temporaneo per l'update*/
  rn->v_u_tmp = malloc((l2_nd+1)*l2_np*sizeof(double));
  /* outout atteso v_u */
  rn->v_Du = malloc(l2_np*sizeof(double));
  /*vettore della somma pesata dei canali dendritici per ogni neurone*/
  rn->v_s2 = malloc(l2_np*sizeof(double));
  /* vettore dell'output di ogni neurone*/
  rn->v_y2 = malloc(l2_np*sizeof(double));
  /* errore: differenza tra target e ottenuto */
  rn->v_dy2 = malloc(l2_np*sizeof(double));
  
  /* inizializzazione pseudocasuale delle connessioni */
  for(int i=0;i<(l2_nd+1)*l2_np;i++)
    rn->v_u[i]=MAX_PESO*(double)rand()/(double)RAND_MAX-MIN_PESO;
  /* copia i pesi inizializzati nel vettore temporaneo */
  memcpy(rn->v_u,rn->v_u_tmp,sizeof(double)*l2_np*(l1_nd+1));
  
  /* RETURN si tratta si un percettrone a due strati*/
  if(rn->N_strati_computazionali == 2) return rn;
  
  /* input dei percettroni dello strato 2*/
  rn->v_x2 = malloc((l3_nd+1)*sizeof(double));
      /* un vettore di l3_nd dendriti per ogniuno dei l1_np neuron1*/
  rn->v_v = malloc((l3_nd+1)*l3_np*sizeof(double));
  /* un vettore temporaneo di l3_nd dendriti per ogniuno dei l1_np neuron1*/
  rn->v_v_tmp = malloc((l3_nd+1)*l3_np*sizeof(double));
  /* variazione v_t */
  rn->v_Dv = malloc(l3_np*sizeof(double));
  /*vettore della somma pesata dei canali dendritici per ogni neurone*/
  rn->v_s3 = malloc(l3_np*sizeof(double));
  /* vettore dell'output di ogni neurone*/
  rn->v_y3 = malloc(l3_np*sizeof(double));
  /* errore: differenza tra target e ottenuto */
  rn->v_dy3 = malloc(l3_np*sizeof(double));
  
  /* inizializzazione pseudocasuale delle connessioni */
  for(int i=0;i<(l3_nd+1)*l3_np;i++)
    rn->v_v[i]=2*MAX_PESO*(double)rand()/(double)RAND_MAX-MIN_PESO;
  
  /* RETURN */
  return rn;
    

}

