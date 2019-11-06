#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cgn_testate.h"

void inibizione_laterale(STRATO *Ul, AREA *ar)
{
  for(int n=0;n<NEURONS; n++)
    {
      //calcolo l'indice di riga e colonna relativo a n
      int i=n/NCOLS;
      int j=n%NCOLS;
      /* somma in contributi eccitatori */
      double e=0;
      double h=0;
      double g=0;
      for(int ij=0;ij<H_HA;ij++)
        {
          int di=ar->g_i[ij];
          int dj=ar->g_j[ij];
          int l=i+di;
          int m=j+dj;
          if(l>=0&&l<NCOLS&&m>=0&&m<NCOLS)
            {
              int nu=l*NCOLS+m;
              h+=Ul->u[n].g[ij]*(Ul->u[nu].segnale);
              g+=Ul->u[n].g[ij];
            }
        }
      e=Ul->u[n].segnale;      
      Ul->u[n].segnale_tmp=phi((e-h)/(1+h));
      if(COMPENSAZIONE_BORDI)
        {
          Ul->u[n].segnale_tmp*=g;
        }
    }
  for(int n=0;n<NEURONS; n++)
    {
      Ul->u[n].segnale=Ul->u[n].segnale_tmp;
     
    }
}

void trasmetti_indietro(STRATO *from, STRATO *to,AREA *ar)
{
  double e=0;
  double h=0;
  double s=0;
  for(int n=0;n<NEURONS; n++)
    {
      //calcolo l'indice di riga e colonna relativo a n
      int i=n/NCOLS;
      int j=n%NCOLS;
      /* somma in contributi eccitatori */
       for(int ij=0;ij<S_CA;ij++)
        {
          int di=ar->a_i[ij];
          int dj=ar->a_j[ij];
          int l=i+di;
          int m=j+dj;
           if(l>=0&&l<NCOLS&&m>=0&&m<NCOLS)
             {
               int nu=l*NCOLS+m;
               to->u[nu].ingresso_ecc+=from->u[n].a[ij]*from->u[n].segnale;
               to->u[nu].ingresso_inib+=from->u[n].a[ij]*from->v[n].segnale;          
             }
        }
    }
  for(int n=0;n<NEURONS; n++)
    {
      e=to->u[n].ingresso_ecc;
      h=to->u[n].ingresso_inib;
      to->u[n].pot=(e-h)/(1+h);
      to->u[n].segnale=phi(to->u[n].pot);
    }
  
}

void trasmetti(STRATO *from, STRATO *to)
{
  for(int n=0;n<NEURONS; n++)
    {
      double e=from->u[n].segnale;
      double h=from->v[n].segnale;
      to->u[n].ingresso_ecc=e;
      to->u[n].ingresso_inib=h;
    }
}

int delta(STRATO *Ul, AREA *ar,int n)
{
  //calcolo l'indice di riga e colonna relativo a n
  int i=n/NCOLS;
  int j=n%NCOLS;
  double v=Ul->u[n].segnale;
  for(int ij=0;ij<O_VA;ij++)
    {
      int di=ar->o_i[ij];
      int dj=ar->o_j[ij];
      int l=i+di;
      int m=j+dj;
      if(l>=0&&l<NCOLS&&m>=0&&m<NCOLS)
        {
          int nu=l*NCOLS+m;
          if(Ul->u[nu].segnale>v) return -1;
        }
    }
      if(v==0) return 0;
      return 1;
}

void calcola_rinforzo(STRATO *Ul, AREA *ar)
{
  for(int n=0;n<NEURONS; n++)
    {      
      int d=delta(Ul,ar,n);
      //calcolo l'indice di riga e colonna relativo a n
      int i=n/NCOLS;
      int j=n%NCOLS;
      if(d==-1)continue;
      if(d==0)
        {
          
          double c=0;
          for(int ij=0;ij<S_CA;ij++)
            {
              int di=ar->a_i[ij];
              int dj=ar->a_j[ij];
              int l=i+di;
              int m=j+dj;
              if(l>=0&&l<NCOLS&&m>=0&&m<NCOLS)
                {
                  int nu=l*NCOLS+m;
                  double da=Q0*((Ul)->u[nu].ingresso_ecc)*((Ul-1)->v[n].c[ij]);
                  (Ul)->u[n].a[ij]+=da;
                  c+=(Ul-1)->v[n].c[ij];
                }
            }
          double db=Q0*((Ul)->u[n].ingresso_inib);
          if(COMPENSAZIONE_BORDI) db*=1/c;
          (Ul)->u[n].b+=db;
        }
      else
        {
          
          double c=0;
          double db=0;
          for(int ij=0;ij<S_CA;ij++)
            {
              int di=ar->a_i[ij];
              int dj=ar->a_j[ij];
              int l=i+di;
              int m=j+dj;
              
              if(l>=0&&l<NCOLS&&m>=0&&m<NCOLS)
                {
                  int nu=l*NCOLS+m;
                  double da=Q1*((Ul)->u[nu].ingresso_ecc)*((Ul-1)->v[n].c[ij]);
                  double u=((Ul)->u[nu].ingresso_ecc);                 
                  db+=Q1*u*u*((Ul-1)->v[n].c[ij])/(2*(Ul)->u[n].ingresso_inib);    
                  c+=(Ul-1)->v[n].c[ij];
                  (Ul)->u[n].a[ij]+=da;
                }
            }
          //Dal paper: no compensazione qui
           
          (Ul)->u[n].b+=db; 
        }
    }
}

void inizializza_connessioni(STRATO *l, AREA *a,double cn_ecc,double cn_inib)
{
  static double conn_ecc[S_CA];
  static double conn_ini[S_CA];
  static double conn_lat[H_HA];
  static char init=0;
        
  for(int i=0;i<S_CA;i++)
    {
      conn_ecc[i]=cn_ecc;
      conn_ini[i]=INIT_C;
    }

  for(int i=0;i<H_HA;i++)
    {
      conn_lat[i]=INIT_H;
    }
  
  for(int i=0; i<NEURONS;i++)
    {
      l->u[i].b=cn_inib;
      //area di connessione
      if(cn_ecc>=0)
        memcpy(l->u[i].a,conn_ecc,sizeof(double)*S_CA);
      if(cn_inib>=0)
        memcpy(l->v[i].c,conn_ini,sizeof(double)*S_CA);
      memcpy(l->u[i].g,conn_lat,sizeof(double)*H_HA);   
    }
}

void carica_segnale(STRATO *l, int indice, AREA *ar)
{
  if(indice==-1)
    {
      for(int i=0;i<NEURONS;i++)
        {
          l->u[i].pot=0;
          l->u[i].segnale=0;
           l->u[i].segnale_tmp=0;
          l->u[i].ingresso_ecc=0;
          l->u[i].ingresso_inib=0;
        }
    }
  
  if(indice==0)
    {
      if(NEURONS!=144)
        {
          fprintf(stderr,"Impostare la macro NEURONS a 144 per usare questi dati\n");
          exit(0);
        }
      double x[]={
        0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,1,1,1,1,0,0,0,0,
	0,0,0,1,0,0,0,0,1,0,0,0,
	0,0,1,0,0,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,0,0,1,0,0,
	0,0,0,1,0,0,0,0,1,0,0,0,
	0,0,0,0,1,1,1,1,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0
      };
      for(int i=0;i<144;i++)
        {
          l->u[i].pot=l->u[i].pot||x[i];
          l->u[i].ingresso_inib+=0;
        }
    }

  if(indice==1)
    {
      if(NEURONS!=144)
        {
          fprintf(stderr,"Impostare la macro NEURONS a 144 per usare questi dati\n");
          exit(0);
        }
      double x[]={
        0,0,0,0,0,1,0,0,0,0,0,0,
        0,0,0,0,0,1,0,0,0,0,0,0,
        0,0,0,0,0,1,0,0,0,0,0,0,
        0,0,0,0,0,1,0,0,0,0,0,0,
        0,0,0,0,0,1,0,0,0,0,0,0,
        0,0,0,0,0,1,0,0,0,0,0,0,
        0,0,0,0,0,1,0,0,0,0,0,0,
        0,0,0,0,0,1,0,0,0,0,0,0,
        0,0,0,0,0,1,0,0,0,0,0,0,
        0,0,0,0,0,1,0,0,0,0,0,0,
        0,0,0,0,0,1,0,0,0,0,0,0,
        0,0,0,0,0,1,0,0,0,0,0,0      
      };
      for(int i=0;i<144;i++)
        {
          l->u[i].pot=l->u[i].pot||x[i];
          l->u[i].ingresso_inib=+0;
        }
    }

  if(indice==2)
    {
      if(NEURONS!=144)
        {
          fprintf(stderr,"Impostare la macro NEURONS a 144 per usare questi dati\n");
          exit(0);
        }
      double x[]={
        0,0,0,1,1,1,1,1,1,0,0,0,
        0,0,1,0,0,0,0,0,0,1,0,0,
        0,1,0,0,0,0,0,0,0,0,1,0,
        0,1,0,0,0,0,0,0,0,0,1,0,
        0,0,0,0,0,0,0,0,0,0,1,0,
        0,0,0,0,0,0,0,0,1,1,0,0,
        0,0,0,0,0,0,1,1,0,0,0,0,
        0,0,0,0,1,1,0,0,0,0,0,0,
        0,0,1,1,0,0,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,0,0,0,0,
        1,1,1,1,1,1,1,1,1,1,1,1,
        0,0,0,0,0,0,0,0,0,0,0,0
      };
      for(int i=0;i<144;i++)
        {
          l->u[i].pot=l->u[i].pot||x[i];
          l->u[i].ingresso_inib+=0;
        }
    }

  if(indice==3)
    {
      if(NEURONS!=144)
        {
          fprintf(stderr,"Impostare la macro NEURONS a 144 per usare questi dati\n");
          exit(0);
        }
      double x[]={
        0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,1,1,1,1,0,0,0,0,
        0,0,0,1,0,0,0,0,1,0,0,0,
        0,0,0,1,0,0,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,1,0,0,0,
        0,0,0,0,0,0,1,1,1,0,0,0,
        0,0,0,0,0,0,0,0,0,1,0,0,
        0,0,1,0,0,0,0,0,0,0,1,0,
        0,0,1,0,0,0,0,0,0,0,1,0,
        0,0,0,1,0,0,0,0,0,1,0,0,
        0,0,0,0,1,1,1,1,1,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0        
      };
      for(int i=0;i<144;i++)
        {
          l->u[i].pot=l->u[i].pot||x[i];
          l->u[i].ingresso_inib+=0;
        }
    }

  if(indice==4)
    {
      if(NEURONS!=144)
        {
          fprintf(stderr,"Impostare la macro NEURONS a 144 per usare questi dati\n");
          exit(0);
        }
      double x[]={
        0,0,0,0,0,0,0,1,0,0,0,0,
        0,0,0,0,0,0,1,1,0,0,0,0,
        0,0,0,0,0,1,0,1,0,0,0,0,
        0,0,0,0,1,0,0,1,0,0,0,0,
        0,0,0,1,0,0,0,1,0,0,0,0,
        0,0,1,0,0,0,0,1,0,0,0,0,
        0,1,0,0,0,0,0,1,0,0,0,0,
        1,1,1,1,1,1,1,1,1,1,1,1,
        0,0,0,0,0,0,0,1,0,0,0,0,
        0,0,0,0,0,0,0,1,0,0,0,0,
        0,0,0,0,0,0,0,1,0,0,0,0,
        0,0,0,0,0,0,0,1,0,0,0,0        
      };
      for(int i=0;i<144;i++)
        {
          l->u[i].pot=l->u[i].pot||x[i];
          l->u[i].ingresso_inib+=0;
        }
    }
    if(indice==5)
    {
      if(NEURONS!=144)
        {
          fprintf(stderr,"Impostare la macro NEURONS a 144 per usare questi dati\n");
          exit(0);
        }
      double x[]={
        0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,1,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0        
      };
      for(int i=0;i<144;i++)
        {
          l->u[i].pot=l->u[i].pot||x[i];
          l->u[i].ingresso_inib+=0;
        }
    }
    //Gradiente
  if(indice==6)
    {
      if(NEURONS!=144)
        {
          fprintf(stderr,"Impostare la macro NEURONS a 144 per usare questi dati\n");
          exit(0);
        }
      double g=0;
      for(int i=0; i<NEURONS/NCOLS-0;i++)
        {
          g=0;
            {
              for(int j=0;j<NCOLS-0;j++)
                {
                  if(j<NCOLS/2)
                    {
                      g+=1;
                      l->u[i*NCOLS+j].pot+=g;
                    }
                  else if(j==NCOLS/2)
                    {
                      g+=0;
                      l->u[i*NCOLS+j].pot+=g;
                    }else
                    
                    {
                      g+=-1;
                      l->u[i*NCOLS+j].pot+=g;
                    }
                  l->u[i*NCOLS+j].ingresso_inib+=0;
                }
            }
        }
    }

}

void calcola_segnale_inibitore(STRATO *Ul,AREA *ar)
{
  for(int n=0;n<NEURONS; n++)
    {
      //calcolo l'indice di riga e colonna relativo a n
      int i=n/NCOLS;
      int j=n%NCOLS;
      double s=0;
       for(int ij=0;ij<S_CA;ij++)
        {
          int di=ar->a_i[ij];
          int dj=ar->a_j[ij];
          int l=i+di;
          int m=j+dj;
           if(l>=0&&l<NCOLS&&m>=0&&m<NCOLS)
            { 
               int nu=l*NCOLS+m;
               s+=Ul->v[n].c[ij]*Ul->u[nu].segnale;
             }
        }
       Ul->v[n].segnale=s;
    }
}

void calcola_potenziale(STRATO *Ul,AREA *ar)
{
  /*
    Per ogni neurone dello strato
    calcola il potenziale sommando
    gli ingressi eccitatori connessi 
    a tutta l'area di connessione
    del neurone e dividendo per gli
    ingrssi inibitori secondo la:
    u=(e-h)/(h+1)
   */

  for(int n=0;n<NEURONS; n++)
    {
      //calcolo l'indice di riga e colonna relativo a n
      int i=n/NCOLS;
      int j=n%NCOLS;
      /* somma in contributi eccitatori */
      double e=0;
      double h=0;
      double s=0;
       for(int ij=0;ij<S_CA;ij++)
        {
          int di=ar->a_i[ij];
          int dj=ar->a_j[ij];
          int l=i+di;
          int m=j+dj;
           if(l>=0&&l<NCOLS&&m>=0&&m<NCOLS)
             {
               int nu=l*NCOLS+m;
               e+=Ul->u[n].a[ij]*Ul->u[nu].ingresso_ecc;          
             }
        }
       h=Ul->u[n].b*Ul->u[n].ingresso_inib;
       Ul->u[n].pot=(e-h)/(1+h);
    } 
}

void filtra_max(STRATO *Ul)
{
  double max=0;
  for(int n=0;n<NEURONS; n++)
    {
      double v=Ul->u[n].segnale;
      if(v>max)
        max=v;
    }
  for(int n=0;n<NEURONS; n++)
    {
      double v=Ul->u[n].segnale;
      if(v<max)
        {
          Ul->u[n].pot=0;
          Ul->u[n].ingresso_ecc=0;
          Ul->u[n].ingresso_inib=0;
          Ul->u[n].segnale=0;
          Ul->v[n].segnale=0;
        }
      
    }
  
}

void calcola_segnale_eccitatore(STRATO *Ul)
{
  for(int n=0;n<NEURONS; n++)
    {
      double tmp;
      tmp=Ul->u[n].pot;
      Ul->u[n].segnale=phi(tmp);
    }
}

double phi(double x)
{
  if (x<0) return 0;
  return x;
}

void inizializza_aree(AREA *ar)
{
  /* inibizione laterale */
  int cn=0;
  for(int i=-INIB_L;i<=INIB_L;i++)
    for(int j=-INIB_L;j<=INIB_L;j++,cn++)
      {
        ar->g_i[cn]=i;
        ar->g_j[cn]=j;
      }
  
  /* area di connessione */
  cn=0;
  for(int i=-CA_L;i<=CA_L;i++)
    for(int j=-CA_L;j<=CA_L;j++,cn++)
      {
        ar->a_i[cn]=i;
        ar->a_j[cn]=j;
      }
  
    /* vicinato */
  cn=0;
  for(int i=-O_L;i<=O_L;i++)
    {
      int lc;
      if(i<=0)lc=(2+i);else lc=(2-i);
      for(int j=-lc;j<=lc;j++,cn++)
        {
          ar->o_i[cn]=i;
          ar->o_j[cn]=j;
        }
    }  
}

void print_object(double x[],int r, int c,int R,int C,char *str)
{
  double min,max;
  min=10000;
  max=-10000;
  for(int i=0;i<r;i++)
    for(int j=0;j<c;j++)
      {
	if(x[i*c+j]>max) max=x[i*c+j];
	if(x[i*c+j]<min) min=x[i*c+j];
      }
  double i_range=max-min;
  if(i_range==0)
    {
      i_range=1;
    }
  double max_c=255;
  double min_c=232;//232;
  double c_range=max_c-min_c;
  double conv=c_range/i_range;

  printf("\x1b[%d;%dH%s  ",R-2,C,str);
  for(int i=0;i<r;i++)
    for(int j=0;j<c;j++)
      {
	double gl;
	gl=x[i*c+j];
	int col=max_c-(min_c+(double)(gl-min)*conv)+min_c;
	if(col<0)printf("%f",gl);
	printf("\x1b[%d;%dH\x1b[48;5;%dm  \x1b[0m",i+R,2*j+C,col);
      }
  fflush(stdout);
}

double * estrai_array(double *dest,STRATO *l,int selettore)
{
  /*
    Il paramtero selettore serve a indicare quale array estrarre
    tra:
    0-segnale ecc
    1-segnale inib
    2-potenziale
    3-ingresso ecc
  */
  for(int n=0;n<NEURONS; n++)
    {
      if(selettore==0)
        dest[n]=l->u[n].segnale;
      if(selettore==1)
        dest[n]=l->v[n].segnale;
      if(selettore==2)
        dest[n]=l->u[n].pot;
      if(selettore==3)
        dest[n]=l->u[n].ingresso_ecc;
    }
  return dest;
}

