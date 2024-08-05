import re

recordPath = r"./record-07-15(VmSize).txt"


class Case:
    def __init__(self) -> None:
        self.attr = {}


class Stat:
    def __init__(self) -> None:
        self.cases = []


stat = Stat()


def readFile(path: str) -> str:
    content = ""
    with open(path, "r") as file:
        content = file.read()
        # print(content)
    return content


def findPattern(content: str, pattern: str, key: str) -> None:
    result = re.findall(pattern, content)
    if len(result) > len(stat.cases):
        for i in range(0, len(result) - len(stat.cases)):
            stat.cases.append(Case())

    for i in range(0, len(result)):
        stat.cases[i].attr[key] = result[i]
    return


def parseContent(content: str, prefix: str = ""):
    # find = re.findall("VmSize\[START\]: (\d+) kB",content)
    # print(find)

    findPattern(content, "VmSize\[START\]: (\d+) kB", prefix + "VmStart")
    findPattern(content, "VmSize\[AST\]: (\d+) kB", prefix + "VmAst")
    findPattern(content, "VmSize\[CPN\]: (\d+) kB", prefix + "VmCpn")
    findPattern(content, "VmSize\[STATE\]: (\d+) kB", prefix + "VmState")
    findPattern(content, "VmSize\[END\]: (\d+) kB", prefix + "VmEnd")

    findPattern(content, "iniFile/(\w+)\.ini", "CaseName")
    return


def analysisStat() -> None:
    for i in range(0, len(stat.cases)):
        stat.cases[i].attr["MS-StateUse"] = \
            int(stat.cases[i].attr["MS-VmState"]) - int(stat.cases[i].attr["MS-VmCpn"])
        stat.cases[i].attr["AS-StateUse"] = \
            int(stat.cases[i].attr["AS-VmState"]) - int(stat.cases[i].attr["AS-VmCpn"])
        
        stat.cases[i].attr["VmPercentageState"] = stat.cases[i].attr["MS-StateUse"]/stat.cases[i].attr["AS-StateUse"]
        stat.cases[i].attr["VmPercentageAll"] = float(stat.cases[i].attr["MS-VmEnd"])/float(stat.cases[i].attr["AS-VmEnd"])
        
    return


if __name__ == "__main__":
    content = readFile(recordPath)
    content2 = readFile(r"./record-07-16(VmSizeUnMerge).txt")

    parseContent(content, "MS-")
    parseContent(content2, "AS-")

    analysisStat()

    if True:
        for i in range(0, len(stat.cases)):
            print(stat.cases[i].attr["CaseName"]+":")

            # print("\tMS-StateUse: "+str(stat.cases[i].attr["MS-StateUse"]))
            # print("\tAS-StateUse: "+str(stat.cases[i].attr["AS-StateUse"]))
            # print("\tPercentage of Memory Use(State): "+str(stat.cases[i].attr["VmPercentageState"]))
            # print("\tPercentage of Memory Use(All): "+str(stat.cases[i].attr["VmPercentageAll"]))
            print("\tVmSize: "+str(stat.cases[i].attr["MS-StateUse"]))
        
    for i in range(0, len(stat.cases)):
        # print(str(round(stat.cases[i].attr["AS-StateUse"], 4))+"--", end="")
        # print(str(round(stat.cases[i].attr["MS-StateUse"], 4)))
        print(str(round(stat.cases[i].attr["MS-StateUse"], 4)))
    
