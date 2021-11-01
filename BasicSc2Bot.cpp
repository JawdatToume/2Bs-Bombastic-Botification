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
    larva_count = CountUnits(obs, UNIT_TYPEID::ZERG_LARVA);
    spawning_pool_count = CountUnits(obs, UNIT_TYPEID::ZERG_SPAWNINGPOOL);
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

    cout << "ability_type_for_structure is Extractor: " << (ability_type_for_structure == ABILITY_ID::BUILD_EXTRACTOR) << endl;  /// 1 occurs

    // location is set to closest to a base
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

        // Check to see if unit can make it there
        if (Query()->PathingDistance(unit, build_location) < 0.1f) {
            return false;
        }

        for (const auto& expansion : expansions) {
            if (Distance2D(build_location, Point2D(expansion.x, expansion.y)) < 7) {
                return false;
            }
        }

        /*if (ability_type_for_structure == ABILITY_ID::BUILD_EXTRACTOR) {
            const Unit* target = observation->GetUnit(location_tag);

            if (Query()->Placement(ability_type_for_structure, target->pos)) {
                Actions()->UnitCommand(unit, ability_type_for_structure, target);
                return true;
            }
        } else {
            // Check to see if unit can build there
            if (Query()->Placement(ability_type_for_structure, build_location)) {
                cout << "Building a " << ability_type_for_structure.to_string() << endl;
                Actions()->UnitCommand(unit, ability_type_for_structure, build_location);
                return true;
            }
        }*/

        // Check to see if unit can build there
        if (ability_type_for_structure == ABILITY_ID::BUILD_EXTRACTOR) {
            cout << "Query()->Placement(ability_type_for_structure, build_location: " << (Query()->Placement(ability_type_for_structure, build_location)) << endl;  /// 1
        }
        if (Query()->Placement(ability_type_for_structure, build_location)) {
            cout << "Building a " << ability_type_for_structure.to_string() << endl;
            Actions()->UnitCommand(unit, ability_type_for_structure, build_location);
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
    } else if (minerals >= 50 && food_cap - food_used > 0 ){

        cout << "Morphing into Drone" << endl;
        Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_DRONE);
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

// GAME START AND STEP ///////////////////////////////////////////////////////////////////

void BasicSc2Bot::OnGameStart() { 
    start_location = Observation()->GetStartLocation();
    expansions = search::CalculateExpansionLocations(Observation(), Query());
    return;
}

// per frame...
void BasicSc2Bot::OnStep() { 
    ObtainInfo();

    // looking through all unit types
    Units units = Observation()->GetUnits(Unit::Alliance::Self);
    for (const auto& unit : units) {

        switch (unit->unit_type.ToType()) {
            case UNIT_TYPEID::ZERG_LARVA: {
                 MorphLarva(unit);
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
    
    bool need_extractor = CountUnits(Observation(), UNIT_TYPEID::ZERG_EXTRACTOR) < Observation()->GetUnits(Unit::Alliance::Self, IsTownHall()).size() * 2;
    // if all bases are destroyed or there's no base, don't build
    if (base_count > 0 && minerals >= 25 && need_extractor) {
        BuildExtractor();
    }

    return;
}

void BasicSc2Bot::OnUnitIdle(const Unit *unit) {
    return;
}



