import sys
import subprocess
import glob
import re
import numpy as np
c = re.compile("tools/in/(.*)\\.txt")
score_c = re.compile("Score = ([0-9]*)")

scores = []
for file in sorted(glob.glob("tools/in/*")):
    m = c.match(file)
    if m is not None:
        target = m.group(1)
        print(target)

        subprocess.run(f"build/main <tools/in/{target}.txt > tools/out/out{target}.txt", shell=True, stderr=subprocess.PIPE)

        proc = subprocess.run([f"tools/target/debug/vis",
                               f"tools/in/{target}.txt",
                               f"tools/out/out{target}.txt"],
                              stdout=subprocess.PIPE,
                              stderr=subprocess.PIPE)
        out = proc.stdout.decode("utf-8")
        print(out)
        score_m = score_c.match(out)
        if score_m is not None:
            scores.append(int(score_m.group(1)))
print("len", len(scores))
print("sum", sum(scores))
print("mean", np.mean(scores))
