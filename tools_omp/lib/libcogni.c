/*______________________________________________
 | ss_snn_lib
 | Scuola Sisini Shallow Neural Network Library
 | Francesco Sisini (c) 2019
 */


/*_______________________________________________________________________
  |LICENZA GPL
  |
  |ss_snn_lib: libreria per costruire una rete di percettroni a più strati
  |Copyright (C) 2019  Francesco Sisini (francescomichelesisini@gmail.com)
  |
  |This program is free software: you can redistribute it and/or modify
  |it under the terms of the GNU General Public License as published by
  |the Free Software Foundation, either version 3 of the License, or
  |(at your option) any later version.
  |
  |This program is distributed in the hope that it will be useful,
  |but WITHOUT ANY WARRANTY; without even the implied warranty of
  |MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  |GNU General Public License for more details.
  |
  |You should have received a copy of the GNU General Public License
  |along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <math.h>
#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "libcogni.h"


/*_____________________________________________________
 | stream: il file su cui scrivere 
 | v_w: l'array sequenziale con tutti i pesi del layer
 | n_dend: numero di dendriti per percettrone
 | n_perc: numero di percettroni nel layer
 */
void layer_writedown(FILE * stream,double *v_w, int n_dend, int n_perc)
{
  fwrite(v_w, sizeof(double), (n_dend+1)*n_perc, stream);
}

/*_____________________________________________________
 | stream: il file da cui
 | v_w: l'array sequenziale con tutti i pesi del layer
 | n_dend: numero di dendriti per percettrone
 | n_perc: numero di percettroni nel layer
 */
void layer_read(FILE * stream,double *v_w, int n_dend, int n_perc)
{
  fread(v_w, sizeof(double), (n_dend+1)*n_perc, stream);
}



/*___________________________________________________________________________________ 
 *|
 *| v_s: vettore delle somme dei canali dendritici per gli n_perc percettroni
 *| v_y: vettore degli output per gli n_perc percetroni
 *| v_w: vettore dei pesi dendritici per gli n_perc percetroni
 *| v_x: vettore degli input al percettrone (uguale per tutti  gli n_perc percetroni)
 *| n_perc: numero di percettroni nello strato
 *| n_dend: numero di dendriti per percettrone */
void layer_feed_forward(double v_s[],double v_y[],double v_w[],double v_x[],int n_perc, int n_dend)
{
  static int cycle[10];
  
  int nthreads, tid, i, chunk;
  chunk = 10;
  i = 0;
  #pragma omp  parallel  private(tid,i)
  {
    tid = omp_get_thread_num();
    if (tid == 0)
      {
	nthreads = omp_get_num_threads();
	//printf("Number of threads = %d\n", nthreads);
      }
    //if(tid!=0)
    //printf("Thread %d starting...\n",tid);
#pragma omp for schedule(dynamic,chunk)
    for(i=0;i<n_perc;i++)
      {
	
	cycle[tid]++;
	//printf("Thread %d, Perc %d \n",tid,i);
	/* calcola l'output per ogni percettrone */
	v_s[i]=perc_calc_output(v_w+i*(n_dend+1),v_x,n_dend);
	v_y[i]=activ_function(v_s[i]);
      }
  }
  /* for(int j=0;j<10;j++)
    {
      if(cycle[j]!=0)
	printf("Cicli(%d) %d\n",j,cycle[j]);
	}*/
}

/*__________________________________________________________________________________________
 *| v_w vettore di dimensione n_dend+1. Il primo elemento è 1, il resto sono i pesi sinaptici
 *| v_x vettore dell'input del percettrone. Il primo elemento è 1
 *| z somma pesata dell'input: v_w < . v_x
 *| d Valore Atteso - Valore Calcolato
 *| rate learning rate
 *| n_dend numero dei dendriti del percettrone
 */
void perc_correzione(double v_w[],double v_x[],double z,double d,double rate,int n_dend)

{
  
  int nthreads, tid, i, chunk;
  #pragma omp  parallel  private(tid,i)
  {
#pragma omp for schedule(dynamic,10) nowait
    for(i=0;i<n_dend+1;i++)
      {
	v_w[i]=v_w[i]+rate*v_x[i]*(d)*Dactiv_function(z);
      }
  }
}

void layer_map_out_in(double v_x[],double v_y[], int n_dend)
{
  v_x[0]=1;
  for(int i=1;i<n_dend+1;i++)v_x[i]=v_y[i-1];
}

/*_________________________________________________________________
 *|  v_w: vettore di dimensione  n_dend+1 di pesi dendritici
 *|  v_x: vettore delgi n_dend+1 (c'è il bias) input al percettrone
 *|  n_dend numero di dendriti  */
double perc_calc_output(double v_w[],double v_x[],int n_dend)
{
  double a=0;
  /*somma pesata degli stimoli di ingresso*/
  for(int i=0;i<n_dend+1;i++) a=a+v_w[i]*v_x[i];
 
  /*Attivazione del percettrone*/
  return a;
}
double activ_function(double summed_input)
{
  /* Come scelta alternativa della funzione di attivazione */
  // double r=tanh(summed_input);
  
  double r=1/(1+exp(-summed_input));
  return r;
}
double Dactiv_function(double summed_input)
{
  /* Se si scegli la tanh come attivazione */
  //double r=tanh(summed_input);
  //return 1-r*r;
  
  double r=activ_function(summed_input);
  return r*(1-r);//corretta, fare i conti!
}

int leggi_dato_classe(int * o,int * out_label,FILE * in_stream)
{
  int r = fscanf(in_stream,"%d",out_label);
  int rd;
  int i = 0;
  do
    {
      r += (rd= fscanf(in_stream,",%d",o+i));
      i++;
    } while(rd>0);
  return r;
}

int leggi_dato(int * o,FILE * in_stream)
{
  int r = fscanf(in_stream,"%d",o);
  int rd;
  int i = 1;
  do
    {
      r += (rd = fscanf(in_stream,",%d",o+i));
      i++;
    } while(rd>0);
  return r;
}



void print_object(double x[],int r, int c,int R,int C)
{
  double min,max;
  min=10;
  max=-10;
  for(int i=0;i<r;i++)
    for(int j=0;j<c;j++)
      {
	if(x[i*c+j]>max) max=x[i*c+j];
	if(x[i*c+j]<min) min=x[i*c+j];
      }
  double i_range=max-min;
  
  double max_c=255;
  double min_c=232;
  double c_range=max_c-min_c;
  double conv=c_range/i_range;
  
  for(int i=0;i<r;i++)
    for(int j=0;j<c;j++)
      {
	double gl;
	gl=x[i*c+j];
	
	int col=max_c-(min_c+(double)(gl-min)*conv)+min_c;
	if(col<0)printf("%f",gl);
	//if(gl>0.9) col=0;
	printf("\x1b[%d;%dH\x1b[48;5;%dm  \x1b[0m",i+R,2*j+C,col);

      }
  fflush(stdout);
}

/*_______________________________
 *
 * Misura righe e colonne del
 * terminale
 */
int dimensioni_finestra(int *rows, int *cols) 
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
  



