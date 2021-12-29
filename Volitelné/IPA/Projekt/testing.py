import subprocess

def run(name):
    res = subprocess.run([f"projekt_{name}.exe", "input\\input2.png", f"{centroidCount}", f"{ci}", "12345", f"out\\{name}_{centroidCount}_{ci}.png"],
            stderr=subprocess.PIPE, stdout=subprocess.DEVNULL)
    res_out = res.stderr.decode('utf-8')
    lines = res_out.splitlines()
    return (int(lines[0]), int(lines[1]))

print("centroidCount;centroidIndex;asmKmeansCycles;asmProcCycles;cppKmeansCycles;cppProcCycles")
for centroidCount in range(2, 11):
    for ci in range(0, centroidCount):
        asm_res = run('asm')
        cpp_res = run('cpp')

        print(f"{centroidCount};{ci};{asm_res[0]};{asm_res[1]};{cpp_res[0]};{cpp_res[1]}")
