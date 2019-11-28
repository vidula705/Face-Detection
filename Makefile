#############################################################################################################################
# File:    Makefile                                                                                                         #           
# Desc:    used to compile source code for face detector     counter                                                            
#                                                                                                                           #
#                                                                                                                                                                                                                                                    
#############################################################################################################################

CC=gcc -g
LD=ld  -g
AS=as    


all:
		$(CC) -Wall -std=c99 eiFaceCount.c  `pkg-config --cflags --libs opencv` -lm
clean:
	rm *.out 
