// This program make a rectangle query to sdss dr16
// Copyright (C) 2021 Adrian Hernandez Padron <adrianhdezp10@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


//compile with: gcc sdss16boxquery.c -lcurl -o sdss16boxquery
//This generate the sdss16boxquery executor-> ./sdss16boxquery
//help panel with ./sdss16boxquery -h

#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

int Radial_query();
int helpPanel();
int writefile();

//This are necessary to save the response table in a variable
struct MemoryStruct {
  char *memory;
  size_t size;
};
 
static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
 
  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(ptr == NULL) {
    /* out of memory! */ 
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
 
  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}
int parameter_counter = 0;
//main function in where we obtain the input and manipulate to build the url
int main(int argc, char **argv)
{
	static struct option long_options[] =
	{
	{"ra", required_argument,0, 'r'},
	{"dec", required_argument,0, 'd'},
	{"limit", optional_argument,0, 'l'},
	{"format", required_argument,0, 'f'},
    {"uband", required_argument,0, 'u'},
    {"gband", required_argument,0, 'g'},
    {"rband", required_argument,0, 'j'},
    {"iband", required_argument,0, 'i'},
    {"zband", required_argument,0, 'z'},
    {"output", required_argument,0, 'o'},
    {"help", no_argument,0, 'h'},
		{0 , 0, 0, 0}
	};
  int option_index = 0;
  int opt;
  char *ramin,*ramax,*decmin,*decmax,*formato, *ra, *dec, *salida, *u, u_band[40] = {} ,*g, g_band[40]= {},*r, r_band[40] = {} ,*i, i_band[40] = {} ,*z, z_band[40] = {} , filename[50];
  formato = NULL;
  int limite = 0;

	while ((opt = getopt_long(argc, argv, "r:c:l:f:o:u:g:j:i:z:h", long_options, &option_index)) != -1) {
		switch (opt) {
      case 'r':
        ra = optarg;
        ++parameter_counter;
        break;
      case 'd':
        dec = optarg;
        ++parameter_counter;
        break;
      case 'l':
        if (optarg) {
          limite = atoi(optarg);
        }
        break;
      case 'f':
        if (optarg) {
            formato = optarg;
        }
        break;
      case 'o':
        salida = optarg;
        break;
      case 'u':
        u = optarg;
        snprintf(u_band, 40, "&uband=%s&check_u=u",u);
        break;
      case 'j':
        r = optarg;
        snprintf(r_band, 40, "&rband=%s&check_r=r",r);
        break;
      case 'g':
        g = optarg;
        snprintf(g_band, 40, "&gband=%s&check_g=g",g);
        break;
      case 'i':
        i = optarg;
        snprintf(i_band, 40, "&iband=%s&check_i=i",i);
        break;
      case 'z':
        z = optarg;
        snprintf(z_band, 40, "&zband=%s&check_z=z",z);
        break;
      case 'h':
	      parameter_counter=3;
        break;
      default:
        helpPanel();
		}
	}
  if (parameter_counter == 2) {
    //separate the centre coordinates in ramin and max
    char *ptr = strtok(ra, ",");
    ramin = ptr;
    ptr = strtok(NULL, ",");
    ramax = ptr;
    ptr = strtok(NULL, ",");
    //same with dec 
    char *ptr2 = strtok(dec, ",");
    decmin = ptr2;
    ptr2 = strtok(NULL, ",");
    decmax = ptr2;
    ptr2 = strtok(NULL, ",");
    //formate by default 
    if (!formato ) {
        formato = "txt"; 
    }
    char url[400];
    //build the url with the query
    snprintf(url, 400, "http://skyserver.sdss.org/dr16/SkyServerWS/SearchTools/RectangularSearch?min_ra=%s&max_ra=%s&min_dec=%s&max_dec=%s&whichway=equatorial&limit=%d&format=%s&whichquery=imaging%s%s%s%s%s",ramin, ramax,decmin ,decmax, limite ,formato,u_band, g_band, r_band, i_band, z_band); //
    printf("\nQuery to: %s\n", url);
    //bands and limits
    //save the output name
    //snprintf(filename, 50,"%s.%s", salida, formato);
    Radial_query(url);

  }
  else if (parameter_counter == 0) {
    printf("\n[!]INPUTERROR[!][!]INPUTERROR[!][!]INPUTERROR[!]\n");
  }
  else {
    helpPanel();    
  }
}


int helpPanel(){
  printf("\n[!] Usage:\n");
  printf("\n\n\t[!]This program require 2 arguments:\n");
  printf("\n\n\t\t[-r][--radius] Radius in arcminutes\n");
  printf("\n\n\t\t[-c][--centre] Centre(ra,dec) in degrees\n");
  printf("\n\n\t[!] Optional argumets:\n");
  printf("\n\n\t\t[-o][--output] File name withtout extension when we want to save the tables(currently disabled)\n");
  printf("\n\n\t\t[-f][--format] Format supported: csv,html,xml,votable,json,fits,mydb(currently disabled)\n");
  printf("\n\n\t\t[-l][--limit] Maximus number of rows\n");
  printf("\n\n\tBands are defined as a comma seperated range e.g uband=0,20.\n");
  printf("\n\n\t\t[-g][--gband] upper limit for gband\n");
  printf("\n\n\t\t[-j][--rband] upper limit for rband\n");
  printf("\n\n\t\t[-z][--zband] upper limit for zband\n");
  printf("\n\n\t\t[-i][--iband] upper limit for iband\n");
  printf("\n\n\t\t[-u][--uband] upper limit for uband\n");
  printf("\n\n[!] Example:\n\n\t\t./sdss16boxquery --ra=250.2,250.5 --dec=35.1,35.5 --gband=0,12 --iband=0,15    \n");
  printf("\n\n\t\t./sdss16boxquery -r 250.2,250.5 -d 35.1,35.5 -g 0,12 -i 0,15     \n");
  parameter_counter = 0;
  return 0;
}


int Radial_query(char *url){

  CURLcode ret;
  CURL *hnd;
  struct MemoryStruct query;
 
  query.memory = malloc(1); 
  query.size = 0; 
  hnd = curl_easy_init();

  curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
  curl_easy_setopt(hnd, CURLOPT_URL, url);
  curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(hnd, CURLOPT_WRITEDATA, (void *)&query);
  curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
  curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.75.0");


  ret = curl_easy_perform(hnd);

  printf("%s", query.memory);

  curl_easy_cleanup(hnd);
  hnd = NULL;

  return 0;
}

/*int writefile(int *name,char *table) {
  FILE *fPointer;
  fPointer = ("name", "w");

  fprintf(fPointer, table);

  fclose(fPointer);

  return 0;

}*/
