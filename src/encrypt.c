#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <rpc/des_crypt.h>

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


void save_to_file(char* cipher){
  //SAVE TO FILE ENCRYPTED
  FILE *f = fopen("./src/crypted.txt", "w");
  if (f == NULL)
  {
      printf("Error opening file!\n");
      exit(1);
  }

  /* print some text */
  fprintf(f, "%s", cipher);

  fclose(f);
}

int main(){
    char cipher[] = "operating systems is fun";
    // long key = (1L<<56)/2 + 50000000L;
    long key = 1000L;
    encrypt(key, cipher, strlen(cipher));


    save_to_file(cipher);
    return 0;
}