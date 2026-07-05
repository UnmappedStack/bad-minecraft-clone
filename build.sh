set -e

cc -pedantic -Wall -c main.c -o bin/main.o -g
cc -pedantic -Wall -c render.c -o bin/render.o -g
cc -pedantic -Wall -c place.c -o bin/place.o -g
cc bin/main.o bin/render.o bin/place.o -o out -g -fno-pie -no-pie -lraylib -lm
