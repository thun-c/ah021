N = 4
id = 0
ids = []
for i in range(N):
    ids.append([])
    for j in range(0, i + 1):
        ids[i].append(id)
        print(id, end=' ')
        id += 1
    print()

print(ids)

for pids in ids:
    