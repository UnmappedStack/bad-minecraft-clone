set -e


cc -pedantic -Wall -c main.c -o bin/main.o -g
cc -pedantic -Wall -c render.c -o bin/render.o -g
cc -pedantic -Wall -c chunk.c -o bin/chunk.o -g
cc -pedantic -Wall -Werror -c place.c -o bin/place.o -g
cc bin/main.o bin/render.o bin/place.o bin/chunk.o -o out -g -lraylib
