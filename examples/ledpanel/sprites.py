#!/usr/bin/env python3

for idx in range(3):
    with open("sprite%d.ppm" % idx) as f:
        lines = f.readlines() 
        lines = [int(s) for s in lines[4:]]
        assert len(lines) == 3*1024

        print("uint32_t sprite%d[1024] = { " % idx, end="")
        for i in range(1024):
            print("%s0x%02x%02x%02x" % (", " if i > 0 else "", lines[3*i+0], lines[3*i+1], lines[3*i+2]), end="")
        print("};");
