
// bruteforce.c
// nota: el key usado es bastante pequenio, cuando sea random speedup variara

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <rpc/des_crypt.h>

char* crypted_file;
char *search;

char* read_crypted(){
  char* cipher;
  long lSize;
  FILE *fp;
  //LECTURA DEL TEXTO
  fp = fopen ("src/crypted.txt", "rb" );
  if(!fp) perror("crypted.txt"),exit(1);

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
  if(argc >= 3){
    crypted_file = argv[1];
    search = argv[2];
  }else{
    crypted_file = "src/crypted.txt";
    search = "systems";
  }
  //LECTURA DEL TEXTO
  cipher = read_crypted();

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
printf("\nKey encontrada: %li\nTexto cifrado %s\n", found, cipher);
clock_t end = clock();
double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

printf("That took %f seconds\n",time_spent);
  


return 0;
}

