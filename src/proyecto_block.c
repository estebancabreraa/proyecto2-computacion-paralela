
// bruteforce.c
// nota: el key usado es bastante pequenio, cuando sea random speedup variara

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include <rpc/des_crypt.h>

char* crypted_file;
char *search;

char* read_crypted(){
  char* cipher;
  long lSize;
  FILE *fp;
  //LECTURA DEL TEXTO
  fp = fopen (crypted_file , "rb" );
  if( !fp ) perror(crypted_file),exit(1);

  fseek( fp , 0L , SEEK_END);
  lSize = ftell( fp );
  rewind( fp );

  /* allocate memory for entire content */
  cipher = calloc( 1, lSize+1 );
  if( !cipher ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);

  /* copy the file into the cipher */
  if( 1!=fread( cipher , lSize, 1 , fp) )
    fclose(fp),free(cipher),fputs("entire read fails",stderr),exit(1);

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
int N, id;
long upper = (1L << 56); // upper bound DES keys 2^56
long mylower, myupper;
MPI_Status st;
MPI_Request req;
int flag;


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

MPI_Comm comm = MPI_COMM_WORLD;

MPI_Init(NULL, NULL);
MPI_Comm_size(comm, &N);
MPI_Comm_rank(comm, &id);
int block_size = 4;
long range_per_node =  ceil(upper / (double)N);
long sub_range_iterations = ceil(range_per_node / (double)block_size);

//Init vars for time
double starttime, endtime;
starttime = MPI_Wtime();
long found = 0;
int ready = 0;
long iteration = 0;
MPI_Irecv(&found, 1, MPI_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &req);
for(long pointer = 0; pointer < sub_range_iterations; pointer++){
  mylower = N*block_size*pointer+id*block_size;
  myupper = mylower+block_size;
  for (long i = mylower; i < myupper; ++i)
  {
    //We will check every 250 iterations so we dont spend more time with MPI_Test
    if((iteration) % 250L == 0)
    {
      MPI_Test(&req, &ready, MPI_STATUS_IGNORE);
      if (ready) {
        pointer = sub_range_iterations;
        break;
        }
    } // ya encontraron, salir
    if (tryKey(i, (char *)cipher, ciphlen))
    {
      found = i;
      for (int node = 0; node < N; node++)
      {
        MPI_Send(&found, 1, MPI_LONG, node, 0, MPI_COMM_WORLD);
      }
      pointer = sub_range_iterations;
      break;
    }
    iteration++;
  }
}

if (id == 0)
{
MPI_Wait(&req, &st);
decrypt(found, (char *)cipher, ciphlen);
printf("\nKey encontrada: %li\nValor del texto: %s\n", found, cipher);
endtime = MPI_Wtime();
printf("That took %f seconds\n",endtime-starttime);
}


MPI_Finalize();
}

