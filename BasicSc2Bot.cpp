#include "BasicSc2Bot.h"
#include "sc2api/sc2_unit_filters.h"
#include <iostream>

using namespace sc2;
using namespace std;

// ignores Overlords, workers, and structures
struct IsArmy {
    IsArmy(const ObservationInterface* obs) : observation_(obs) {}

    bool operator()(const Unit& unit) {
        // From bot_examples.cc
        auto attributes = observation_->GetUnitTypeData().at(unit.unit_type).attributes;
        for (const auto& attribute : attributes) {
            if (attribute == Attribute::Structure) {
                return false;
            }
        }
        switch (unit.unit_type.ToType()) {
            case UNIT_TYPEID::ZERG_OVERLORD: return false;
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
    hydralisk_count = CountUnits(obs, UNIT_TYPEID::ZERG_HYDRALISKDEN);
    base_count = obs->GetUnits(Unit::Alliance::Self, IsTownHall()).size();
    food_workers = obs->GetFoodWorkers();
    zergling_count = CountUnits(obs, UNIT_TYPEID::ZERG_ZERGLING);
    spore_crawler_count = CountUnits(obs, UNIT_TYPEID::ZERG_SPORECRAWLER);
}

// For debugging purposes
void BasicSc2Bot::PrintInfo() {
    cout << "Info:" << endl;
    cout << "Larva Count: " << larva_count << endl;
    cout << "Minerals: " << minerals << endl;
    cout << "Vespene: " << vespene << endl << endl;
}


void BasicSc2Bot::GetMostDamagedBuilding() {
    Units buildings = Observation()->GetUnits(Unit::Alliance::Self, IsTownHall());
    const Unit *most_damaged;
    bool any_damaged = false;
    for (const Unit *unit : buildings) {
        if (unit->health < unit->health_max) {
            any_damaged = true;
            if (most_damaged == NULL || (unit->health/double(unit->health_max)) < (most_damaged->health/double(most_damaged->health_max))) {
                most_damaged = unit;
            }
        }
    }

    if (any_damaged && defense_focus != most_damaged) {
        defense_focus = most_damaged;
        BasicSc2Bot::MoveDefense(Point2D(defense_focus->pos.x, defense_focus->pos.y));
    }
}

// UNIT SPAWNING AND BUILDING ////////////////////////////////////////////////////////////

// From bot_examples.cc
bool BasicSc2Bot::TryBuild(AbilityID ability_type_for_structure, UnitTypeID unit_type, Tag location_tag, Point3D location_point3d, bool is_expansion) {
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

// choose what unit type to transform larva into
void BasicSc2Bot::MorphLarva(const Unit *unit) {
    // Become overlord if maxed food cap
    // TODO: Multiple spawned at once. Make only spawn one?
    if (food_used == food_cap && minerals >= 100) {

        cout << "Morphing into Overlord" << endl;
        Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_OVERLORD);
    }
    else if (minerals >= 25 && spawning_pool_count > 0 && food_workers > 30 && zergling_count < 101) {
        cout << "Morphing into Zergling" << endl;
        Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_ZERGLING);
    }
    else if (minerals >= 50 && food_cap - food_used > 0){
        cout << "Morphing into Drone" << endl;
        Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_DRONE);
    }
    else if (minerals >= 100 && vespene >= 50 && hydralisk_count > 0) {
        cout << "Morphing into Hydralisk" << endl;
        Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_HYDRALISK);
    }
    else {
        return;
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

// Borrowed from bot_examples.cc
// To ensure that we do not over or under saturate any base
void BasicSc2Bot::ManageWorkers(UNIT_TYPEID worker_type, AbilityID worker_gather_command, UNIT_TYPEID building_type) {
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

// Borrowed from bot_example.cc
// Mine the nearest mineral to Town hall.
// If we don't do this, drones may mine from other patches if they stray too far from the base after building.
void BasicSc2Bot::MineIdleWorkers(const Unit* worker, AbilityID worker_gather_command, UnitTypeID building_type) {
    const ObservationInterface* observation = Observation();
    Units bases = observation->GetUnits(Unit::Alliance::Self, IsTownHall());
    Units geysers = observation->GetUnits(Unit::Alliance::Self, IsUnit(building_type));

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
bool BasicSc2Bot::BuildNewHatchery() {
    int max_worker_count = 70;

    // Don't have more active bases than we can provide workers for
    if (GetExpectedWorkers(UNIT_TYPEID::ZERG_EXTRACTOR) > max_worker_count) {
        return false;
    }
    // if we have extra workers around and more than enough minerals, try to build another base
    if (GetExpectedWorkers(UNIT_TYPEID::ZERG_EXTRACTOR) < food_workers && minerals > min<size_t>(base_count * 300, 1200)) {
        return TryExpand(ABILITY_ID::BUILD_HATCHERY, UNIT_TYPEID::ZERG_DRONE);
    }
    //Only build another Hatch if we are floating extra minerals
    /*if (minerals > min<size_t>(base_count * 300, 1200)) {
        return TryExpand(ABILITY_ID::BUILD_HATCHERY, UNIT_TYPEID::ZERG_DRONE);
    }*/
    return false;
}

//Expands to nearest location and updates the start location to be between the new location and old bases
bool BasicSc2Bot::TryExpand(AbilityID build_ability, UnitTypeID unit_type) {
    const ObservationInterface* observation = Observation();
    float minimum_distance = std::numeric_limits<float>::max();
    Point3D closest_expansion;
    for (const auto& expansion : expansions) {
        float current_distance = Distance2D(start_location, expansion);
        if (current_distance < .01f) {
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
int BasicSc2Bot::GetExpectedWorkers(UNIT_TYPEID building_type) {
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

// Get the queens to inject Larva when they are able to, decides whether a queen spreads creep tumors or injects larva at a hatchery
void BasicSc2Bot::QueenAction(const Unit* unit, int num) {
    Units hatcheries = Observation()->GetUnits(Unit::Alliance::Self, IsTownHall());
    Units lairs = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_LAIR));
    if (tumor_count > 0) {
        num %= hatcheries.size() + lairs.size();
    }
    else {
        num %= hatcheries.size() + lairs.size() + 1;
    }

    // if there are no creep tumors, make one so it can start spreading creep
    if (unit->energy >= 25 && unit->orders.empty() && tumor_count == 0 && num >= hatcheries.size()+lairs.size()) {
        // move towards expand location until we find a point where there is no creep, then drop a tumor
        if (Observation()->HasCreep(unit->pos)) {
            Actions()->UnitCommand(unit, ABILITY_ID::GENERAL_MOVE, staging_location);
        }
        if (!Observation()->HasCreep(unit->pos)) {
            Actions()->UnitCommand(unit, ABILITY_ID::GENERAL_MOVE, unit->pos);
            Actions()->UnitCommand(unit, ABILITY_ID::BUILD_CREEPTUMOR);
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
    for (size_t i = 0; i < lairs.size(); i++) {
        if (num == i) {
            // if hatchery is not completely built yet
            if (lairs.at(i)->build_progress != 1) {
                num++;
            }
            // prevents impossible requests
            else if (unit->energy >= 25 && unit->orders.empty()) {
                Actions()->UnitCommand(unit, ABILITY_ID::EFFECT_INJECTLARVA, lairs.at(i));
            }
        }
    }
}

// Decides hatchery actions (Queen can't evolve from Larva)
void BasicSc2Bot::Hatch(const Unit* unit) {
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

// UNIT CONTROL ///////////////////////////////////////////////////////////////////

void BasicSc2Bot::MoveDefense(Point2D& pos) {
    Units units = Observation()->GetUnits(Unit::Alliance::Self);
    cout << "aah! move defense!" << endl;

    for (const Unit *unit : units) {
        if (unit->unit_type.ToType() == UNIT_TYPEID::ZERG_SPINECRAWLER) {
            Actions()->UnitCommand(unit, ABILITY_ID::MORPH_SPINECRAWLERUPROOT);
        }
        if (unit->unit_type.ToType() == UNIT_TYPEID::ZERG_ZERGLING) {
            Actions()->UnitCommand(unit, ABILITY_ID::GENERAL_MOVE, pos, true);
        }
    }
}

// make queen heal other biological units
void BasicSc2Bot::HealUnits(const Unit* unit) {
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

void BasicSc2Bot::OnGameStart() { 
    start_location = Observation()->GetStartLocation();
    staging_location = start_location;
    expansions = search::CalculateExpansionLocations(Observation(), Query());
    defense_focus =  Observation()->GetUnits(Unit::Alliance::Self, IsTownHall())[0];
    return;
}

// per frame...
void BasicSc2Bot::OnStep() { 
    ObtainInfo();
    GetMostDamagedBuilding();
    int queens = 0; // used for queens to be able to inject larva into more than 1 hatchery at once

    // looking through all unit types
    Units units = Observation()->GetUnits(Unit::Alliance::Self);
    for (const auto& unit : units) {

        switch (unit->unit_type.ToType()) {
            case UNIT_TYPEID::ZERG_LARVA: {
                MorphLarva(unit);
                break;
            }
            case UNIT_TYPEID::ZERG_QUEEN: {
                QueenAction(unit, queens);
                queens++;
                HealUnits(unit);
                break;
            }
            case UNIT_TYPEID::ZERG_CREEPTUMOR: {
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
                }
                break;
            }
            case UNIT_TYPEID::ZERG_LAIR: {
                Hatch(unit); // No specialization for now
                break;
            }
            case UNIT_TYPEID::ZERG_SPINECRAWLER: {
                //Actions()->UnitCommand(unit, ABILITY_ID::MORPH_SPINECRAWLERUPROOT, true);
                //Actions()->UnitCommand(unit, ABILITY_ID::MORPH_SPINECRAWLERROOT, true);

            }
            case UNIT_TYPEID::ZERG_SPINECRAWLERUPROOTED: {
                if (defense_focus != NULL) {
                   Point2D pos = Point2D(defense_focus->pos.x +  GetRandomScalar() * 10, defense_focus->pos.y + GetRandomScalar());
                   Actions()->UnitCommand(unit, ABILITY_ID::MORPH_SPINECRAWLERROOT, pos, true);
                } 
            }
            case UNIT_TYPEID::ZERG_OVERLORD: {
                if (lair_count > 0) {  // available once lair built
                    GenerateCreep(unit);
                    // start generating creep then move to staging location to spread it
                    Actions()->UnitCommand(unit, ABILITY_ID::GENERAL_MOVE, staging_location);
                }
                break;
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
    if (spine_crawler_count < 3 && minerals >= 100) {
        TryBuild(ABILITY_ID::BUILD_SPINECRAWLER, UNIT_TYPEID::ZERG_DRONE);
    }
    // build spore crawler for defending air attacks
    if (spore_crawler_count < 5 && minerals >= 75) {
        TryBuild(ABILITY_ID::BUILD_SPORECRAWLER, UNIT_TYPEID::ZERG_DRONE);
    // built hydralisk den
    if (lair_count > 0 && hydralisk_count < 1 && minerals >= 100 && vespene >= 100) {
        TryBuild(ABILITY_ID::BUILD_HYDRALISKDEN, UNIT_TYPEID::ZERG_DRONE);
        ready_to_expand = true;
    }

    bool not_enough_extractor = CountUnits(Observation(), UNIT_TYPEID::ZERG_EXTRACTOR) < Observation()->GetUnits(Unit::Alliance::Self, IsTownHall()).size() * 2;
    // if all bases are destroyed or there's no base, don't build
    if (base_count > 0 && minerals >= 25 && not_enough_extractor) {
        BuildExtractor();
    }

    if (ready_to_expand) {
        TryExpand(ABILITY_ID::BUILD_SPAWNINGPOOL, UNIT_TYPEID::ZERG_DRONE);
    }

    ManageWorkers(UNIT_TYPEID::ZERG_DRONE, ABILITY_ID::HARVEST_GATHER, UNIT_TYPEID::ZERG_EXTRACTOR);
    ManageWorkers(UNIT_TYPEID::ZERG_DRONE, ABILITY_ID::HARVEST_GATHER, UNIT_TYPEID::ZERG_HATCHERY);
    BuildNewHatchery();

    return;
}

void BasicSc2Bot::OnUnitIdle(const Unit *unit) {
    if (unit->unit_type.ToType() == UNIT_TYPEID::ZERG_OVERLORD) {
        float rx = GetRandomScalar() * defensive_overlord_scatter_distance;
        float ry = GetRandomScalar() * defensive_overlord_scatter_distance;
        float rz = GetRandomScalar() * defensive_overlord_scatter_distance;
        Actions()->UnitCommand(unit, ABILITY_ID::MOVE_MOVEPATROL, unit->pos + Point3D(rx, ry, rz));
    }

    if (unit->unit_type.ToType() == UNIT_TYPEID::ZERG_ZERGLING) {
        auto spawn_points = Observation()->GetGameInfo().enemy_start_locations;
        if (zergling_sent == NULL) {
            Actions()->UnitCommand(unit, ABILITY_ID::MOVE_MOVE, spawn_points[checked_spawn]);
            zergling_sent = unit->tag;
            checked_spawn = 1;
        }
        else if (unit->tag == zergling_sent) {
            Actions()->UnitCommand(unit, ABILITY_ID::MOVE_MOVE, spawn_points[checked_spawn]);
            checked_spawn++;
            if (checked_spawn == spawn_points.size()) checked_spawn = 0;
        }
    }
}

void BasicSc2Bot::OnUnitDestroyed(const Unit* unit) {
    if (unit->tag == zergling_sent) {
        zergling_sent = NULL;
    }
}
