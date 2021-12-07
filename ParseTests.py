import os
import matplotlib.pyplot as plt

difficulties = ["Easy", "Medium", "Hard"]
races = ["zerg", "protoss", "terran"]
maps = ["CactusValleyLE.SC2Map", "BelShirVestigeLE.SC2Map", "ProximaStationLE.SC2Map"]

class Result:
    def __init__(self, test):
        self.test = test
        params = test.split("-")
        self.diff = params[0]
        self.race = params[1]
        self.map = params[2]
        self.iter = params[3]

        file = open("tests\\" + test)
        lines = file.readlines()

        self.time = lines[-1].replace("\n", "")
        self.result = lines[-2].replace("\n", "")

    def isWin(self):
        return self.result == "Team2B lost :("

    def __str__(self):
        return self.test + ": " + self.result + ", " + self.time

tests = os.listdir("tests\\")

results = []

for test in tests:
    res = Result(test)
    results.append(res)

for res in results:
    print(res)

wins = []
losses = []
for res in results:
    if res.isWin():
        wins.append(res)
    else:
        losses.append(res)

print(wins)

#Plot wins vs losses per difficulty
y_axis = [0, 0, 0]
for res in wins:
    if res.diff == "Easy":
        y_axis[0] += 1
    if res.diff == "Medium":
        y_axis[1] += 1
    if res.diff == "Hard":
        y_axis[2] += 1

fig = plt.subplots()
x_axis = difficulties
plt.bar(x_axis,y_axis)
plt.show()
