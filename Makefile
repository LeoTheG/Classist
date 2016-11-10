converter:
	g++ -std=c++0x -L/usr/local/lib -o converter converter.cpp clean_text.cc -lcurl -lgumbo
	#g++ -std=c++0x -o converter converter.cpp -lcurl -lgumbo
clean:
	rm converter
