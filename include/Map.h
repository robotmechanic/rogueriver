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

#ifndef INCLUDE_MAP_H_
#define INCLUDE_MAP_H_

#include "River.h"
#include "Color.h"
#include "Actor.h"

#include "BearLibTerminal.h"

struct Position {
    int x, y;
    Position() : x(0), y(0) {};
    Position(int x, int y) : x(x), y(y) {};
};

struct Panel {
    Position tl_corner;
    Position br_corner;
    int width, height;
    Panel(Position tl_corner, Position br_corner)
        : tl_corner(tl_corner), br_corner(br_corner) {};
    Panel() {};
    void Update(int x1, int y1, int x2, int y2) {
        tl_corner.x = x1; tl_corner.y = y1;
        br_corner.x = x2; br_corner.y = y2;
        width = x2 - x1 + 1;
        height = y2 - y1 + 1;
    };
};

struct Tile {
    bool canWalk;
    float vel, u, v;
    Color color;
    Tile() : canWalk(true), vel(0.0) {};
};

class Map {
 protected:
  Color beach_color, water_color, bg_color, rock_color;
  std::vector<Tile> tiles;
  River* river;
  void AddMonster(int x, int y);
  void AddWeapon(int x, int y);
  void AddArmor(int x, int y);
  void PlaceItems();
  void PlaceMonsters();
  void PlaceRocks();
  
  void SetWall(int x, int y);
  bool inBounds(int x, int y) const;
  void SetColors();
 public:
   enum MonsterType {
      GHOST,
      SKELETON,
      GHOUL,
      CENTAUR,
      HARPY,
      STYMP,
      GIANT,
      CYCLOPS,
      CHIMERA,
      MANTICORE,
      DRAGON,
      CERBERUS,
      THANATOS
  };
  enum ItemType {
    SHORTBOW,
    JAVELIN,
    LONGBOW,
    ARTEMIS,
    LEATHER,
    BRONZE,
    ADAMANT,
    ACHILLES
  };
  Actor* CreateMonster(MonsterType monster_type, int x, int y);
  Actor* CreateItem(ItemType item_type, int x, int y);
  int width, height;
  Position camera;

  Map(int width, int height);
  ~Map();
  void Init(bool withActors);
  bool isWall(int x, int y) const;
  bool isWater(int x, int y) const;
  bool isBeach(int x, int y) const;
  bool isRock(int x, int y) const;
  Position GetPlayerStart() const;
  float GetUVelocity(int x, int y) const;
  float GetVVelocity(int x, int y) const;
  bool CanWalk(int x, int y) const;
  void Render(Panel panel, Position* camera) const;
};



#endif /* INCLUDE_MAP_H_ */
