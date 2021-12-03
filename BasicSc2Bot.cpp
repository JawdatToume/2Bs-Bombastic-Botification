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
		nodes[i]->OnStart();
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
}

//Called per each unit without a job
void BasicSc2Bot::OnUnitIdle(const sc2::Unit* unit) {
	for (int i = 0; i < nodes.size(); i++) {
		if (nodes[i]->unitBelongs(unit)) nodes[i]->OnUnitWait(unit);
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


void BasicSc2Bot::OnUnitCreated(const sc2::Unit* unit) {
	std::cout << "Unit Created" << std::endl;
	if (unit->alliance == Unit::Alliance::Self) {
		if (unit->unit_type.ToType() == UNIT_TYPEID::ZERG_HATCHERY) {
			Node* n = new Node();
			n->addUnit(unit);
			nodes.push_back(n);
		}
		else {
			addUnitToClosestNode(unit);
		}
	}
}

void BasicSc2Bot::updateNode(Node* n) {
	n->actions = Actions();
	n->observation = Observation();
	n->query = Query();
}

void BasicSc2Bot::addUnitToClosestNode(const sc2::Unit* unit) {
	Node* closest = NULL;
	float mindistance = INFINITY;
	sc2::Point3D unitPos = unit->pos;
	for (int i = 0; i < nodes.size(); i++) {
		float distance = DistanceSquared2D(nodes[i]->getBasePosition(), unitPos);
		if (distance < mindistance) {
			mindistance = distance;
			closest = nodes[i];
		}
	}
	closest->addUnit(unit);
}
