
// bruteforce.c
// nota: el key usado es bastante pequenio, cuando sea random speedup variara

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include <rpc/des_crypt.h>

char* read_file(){
  char* cipher;
  long lSize;
  FILE *fp;
  //LECTURA DEL TEXTO
  fp = fopen ("src/texto.txt", "rb" );
  if(!fp) perror("texto.txt"),exit(1);

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
  int N, id;
  long upper = (1L << 56); // upper bound DES keys 2^56
  long mylower, myupper;
  MPI_Status st;
  MPI_Request req;
  int flag;
  char *cipher;

  //LECTURA DEL TEXTO
  cipher = read_crypted();

  //Inmediate finds it in case procesors 4

  //Case it takes a loong long time
  // long key = (1L<<56)/2 + (1L<<56)/8;
  // long key = (1L<<56)/2 + 50000000L;
  // encrypt(key, cipher, strlen(cipher));


  // save_to_file(cipher);

  int ciphlen = strlen(cipher);

  MPI_Comm comm = MPI_COMM_WORLD;

  MPI_Init(NULL, NULL);
  MPI_Comm_size(comm, &N);
  MPI_Comm_rank(comm, &id);

  long range_per_node =  ceil(upper / (double)N);
  mylower = range_per_node * id;
  myupper = range_per_node * (id + 1) - 1;


  if (id == N - 1)
  {
    myupper = upper;
  }


  //Init vars for time
  double starttime, endtime;
  starttime = MPI_Wtime();
  long found = 0;
  int ready = 0;
  MPI_Irecv(&found, 1, MPI_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &req);
  for (long i = mylower; i < myupper; ++i)
  {
    //We will check every 250 iterations so we dont spend more time with MPI_Test
    if((i - mylower) % 250L == 0)
    {
      MPI_Test(&req, &ready, MPI_STATUS_IGNORE);
      if (ready) break;
    } // ya encontraron, salir
    if (tryKey(i, (char *)cipher, ciphlen))
    {
      found = i;
      for (int node = 0; node < N; node++)
      {
        MPI_Send(&found, 1, MPI_LONG, node, 0, MPI_COMM_WORLD);
      }
      break;
    }
  }

  if (id == 0)
  {
    MPI_Wait(&req, &st);
    decrypt(found, (char *)cipher, ciphlen);
    printf("\n%li %s\n", found, cipher);
    endtime = MPI_Wtime();
    printf("That took %f seconds\n",endtime-starttime);
  }


  MPI_Finalize();
}

