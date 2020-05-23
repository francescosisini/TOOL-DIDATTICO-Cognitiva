/*______________________________________________
 | rnss_libpercettrone
 | 
 | Francesco e Valentina Sisini (c) 2020
 */

typedef struct
{
  double * strato_ingresso;
  double * strato_uscita;
  
  /* Numero di elementi dell'array che saranno allocati */ 
  int N_neuroni_sensitivi;
  int N_neuroni_afferenti;
  int N_neuroni_primo_strato_intercalare; 
  int N_neuroni_secondo_strato_intercalare;
  int N_strati_computazionali;

  /* Errore quadratico medio */
  double EQM;

  /* Risultato atteso, o taret output */
  double * v_d;

  /* Strato 1: dall'ingresso 0 all'uscita 1 */
  double * v_x0;
  double * v_t;
  double * v_Dt;
  double * v_s1;
  double * v_y1;
  

  /* Strato 2: dall'ingresso 1 all'uscita 2 */
  double * v_x1;
  double * v_u;
  double * v_u_tmp;
  double * v_Du;
  double * v_s2;
  double * v_y2;
  double * v_dy2;

  /* Strato 3: dall'ingresso 2 all'uscita 3 */
  double * v_x2;
  double * v_v;
  double * v_v_tmp;
  double * v_Dv;
  double * v_s3;
  double * v_y3;
  double * v_dy3;

  /* neuroni e dendriti*/
  int l1_np, l1_nd;
  int l2_np, l2_nd;
  int l3_np, l3_nd;
  
} rele_rete;


typedef struct
{
  double fattore_apprendimento;
  double max_per_normalizzazione;
  int seme_pseudocasuale;
  int epoche;
  int campioni;
}rele_parametri;


/* crea una rete neurale che può essere addestrata.
   Inizializza i pesi dendritici (pesi delle connessioni)
   tra -0.1 e 0.1
 */
rele_rete * rele_Crea_rete(
			   int N_neuroni_sensitivi,
			   int N_neuroni_afferenti,
			   int N_neuroni_primo_strato_intercalare,
			   int N_neuroni_secondo_strato_intercalare);

/* libera le risorse della rete */
void  rele_Libera_rete(rele_rete * rete);

/* 
   addestra una rete neurale usando i dati e le classi passate
   ogni chiamata viene eseguita una singola iterazione
*/
rele_rete *  rele_Addestra(rele_rete * rn,
			   rele_parametri * par,
			   double * dati,
			   double * classi);
/* 
   Usa una rete già addestrata per classificare i dati 
   il risultato è puntato da rete->strato_uscita
*/
void  rele_Classifica(rele_rete * rete, double * dati);
