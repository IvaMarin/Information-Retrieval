FILE_NAME = 'corpus/wikipedia'
PREVIEW_SIZE = 1557199
# FILE_NAME = 'id'
# PREVIEW_SIZE = 1000000
BUFFER = list()
TOTAL = 0

with open(FILE_NAME) as f:
    for _ in range(PREVIEW_SIZE):
        BUFFER.append(f.readline())

with open(f'{FILE_NAME}_preview', 'w') as f:
    for i in range(PREVIEW_SIZE):
        if (BUFFER[i] == "</doc>\n"):
            TOTAL += 1
        f.write(BUFFER[i])

print(TOTAL)