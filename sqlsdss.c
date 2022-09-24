#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>


int sql_query();
int helpPanel();
int writefile();




/*char* urlencode(originalText){


{
    // allocate memory for the worst possible case (all characters need to be encoded)
    char *encodedText = (char *)malloc(sizeof(char)*strlen(originalText)*3+1);
    
    const char *hex = "0123456789abcdef";
    
    int pos = 0;
    for (int i = 0; i < strlen(originalText); i++) {
        if (('a' <= originalText[i] && originalText[i] <= 'z')
            || ('A' <= originalText[i] && originalText[i] <= 'Z')
            || ('0' <= originalText[i] && originalText[i] <= '9')) {
                encodedText[pos++] = originalText[i];
            } else {
                encodedText[pos++] = '%';
                encodedText[pos++] = hex[originalText[i] >> 4];
                encodedText[pos++] = hex[originalText[i] & 15];
            }
    }
    encodedText[pos] = '\0';
    return encodedText;
}*/




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



int option_index = 0;
int opt;
int parameter_counter = 0;
int main(int argc, char **argv)
{

	static struct option long_options[] =
	{
    {"sqlquery", required_argument,0, 's'},
    {"format", required_argument,0, 'f'},
    {"help", no_argument,0, 'h'},
		{0 , 0, 0, 0}
	};



  char *formato; char *sql;
  formato = NULL;
	while ((opt = getopt_long(argc, argv, "s:f:h", long_options, &option_index)) != -1) {
		switch (opt) {
    case 's':
      sql=optarg;
      ++parameter_counter;
    break;  
    case 'f':
      formato=optarg;
      break;
    default:
      helpPanel();
		}
	}
  if (parameter_counter == 1) {
    
    if (!formato ) {
      formato = "txt"; 
    }

    //encode the sql petition
    char *url_encoded;
    CURL *curl= curl_easy_init();
    if(curl) {
      char *url_encoded = curl_easy_escape(curl, sql, strlen(curl));
      if(url_encoded) {
      
      char url[2000];
      //and now we build the url with the query
      snprintf(url, 2000, "http://skyserver.sdss.org/dr16/SkyServerWS/SearchTools/SqlSearch?cmd=%s&format=%s",url_encoded,formato); //
      printf("\nQuery to: %s\n", url);
      sql_query(url);
      curl_free(url_encoded);
      }
    }


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
  printf("\n\n[!] Example:\n\n\t\t./sdss16 --radius=1 --centre=255.99085171002997,40.2505413655 --uband=0,20 \n");
  printf("\n\n\t\t./sdss16 -r 1 -c 255.99085171002997,40.2505413655 -u 0,20\n");
  parameter_counter = 0;
  return 0;
}




int sql_query(char *url){

  
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


