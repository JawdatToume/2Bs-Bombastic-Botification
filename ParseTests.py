import os
import matplotlib.pyplot as plt

difficulties = ["Easy", "Medium", "Hard"]
races = ["zerg", "protoss", "terran"]
maps = ["CactusValleyLE.SC2Map", "BelShirVestigeLE.SC2Map", "ProximaStationLE.SC2Map"]

class Result:
    def __init__(self, test, root):
        self.test = test
        params = test.split("-")
        self.diff = params[0]
        self.race = params[1]
        self.map = params[2]
        self.iter = params[3]

        file = open(root + test)
        lines = file.readlines()

        self.time = lines[-1].replace("\n", "")
        self.real_time = float(self.time[14:])
        self.result = lines[-2].replace("\n", "")

    def isWin(self):
        return self.result != "Team2B lost :("

    def __str__(self):
        return self.test + ": " + self.result + ", " + self.time

tests = os.listdir("tests\\")

results = []

#add alikay tests
for test in tests:
    res = Result(test, "tests\\")
    results.append(res)

tests = os.listdir("JawdatActualTests\\")

#add jawdat tests
for test in tests:
    res = Result(test, "JawdatActualTests\\")
    results.append(res)

for res in results:
    print(res)

#compute wins and losses
wins = []
losses = []
for res in results:
    if res.isWin():
        wins.append(res)
    else:
        losses.append(res)

#Plot wins vs losses per difficulty
y_axis = [0, 0, 0]
for res in wins:
    y_axis[difficulties.index(res.diff)] += 1

print(y_axis)

fig, ax = plt.subplots()
x_axis = difficulties
ax.bar(x_axis,y_axis)
ax.set_ylabel("Wins")
ax.set_title("Wins Per Difficulty (Max = 63)")
plt.show()

#Plot wins by race
y_axis = [0, 0, 0]
for res in wins:
    y_axis[races.index(res.race)] += 1

print(y_axis)

fig, ax = plt.subplots()
x_axis = races
ax.bar(x_axis,y_axis)
ax.set_ylabel("Wins")
ax.set_title("Wins Per Race (Max = 63)")
plt.show()

#Plot wins vs map
y_axis = [0, 0, 0]
for res in wins:
    y_axis[maps.index(res.map)] += 1

print(y_axis)

fig, ax = plt.subplots()
x_axis = maps
ax.bar(x_axis,y_axis)
ax.set_ylabel("Wins")
ax.set_title("Wins Per Map (Max = 63)")
plt.show()

#Average times
avg = [0, 0, 0]
nums = [0, 0, 0]
for win in wins:
    avg[difficulties.index(win.diff)] += win.real_time
    nums[difficulties.index(win.diff)] += 1

average = (avg[0] + avg[1] + avg[2]) / (nums[0] + nums[1] + nums[2])
print("AVG Time per win:", average)
print("    AVG Time per Easy   win", avg[0]/nums[0])
print("    AVG Time per Medium win", avg[1]/nums[1])
print("    AVG Time per Hard   win", avg[2]/nums[2])

avg = [0, 0, 0]
nums = [0, 0, 0]
for lose in losses:
    avg[difficulties.index(lose.diff)] += lose.real_time
    nums[difficulties.index(lose.diff)] += 1

average = (avg[0] + avg[1] + avg[2]) / (nums[0] + nums[1] + nums[2])
print("AVG Time per lose:", average)
print("    AVG Time per Easy   lose", avg[0]/nums[0])
print("    AVG Time per Medium lose", avg[1]/nums[1])
print("    AVG Time per Hard   lose", avg[2]/nums[2])