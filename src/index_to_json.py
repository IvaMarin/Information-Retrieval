import json
import sys

data = list()

with open(sys.argv[1], 'r', encoding='utf-8') as file:
    total = file.readline().rstrip()
    id = 0
    while (line := file.readline().rstrip()):
        try:
            title, url, words = line.split("\" \"")
            data.append({
                "id": id,
                "title": title[1:],
                "url": url,
                "words": words[:-1]
            })
            id += 1
        except:
            pass

with open(sys.argv[2], "w", encoding='utf-8') as write_file:
    json.dump(data, write_file)
