.PHONY = main
main:
	mkdir -p obj
	g++ -o obj/datagen datagen.cpp -Wall -pedantic -O2
	g++ -o obj/traverse main.cpp tree.cpp -Wall -pedantic -O2 -lpthread -mavx -mavx2 -mfma

clean:
	rm -r ./obj

