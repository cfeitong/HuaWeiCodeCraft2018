import subprocess as sp
from math import sqrt
from datetime import datetime
from collections import defaultdict

CPU = {
    "flavor1": 1,
    "flavor2": 1,
    "flavor3": 1,
    "flavor4": 2,
    "flavor5": 2,
    "flavor6": 2,
    "flavor7": 4,
    "flavor8": 4,
    "flavor9": 4,
    "flavor10": 8,
    "flavor11": 8,
    "flavor12": 8,
    "flavor13": 16,
    "flavor14": 16,
    "flavor15": 16,
}

MEM = {
    "flavor1": 1,
    "flavor2": 2,
    "flavor3": 4,
    "flavor4": 2,
    "flavor5": 4,
    "flavor6": 8,
    "flavor7": 4,
    "flavor8": 8,
    "flavor9": 16,
    "flavor10": 8,
    "flavor11": 16,
    "flavor12": 32,
    "flavor13": 16,
    "flavor14": 32,
    "flavor15": 64,
}


def main():
    tot_acc = 0
    for i in range(1, 15):
        datacase = "data/test/Datacase{}.txt".format(i)
        inputcase = "data/test/input{}.txt".format(i)
        testcase = "data/test/Testcase{}.txt".format(i)
        sp.run(["./ecs/bin/ecs", datacase, inputcase, "out.txt"])

        res, phy = parse_output("out.txt")
        cpu_lim, mem_lim = parse_input(inputcase)
        vir = parse_test(testcase)
        acc = scoring1(res, vir)
        tot_acc += acc
        cpu_score, mem_score = scoring2(phy, cpu_lim, mem_lim)
        print("case: {:2} acc: {:.3f}, cpu: {:.3f}, mem: {:.3f}".format(i, acc, cpu_score, mem_score))

    print("total acc: {:.2}".format(tot_acc))


def scoring2(phy, cpu_lim, mem_lim):
    cpu_total = cpu_lim * len(phy)
    mem_total = mem_lim * len(phy)
    cpu_count, mem_count = 0, 0

    for i, machine in phy.items():
        for flavor, count in machine.items():
            try:
                cpu_count += CPU[flavor] * count
                mem_count += MEM[flavor] * count
            except KeyError:
                continue

    try:
        cpu_score = cpu_count / cpu_total
    except ZeroDivisionError:
        cpu_score = 0
    try:
        mem_score = mem_count / mem_total
    except ZeroDivisionError:
        mem_score= 0

    return cpu_score, mem_score


def scoring1(res, vir):
    keys = set(res.keys()).union(vir.keys())
    N = len(keys)

    y1y2, y1y1, y2y2 = 0, 0, 0
    for key in keys:
        try:
            y1y2 += (res[key] - vir[key]) ** 2 / N
            y1y1 += res[key] ** 2 / N
            y2y2 += vir[key] ** 2 / N
        except KeyError:
            continue

    score1 = (1 - sqrt(y1y2) / (sqrt(y1y1) + sqrt(y2y2)))
    return score1


def parse_test(path):
    with open(path) as f:
        lines = f.readlines()
    vir = defaultdict(int)
    for line in lines:
        l = line.split()
        vir[l[1]] += 1
    return vir


def parse_input(path):
    with open(path) as f:
        line = f.readline()
    l = line.split()
    return int(l[0]), int(l[1])


def parse_output(path):
    with open(path) as f:
        lines = f.readlines()
    brk = False
    pred_res = []
    phy_res = []
    phy = {}
    resource = {}
    for line in lines:
        line = line.strip()
        if brk:
            if line != "":
                phy_res.append(line)
        else:
            if line != "":
                pred_res.append(line)
        if line == "":
            brk = True

    pred_res = pred_res[1:]
    phy_res = phy_res[1:]

    for pred in pred_res:
        p = pred.split()
        resource[p[0]] = int(p[1])

    for phy_line in phy_res:
        p = phy_line.split()
        phy_id = int(p[0])
        phy[phy_id] = defaultdict(int)
        for j in range(1, len(p), 2):
            phy[phy_id][p[j]] += int(p[j + 1])

    return resource, phy


def parse_time(line):
    t = line.split()
    y, m, d = t[0].split("-")
    y, m, d = int(y), int(m), int(d)
    h, mi, s = t[1].split(":")
    h, mi, s = int(h), int(mi), int(s)
    time = datetime(y, m, d, h, mi, s)
    return time


if __name__ == "__main__":
    main()
