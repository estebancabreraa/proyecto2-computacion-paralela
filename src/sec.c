
// bruteforce.c
// nota: el key usado es bastante pequenio, cuando sea random speedup variara

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <rpc/des_crypt.h>

char* read_file(){
  char* cipher;
  long lSize;
  FILE *fp;
  //LECTURA DEL TEXTO
  fp = fopen ("src/secuencial_texto.txt", "rb" );
  if(!fp) perror("secuencial_texto.txt"),exit(1);

  fseek(fp, 0L, SEEK_END);
  lSize = ftell(fp);
  rewind(fp);

  /* allocate memory for entire content */
  cipher = calloc(1, lSize+1);
  if(!cipher) fclose(fp), fputs("memory alloc fails", stderr),exit(1);

  /* copy the file into the cipher */
  if(1!=fread( cipher , lSize, 1 , fp))
    fclose(fp), free(cipher), fputs("entire read fails", stderr),exit(1);
  /* do your work here, cipher is a string contains the whole text */
  fclose(fp);
  return cipher;
}

void save_to_file(char* cipher){
  //SAVE TO FILE ENCRYPTED
  FILE *f = fopen("crypted.txt", "w");
  if (f == NULL)
  {
      printf("Error opening file!\n");
      exit(1);
  }

  /* print some text */
  fprintf(f, "%s", cipher);

  fclose(f);
}

void decrypt(long key, char *ciph, int len)
{
  // set parity of key and do decrypt
  long k = 0;
  for (int i = 0; i < 8; ++i)
  {
    key <<= 1;
    k += (key & (0xFE << i * 8));
  }
  des_setparity((char *)&k); // el poder del casteo y &
  ecb_crypt((char *)&k, (char *)ciph, len, DES_DECRYPT);
}

void encrypt(long key, char *ciph, int len)
{
// set parity of key and do decrypt
long k = 0;
for (int i = 0; i < 8; ++i)
{
key <<= 1;
k += (key & (0xFE << i * 8));
}
des_setparity((char *)&k); // el poder del casteo y &
ecb_crypt((char *)&k, (char *)ciph, len, DES_ENCRYPT);
}

char search[] = " systems ";
int tryKey(long key, char *ciph, int len)
{
char temp[len + 1];
memcpy(temp, ciph, len);
temp[len] = 0;
decrypt(key, temp, len);
return strstr((char *)temp, search) != NULL;
}

int main(int argc, char *argv[])
{ // char **argv
  long upper = (1L << 56); // upper bound DES keys 2^56
  long mylower, myupper;
  char *cipher;

  //LECTURA DEL TEXTO
  cipher = read_file();

  long key = 300L;
  encrypt(key, cipher, strlen(cipher));
  save_to_file(cipher);

  int ciphlen = strlen(cipher);



  //Init vars for time
  clock_t begin = clock();

  long found = 0;

  for (long i = 0; i < upper; ++i)
  {
    if (tryKey(i, (char *)cipher, ciphlen))
    {
      found = i;
      break;
    }
  }

  

decrypt(found, (char *)cipher, ciphlen);
printf("\nKey:%li\nTexto cifrado%s\n", found, cipher);
clock_t end = clock();
double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

printf("That took %f seconds\n",time_spent);
  


return 0;
}

