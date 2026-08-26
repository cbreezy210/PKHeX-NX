import os, re, sys, shutil

# Usage:  py make_assets.py [path-to-pkhex-ref] [output-dir]
BASE = sys.argv[1] if len(sys.argv) > 1 else r"C:\switchdev\pkhex-ref"
OUTD = sys.argv[2] if len(sys.argv) > 2 else r"C:\switchdev\dumps"

personal = os.path.join(BASE, "PKHeX.Core", "Resources", "byte", "personal", "personal_sv")
levelup  = os.path.join(BASE, "PKHeX.Core", "Resources", "byte", "levelup", "lvlmove_sv.pkl")
moveinfo = os.path.join(BASE, "PKHeX.Core", "Moves", "MoveInfo9.cs")
moves_txt = os.path.join(BASE, "PKHeX.Core", "Resources", "text", "other", "en", "text_Moves_en.txt")

data = open(personal, 'rb').read()

# Auto-detect personal entry size using Pikachu base stats (35,55,40,90,50,50)
PIKA = [35,55,40,90,50,50]
size = None
for s in range(0x30, 0x90):
    o = 25*s
    if o+6 <= len(data) and list(data[o:o+6]) == PIKA:
        size = s; break
n = len(data)//size

# stats.txt  (base stats)
with open(os.path.join(OUTD,"stats.txt"),"w") as f:
    f.write("0 0 0 0 0 0\n")
    for i in range(1,n):
        b = data[i*size:i*size+6]
        f.write("%d %d %d %d %d %d\n"%(b[0],b[1],b[2],b[3],b[4],b[5]))

# abilities.txt  (u16 at 0x12 / 0x14 / 0x16)
with open(os.path.join(OUTD,"abilities.txt"),"w") as f:
    f.write("0 0 0\n")
    for i in range(1,n):
        e = data[i*size:i*size+size]
        f.write("%d %d %d\n"%(e[0x12]|(e[0x13]<<8), e[0x14]|(e[0x15]<<8), e[0x16]|(e[0x17]<<8)))

# growth.txt  (EXP growth byte @ 0x0F)
with open(os.path.join(OUTD,"growth.txt"),"w") as f:
    f.write("0\n")
    for i in range(1,n):
        f.write("%d\n" % data[i*size+0x0F])

# learnsets.bin  (copy of lvlmove_sv.pkl)
shutil.copy(levelup, os.path.join(OUTD,"learnsets.bin"))

# pp.txt  (max PP per move, index = move ID) from MoveInfo9.cs
text = open(moveinfo, encoding="utf-8").read()
m = re.search(r'public static ReadOnlySpan<byte> PP =>\s*\[(.*?)\];', text, re.DOTALL)
nums = re.findall(r'\b(\d+)\b', m.group(1))
with open(os.path.join(OUTD,"pp.txt"),"w") as f:
    f.write(" ".join(nums))

# NEW: moves.txt (move names, index = move ID)
# Read with utf-8 to handle the weird dashes correctly
moves = open(moves_txt, encoding="utf-8").read().splitlines()
with open(os.path.join(OUTD,"moves.txt"),"w", encoding="utf-8") as f:
    # Replace the weird em-dash at index 0 with "---" for a cleaner UI
    for i, name in enumerate(moves):
        if i == 0 and not name.isalnum():
            f.write("---\n")
        else:
            f.write(name + "\n")

print(f"Wrote stats, abilities, growth, learnsets, pp, and moves ({len(moves)} total) to", OUTD)
