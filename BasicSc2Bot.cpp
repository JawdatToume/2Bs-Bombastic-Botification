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

}

// For debugging purposes
void BasicSc2Bot::PrintInfo() {
    cout << "Info:" << endl;
    cout << "Larva Count: " << larva_count << endl;
    cout << "Minerals: " << minerals << endl << endl;
}


// UNIT SPAWNING AND BUILDING ////////////////////////////////////////////////////////////

// From bot_examples.cc 
bool BasicSc2Bot::TryBuild(AbilityID ability_type_for_structure, UnitTypeID unit_type) {
    float rx = GetRandomScalar();
    float ry = GetRandomScalar();
    const ObservationInterface* observation = Observation();
    Point2D build_location = Point2D(start_location.x + rx * 15, start_location.y + ry * 15);

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

        // Check to see if unit can build there
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

// Get the queens to inject Larva when they are able to, decides whether a queen spreads creep tumors or injects larva at a hatchery
void BasicSc2Bot::InjectLarva(const Unit* unit, int num) {
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
    Actions()->UnitCommand(unit, ABILITY_ID::EFFECT_INJECTLARVA);
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
                InjectLarva(unit, queens);
                queens++;
            }
            default: {
                break;
            }
           
        }
    }

    // building
    if (spawning_pool_count < 1 && minerals >= 200) {
        TryBuild(ABILITY_ID::BUILD_SPAWNINGPOOL, UNIT_TYPEID::ZERG_DRONE);
    }

    return;
}

void BasicSc2Bot::OnUnitIdle(const Unit *unit) {
    return;
}



