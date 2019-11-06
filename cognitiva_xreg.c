/**
  Il FILE cognitiva_xreg.c è parte del
  Cognitiva: sistema di pattern recongnition
  Copyright (C) 2018  Francesco Sisini (francescomichelesisini@gmail.com)
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <ctype.h>
#include <errno.h>
#include <locale.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>



#define CTRL_KEY(k) ((k) & 0x1f)

#define RIGA 25

enum editorKey {
  BACKSPACE = 127,
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  DEL_KEY,
  HOME_KEY,
  END_KEY,
  PAGE_UP,
  PAGE_DOWN
};


struct termios orig_termios;
int rows,cols;

/*_______________________________________________
  descrizione: il sistema (sys) genera 
  dei record di vendita del tipo:

  <data|dimensione casa|prezzo casa>

  Tra dimensione e prezzo esiste una relazione
  lineare prz = a+ b*dim i cui parametri a e b
  possono varirare anche improvisamente

  - L'utente può ricevere i dati del mercato delle
  vendite e in questo modo aggiornare il modello
  
  Il sistema genera delle richieste di valutazione
  da parte dei clienti, se le valutazioni sono
  in linea con il mercato il cliente e soddisfatto
  altrimenti il cliente non  è soddisfatto
  
  - L'utente può azzerare e reimpostare il modello
  quando crede
 
 */

typedef struct {

  // coefficienti lienari della relazione
  // area prezzo immobile
  // prz = a+ b*area
  int a,b;

} MERCATO;

typedef struct {
  int gg,mm,aaaa;
  int metri_q;
  int prezzo;
} VENDITA;

typedef struct {
  VENDITA v;
  char letto;

} DATA;


/*____________________________________*/

/* impostazioni */
char glb_ricevi_dati = 0;
char glb_apprendi = 0;

/* Andamento reale del mercato */
MERCATO glb_mrc;

/* modello andamento del mercato */
MERCATO glb_mdl;
DATA glb_campione;

/* rating del servizio */
int glb_richieste;
int glb_voti;
float glb_rate;
double glb_mu_area,glb_mu_prz,glb_sig=0,glb_sig_xy,glb_n;


/*____________________________________*/
void user_voto(int voto);

VENDITA user_ricevi_dati_vendite();

void user_reset_modello();

void user_apprendi();

/*____________________________________*/

double mdl_covarianza(double x, double y, double x_mu, double y_mu,double sig, int n);

double mdl_media(double s, double s_mu, int n);

void mdl_regressione();

int mdl_suggerisci_prezzo(int metri_q_abitazione);

/*_____________________________________*/

void * sys_canale_vendite(void *);

void * sys_cliente_richiede_valutazione(void *);

void * sys_apprendimento(void *);

void sys_init_mercato();

/*______________________________________*/

int trm_dimensioni_finestra(int *rows, int *cols);

void trm_terminale_cucinato();

void trm_terminale_crudo();

int trm_leggi_tastiera();

void trm_die(const char *s);

int trm_init();

/*______________________________________*/
int main()
{

   
  /* presentazione */
  printf("\x1b[2J\x1b[1;1H\ncognitiva_: regressione lineare (xreg) \n");
  printf("\n(C)Scuola_Sisini 2019 https://pumar.it/cognitiva\n\n\n");
  printf("\nSERVIZIO DI VALUTAZIONE IMMOBILIARE\n"
         "Tu sei il gestore del servizio. I clienti inviano in tempo reale richieste di valutazione"
         "\ndel prezzo di immobili specificandone l'area"
         "\nIl servizio fornisce la valutazione calcolando il prezzo suggerito come: prezzo = a + area*b"
         "\nI clienti ti assegnano un voto positivo se il prezzo non si discosta troppo da quello di mercato"
         "\nTu devi impostare i valori di a e b per avere un prezzo conforme all'andamento del mercato"
         "\nPuoi ottenere i dati (area,prezzo) delle vendite nel tuo mercato di riferimento corrente "
         "\n\nCOMANDI:\n"
         "-) frecce SU e GIU modifica parametro a\n"
         "-) frecce SINISTRA e DESTRA modifica parametro b\n"
         "-) tasto c reimposta casualmente i parametri a e b del tuo mercato di riferimento\n"
         "-) tasto r ricevi dati di vendite effettuate nel tuo mercato di riferimento\n"
         "-) tasto a abilita/disabilita machine learning\n"
         "-) tasto x reset memoria machine learning\n"
         "-) CTRL-q esci\n");
  fflush(stdout); 
  
  
  char attivo = 1;
  glb_mdl.a = 0;
  glb_mdl.b = 0;

  trm_init();

  sys_init_mercato();

  srand((long)time(0));

  pthread_t canale_in;

  pthread_t clienti_richieste;

  pthread_t modello_apprendimento;
  
  if(pthread_create(&canale_in,0,&sys_canale_vendite,0) != 0)
    trm_die("\nImpossibile aprire il thread di aggiornamento");

  if(pthread_create(&clienti_richieste,0,&sys_cliente_richiede_valutazione,0) != 0)
    trm_die("\nImpossibile aprire il thread di vendita");

  if(pthread_create(&modello_apprendimento,0,&sys_apprendimento,0) != 0)
    trm_die("\nImpossibile aprire il thread di apprendimento");
  
  while(attivo)
    {

      
      
      int ch = trm_leggi_tastiera();

       switch (ch)
          {      
          case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            write(STDOUT_FILENO,"\x1b[48:5:0m",9);
            exit(0);
            break;
          case 'a':
            user_apprendi();
            break;
          case 'c':
            sys_init_mercato();
            break;
          case 'r':
            glb_ricevi_dati = !glb_ricevi_dati;
            break;
          case 'x':
            user_reset_modello();
            break;
            
          case ARROW_UP:
            glb_mdl.a+=50;
            break;
          case ARROW_DOWN:
            glb_mdl.a-=50;
            break;
            case ARROW_LEFT:
            glb_mdl.b-=50;
            break;
          case ARROW_RIGHT:
            glb_mdl.b+=50;
            break;
          }
      
      /* aggiorna il modello */
       printf("\x1b[29;1H--PARAMETRI MODELLO--");
       printf("\x1b[30;1H  a=%d, b=%d   ",glb_mdl.a,glb_mdl.b); 
    }
}

void user_reset_modello()
{
  glb_mu_area = 0;
  glb_mu_prz = 0;
  glb_sig = 0;
  glb_sig_xy = 0;
  glb_n = 0;
  glb_mdl.a = 0;
  glb_mdl.b = 0;
  
}

void * sys_apprendimento(void * ptr)
{
  
  struct timespec tc;
  tc.tv_sec = 1;
  tc.tv_nsec = 999999999;
  while(1)
    {
      if(glb_apprendi && glb_ricevi_dati)
        {
          if(!glb_campione.letto)
            {
              glb_campione.letto = 1;
              glb_n++;
              int n = glb_n;
              glb_mu_area = mdl_media(glb_campione.v.metri_q,glb_mu_area,n);
              glb_mu_prz = mdl_media(glb_campione.v.prezzo,glb_mu_prz,n);
              glb_sig_xy = mdl_covarianza(glb_campione.v.metri_q,glb_campione.v.prezzo , glb_mu_area, glb_mu_prz,glb_sig_xy, n);
              glb_sig = mdl_covarianza(glb_campione.v.metri_q,glb_campione.v.metri_q , glb_mu_area, glb_mu_area,glb_sig, n);


              /* calcolo a e b modello */
              glb_mdl.b = glb_sig_xy/glb_sig;
              
              glb_mdl.a = glb_mu_prz - glb_mdl.b*glb_mu_area;

              /* aggiorna il modello */
              printf("\x1b[29;1H--PARAMETRI MODELLO--");
              printf("\x1b[30;1H  a=%d, b=%d                         ",glb_mdl.a,glb_mdl.b); 
              
               
              printf("\x1b[36;1H%d) area media: %.0lf  prezzo medio: %.0lf ",n,glb_mu_area,glb_mu_prz);
              printf("\x1b[37;1H%d) cov(area,prezzo): %.0lf cov(area,area): %.0lf ",n,glb_sig_xy,glb_sig);
              //printf("\x1b[42;1H%d) a: %d b: %d                       ",n,glb_mdl.a,glb_mdl.b);
              fflush(stdout);
            }
          
        }
      nanosleep(&tc, 0);
    }
      
}

void user_apprendi()
{
  glb_apprendi = !glb_apprendi;
  if(glb_apprendi)
    printf("\x1b[35;1HMACHINE LEARNING: ON ");
  else
    printf("\x1b[35;1HMACHINE LEARNING: OFF");
}

void user_voto(int voto)
{
  glb_richieste++;
  glb_voti+=voto;
  glb_rate=(glb_voti/(double)glb_richieste)*5.;
  
  
  printf("\x1b[27;1HRATING %f/5    ",glb_rate);
  
}



/*______________________________________

  Calcola il prezzo secondo il modello
  
*/

int mdl_suggerisci_prezzo(int m2)
{
  int prz = glb_mdl.a + glb_mdl.b*m2;
  return prz;
}

void * sys_cliente_richiede_valutazione(void * ptr)
{
 
  struct timespec tc;
  tc.tv_sec = 1;
  tc.tv_nsec = 999999999;
  
  while(1)
    {
      /* area della casa per cui viene richiesta la valutazione */
      int m2 =50+rand()/(double)RAND_MAX*300.0;
      
      int prezzo_suggerito = mdl_suggerisci_prezzo(m2);

      int prezzo_mercato = glb_mrc.a + glb_mrc.b * m2;

      /* differenza tra proposto e mercato */

      float diff = (prezzo_suggerito - prezzo_mercato)/(double)prezzo_mercato;

      if(diff*diff<0.01)
        user_voto(+1);
      else
        user_voto(-1);

      printf("\x1b[25;1H--RICHIESTE DI VALUTAZIONE IN TEMPO REALE--"); 
      printf("\x1b[26;1H%d) area immobile: %d m^2 - tua valutazione prezzo: %d,  prezzo mercato: %d  ",glb_richieste,m2,prezzo_suggerito,prezzo_mercato); 
      nanosleep(&tc, 0);
      nanosleep(&tc, 0);
      fflush(stdout);
      
    }
  
}

void* sys_canale_vendite(void * ptr)
{
  static int lettura = 0;
  struct timespec tc;
  tc.tv_sec = 1;
  tc.tv_nsec = 999999999;
  
  while(1)
    {
      if(glb_ricevi_dati)
        {
          VENDITA v = user_ricevi_dati_vendite();
          glb_campione.v = v;
          glb_campione.letto = 0;
          printf("\x1b[32;1H--VENDITE IMMOBILI IN TEMPO REALE--"); 
          printf("\x1b[33;1H%d) area immobile: %d m^2 - prezzo di vendita: %d euro   \n",++lettura,v.metri_q,v.prezzo);
        }
      nanosleep(&tc, 0);
      fflush(stdout);
    }
}

/*_________________________________________
  
  genera dei dati casuali di vendita
  m^2 e prezzo, basandosi su una relazione
  lineare tra m^2 e prezzo e aggiungendo un
  errore casuale
 */

VENDITA user_ricevi_dati_vendite()
{

  float perc_contrattazione = 0.15;
  
  float m2 =50+rand()/(double)RAND_MAX*300.0;

  float prz = glb_mrc.a+glb_mrc.b*m2;

  float caso = -prz*perc_contrattazione/2. + (rand()/(double)RAND_MAX)*prz*perc_contrattazione;

  VENDITA v;
  v.gg=10;
  v.mm = 12;
  v.aaaa = 2019;
  v.metri_q = m2;
  v.prezzo= prz;
  return v;
}

void sys_init_mercato()
{
  

  //Prezzo fisso da 10000 a 15000 al metro da 500 a 1200 € m^2
  glb_mrc.a = 10000+(rand()/(double)RAND_MAX*5000.0);
  glb_mrc.b = 500+(rand()/(double)RAND_MAX*700.0);

  //printf("\x1b[25;1HPARAMETRI MERCATO: prezzo = %d + m^2*%d",glb_mrc.a,glb_mrc.b);
  
}


/*_______________________________________________
 *
 * calcola la covarianza cov(x,y)
 * in modo ricorsivo
 *
 */
double mdl_covarianza(double x, double y, double x_mu, double y_mu,double sig, int n)
{
  double dn = (double)n;
  double sig_n = (x-x_mu)*(y-y_mu)/dn+(dn-1)/(dn)*sig;
}

/*_______________________________________________
 *
 * cacola la media in modo ricorsivo
 */
double mdl_media(double s, double s_mu, int n)
{
  double dn = (double)n;
  double mu =   (dn-1.)/(dn)*s_mu+1./dn*s;
  return mu;
}



void trm_die(const char *s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  perror(s);
  exit(1);
}

int trm_dimensioni_finestra(int *rows, int *cols) 
{
  struct winsize ws;  
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
      return -1;
    } else
    {
      *cols = ws.ws_col;
      *rows = ws.ws_row;
      return 0;
    }
}

  
void trm_terminale_cucinato()
{
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    trm_die("tcsetattr");
}

void terminale_crudo()
{
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) 
    trm_die("tcgetattr");
  
  atexit(trm_terminale_cucinato);
  
  struct termios raw = orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;
  
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) 
    trm_die("tcsetattr");  
}

int trm_init()
{
  terminale_crudo();
  trm_dimensioni_finestra(&rows, &cols);
  if(rows<44 || cols<95)
    {
      trm_die("\n*** Ridimensionare il terminale ad almeno 95 colonne e 44 righe\n");
    }
  
  setlocale(LC_CTYPE, "");
}

int trm_leggi_tastiera() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) trm_die("read");
  }
  if (c == '\x1b') {
    char seq[3];
    if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';
    if (seq[0] == '[')
      {
	if (seq[1] >= '0' && seq[1] <= '9')
	  {
	    if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
	    if (seq[2] == '~') {
	      switch (seq[1]) {
	      case '1': return HOME_KEY;
	      case '3': return DEL_KEY;
	      case '4': return END_KEY;
	      case '5': return PAGE_UP;
	      case '6': return PAGE_DOWN;
	      case '7': return HOME_KEY;
	      case '8': return END_KEY;
	      }
	    }
	  } else
	  {
	    switch (seq[1])
	      {
	      case 'A': return ARROW_UP;
	      case 'B': return ARROW_DOWN;
	      case 'C': return ARROW_RIGHT;
	      case 'D': return ARROW_LEFT;
	      case 'H': return HOME_KEY;
	      case 'F': return END_KEY;
	      }
	  }
      }
    
    else if (seq[0] == 'O')
      {
	switch (seq[1]) {
	case 'H': return HOME_KEY;
	case 'F': return END_KEY;
	}
      }
    return '\x1b';
    
    
  } else
    {
      return c;
      
    }
}





