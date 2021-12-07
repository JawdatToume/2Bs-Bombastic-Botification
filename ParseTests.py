import os

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

    def __str__(self):
        return self.test + ": " + self.result + ", " + self.time

tests = os.listdir("tests\\")

results = []

for test in tests:
    res = Result(test)
    results.append(res)
