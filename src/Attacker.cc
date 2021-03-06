/**
 *  \brief
 *
 *  Copyright (C) 2017  Chaos-Dev
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <algorithm>    // std::min
#include <exception>
#include <random>
#include <cmath>

#include "Actor.h"
#include "Engine.h"

Attacker::Attacker() : firing(false) {
};

/** Allows initialization with combat attributes.
 */
Attacker::Attacker(int attack, int dodge, int mean_damage, int max_range)
    : attack(attack), dodge(dodge), mean_damage(mean_damage),
      max_range(max_range), firing(false) {
};

/** This is the core functionality behind all attacks (melee, ranged, etc.)
 *
 * @param owner - A pointer to the actor who is attacking.
 * @param target - A pointer to the actor being attacked.
 * @param temp_damager - A raw pointer pointing to the damage object used.
 *   This function does not take ownership of the damager object.
 * @param mod - An integer representing a bonus applied to both the attack and
 *   the damage.
 */
void Attacker::Attack(Actor *owner, Actor *target, int mod) {

    std::normal_distribution<float> dist(owner->attacker->attack-3,
                                         (owner->attacker->attack-3.0)/3);
	int attack_roll = std::max(0,(int)dist(engine.rng));
#ifndef NDEBUG
        engine.gui->log->Print("[color=grey]The attack roll was: %d / %d", attack_roll, owner->attacker->attack);
#endif
	int damage = 0;
	bool dodged = false;
	bool hits = false;
	bool penetrates = false;

    hits = owner->attacker->DoesItHit(attack_roll, mod, target);
	if (hits) {
		damage = owner->attacker->GetDamage(owner->attacker->mean_damage, 0, target);
		if (damage > 0) penetrates = true;
	    damage = std::min(target->destructible->hp,damage);
    } else if (attack_roll > 10) {
        dodged = true;
    }
    
	owner->attacker->Message(hits,penetrates,dodged,damage,owner,target);
	//Taking damage must happen after the message is displayed. Otherwise the
	//  messages about leveling up, killing, etc. happen before the attack
	//  messages. This would also lead to the killing blow reading
	//  "[name] deals 4 damage to [corpse]"
	if (target->destructible)
	    damage = target->destructible->takeDamage(target, damage);
    
};

/** Checks to see if a particular attack successfully hits the target.
 *
 * Landing a hit and penetrating armor are two separate things.  This function
 * only addresses hitting the target.
 *
 * @param dice - An integer representing the combat "roll", plus modifiers
 * @param target - A pointer to the actor being attacked
 * @return True if the attack successfully hits the target
 */
bool Attacker::DoesItHit(int attack_roll, int mod, Actor *target) {
	if (target->attacker) {
	    std::normal_distribution<float> dist(target->attacker->dodge-3,
                                             (target->attacker->dodge-3.0)/3);
        int dodge_roll = std::max(0,(int)dist(engine.rng));
#ifndef NDEBUG
        engine.gui->log->Print("[color=grey]The dodge roll was: %d / %d", dodge_roll, target->attacker->dodge);
#endif
        if (attack_roll > dodge_roll + mod) {
            return true;
        } else {
            return false;
        };
	} else {
		return true;
	};
};

void Attacker::Message(bool hits, bool penetrates, bool dodged, int damage, Actor *owner, Actor *target) {
	if (target->destructible) {
	    if ( dodged ) {
		    const char* temp_word = ((owner == engine.player)? "r" : "'s");
		    const char* temp_word2 = ((target == engine.player)? "dodge" : "dodges");
			engine.gui->log->Print("%s %s away from %s%s %s.",
			                       target->words->Name, temp_word2,
			                       owner->words->name, temp_word, 
			                       owner->words->weapon.c_str());
	    } else if ( penetrates ) {
		    const char* temp_word = ((owner == engine.player)? "hit" : "hits");
			engine.gui->log->Print("%s %s %s with %s %s, dealing %d damage.", 
			                       owner->words->Name,
			                       temp_word,
			                       target->words->name, 
			                       owner->words->possessive,
			                       owner->words->weapon.c_str(), damage);
		} else if ( !hits ) {
		    const char* temp_word = ((owner == engine.player)? "miss" : "misses");
			engine.gui->log->Print("%s %s %s with %s %s.",
			                       owner->words->Name, temp_word,
			                       target->words->name, 
			                       owner->words->possessive,
			                       owner->words->weapon.c_str());
		} else if ( !penetrates ) {
		    const char* temp_word = ((owner == engine.player)? "r" : "'s");
		    const char* temp_word2 = ((target == engine.player)? "r" : "'s");
			engine.gui->log->Print("%s%s attack bounces off %s%s %s.",
			                       owner->words->Name, temp_word,
			                       target->words->name, temp_word2, 
			                       target->words->armor.c_str());
		};
	} else {
		engine.gui->log->Print("%s attacks %s in vain.",
		                       owner->words->Name,
			                     target->words->name);
	};
};

int Attacker::GetDamage(int mean_damage, int mod, Actor* target) {
    std::normal_distribution<float> dist(mean_damage, mean_damage/3);
    int damage = (int)std::max(0, (int)dist(engine.rng));
#ifndef NDEBUG
        engine.gui->log->Print("[color=grey]The damage roll was: %d / %d", damage, mean_damage);
#endif
    if (target->destructible)
        damage -= target->destructible->armor;
    return damage;
};

int Attacker::GetRangeModifier(Actor* owner, Actor* target) {
    if (max_range == 0) {
        return -5;
    } else {
        float distance = owner->GetDistance(target->x, target->y);
        int modifier = int(15.0*std::log(distance)/
                           std::log(float(owner->attacker->max_range)) - 5.0);
        return modifier;
    };
        
};

void Attacker::SetAim(Actor* target) {
#ifndef NDEBUG
    engine.gui->log->Print("[color=grey]Setting aim on: %s",target->words->name);
#endif
    firing = true;
    current_target = target;
};

bool Attacker::UpdateFiring(Actor* owner) {
  if (firing) {
    int mod = GetRangeModifier(owner, current_target);
    Attack(owner, current_target, mod);
    firing = false;
    return true;
  }
  return false;
};

bool Attacker::InRange(Actor* owner, Actor* target) {
  float distance = owner->GetDistance(target->x, target->y);
  if (max_range <= 1) {
    return false;
  } else if (distance > std::min(70,owner->attacker->max_range)) {
    return false;
  } else if (distance <= 3) {
    return true;
  } else if (target->attacker) {
    int modifier = GetRangeModifier(owner, target);
    if (target->attacker->dodge + modifier < owner->attacker->attack) {
        return true;
    }
  }
  return false;
};
