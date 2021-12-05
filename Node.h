#ifndef BASIC_SC_2_BOT_NODE_H
#define BASIC_SC_2_BOT_NODE_H

#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"

#include <vector>

class Node{
public:
	Node();
	~Node();

	std::vector<sc2::Tag> nodeUnits;

	void OnGameStart();
	void OnStep();
	void OnUnitIdle(const sc2::Unit* unit);
	void OnUnitDestroyed(const sc2::Unit* unit);

private:
	void ObtainInfo();
	void PrintInfo();
	int CountUnits(const sc2::ObservationInterface* observation, sc2::UnitTypeID unit_type);
	//void GetMostDamagedBuilding();
	void GetClosestEnemy();

	sc2::Tag FindClosestGeyser(sc2::Point2D base_location);  // find closest geyser to a base
	bool TryBuild(sc2::AbilityID ability_type_for_structure, 
	              sc2::UnitTypeID unit_type,
				  sc2::Tag location_tag = 0, 
				  sc2::Point3D location_point3d = sc2::Point3D(-1,-1,-1), 
				  bool is_expansion = false);
	void MorphLarva(const sc2::Unit* unit);
	void GenerateCreep(const sc2::Unit* unit);
	void BuildExtractor();
	void ManageWorkers(sc2::UNIT_TYPEID worker_type, sc2::AbilityID worker_gather_command, sc2::UNIT_TYPEID building_type);
	void MineIdleWorkers(const sc2::Unit* worker, sc2::AbilityID worker_gather_command, sc2::UnitTypeID building_type);
	const sc2::Unit* FindNearestMineralPatch(const sc2::Point2D& start);
	bool BuildNewHatchery();
	int GetExpectedWorkers(sc2::UNIT_TYPEID building_type);
	bool TryExpand(sc2::AbilityID build_ability, sc2::UnitTypeID worker_type);
	void QueenAction(const sc2::Unit* unit, int num);
	void Hatch(const sc2::Unit* unit);
	void Ambush();
	void HealUnits(const sc2::Unit* unit);
	//void MoveDefense(sc2::Point2D &point);

	int food_cap, food_used, food_workers;
	int minerals = 0, vespene = 0;

	sc2::Point3D start_location, staging_location;  // on game start
	std::vector<sc2::Point3D> expansions;

	int larva_count,
		drone_count,
		spine_crawler_count,
		spawning_pool_count,
		tumor_count,
		lair_count,
		base_count,
		hatchery_count,
		queen_count,
		zergling_count,
		spore_crawler_count,
		hydralisk_count;

	bool ready_to_expand = false,
		 researched_burrow = false,
		 researched_metabolic = false;

	sc2::Tag zergling_sent = NULL;
	int checked_spawn = 0;

	int timer = 0;

	double defensive_overlord_scatter_distance = 10.0;

	const sc2::Unit *defense_focus; 

	sc2::Point2D enemy_location = start_location;

//Node meta functions
public:
	bool unitBelongs(const sc2::Unit* unit);
	void addUnit(const sc2::Unit* unit);
	void addUnit(sc2::Tag tag);

	void moveDefense();

	const sc2::ObservationInterface* Observation();
	sc2::QueryInterface* Query();
	sc2::ActionInterface* Actions();
	sc2::Point3D getBasePosition();
	
	float ratio;

	const sc2::ObservationInterface* observation;
	sc2::QueryInterface* query;
	sc2::ActionInterface* actions;
};

#endif