#ifndef BASIC_SC2_BOT_H_
#define BASIC_SC2_BOT_H_

#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"

#include <vector>

using namespace std;
using namespace sc2;

class Node;

class BasicSc2Bot : public sc2::Agent {
public:
	std::vector<Node*> nodes;

	BasicSc2Bot();
	~BasicSc2Bot();

	void OnGameStart();
	void OnStep();
	void OnUnitIdle(const sc2::Unit* unit);
	void OnUnitDestroyed(const sc2::Unit* unit);
	void OnUnitCreated(const sc2::Unit* unit);

private:
	Node* GetOwningNode(const sc2::Unit* u);
	void updateNode(Node* n);
	void addUnitToClosestNode(const sc2::Unit* unit);

	void focusDefense();
	Node* focus;
	bool firstNode = true;
};

#include "Node.h"

#endif