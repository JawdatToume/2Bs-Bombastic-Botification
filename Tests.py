import os

difficulties = ["Easy", "Medium", "Hard"]
races = ["zerg", "protoss", "terran"]
maps = ["CactusValleyLE.SC2Map", "BelShirVestigeLE.SC2Map", "ProximaStationLE.SC2Map"]

def getFileName(race, diff, map, iter):
    return "build\\" + diff + "-" + race + "-" + map + "-" + str(iter) + ".txt"

iter = 0
while True:
    iter += 1
    for diff in difficulties:
        for race in races:
            for map in maps:
                command = ".\build\bin\BasicSc2Bot.exe -c -a " + race + " -d " + diff + " -m " + map + " > " + getFileName(race, diff, map, iter)
                os.system(command)