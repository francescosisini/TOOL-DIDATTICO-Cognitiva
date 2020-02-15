#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "bow_lib.h"

void inizia_bow(Bow *pbow)
{
  *pbow = 0;
}

int aggiorna_blocco(Bow *pb, char * v)
{
  //Cerco il nodo ciclando sulla copia della lista
  int l=-1;
  while(*pb)
    {
      l = strcmp(v,(*pb)->voce.parola);
      if(l == 0)
        {
          (*pb)->voce.ricorrenza +=1;
          return ((*pb)->voce.ricorrenza);
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
  pbl->voce.ricorrenza = 1;
  
  pbl->prox = *pb;
 
  *pb = pbl;
  
    
  
}

void mostra_bow(Bow b)
{
  while(b)
    {
      printf("%s;%d\n",b->voce.parola, b->voce.ricorrenza);
      b = b->prox;
    }

}

void scrivi_bow(Bow b,char * nome_file)
{
  FILE *f = fopen(nome_file,"w");
  
  if(f == 0)
    {
      printf("File <%s> non trovato\n",nome_file);
      exit(1);
    }

  while(b)
    {
      fprintf(f,"%s %d\n",b->voce.parola,b->voce.ricorrenza);
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
  while(fscanf(f,"%s %d",p,&r)>0)
    {
      cnt += r;
      printf("Aggiorno %s %d ricorrenze\n",p,r);
      for(int i = 1; i<=r; i++)
        {
          aggiorna_blocco(pb,p);
        }
    }
  fclose(f);
  
}

int ricorrenza_parola(Bow b, char *s)
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


