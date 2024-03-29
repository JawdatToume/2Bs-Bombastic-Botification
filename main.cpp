#include <iostream>
#include "sc2api/sc2_api.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"

#include "BasicSc2Bot.h"
#include "LadderInterface.h"
#include "Node.h"

// LadderInterface allows the bot to be tested against the built-in AI or
// played against other bots
int main(int argc, char* argv[]) {
	BasicSc2Bot* bot = new BasicSc2Bot();
	RunBot(argc, argv, bot, sc2::Race::Zerg);

    cout << "Game ended" << endl;
    if (Node::base_count == 0) {
        cout << "Team 2B lost :(" << endl;
    } else {
        cout << "Team 2B won!" << endl;
    }

    std::cout << "Elapsed Time: " << Node::elapsedTime << std::endl;

	//std::cout << "Ran off the end" << std::endl;
	return 0;
}