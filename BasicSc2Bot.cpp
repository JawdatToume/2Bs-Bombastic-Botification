#include "BasicSc2Bot.h"
#include "sc2api/sc2_unit_filters.h"
#include <iostream>

#include "Node.h"

using namespace sc2;
using namespace std;

//Constructor
BasicSc2Bot::BasicSc2Bot() {
	std::cout << "Spawned" << std::endl;
	Node* n = new Node;
	focus = n;
	nodes.push_back(n);
}

//Destructor
BasicSc2Bot::~BasicSc2Bot() {
	cout << "Bot Created..." << endl;
	for (int i = 0; i < nodes.size(); i++) {
		delete nodes[i];
	}
}

//Called on start
void BasicSc2Bot::OnGameStart() {
	cout << "Start Game!" << endl;
	for (int i = 0; i < nodes.size(); i++) {
		updateNode(nodes[i]);
		nodes[i]->OnGameStart();
	}

	Units bases = Observation()->GetUnits(Unit::Alliance::Self, IsTownHall());
	nodes[0]->addUnit(bases[0]->tag);
}

//Called each frame
void BasicSc2Bot::OnStep() {
	for (int i = 0; i < nodes.size(); i++) {
		updateNode(nodes[i]);
		nodes[i]->OnStep();
	}
	focusDefense();
}

//Called per each unit without a job
void BasicSc2Bot::OnUnitIdle(const sc2::Unit* unit) {
	bool belongs = false;
	for (int i = 0; i < nodes.size(); i++) {
		if (nodes[i]->unitBelongs(unit)) {
			belongs = true;
			nodes[i]->OnUnitIdle(unit);
		}
	}
	if (!belongs) {
		addUnitToClosestNode(unit);
	}
}

//Called when a unit dies
void BasicSc2Bot::OnUnitDestroyed(const sc2::Unit* unit) {
	for (int i = 0; i < nodes.size(); i++) {
		if (nodes[i]->unitBelongs(unit)) nodes[i]->OnUnitDestroyed(unit);
	}
}

//Gets the node this unit belongs to
Node* BasicSc2Bot::GetOwningNode(const sc2::Unit* u) {
	Node* n = NULL;
	for (int i = 0; i < nodes.size(); i++) {
		if (nodes[i]->unitBelongs(u)) {
			n = nodes[i];
			break;
		}
	}
	return n;
}

// Creates new node if new unit is a hatchery and moves defense to it,
// else assigns unit to closest node.
void BasicSc2Bot::OnUnitCreated(const sc2::Unit* unit) {
	std::cout << "Unit Created" << std::endl;
	if (unit->alliance == Unit::Alliance::Self) {
		if (unit->unit_type.ToType() == UNIT_TYPEID::ZERG_HATCHERY) {
			if (firstNode) {
				firstNode = false;
				return;
			}
			Node* n = new Node();
			n->addUnit(unit);
			updateNode(n);
			nodes.push_back(n);
			n->moveDefense();

		}
		else {
			addUnitToClosestNode(unit);
		}
	}
}

// Updates Starcraft data for all nodes 
void BasicSc2Bot::updateNode(Node* n) {
	n->actions = Actions();
	n->observation = Observation();
	n->query = Query();
}

// Unit is added to the list of closest node
void BasicSc2Bot::addUnitToClosestNode(const sc2::Unit* unit) {
	Node* closest = NULL;
	float mindistance = INFINITY;
	sc2::Point3D unitPos = unit->pos;
	//cout << "adding unit to closest node..." << endl;
	for (int i = 0; i < nodes.size(); i++) {
		float distance = DistanceSquared2D(nodes[i]->getBasePosition(), unitPos);
		if (distance < mindistance) {
			mindistance = distance;
			closest = nodes[i];
		}
	}
	closest->addUnit(unit);
}

// Finds weakest base and moves defense to its location
void BasicSc2Bot::focusDefense() {
	float lowest_health = 1.0;
	int lowest_index = 0;
	bool any_damaged = false;
	for (int i = 0; i < nodes.size(); i++) {
		if (nodes[i]->ratio < lowest_health) {
			lowest_index = i;
			lowest_health = nodes[i]->ratio;
			any_damaged = true;
		}
	}

	if (any_damaged && nodes[lowest_index] != focus) {
		focus = nodes[lowest_index];
		focus->moveDefense();
	}
}
