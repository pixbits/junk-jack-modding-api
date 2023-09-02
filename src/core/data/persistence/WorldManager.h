//
//  WorldManager.h
//  Junk Jack
//
//  Created by Jack on 7/11/11.
//  Copyright 2012 PixBits. All rights reserved.
//

#ifndef _WORLD_MANAGER_H_
#define _WORLD_MANAGER_H_

#include "data/TreasureTypes.h"
#include "game/entities/Entity.h"

#include <list>
#include <set>

class Player;
class BlockMap;
class Shelves;
class World;
enum DifficultyId : u8;
enum StabledMob : s16;
enum MobType : s16;

enum class GameVersion : u32
{
  VERSION_NONE    = 0,
  VERSION_2_4_9_0 = 24900,
  VERSION_2_9_1_0 = 29100,
  VERSION_3_0_0_0 = 30000,
  
  VERSION_LAST = VERSION_3_0_0_0
};

#if defined(ENABLE_NET_CODE_PACKING)
#pragma pack(push, 1)
#endif

struct SaveMapTile
{
  struct Flags
  {
    BlockRotation rotation : 3;
    bool spring : 1;
    bool locked : 1;
  };
  
  MapBlock top, middle;
  MapDeco decos[MAX_DECOS];
  Electro electro;
  MapFluid fluid;
  Flags flags;
  
  static void storeTiles(SaveMapTile* smap, const MapTile* map, u32 t);
  static void loadTileMap(const SaveMapTile* smap, MapTile* map, u32 w, u32 h);
  static void loadTileColumn(const SaveMapTile* smap, MapTile* column, u32 x, u32 h);

};

#if defined(ENABLE_NET_CODE_PACKING)
#pragma pack(pop)
#endif

struct MapHeader
{
  uuid_identifier uuid;
  u32 timestamp;
  GameVersion version;
  char name[32];
  char author[16];
  u16 width;
  u16 height;
  Coordinate viewport;
  Coordinate spawn;
  WorldOptions options;
  u32 padding[32];
};

struct SavePlayerPotionEffect
{
  PotionEffect effect;
  u16 ticks;
  SavePlayerPotionEffect(PotionEffect effect, u16 ticks) : effect(effect), ticks(ticks) { }
  SavePlayerPotionEffect() = default;
};

/*struct PlayerInfo
{
  char name[16];
  u16 themes;
  BodyStyle bodyStyle;
};*/

struct SavePlayerHeader
{
  uuid_identifier uuid;
  char name[16];
  GameVersion version;
  u32 themes;
  u32 flags;
  BodyStyle bodyStyle;
  DifficultyId difficulty;
};

struct SavePlayerInventory
{
  Item quickslots[QUICKSLOT_MAX_COUNT];
  Item cquickslots[QUICKSLOT_MAX_COUNT];
  Item minicraft[CRAFT_WIDTH][CRAFT_HEIGHT];
  Item bag[BAG_WIDTH][BAG_HEIGHT];
  Item equip[EQUIP_PIECES];
  Item fancyEquip[EQUIP_PIECES];
  Item minicraftOutput;
  Item ammoSlot;
};

struct SavePlayerStatus
{
  float hitPoints;
  SavePlayerPotionEffect effects[4];
};

struct SaveTreasureEntity
{
  Item item;
  EntityData data;
  u16 x, y;
};

struct WorldEntry;
class PlayerEntry;

using DupeItemFunction = std::function<Item*(const Item&)>;

template<typename T> class LogicBlockContainer;

class WorldManager
{
private:
  static bool isSavingMap, isSavingChar;

  static Item* dupeItem(const Item& item);
  static DupeItemFunction dupeItemFunctionForVersion(GameVersion version);

  //static SaveMapTile saveTileFromTile(TMapTile);
  //static void tileFromSaveTile(SaveMapTile saveTile, TMapTile tile);
  static void sendCompressedWorldDataDataToBeSaved(World* world);
  static void restoreWorldDataToBeSaved(World* world);
  

  static PlayerEntry* scanPlayer(const pix::Path& path, const pix::FileHandle& file);
  static bool savePlayer(const pix::FileHandle&, PlayerEntry* entry, Player* player);
  static bool saveWorld(const pix::FileHandle&, WorldEntry *entry, World* world, Player* player);
  static bool saveAdventure(const pix::FileHandle&, World* world);

  static WorldEntry* scanWorld(const pix::Path& path, const pix::FileHandle& file);
  static bool loadPlayer(const pix::FileHandle&, Player* player);
  static bool loadWorld(const pix::FileHandle&, World* world, Player* player);
  static bool loadAdventure(const pix::FileHandle&);

  static void crypt(byte* data, u32 length);


public:

  friend class FileManager;

  static const GameVersion GAME_VERSION = GameVersion::VERSION_3_0_0_0;
};

#endif
