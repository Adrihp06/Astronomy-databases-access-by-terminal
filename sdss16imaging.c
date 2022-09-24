// This program obtain an image around a given center from the sdss16 api.
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


//compile with: gcc sdss16imaging.c -lcurl -o sdss16imaging
//This generate the sdss16 executor-> ./sdss16imaging
//help panel with ./sdss16imaging -h

#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

int Radial_query();
int helpPanel();

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

int main(int argc, char **argv)
{
	static struct option long_options[] =
	{
		{"scale", optional_argument,0, 's'},
		{"centre", required_argument,0, 'c'},
		{"height", optional_argument,0, 'h'},
		{"wide", optional_argument,0, 'w'},
        {"overlay", required_argument,0, 'l'},
        {"output", required_argument,0, 'o'},
        {"help", no_argument,0, 'g'},
		{0 , 0, 0, 0}
	};
  int option_index = 0;
  int opt, parameter_counter = 0;
  char *overlay, *centro, *ra, *dec, *salida, filename[50];
  int wide = 512, high = 512;
  float scale=0.2;

	while ((opt = getopt_long(argc, argv, "s:c:h:w:l:o:g", long_options, &option_index)) != -1) {
		switch (opt) {
      case 's':
        if (optarg) {
          scale = atoi(optarg);
        }
      case 'c':
        centro = optarg;
        ++parameter_counter;
        break;
      case 'h':
        if (optarg) {
          high = atoi(optarg);
        }
        break;
      case 'w':
        if (optarg) {
          wide = atoi(optarg);
        }
        break;
      case 'l':
        overlay=optarg;
        ++parameter_counter;
        break;
      case 'o':
        salida = optarg;
        ++parameter_counter;
        break;
      case 'g':
	      parameter_counter=5;
        break;
      default:
        helpPanel();
		}
	}


  if (parameter_counter == 3) {
    //separate the centre coordinates in ra and dec
    char *ptr = strtok(centro, ",");
    ra = ptr;
    ptr = strtok(NULL, ",");
    dec = ptr;
    ptr = strtok(NULL, ",");

    char url[200];
    snprintf(url, 200, "http://skyserver.sdss.org/dr16/SkyServerWS/ImgCutout/getjpeg?ra=%s&dec=%s&scale=%f&height=%d&width=%d&opt=%s ",ra, dec, scale, wide ,high, overlay);
    snprintf(filename, 50,"%s", salida);
    Radial_query(url, filename);

  }
  else {
    helpPanel();
  }
}


int helpPanel(){
  printf("\n[!] Usage:\n");
  printf("\n\n\t[!]This program require 3 arguments:\n");
  printf("\n\n\t\t[-c][--centre] Centre(ra,dec) in degrees\n");
  printf("\n\n\t\t[-o][--output] File name withtout extension when we want to save the tables\n");
  printf("\n\n\t\t[-l][--overlay] string of characters for overlays on image[G,L,P,S,T,O,B,F,M,Q,I]\n");
  printf("\n\n\t[!] Optional argumets:\n");
  printf("\n\n\t\t[-w][--widht] In pixels, default 512\n");
  printf("\n\n\t\t[-w][--widht] In pixels, default 512\n");
  printf("\n\n\t\t[-s][--scale] In pixels, default 0.4\n");
  //printf("\n\n[!] Example:\n\n\t\t./sdss16imaging --centre=255.99085171002997,40.2505413655 --output=my-image --overlay=GOL\n");
  //printf("\n\n\t\t./sdss16imaging -c 255.99085171002997,40.2505413655 -o my-image -l GOL\n");
  //printf("\nThe result is a image of the target.\n");
}


int Radial_query(char *url, char *filename){

  CURLcode ret;
  CURL *hnd;
  FILE *fp;
  char *outfilename= filename;

  hnd = curl_easy_init();
  fp = fopen(outfilename,"wb");
  curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
  curl_easy_setopt(hnd, CURLOPT_URL, url);
  curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
  curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.75.0");
  curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_data);
  curl_easy_setopt(hnd, CURLOPT_WRITEDATA, fp);


  ret = curl_easy_perform(hnd);

  curl_easy_cleanup(hnd);
  hnd = NULL;

  return (int)ret;
}



