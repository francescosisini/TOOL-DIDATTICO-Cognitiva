#define MAX 30
#define NCLASSI 10

/*________________________________________
  
  Bag of words

  ________________________________________*/

typedef struct
{
  char parola[MAX];
  int ricorrenza[NCLASSI];
} Voce;

typedef struct blocco
{
  Voce voce;
  struct blocco * prox;

} Blocco;

typedef Blocco * Bow;


/*________________________
  
  fa puntare la Bow a 0
*/
void inizia_bow(Bow *pbow);

/*__________________________________________________

  cerca il blocco che ha parola == v.parola
  ed aumenta la ricorrenza della parola di un'unità
  Torna il numero totale di ricorrenze della parola
  dopo l'aggiornamento

  Se una voce non è presente inserisce un nuovo
  blocco in modo ordinato
*/
int aggiorna_blocco(Bow *pbow, char* parola, int classe);


/*_________________________________________________

  Stampa a scehrmo la Bow
*/
void mostra_bow(Bow b, int classe);

/*_________________________________________________

  Cerca il blocco corrispondente alla parola e
  restituisce l'array della ricorrenza
*/
int * ricorrenza_parola(Bow b, char * parola);

/*_________________________________________________

  Legge da file la Bow
*/
void leggi_bow(Bow *pb,char * nome_file);

/*_________________________________________________

  Scrivi Bow su file
*/
void scrivi_bow(Bow b,char * nome_file);

/*_________________________________________________

  Conta le ricorrenze presenti nella Bow
*/
long int conta_parole(Bow b,int classe);

/*_________________________________________________

  Calcola la probabilità codizionata di un testo
  rispetto ai parametri
  il risultato viene momorizzato in classi
*/
void classifica_testo(Bow testo,Bow param, double classi[NCLASSI]);


