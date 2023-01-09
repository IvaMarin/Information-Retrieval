CC = g++
FLAGS = -std=c++17 -O3 -pedantic -lm
NAME = search_engine
SOURCE_FILES = *.cpp
CORPUS = corpus/wikipedia_preview
INDEX_FILE = index/id
INPUT_FILE = queries/in
OUTPUT_FILE = results/out

all: search_engine

search_engine:
	$(CC) $(FLAGS) -o $(NAME) $(SOURCE_FILES)

debug:
	$(CC) -g $(FLAGS) -o $(NAME) $(SOURCE_FILES)

create:
	./$(NAME) index --input $(CORPUS) --output $(INDEX_FILE)

search-console:
	./$(NAME) search --index $(INDEX_FILE) --output $(OUTPUT_FILE)

search:
	./$(NAME) search --index $(INDEX_FILE) --input $(INPUT_FILE) --output $(OUTPUT_FILE)

clean:
	rm -f *.o search_engine