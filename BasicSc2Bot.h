#ifndef BASIC_SC2_BOT_H_
#define BASIC_SC2_BOT_H_

#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"

class BasicSc2Bot : public sc2::Agent {
public:
	virtual void OnGameStart();
	virtual void OnStep();
	virtual void OnUnitIdle(const sc2::Unit* unit) final;
	virtual void OnUnitDestroyed(const sc2::Unit* unit) final;

private:
	void ObtainInfo();
	void PrintInfo();

	sc2::Tag FindClosestGeyser(sc2::Point2D base_location);  // find closest geyser to a base
	bool TryBuild(sc2::AbilityID ability_type_for_structure, sc2::UnitTypeID unit_type, sc2::Tag location_tag = 0);
	void MorphLarva(const sc2::Unit* unit);
	void GenerateCreep(const sc2::Unit* unit);
	void BuildExtractor();
	void ManageWorkers(sc2::UNIT_TYPEID worker_type, sc2::AbilityID worker_gather_command, sc2::UNIT_TYPEID vespene_building_type);
	void MineIdleWorkers(const sc2::Unit* worker, sc2::AbilityID worker_gather_command, sc2::UnitTypeID vespene_building_type);
	const sc2::Unit* FindNearestMineralPatch(const sc2::Point2D& start);
	bool TryBuildExpansionHatch();
	void QueenAction(const sc2::Unit* unit, int num);
	void Hatch(const sc2::Unit* unit);

	int food_cap, food_used;
	int minerals = 0, vespene = 0;

	sc2::Point2D start_location;
	std::vector<sc2::Point3D> expansions;

	int larva_count,
		drone_count,
		spine_crawler_count,
		spawning_pool_count,
		lair_count,
		base_count,
		hatchery_count,
		queen_count;

	sc2::Tag zergling_sent = NULL;
	int checked_spawn = 0;

	double defensive_overlord_scatter_distance = 10.0;
	    
};

#endif