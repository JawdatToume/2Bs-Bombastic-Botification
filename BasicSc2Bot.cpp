#include "BasicSc2Bot.h"
#include "sc2api/sc2_unit_filters.h"

using namespace sc2;
using namespace std;

// INFO COLLECTION ///////////////////////////////////////////////////////////////////

// From bot_examples.cc 
int CountUnits(const ObservationInterface* observation, UnitTypeID unit_type) {
    int count = 0;
    Units my_units = observation->GetUnits(Unit::Alliance::Self);
    for (const auto unit : my_units) {
        if (unit->unit_type == unit_type)
            ++count;
    }

    return count;
}

// Collects info from observation. Stores into class members
void BasicSc2Bot::ObtainInfo() {
    const ObservationInterface* obs = Observation();
    spawning_pool_count = CountUnits(obs, UNIT_TYPEID::ZERG_SPAWNINGPOOL);
    hatchery_count = CountUnits(obs, UNIT_TYPEID::ZERG_HATCHERY) + CountUnits(obs, UNIT_TYPEID::ZERG_LAIR);
    extractor_count = CountUnits(obs, UNIT_TYPEID::ZERG_EXTRACTOR);
    larva_count = CountUnits(obs, UNIT_TYPEID::ZERG_LARVA);
    queen_count = CountUnits(obs, UNIT_TYPEID::ZERG_QUEEN);
    drone_count = CountUnits(obs, UNIT_TYPEID::ZERG_DRONE);
    food_cap = obs->GetFoodCap();
    food_used = obs->GetFoodUsed();
    minerals = obs->GetMinerals();
    vespene = obs->GetVespene();
    lair_count = CountUnits(obs, UNIT_TYPEID::ZERG_LAIR);
    base_count = obs->GetUnits(Unit::Alliance::Self, IsTownHall()).size();
}

// For debugging purposes
void BasicSc2Bot::PrintInfo() {
    cout << "Info:" << endl;
    cout << "Larva Count: " << larva_count << endl;
    cout << "Minerals: " << minerals << endl;
    cout << "Vespene: " << vespene << endl << endl;
}

// from bot_example.cc
Tag BasicSc2Bot::FindClosestGeyser(Point2D base_location) {
    Units geysers = Observation()->GetUnits(Unit::Alliance::Neutral, IsUnit(UNIT_TYPEID::NEUTRAL_VESPENEGEYSER));
    // only search for closest geyser within this radius
    float minimum_distance = 15.0f;
    Tag closestGeyser = 0;
    
    for (const auto& geyser : geysers) {
        float current_distance = Distance2D(base_location, geyser->pos);
        if (current_distance < minimum_distance) {
            if (Query()->Placement(ABILITY_ID::BUILD_EXTRACTOR, geyser->pos)) {
                minimum_distance = current_distance;
                closestGeyser = geyser->tag;
            }
        }
    }
    return closestGeyser;
}

// UNIT SPAWNING AND BUILDING ////////////////////////////////////////////////////////////

// From bot_examples.cc
bool BasicSc2Bot::TryBuild(AbilityID ability_type_for_structure, UnitTypeID unit_type, Tag location_tag) {
    float rx = GetRandomScalar();
    float ry = GetRandomScalar();
    const ObservationInterface* observation = Observation();
   
    // default: pick a random location
    Point2D build_location = Point2D(start_location.x + rx * 15, start_location.y + ry * 15);

    // location is set to a geyser location that is closest to a base
    if (ability_type_for_structure == ABILITY_ID::BUILD_EXTRACTOR) {
        build_location = (observation->GetUnit(location_tag))->pos;
    }
    
    if (observation->HasCreep(build_location)) {

        const ObservationInterface* observation = Observation();

        Units workers = observation->GetUnits(Unit::Alliance::Self, IsUnit(unit_type));

        //if we have no workers Don't build
        if (workers.empty()) {
            return false;
        }

        // Check to see if there is already a worker heading out to build it
        for (const auto& worker : workers) {
            for (const auto& order : worker->orders) {
                if (order.ability_id == ability_type_for_structure) {
                    return false;
                }
            }
        }

        // If no worker is already building one, get a random worker to build one
        const Unit* unit = GetRandomEntry(workers);

        // skipping distance checking for building extractor because the chosen target location is already closest to a base
        if (ability_type_for_structure != ABILITY_ID::BUILD_EXTRACTOR) {
            // Check to see if unit can make it there
            if (Query()->PathingDistance(unit, build_location) < 0.1f) {
            return false;
            }
        }

        for (const auto& expansion : expansions) {
            if (Distance2D(build_location, Point2D(expansion.x, expansion.y)) < 7) {
                return false;
            }
        }

        // Check to see if unit can build there
        if (Query()->Placement(ability_type_for_structure, build_location)) {
            cout << "Building a " << ability_type_for_structure.to_string() << endl;
            if (ability_type_for_structure == ABILITY_ID::BUILD_EXTRACTOR) {
                // must pass in Unit type, Point2D does not work for building extractor
                Actions()->UnitCommand(unit, ability_type_for_structure, observation->GetUnit(location_tag));
                cout << "Finished building an extractor" << endl;
            } else {
                Actions()->UnitCommand(unit, ability_type_for_structure, build_location);
            }
            return true;
        }
        return false;
    }
    return false;
}

// choose what unit type to transform larva into
void BasicSc2Bot::MorphLarva(const Unit *unit) {
    // Become overlord if maxed food cap
    // TODO: Multiple spawned at once. Make only spawn one?
    if (food_used == food_cap && minerals >= 100) {

        cout << "Morphing into Overlord" << endl;
        Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_OVERLORD);
    }
    else if (minerals >= 50 && food_cap - food_used > 0){

        cout << "Morphing into Drone" << endl;
        Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_DRONE);
    }
}

// Get the queens to inject Larva when they are able to, decides whether a queen spreads creep tumors or injects larva at a hatchery
void BasicSc2Bot::QueenAction(const Unit* unit, int num) {
    Units hatcheries = Observation()->GetUnits(Unit::Alliance::Self, IsTownHall());
    // decide which hatchery we're building at, cycles between queens
    int mode = num % (hatcheries.size()+1);
    for (size_t i = 0; i < hatcheries.size(); i++) {
        if (mode == i) {
            // if hatchery is not completely built yet
            if (hatcheries.at(i)->build_progress != 1) {
                mode++;
            }
            // prevents impossible requests
            else if(unit->energy >= 25 && unit->orders.empty()){
                Actions()->UnitCommand(unit, ABILITY_ID::EFFECT_INJECTLARVA, hatcheries.at(i));
            }
        }
    }
    if (unit->energy >= 25 && unit->orders.empty()) {
        Actions()->UnitCommand(unit, ABILITY_ID::BUILD_CREEPTUMOR);
    }
}

// Decides hatchery actions (Queen can't evolve from Larva)
void BasicSc2Bot::Hatch(const Unit* unit) {
    // check if we can make queen, have a limit so we can also make drones
    if (minerals >= 150 && spawning_pool_count > 0 && queen_count < hatchery_count) {
        Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_QUEEN);
        cout << "Queen" << endl;
    }
    else {
        cout << "Larva" << endl;
    }
}
}

// make overlord generate creep beneath it
void BasicSc2Bot::GenerateCreep(const Unit *unit) {
    Actions()->UnitCommand(unit, ABILITY_ID::BEHAVIOR_GENERATECREEPON);
}

// build extractor to collect vespene gas
void BasicSc2Bot::BuildExtractor() {
    const ObservationInterface* obs = Observation();
    Units bases = obs->GetUnits(Unit::Alliance::Self, IsTownHall());  // return all command centers of current player

    // build extractor
    for (const auto& base: bases) {
        if (base->assigned_harvesters >= base->ideal_harvesters) {  // if there are more drone than ideal working in a base
            if (base->build_progress == 1) {
                Tag geyser = FindClosestGeyser(base->pos);
                if (geyser != 0) {  // only build if found closest geyser to a base
                    TryBuild(ABILITY_ID::BUILD_EXTRACTOR, UNIT_TYPEID::ZERG_DRONE, geyser);
                }
            }
        }
    }
}

// Borrowed from bot_examples.cc
// To ensure that we do not over or under saturate any base
void BasicSc2Bot::ManageWorkers(UNIT_TYPEID worker_type, AbilityID worker_gather_command, UNIT_TYPEID vespene_building_type) {
    const ObservationInterface* observation = Observation();
    Units bases = observation->GetUnits(Unit::Alliance::Self, IsTownHall());
    Units geysers = observation->GetUnits(Unit::Alliance::Self, IsUnit(vespene_building_type));

    if (bases.empty()) {
        return;
    }

    for (const auto& base : bases) {
        //If we have already mined out or still building here skip the base.
        if (base->ideal_harvesters == 0 || base->build_progress != 1) {
            continue;
        }
        //if base is over populated
        if (base->assigned_harvesters > base->ideal_harvesters) {
            Units workers = observation->GetUnits(Unit::Alliance::Self, IsUnit(worker_type));

            for (const auto& worker : workers) {
                if (!worker->orders.empty()) {  // if worker is busy
                    // if worker's next task is to work at the base
                    if (worker->orders.front().target_unit_tag == base->tag) {
                        //This should allow them to be picked up by mineidleworkers()
                        MineIdleWorkers(worker, worker_gather_command, vespene_building_type);
                        return;
                    }
                }
            }
        }
    }
    // check for over-populated geysers
    Units workers = observation->GetUnits(Unit::Alliance::Self, IsUnit(worker_type));
    for (const auto& geyser : geysers) {
        if (geyser->ideal_harvesters == 0 || geyser->build_progress != 1) {
            continue;
        }
        if (geyser->assigned_harvesters > geyser->ideal_harvesters) {
            for (const auto& worker : workers) {
                if (!worker->orders.empty()) {
                    if (worker->orders.front().target_unit_tag == geyser->tag) {
                        //This should allow them to be picked up by mineidleworkers()
                        MineIdleWorkers(worker, worker_gather_command, vespene_building_type);
                        return;
                    }
                }
            }
        }
        else if (geyser->assigned_harvesters < geyser->ideal_harvesters) {
            for (const auto& worker : workers) {
                if (!worker->orders.empty()) {
                    //This should move a worker that isn't mining gas to gas
                    const Unit* target = observation->GetUnit(worker->orders.front().target_unit_tag);
                    if (target == nullptr) {
                        continue;
                    }
                    if (target->unit_type != vespene_building_type) {
                        //This should allow them to be picked up by mineidleworkers()
                        MineIdleWorkers(worker, worker_gather_command, vespene_building_type);
                        return;
                    }
                }
            }
        }
    }
}

// Borrowed from bot_example.cc
// Mine the nearest mineral to Town hall.
// If we don't do this, probes may mine from other patches if they stray too far from the base after building.
void BasicSc2Bot::MineIdleWorkers(const Unit* worker, AbilityID worker_gather_command, UnitTypeID vespene_building_type) {
    const ObservationInterface* observation = Observation();
    Units bases = observation->GetUnits(Unit::Alliance::Self, IsTownHall());
    Units geysers = observation->GetUnits(Unit::Alliance::Self, IsUnit(vespene_building_type));

    const Unit* valid_mineral_patch = nullptr;

    if (bases.empty()) {
        return;
    }
    
    // assign more workers to geyser if not over-populated
    for (const auto& geyser : geysers) {
        if (geyser->assigned_harvesters < geyser->ideal_harvesters) {
            Actions()->UnitCommand(worker, worker_gather_command, geyser);
            return;
        }
    }
    // Search for a base that is missing workers.
    for (const auto& base : bases) {
        // If we have already mined out here skip the base.
        if (base->ideal_harvesters == 0 || base->build_progress != 1) {
            continue;
        }
        // if missing workers at a base
        if (base->assigned_harvesters < base->ideal_harvesters) {
            valid_mineral_patch = FindNearestMineralPatch(base->pos);
            Actions()->UnitCommand(worker, worker_gather_command, valid_mineral_patch);
            return;
        }
    }

    if (!worker->orders.empty()) {
        return;
    }

    //If all workers spots are filled just go to any base.
    const Unit* random_base = GetRandomEntry(bases);
    valid_mineral_patch = FindNearestMineralPatch(random_base->pos);
    Actions()->UnitCommand(worker, worker_gather_command, valid_mineral_patch);
}

// Borrowed from bot_example.cc
const Unit* BasicSc2Bot::FindNearestMineralPatch(const Point2D& start) {
    Units units = Observation()->GetUnits(Unit::Alliance::Neutral);
    float distance = std::numeric_limits<float>::max();
    const Unit* target = nullptr;
    for (const auto& u : units) {
        if (u->unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD) {
            float d = DistanceSquared2D(u->pos, start);
            if (d < distance) {
                distance = d;
                target = u;
            }
        }
    }
    //If we never found one return false;
    if (distance == std::numeric_limits<float>::max()) {
        return target;
    }
    return target;
}

// GAME START AND STEP ///////////////////////////////////////////////////////////////////

void BasicSc2Bot::OnGameStart() { 
    start_location = Observation()->GetStartLocation();
    expansions = search::CalculateExpansionLocations(Observation(), Query());
    return;
}

// per frame...
void BasicSc2Bot::OnStep() { 
    ObtainInfo();
    int queens = 0;

    // looking through all unit types
    Units units = Observation()->GetUnits(Unit::Alliance::Self);
    for (const auto& unit : units) {

        switch (unit->unit_type.ToType()) {
            case UNIT_TYPEID::ZERG_LARVA: {
                MorphLarva(unit);
            }
            case UNIT_TYPEID::ZERG_QUEEN: {
                QueenAction(unit, queens);
                queens++;
            }
            case UNIT_TYPEID::ZERG_CREEPTUMOR: {
                // builds creep tumor when it can, this is its only available action and can only happen once
                if (unit->energy >= 25 && unit->orders.empty()) {
                    Actions()->UnitCommand(unit, ABILITY_ID::BUILD_CREEPTUMOR);
                }
            }
            case UNIT_TYPEID::ZERG_HATCHERY: {
                Hatch(unit);
                if (minerals >= 150 && vespene >= 100) { // upgrade 

                }
            }
            case UNIT_TYPEID::ZERG_LAIR: {
                Hatch(unit); // No specialization for now
            }
            case UNIT_TYPEID::ZERG_OVERLORD: {
                if (lair_count > 0 /*&& overlord BEHAVIOR_GENERATECREEPOFF == true*/) {  // available once lair built
                    GenerateCreep(unit);
                }
            }
            default: {
                break;
            }
           
        }
    }

    // building spawning pool
    if (spawning_pool_count < 1 && minerals >= 200) {
        TryBuild(ABILITY_ID::BUILD_SPAWNINGPOOL, UNIT_TYPEID::ZERG_DRONE);
        
    }
    
    bool not_enough_extractor = CountUnits(Observation(), UNIT_TYPEID::ZERG_EXTRACTOR) < Observation()->GetUnits(Unit::Alliance::Self, IsTownHall()).size() * 2;
    // if all bases are destroyed or there's no base, don't build
    if (base_count > 0 && minerals >= 25 && not_enough_extractor) {
        BuildExtractor();
    }

    ManageWorkers(UNIT_TYPEID::ZERG_DRONE, ABILITY_ID::HARVEST_GATHER, UNIT_TYPEID::ZERG_EXTRACTOR);

    return;
}

void BasicSc2Bot::OnUnitIdle(const Unit *unit) {
    return;
}




