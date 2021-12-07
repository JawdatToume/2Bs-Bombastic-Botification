#include "Node.h"
#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"
#include "sc2api/sc2_unit_filters.h"
#include <iostream>
#include "Globals.h"

using namespace sc2;
using namespace std;

//Constructor
Node::Node() {
    larva_count = 0;
    drone_count = 0;
    spine_crawler_count = 0;
    spawning_pool_count = 0;
    tumor_count = 0;
    lair_count = 0;
    base_count = 0;
    hatchery_count = 0;
    queen_count = 0;
    zergling_count = 0;
    attack_count = 0;
    spore_crawler_count = 0;
    hydralisk_count = 0;
    observation = Observation();
    query = Query();
    actions = Actions();
    ratio = 0;
    radius = 1000;
}

//Destructor
Node::~Node() {

}

int Node::base_count = 0;
Point2D Node::focus;
sc2::Tag Node::zergling_sent;
std::vector<float> Node::baseCoordsX;
std::vector<float> Node::baseCoordsY;

// From bot_examples.cc 
// ignores Overlords, workers, and structures
struct IsArmy {
    IsArmy(const ObservationInterface* obs) : observation_(obs) {}

    bool operator()(const Unit unit) {
        // From bot_examples.cc
        auto attributes = observation_->GetUnitTypeData().at(unit.unit_type).attributes;
        for (const auto& attribute : attributes) {
            if (attribute == Attribute::Structure) {
                return false;
            }
        }
        if (unit.tag == Node::zergling_sent) {
            return false;
        }
        switch (unit.unit_type.ToType()) {
            case UNIT_TYPEID::ZERG_OVERLORD: return false;
            case UNIT_TYPEID::ZERG_QUEEN: return false;
            case UNIT_TYPEID::ZERG_DRONE: return false;
            case UNIT_TYPEID::ZERG_LARVA: return false;
            case UNIT_TYPEID::ZERG_EGG: return false;
            default: return true;
        }
    }

    const ObservationInterface* observation_;
};

// INFO COLLECTION ///////////////////////////////////////////////////////////////////

// From bot_examples.cc 
int Node::CountUnits(const ObservationInterface* observation, UnitTypeID unit_type) {
    int count = 0;
    Units my_units = observation->GetUnits(Unit::Alliance::Self);
    for (const auto unit : my_units) {
        if (unit->unit_type == unit_type && unitBelongs(unit))
            ++count;
    }

    return count;
}

// Collects info from observation. Stores into class members
void Node::ObtainInfo() {
    const ObservationInterface* obs = Observation();
    spawning_pool_count = CountUnits(obs, UNIT_TYPEID::ZERG_SPAWNINGPOOL);
    hatchery_count = CountUnits(obs, UNIT_TYPEID::ZERG_HATCHERY) + CountUnits(obs, UNIT_TYPEID::ZERG_LAIR) + CountUnits(obs, UNIT_TYPEID::ZERG_HIVE);
    larva_count = CountUnits(obs, UNIT_TYPEID::ZERG_LARVA);
    queen_count = CountUnits(obs, UNIT_TYPEID::ZERG_QUEEN);
    drone_count = CountUnits(obs, UNIT_TYPEID::ZERG_DRONE);
    tumor_count = CountUnits(obs, UNIT_TYPEID::ZERG_CREEPTUMOR);
    spine_crawler_count = CountUnits(obs, UNIT_TYPEID::ZERG_SPINECRAWLER) + CountUnits(obs, UNIT_TYPEID::ZERG_SPINECRAWLERUPROOTED);
    food_cap = obs->GetFoodCap();
    food_used = obs->GetFoodUsed();
    minerals = obs->GetMinerals();
    vespene = obs->GetVespene();
    lair_count = CountUnits(obs, UNIT_TYPEID::ZERG_LAIR);
    spire_count = CountUnits(obs, UNIT_TYPEID::ZERG_SPIRE);
    hydralisk_count = CountUnits(obs, UNIT_TYPEID::ZERG_HYDRALISKDEN);
    base_count = obs->GetUnits(Unit::Alliance::Self, IsTownHall()).size();
    food_workers = obs->GetFoodWorkers();
    zergling_count = CountUnits(obs, UNIT_TYPEID::ZERG_ZERGLING);
    attack_count = CountUnits(obs, UNIT_TYPEID::ZERG_ROACH) + CountUnits(obs, UNIT_TYPEID::ZERG_ZERGLING) + CountUnits(obs, UNIT_TYPEID::ZERG_MUTALISK) +
                   CountUnits(obs, UNIT_TYPEID::ZERG_HYDRALISK) + CountUnits(obs, UNIT_TYPEID::ZERG_ULTRALISK) + CountUnits(obs, UNIT_TYPEID::ZERG_BROODLORD) +
                   CountUnits(obs, UNIT_TYPEID::ZERG_CORRUPTOR);
    spore_crawler_count = CountUnits(obs, UNIT_TYPEID::ZERG_SPORECRAWLER);
}

// For debugging purposes
void Node::PrintInfo() {
    cout << "Info:" << endl;
    cout << "Larva Count: " << larva_count << endl;
    cout << "Minerals: " << minerals << endl;
    cout << "Vespene: " << vespene << endl << endl;
}

// Get the closest enemy location
void Node::GetClosestEnemy() {
    Units enemies = Observation()->GetUnits(Unit::Alliance::Enemy);
    if (enemies.size() > 0) {
        sc2::Point2D closest_enemy = staging_location;
        int zerglings = 0;
        for (const Unit* unit : enemies) {
            if ((Distance2D(unit->pos, start_location) < Distance2D(closest_enemy, start_location) || closest_enemy == staging_location) &&  !unit->is_flying ) {
                closest_enemy = unit->pos;
            }
        }
        enemy_location = closest_enemy;
    }
}

// UNIT SPAWNING AND BUILDING ////////////////////////////////////////////////////////////

// From bot_examples.cc
bool Node::TryBuild(AbilityID ability_type_for_structure, UnitTypeID unit_type, Tag location_tag, Point3D location_point3d, bool is_expansion) {
    float rx = GetRandomScalar();
    float ry = GetRandomScalar();
    const ObservationInterface* observation = Observation();
   
    // default: pick a random location
    Point2D build_location = Point2D(start_location.x + rx * 15, start_location.y + ry * 15);

    // location is set to a geyser location that is closest to a base
    if (ability_type_for_structure == ABILITY_ID::BUILD_EXTRACTOR) {
        build_location = (observation->GetUnit(location_tag))->pos;
    }
    
    // building hatchery doesn't require location to have creep
    if (ability_type_for_structure != ABILITY_ID::BUILD_HATCHERY) {
        if (!observation->HasCreep(build_location)) { return false; }

        
    } else {
        cout << "trying yo build hatchery..." << endl;
        build_location = Point2D(start_location.x + baseCoordsX[0], start_location.y + baseCoordsY[0]);
        if (!Query()->Placement(ability_type_for_structure, build_location)) {
            build_location = Point2D(start_location.x + baseCoordsX[1], start_location.y + baseCoordsY[1]);
            if (!Query()->Placement(ability_type_for_structure, build_location)) {
                build_location = Point2D(start_location.x + baseCoordsX[2], start_location.y + baseCoordsY[2]);
                if (!Query()->Placement(ability_type_for_structure, build_location)) {
                    return false;
                }
            }
        }
    }

    //const ObservationInterface* observation = Observation();

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
    
    // expansion uses Point3D
    if (!is_expansion) {
        for (const auto& expansion : expansions) {
            if (Distance2D(build_location, Point2D(expansion.x, expansion.y)) < 7) {
                return false;
            }
        }
    }

    // Check to see if unit can build there
    if (Query()->Placement(ability_type_for_structure, build_location)) {
        //cout << "Building a " << ability_type_for_structure.to_string() << endl;
        if (ability_type_for_structure == ABILITY_ID::BUILD_EXTRACTOR) {
            // must pass in Unit type, Point2D does not work for building extractor
            Actions()->UnitCommand(unit, ability_type_for_structure, observation->GetUnit(location_tag));
            //cout << "Finished building an extractor" << endl;
        } else {
            Actions()->UnitCommand(unit, ability_type_for_structure, build_location);
        }
        return true;
    }
    
    return false;
}

/*bool Node::TryBuildUnit(sc2::AbilityID ability_type_for_unit) {
    const ObservationInterface* observation = Observation();
    int larva_count = CountUnits(Observation(), UNIT_TYPEID::ZERG_LARVA);
    // if we have no larva, we can't make units
    if (larva_count < 1) {
        return false;
    }

    Units larvas = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_LARVA));

    // Get a random larva to evolve
    const Unit* larva = GetRandomEntry(larvas);
    if (!larva->orders.empty()) {
        return false;
    }

    Actions()->UnitCommand(larva, ability_type_for_unit);

    return true;
}*/

// choose what unit type to transform larva into
void Node::MorphLarva(const Unit* unit) {
    // Become overlord if maxed food cap
    // TODO: Multiple spawned at once. Make only spawn one?
    int larva_count = CountUnits(observation, UNIT_TYPEID::ZERG_LARVA);

    if (minerals >= 25 && spawning_pool_count > 0 && zergling_count < 1) {
        cout << "Morphing into Zergling" << endl;
        Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_ZERGLING);
    }
    if (food_used == food_cap && minerals >= 100 && Observation()->GetFoodCap() != 200) {
        cout << "Morphing into Overlord" << endl;
        Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_OVERLORD);
    }
    if (drone_count < 10 && minerals >= 50 && food_cap - food_used > 0) {
        cout << "Morphing into Drone" << endl;
        Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_DRONE);
    }
    if (minerals >= 300 && food_cap - food_used > 0 && vespene >= 200 && CountUnits(observation, UNIT_TYPEID::ZERG_ULTRALISKCAVERN) > 0 && elapsedTime > 560) {
        cout << "Morphing into Ultralisk" << endl;
        Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_ULTRALISK);
    }
    if (minerals >= 150 && food_cap - food_used > 0 && vespene >= 100 && CountUnits(observation, UNIT_TYPEID::ZERG_HIVE) > 0) {
        cout << "Morphing into Corruptor" << endl;
        Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_CORRUPTOR);
    }
    if (minerals >= 100 && food_cap - food_used > 0 && vespene >= 50 && hydralisk_count > 0 && CountUnits(observation, UNIT_TYPEID::ZERG_HYDRALISK) <= CountUnits(observation, UNIT_TYPEID::ZERG_MUTALISK)) {
        cout << "Morphing into Hydralisk" << endl;
        Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_HYDRALISK);
    }
    if (minerals >= 100 && food_cap - food_used > 0 && vespene >= 50 && spire_count > 0 && CountUnits(observation, UNIT_TYPEID::ZERG_HYDRALISK) > CountUnits(observation, UNIT_TYPEID::ZERG_MUTALISK)) {
        cout << "Morphing into Mutalisk" << endl;
        Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_MUTALISK);
    }
    if (minerals >= 75 && food_cap - food_used > 0 && spawning_pool_count > 0 && zergling_count >= 1) {
        cout << "Morphing into Roach" << endl;
        Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_ROACH);
    }
    if (minerals >= 50 && food_cap - food_used > 0 && CountUnits(observation, UNIT_TYPEID::ZERG_DRONE) < 16){
        cout << "Morphing into Drone" << endl;
        Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_DRONE);
    }
    if (minerals >= 25 && spawning_pool_count > 0  && food_workers > 20 && zergling_count < 30) {
        cout << "Morphing into Zergling" << endl;
        Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_ZERGLING);
    }
}

// make overlord generate creep beneath it
void Node::GenerateCreep(const Unit *unit) {
    Actions()->UnitCommand(unit, ABILITY_ID::BEHAVIOR_GENERATECREEPON, true);
}

// build extractor to collect vespene gas
void Node::BuildExtractor() {
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

// from bot_example.cc
Tag Node::FindClosestGeyser(Point2D base_location) {
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

// Borrowed from bot_examples.cc
// To ensure that we do not over or under saturate any base
void Node::ManageWorkers(UNIT_TYPEID worker_type, AbilityID worker_gather_command, UNIT_TYPEID building_type) {
    const ObservationInterface* observation = Observation();
    Units bases = observation->GetUnits(Unit::Alliance::Self, IsTownHall());
    Units geysers = observation->GetUnits(Unit::Alliance::Self, IsUnit(building_type));

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
                        MineIdleWorkers(worker, worker_gather_command, building_type);
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
                        MineIdleWorkers(worker, worker_gather_command, building_type);
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
                    if (target->unit_type != building_type) {
                        //This should allow them to be picked up by mineidleworkers()
                        MineIdleWorkers(worker, worker_gather_command, building_type);
                        return;
                    }
                }
            }
        }
    }
}
// bloop
// Borrowed from bot_example.cc
// Mine the nearest mineral to Town hall.
// If we don't do this, drones may mine from other patches if they stray too far from the base after building.
void Node::MineIdleWorkers(const Unit* worker, AbilityID worker_gather_command, UnitTypeID building_type) {
    const ObservationInterface* observation = Observation();
    Units bases = observation->GetUnits(Unit::Alliance::Self, IsTownHall());
    Units geysers = observation->GetUnits(Unit::Alliance::Self, IsUnit(building_type));

    const Unit* valid_mineral_patch = nullptr;

    // if no base, return
    if (bases.empty()) {
        return;
    }

    // assign more workers to geyser if not over-populated
    for (const auto& geyser : geysers) {
        if (geyser->assigned_harvesters < geyser->ideal_harvesters) {
            Actions()->UnitCommand(worker, worker_gather_command, geyser);
            //cout << "MINEIDLEWORKERS!" << endl;
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
            cout << "We are missing workers at base!!" << endl;
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
const Unit* Node::FindNearestMineralPatch(const Point2D& start) {
    Units units = Observation()->GetUnits(Unit::Alliance::Neutral);  // need scouting to expand view point to spot more mine fields
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

// expand hatchery when reached ideal number of workers
bool Node::BuildNewHatchery() {
    int max_worker_count = 70;

    // Don't have more active bases than we can provide workers for
    if (GetExpectedWorkers(UNIT_TYPEID::ZERG_EXTRACTOR) > max_worker_count) {
        return false;
    }
    // if we have extra workers around and more than enough minerals, try to build another base
    if (GetExpectedWorkers(UNIT_TYPEID::ZERG_EXTRACTOR) < food_workers && minerals > min<size_t>((size_t)base_count * 300, 1200)) {
        return TryExpand(ABILITY_ID::BUILD_HATCHERY, UNIT_TYPEID::ZERG_DRONE);
    }
    return false;
}

//Expands to nearest location and updates the start location to be between the new location and old bases
bool Node::TryExpand(AbilityID build_ability, UnitTypeID unit_type) {
    const ObservationInterface* observation = Observation();
    float minimum_distance = std::numeric_limits<float>::max();
    Point3D closest_expansion;
    for (const auto& expansion : expansions) {
        float current_distance = Distance2D(start_location, expansion);
        if (current_distance < radius) {
            continue;
        }
        // find closest ideal location for to expand base
        if (current_distance < minimum_distance) {
            if (Query()->Placement(build_ability, expansion)) {
                closest_expansion = expansion;
                minimum_distance = current_distance;
            }
        }
    }
    // only update staging location up till 3 bases
    if (TryBuild(build_ability, unit_type, 0, closest_expansion, true) && base_count < 4) {
        cout << "Building new base" << endl;
        staging_location = Point3D(((staging_location.x + closest_expansion.x) / 2), ((staging_location.y + closest_expansion.y) / 2),
            ((staging_location.z + closest_expansion.z) / 2));
        return true;
    }
    return false;

}

// Borrowed from bot_examples.cc
// An estimate of how many workers we should have based on what buildings we have
int Node::GetExpectedWorkers(UNIT_TYPEID building_type) {
    const ObservationInterface* observation = Observation();
    Units bases = observation->GetUnits(Unit::Alliance::Self, IsTownHall());
    Units geysers = observation->GetUnits(Unit::Alliance::Self, IsUnit(building_type));
    int expected_workers = 0;
    for (const auto& base : bases) {
        if (base->build_progress != 1) {
            continue;
        }
        expected_workers += base->ideal_harvesters;
    }

    for (const auto& geyser : geysers) {
        if (geyser->vespene_contents > 0) {
            if (geyser->build_progress != 1) {
                continue;
            }
            expected_workers += geyser->ideal_harvesters;
        }
    }

    return expected_workers;
}


// UNIT CONTROL ////////////////////////////////////////////////////////////////////////////////////////////////

// Get the queens to inject Larva when they are able to, decides whether a queen
// spreads creep tumors or injects larva at a hatchery
void Node::QueenAction(const Unit* unit, int num) {
    Units hatcheries = Observation()->GetUnits(Unit::Alliance::Self, IsTownHall());
    num %= hatcheries.size() + 1;

    // if there are no creep tumors, make one so it can start spreading creep
    if (unit->energy >= 25 && unit->orders.empty() && num >= hatcheries.size()) {
        // move towards expand location until we find a point where there is no creep, then drop a tumor
        // idk if this works rn I'll have to adjust it later
        if (tumor_count == 0) {
            Actions()->UnitCommand(unit, ABILITY_ID::BUILD_CREEPTUMOR);
        }
        else {
            HealUnits(unit);
        }
    }
    // decide which hatchery we're building at, cycles between queens
    for (size_t i = 0; i < hatcheries.size(); i++) {
        if (num == i) {
            // if hatchery is not completely built yet
            if (hatcheries.at(i)->build_progress != 1) {
                num++;
            }
            // prevents impossible requests
            else if(unit->energy >= 25 && unit->orders.empty()){
                Actions()->UnitCommand(unit, ABILITY_ID::EFFECT_INJECTLARVA, hatcheries.at(i));
            }
        }
    }
}

// Decides hatchery actions (Queen can't evolve from Larva)
void Node::Hatch(const Unit* unit) {
    // check if we can make queen, have a limit so we can also make drones and zerglings
    if (minerals >= 150 && spawning_pool_count > 0 && queen_count < hatchery_count) {
        Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_QUEEN);
    }
    if (minerals >= 100 && vespene >= 100 && !researched_burrow) {
        // research burrow, this may be useful
        Actions()->UnitCommand(unit, ABILITY_ID::RESEARCH_BURROW);
        researched_burrow = true;
    }
}

// Send zerglings to attack
// TODO: Get more unit types
void Node::Ambush() {
    Units units = Observation()->GetUnits(Unit::Alliance::Self, IsArmy(Observation()));
    int count = 0;
    for (const Unit* unit : units) {
        if (count < army_size) {
            Actions()->UnitCommand(unit, ABILITY_ID::ATTACK, enemy_location);
        }
        else {
            break;
        }
        count++;
    }
}

// Search for the enemy actively
void Node::SearchAndAmbush() {
    Units enemies = Observation()->GetUnits(Unit::Alliance::Enemy);
    Units units = Observation()->GetUnits(Unit::Alliance::Self, IsArmy(Observation()));
    for (const Unit* unit : units) {
        if (enemies.size() > 0) {
            Actions()->UnitCommand(unit, ABILITY_ID::ATTACK, enemies[0]->pos);
        }
        else {
            Actions()->UnitCommand(unit, ABILITY_ID::ATTACK, enemy_location);
        }
    }
}

void Node::moveDefense() {
    Units units = Observation()->GetUnits(Unit::Alliance::Self);
    cout << "aah! move defense!" << endl;

    focus = Point2D(getBasePosition().x, getBasePosition().y);

    for (const Unit *unit : units) {
        if (unit->unit_type.ToType() == UNIT_TYPEID::ZERG_SPINECRAWLER) {
            //TODO: Change tags
            Actions()->UnitCommand(unit, ABILITY_ID::MORPH_SPINECRAWLERUPROOT);
        }
        if (unit->unit_type.ToType() == UNIT_TYPEID::ZERG_SPORECRAWLER) {
            Actions()->UnitCommand(unit, ABILITY_ID::MORPH_SPORECRAWLERUPROOT);
        }
        if (unit->unit_type.ToType() == UNIT_TYPEID::ZERG_ZERGLING) {
            Actions()->UnitCommand(unit, ABILITY_ID::GENERAL_MOVE, focus, true);
        }
    }

    Units army = Observation()->GetUnits(Unit::Alliance::Self, IsArmy(Observation()));
    for (const Unit* unit : army){
        Actions()->UnitCommand(unit, ABILITY_ID::GENERAL_MOVE, focus, true);
    }
}

// make queen heal other biological units
void Node::HealUnits(const Unit* unit) {
    Units army = Observation()->GetUnits(Unit::Alliance::Self, IsArmy(Observation()));
    if (unit->orders.empty()) {
        for (size_t i = 0; i < army.size(); i++) {
            // heal injured units
            if (army.at(i)->health < army.at(i)->health_max) {
                //cout << army.at(i) << " is injured" << endl;
                Actions()->UnitCommand(unit, ABILITY_ID::EFFECT_TRANSFUSION, army.at(i)); 
                //cout << "queen is healing " << army.at(i) << endl;
                break;
            }
        }
    }
}

// GAME START AND STEP ///////////////////////////////////////////////////////////////////

void Node::OnGameStart() { 
    cout << "start!!!!!!" << endl;
    start_location = Observation()->GetStartLocation();
    staging_location = start_location;
    expansions = search::CalculateExpansionLocations(Observation(), Query());
    cout << Observation()->GetGameInfo().map_name << endl;

    std::vector<float> A = {-33, -60, -21};
    std::vector<float> B = {2.5, -3, -26};
    std::vector<float> Aneg = {33, 60, 21};
    std::vector<float> Bneg = {-2.5, 3, 26};

    if (Observation()->GetGameInfo().map_name == "Bel'Shir Vestige LE (Void)") {
        cout << "MAP2" << endl;
        A = {-32, -0, -68};
        B = {1, -38, 4};
        Aneg = {32, 0, 68};
        Bneg = {-1, 38, -4};
    } else if (Observation()->GetGameInfo().map_name == "Proxima Station LE") {
        A = {-0, -32, -28};
        B = {27, 10, -19};
        Aneg = {0, 32, 28};
        Bneg = {-27, -10, 19};
    }
    
    if (start_location.y > Observation()->GetGameInfo().height / 2.0) {
        // top left
        if (start_location.x < Observation()->GetGameInfo().width / 2.0) {
            baseCoordsX = Aneg;
            baseCoordsY = B;
        }
        // top right
        else {
            baseCoordsX = B;
            baseCoordsY = A;
        }
    } else {
        // bot left
        if (start_location.x < Observation()->GetGameInfo().width / 2.0) {
            baseCoordsX = Bneg;
            baseCoordsY = Aneg;
        }
        // bot right
        else {
            baseCoordsX = A;
            baseCoordsY = Bneg;
        }
    }

    //cout << baseCoordsX[0] << " " << baseCoordsY[0] << endl;

    return;
}

// per frame...
void Node::OnStep() {
    timer++;
    army_size = (int)(elapsedTime / 40.0);
    if (army_size > 20) {
        army_size = 20.0;
    }
    ObtainInfo();
    GetClosestEnemy();
    if (attack_count >= army_size) {
        Ambush();
    }
    if (attack_count >= 50) {
        SearchAndAmbush();
    }
    Units bases = Observation()->GetUnits(Unit::Alliance::Self, IsTownHall());
    const Unit* base;
    bool baseFound = false;
    for (int j = 0; j < bases.size(); j++) {
        for (int i = 0; i < nodeUnits.size(); i++) {
            if (nodeUnits[i] == bases[j]->tag) {
                base = bases[j];
                baseFound = true;
                break;
            }
        }
    }
    if (baseFound) {
        ratio = base->health / base->health_max;
    } else {
        ratio = 1;
    }
    
    int queens = 0; // used for queens to be able to inject larva into more than 1 hatchery at once

    // looking through all unit types
    Units units = Observation()->GetUnits(Unit::Alliance::Self);
    for (const auto& unit : units) {
        if (unitBelongs(unit) ) {

            switch (unit->unit_type.ToType()) {
                case UNIT_TYPEID::ZERG_LARVA: {
                    MorphLarva(unit);
                    break;
                }
                case UNIT_TYPEID::ZERG_QUEEN: {
                    if (queens < queen_count) {
                        QueenAction(unit, queens);
                    }
                    if (unit->energy >= 25 && unit->orders.empty()) {  // queen plant creep tumor
                        if (Observation()->HasCreep(unit->pos)) {
                            Actions()->UnitCommand(unit, ABILITY_ID::GENERAL_MOVE, staging_location);
                        }
                        if (!Observation()->HasCreep(unit->pos)) {
                            Actions()->UnitCommand(unit, ABILITY_ID::BUILD_CREEPTUMOR);
                        }
                    }
                    else {
                        HealUnits(unit);
                    }
                    queens++;
                    break;
                }
                case UNIT_TYPEID::ZERG_CREEPTUMOR: {  // creep tumor generates creep tumor
                    // builds creep tumor when it can, this is its only available action and can only happen once
                    // move until we find a good place to place creep
                    if (unit->energy >= 25 && unit->orders.empty()) {
                        if (Observation()->HasCreep(unit->pos)) {
                            Actions()->UnitCommand(unit, ABILITY_ID::MOVE_MOVE, staging_location);
                        }
                        if (!Observation()->HasCreep(unit->pos)) {
                            Actions()->UnitCommand(unit, ABILITY_ID::BUILD_CREEPTUMOR);
                        }
                    }
                    break;
                }
                case UNIT_TYPEID::ZERG_SPAWNINGPOOL: {
                    if (minerals >= 100 && vespene >= 100 && hatchery_count > 0 && !researched_metabolic) { // Research metabolic boost, but only after the hatchery is ready 
                        Actions()->UnitCommand(unit, ABILITY_ID::RESEARCH_ZERGLINGMETABOLICBOOST);
                        researched_metabolic = true;
                    }
                    break;
                }
                case UNIT_TYPEID::ZERG_HATCHERY: {
                    Hatch(unit);
                    if (minerals >= 150 && vespene >= 100) { // upgrade 
                        Actions()->UnitCommand(unit, ABILITY_ID::MORPH_LAIR);
                        ready_to_expand = true;
                    }
                    break;
                }
                case UNIT_TYPEID::ZERG_LAIR: {
                    Hatch(unit); // No specialization for now
                    if (minerals >= 200 && vespene >= 150) { // upgrade 
                        Actions()->UnitCommand(unit, ABILITY_ID::MORPH_HIVE);
                        //Actions()->SendChat("Insolent little fool!");
                        //Actions()->SendChat("Prepare for the darkness owo.");
                    }
                    break;
                }
                case UNIT_TYPEID::ZERG_HIVE: {
                    Hatch(unit); // No specialization for now
                    break;
                }
                // Uprooted spore and spinecrawlers move to defense focus
                case UNIT_TYPEID::ZERG_SPORECRAWLERUPROOTED: {
                    if (unit->orders.empty()) {
                        Point2D pos = Point2D(focus.x +  GetRandomScalar() * 10, focus.y + GetRandomScalar() * 10);
                        Actions()->UnitCommand(unit, ABILITY_ID::MORPH_SPORECRAWLERROOT, pos, true);
                    }
                    break;
                }
                case UNIT_TYPEID::ZERG_SPINECRAWLERUPROOTED: { 
                    //Point2D goTo = Point2D(getBasePosition().x, getBasePosition().y);
                    if (unit->orders.empty()) {
                        Point2D pos = Point2D(focus.x +  GetRandomScalar() * 10, focus.y + GetRandomScalar() * 10);
                        Actions()->UnitCommand(unit, ABILITY_ID::MORPH_SPINECRAWLERROOT, pos, true);
                    }
                    break; 
                }
                case UNIT_TYPEID::ZERG_OVERLORD: {
                    if (lair_count > 0) {  // available once lair built
                        // start generating creep if there is no creep, or for 5 seconds every 5 seconds (theoretically)
                        GenerateCreep(unit);
                        //if (Observation()->HasCreep(unit->pos) || timer%3000 < 300) {
                            //Actions()->UnitCommand(unit, ABILITY_ID::GENERAL_MOVE, staging_location);
                            // update staging_location
                            //staging_location = Point3D((staging_location.x + 20), (staging_location.y + 20), (staging_location.z));
                        //}
                        //else {
                        //    Actions()->UnitCommand(unit, ABILITY_ID::STOP);
                           
                        //}
                        

                    }
                    // Testing base positions
                    // Point2D pon = Point2D(start_location.x + baseCoordsX[2], start_location.y + baseCoordsY[2]);
                    // Actions()->UnitCommand(unit, ABILITY_ID::GENERAL_MOVE, pon);
                    break;
                }
                case UNIT_TYPEID::ZERG_SPIRE: {
                    if (minerals >= 100 && vespene >= 150) {
                        Actions()->UnitCommand(unit, ABILITY_ID::MORPH_GREATERSPIRE);
                    }
                }
                case UNIT_TYPEID::ZERG_CORRUPTOR: {
                    if (minerals >= 150 && food_cap - food_used > 0 && vespene >= 150 && CountUnits(observation, UNIT_TYPEID::ZERG_GREATERSPIRE) > 0) {
                        cout << "Morphing into Brood Lord" << endl;
                        Actions()->UnitCommand(unit, ABILITY_ID::MORPH_BROODLORD);
                    }
                    break;
                }
                case UNIT_TYPEID::ZERG_ULTRALISK: {
                    //SearchAndAmbush();
                }
                default: {
                    break;
                }
            }

        }
    }

    ManageWorkers(UNIT_TYPEID::ZERG_DRONE, ABILITY_ID::HARVEST_GATHER, UNIT_TYPEID::ZERG_EXTRACTOR);

    // building spawning pool
    if (spawning_pool_count < 1 && minerals >= 200) {
        TryBuild(ABILITY_ID::BUILD_SPAWNINGPOOL, UNIT_TYPEID::ZERG_DRONE);
    }
    if (spawning_pool_count > 0 && minerals >= 150 && CountUnits(observation, UNIT_TYPEID::ZERG_ROACHWARREN) < 1) {
        TryBuild(ABILITY_ID::BUILD_ROACHWARREN, UNIT_TYPEID::ZERG_DRONE);
    }
    if (spine_crawler_count < 2 && minerals >= 100) {
        TryBuild(ABILITY_ID::BUILD_SPINECRAWLER, UNIT_TYPEID::ZERG_DRONE);
    }
    // build spore crawler for defending air attacks
    if (spore_crawler_count < 2 && minerals >= 75) {
        TryBuild(ABILITY_ID::BUILD_SPORECRAWLER, UNIT_TYPEID::ZERG_DRONE);
    }
    // built hydralisk den
    if (lair_count > 0 && minerals >= 100 && vespene >= 100 && CountUnits(observation, UNIT_TYPEID::ZERG_INFESTATIONPIT) < 1) {
        TryBuild(ABILITY_ID::BUILD_INFESTATIONPIT, UNIT_TYPEID::ZERG_DRONE);
    }
    if (lair_count > 0 && hydralisk_count < 1 && minerals >= 100 && vespene >= 100) {
        TryBuild(ABILITY_ID::BUILD_HYDRALISKDEN, UNIT_TYPEID::ZERG_DRONE);
    }
    if (lair_count > 0 && spire_count < 1 && minerals >= 100 && vespene >= 100) {
        TryBuild(ABILITY_ID::BUILD_SPIRE, UNIT_TYPEID::ZERG_DRONE);
    }
    if (minerals >= 150 && vespene >= 200 && CountUnits(observation, UNIT_TYPEID::ZERG_HIVE) > 0 && CountUnits(observation, UNIT_TYPEID::ZERG_ULTRALISKCAVERN) < 1) {
        TryBuild(ABILITY_ID::BUILD_ULTRALISKCAVERN, UNIT_TYPEID::ZERG_DRONE);
    }

    bool not_enough_extractor = CountUnits(Observation(), UNIT_TYPEID::ZERG_EXTRACTOR) < Observation()->GetUnits(Unit::Alliance::Self, IsTownHall()).size() * 2;
    // if all bases are destroyed or there's no base, don't build
    if (base_count > 0 && minerals >= 25 && not_enough_extractor) {
        BuildExtractor();
    }

    /*if (ready_to_expand) {
        TryExpand(ABILITY_ID::BUILD_SPAWNINGPOOL, UNIT_TYPEID::ZERG_DRONE);
    }*/

    if (BuildNewHatchery()) { return; };  // try build new base

    return;
}

void Node::OnUnitIdle(const Unit *unit) {
   
    switch (unit->unit_type.ToType()) {

        case UNIT_TYPEID::ZERG_OVERLORD: {
            //float rx = GetRandomScalar() * defensive_overlord_scatter_distance;
            //float ry = GetRandomScalar() * defensive_overlord_scatter_distance;
            //float rz = GetRandomScalar() * defensive_overlord_scatter_distance;
            //Actions()->UnitCommand(unit, ABILITY_ID::MOVE_MOVEPATROL, unit->pos + Point3D(rx, ry, rz));
            break;
        }
        case UNIT_TYPEID::ZERG_ZERGLING: {
            auto spawn_points = Observation()->GetGameInfo().enemy_start_locations;
            if (zergling_sent == NULL) {
                Actions()->UnitCommand(unit, ABILITY_ID::MOVE_MOVE, spawn_points[checked_spawn]);
                zergling_sent = unit->tag;
                checked_spawn = 0;
                if (spawn_points.size() > 0) {
                    checked_spawn++;
                }
            }
            else if (unit->tag == zergling_sent) {
                Actions()->UnitCommand(unit, ABILITY_ID::MOVE_MOVE, spawn_points[checked_spawn]);
                checked_spawn++;
                if (checked_spawn == spawn_points.size()) checked_spawn = 0;
            }
            break;
        }
        case UNIT_TYPEID::ZERG_DRONE: {
            MineIdleWorkers(unit, ABILITY_ID::HARVEST_GATHER,UNIT_TYPEID::ZERG_EXTRACTOR);
            break;
        }
        default:
            break;
    }
}

void Node::OnUnitDestroyed(const Unit* unit) {
    if (unit->tag == zergling_sent) {
        zergling_sent = NULL;
    }
    if (unit->unit_type.ToType() == UNIT_TYPEID::ZERG_HATCHERY || unit->unit_type.ToType() == UNIT_TYPEID::ZERG_LAIR || unit->unit_type.ToType() == UNIT_TYPEID::ZERG_HIVE) {
        hatchery_count--;
        base_count--;
    }
}

/*--------------META NODE FILES----------------*/

//Checks if a unit belongs to this node
bool Node::unitBelongs(const Unit* unit) {
    for (int i = 0; i < nodeUnits.size(); i++) {
        if (unit->tag == nodeUnits[i]) return true;
    }
    return false;
}

//get the position of the base in this node
sc2::Point3D Node::getBasePosition() {
    Units bases = Observation()->GetUnits(Unit::Alliance::Self, IsTownHall());
    for (int base = 0; base < bases.size(); base++) {
        for (int i = 0; i < nodeUnits.size(); i++) {
            if (nodeUnits[i] == bases[base]->tag) {
                return bases[base]->pos;
            }
        }
    }

    //should never run, each node should own at least 1 hatchery. may run on a node where the hatchery has been destroyed. in that event, consider distributing units to neighbour nodes? nodes dont get destroyed when destroyed in game rn
    return start_location;
}

//Adds the given unit to the node
void Node::addUnit(const sc2::Unit* unit) {
    nodeUnits.push_back(unit->tag);
}

//Adds the given unit tag to the node
void Node::addUnit(sc2::Tag tag) {
    nodeUnits.push_back(tag);
}

//replacement for sc2::Agent::Observation
const sc2::ObservationInterface* Node::Observation() {
    return observation;
}

//replacement for sc2::Agent::Actions
sc2::ActionInterface* Node::Actions() {
    return actions;
}

//replacement for sc2::Agent::Query
sc2::QueryInterface* Node::Query() {
    return query;
}