# o ficheiro 'output' será recriado sempre que o ficheiro 'main.o' ou 'parse.o' forem alterados
output: main.o parse.o
	# Recompila ambos os ficheiros no ficheiro executável 'output'
	gcc main.o parse.o -o output -lreadline -lpthread

main.o: main.c header.h
	# Compila o ficheiro 'main.c' sempre que 'main.c' ou 'header.h' forem alterados
	# -c para não criar o ficheiro executável correspondente - cria o ficheiro 'main.o'
	gcc -c -lpthread main.c

parse.o: parse.c header.h
	gcc -c parse.c

# Elimina todos os ficheiros '*.o', bem como o ficheiro 'output'
clean:
	rm *.o output

#ficheiro_objectivo (nome do executável): dependências
