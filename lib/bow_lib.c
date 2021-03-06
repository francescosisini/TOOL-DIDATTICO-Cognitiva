#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "bow_lib.h"

void inizia_bow(Bow *pbow)
{
  *pbow = 0;
}

int aggiorna_blocco(Bow *pb, char * v,int c)
{
  //Cerco il nodo ciclando sulla copia della lista
  int l=-1;
  while(*pb)
    {
      l = strcmp(v,(*pb)->voce.parola);
      if(l == 0)
        {
          (*pb)->voce.ricorrenza[c] +=1;
          return ((*pb)->voce.ricorrenza[c]);
        }
      //Esco se ho superato l'ordine alfabetico
      if( l< 0)
        break;

      //l<0 quindi o è finita la lista o sono ancora in ordine
      pb = &((*pb)->prox);
    }

  //Non è stato ancora creato il blocco relativo a questa voce
  Blocco * pbl = malloc(sizeof(Blocco));
  strcpy(pbl->voce.parola,v);
  memset(pbl->voce.ricorrenza,0,sizeof(int)*NCLASSI);
  pbl->voce.ricorrenza[c] = 1;
  
  pbl->prox = *pb;
 
  *pb = pbl;
  
    
  
}

void mostra_bow(Bow b,int c)
{
  while(b)
    {
      printf("%s;%d\n",b->voce.parola, b->voce.ricorrenza[c]);
      b = b->prox;
    }

}

void scrivi_bow(Bow b,char * nome_file)
{
  FILE *f = fopen(nome_file,"wb");
  
  if(f == 0)
    {
      printf("File <%s> non trovato\n",nome_file);
      exit(1);
    }

  while(b)
    {
      //fprintf(f,"%s %d\n",b->voce.parola,b->voce.ricorrenza);
      fwrite(&(b->voce),sizeof(Voce),1,f);
      b = b->prox;
    }
  fclose(f);
}

void leggi_bow(Bow *pb,char * nome_file)
{
  FILE *f = fopen(nome_file,"rt");
  char p[MAX];
  int r,cnt = 0;
  if(f == 0)
    {
      printf("File <%s> non trovato\n",nome_file);
      return;
    }
  //while(fscanf(f,"%s %d",p,&r)>0)
  Voce v;
  int rd;
  while((rd=fread(&v,sizeof(Voce),1,f))>0)
    {
      //printf("Letti %d byte da %s\n",rd,nome_file);
      
      for(int c = 0; c< NCLASSI; c++)
        {
          //printf("Leggo classe %d\n",c);
          int r = v.ricorrenza[c];
          for(int i = 1; i<=r; i++)
            {
              aggiorna_blocco(pb,v.parola,c);
            }
        }
    }
  fclose(f);
  
}

int * ricorrenza_parola(Bow b, char *s)
{
  while(b)
    {
      if(!strcmp(b->voce.parola,s))
        {
          return (b->voce.ricorrenza);
        }
      b = b->prox;
    }
  return 0;
}

long int conta_parole(Bow b,int c)
{
  long int cnt=0;
   while(b)
     {
       cnt += b->voce.ricorrenza[c];
       b = b->prox;
     }
   return cnt;

}

void classifica_testo(Bow testo,Bow param, double classi[NCLASSI])
{
  //Inizializza a 0 il vettore dei risultati
  memset(classi,0,sizeof(double)*NCLASSI);

  //Memmorizzo in nparole il numero di parole presenti in ogni classe di testo
  int nparole[NCLASSI];

  for(int i=0; i<NCLASSI;i++)
    nparole[i]=conta_parole(param,i);

  int nparole_testo = conta_parole(testo,0);
  
  double p = 0 ;
  double x = 0 ;
  
  while(testo)
    {
      //Ricorrenza della parola nel testo
      x = (double)(testo->voce.ricorrenza[0]);
      // printf("Testo, %s:%f\n",testo->voce.parola,x);
      
      //Cerco l'array di ricorrenze della parola 
      int *ric = ricorrenza_parola(param,testo->voce.parola);
      if(ric != 0)
        {
          for(int cls = 0; cls<NCLASSI; cls++)
            {
              if(ric[cls]>0)
                {
                  double p = (double)ric[cls]/(double)nparole[cls];
                  classi[cls] += x*log10(p)/(double)nparole[cls];
                  //printf("%s %d %d p=%f\n",testo->voce.parola,testo->voce.ricorrenza[0],ric[cls],classi[cls]);
                }
              else
                {
                  if((double)nparole[cls]>0)
                    classi[cls] += x*log10(1/(double)nparole[cls]);
                }
            }
        }
          testo = testo->prox;
    }
  for(int cls = 0; cls<NCLASSI; cls++)
    {
      if((double)nparole[cls]>0)
        classi[cls] +=log10(1./(double)NCLASSI);
    }
  
}
