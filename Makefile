

proyect: encrypt.o proyecto_block.o proyecto.o sec.o
	echo Terminado

encrypt.o: ./src/encrypt.c
	gcc ./src/encrypt.c -o ./build/encrypt

proyecto_block.o: ./src/proyecto_block.c
	mpicc ./src/proyecto_block.c -o ./build/proyecto_block -lm

proyecto.o: ./src/proyecto.c
	mpicc ./src/proyecto.c -o ./build/proyecto -lm

sec.o: ./src/sec.c
	mpicc ./src/sec.c -o ./build/sec -lm


clean: 
	rm ./build/*