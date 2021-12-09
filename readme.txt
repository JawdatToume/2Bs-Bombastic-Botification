///////////////////////////////////////////////////////
CMPUT 350 - Starcraft 2 Bot Project
Team 2B
Ava Guo, Alexander Salm, Ronan Sandoval, Jawdat Toume
///////////////////////////////////////////////////////

How to compile and build the bot:

1. To properly set up paths, you will need to rebuild the project using CMAKE:
   https://blizzard.github.io/s2client-api/md_docs_building.html

2. Import the project into visual studio and compile. Ensure that Node.cpp and Node.h are added to the project.

3. After building the project, go the the bin folder and use the command:
   .\BasicSc2Bot.exe -c -a [enemy race] -d [enemy difficulty] -m [map] 
    Example command: ./BasicSc2Bot.exe -c -a zerg -d Hard -m CactusValleyLE.SC2Map