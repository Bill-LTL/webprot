make : init.h readconfig.h main.c
	gcc -o webprot.out main.c -lpthread