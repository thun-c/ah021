import sys
import subprocess
args = sys.argv

assert (len(args) == 2)
target = args[1]
subprocess.run(f"build/main <tools/in/{target}.txt > tools/out/out{target}.txt", shell=True)
subprocess.run(f"tools/target/debug/vis tools/in/{target}.txt tools/out/out{target}.txt", shell=True)
