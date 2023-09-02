//
//  WorldManager.cpp
//  Junk Jack
//
//  Created by Jack on 7/11/11.
//  Copyright 2012 PixBits. All rights reserved.
//

#include "WorldManager.h"

#include "pix/io/FileHandle.h"

#include "game/world/Portals.h"

#include "game/gameplay/Player.h"

#include "game/entities/Entities.h"
#include "gfx/Textures.h"
#include "game/gameplay/CraftBook.h"

#include "game/gameplay/Game.h"
#include "game/gameplay/Inventory.h"

#include "data/CustomInfo.h"
#include "data/Data.h"
#include "common/stats/Achievements.h"
#include "game/world/World.h"

#include "game/gameplay/Logic.h"
#include "common/Shop.h"
#include "common/net/Net.h"
#include "data/persistence/FileManager.h"
#include "data/persistence/Binary.h"
#include "game/map/WorldThemes.h"

#include "gfx/sprites/PlayerSprite.h"

#include "game/entities/MobEntity.h"
#include "gfx/sprites/EntitySprite.h"

#include "MobTypes.h"

#include <cstdio>
#include <cstring>

#include <type_traits>
#include <map>

using namespace std;
using namespace pix::archive;

using ChunkType = ::ChunkType;
using FileHandle = pix::FileHandle;

class Context
{
private:
  PlayerEntry* const _pentry;
  Player* const _player;
  WorldEntry* const _entry;
  World* const _world;
  DupeItemFunction dupeFunction;
public:
  Context(World* world, WorldEntry* entry, Player* player, PlayerEntry* pentry, DupeItemFunction dupeFunction) : _world(world), _entry(entry), _player(player), _pentry(pentry), dupeFunction(dupeFunction) { }
  
  Item* dupe(const Item& item) const { return dupeFunction(item); }
  DupeItemFunction dupeFunc() const { return dupeFunction; }
  TMapTile tileAt(u16 x, u16 y) const { return _world->tileAt(x, y); }
  World* world() const { return _world; }
  WorldEntry* entry() const { return _entry; }
  Player* player() const { return _player; }
  PlayerEntry* pentry() const { return _pentry; }
  
  void updateDupeFunction(DupeItemFunction dupe) { this->dupeFunction = dupe; }
};

template<typename W, typename T> class Archiver;
template<typename W, typename T> inline void sss(const W& w, const T* t) { Archiver<W,T*>::serialize(w, t); }
template<typename W, typename T> inline void sss(const W& w, const T& t) { Archiver<W,T>::serialize(w, t); }
template<typename W, typename T> inline void sss(const W& w, const Context& c, const T& t) { Archiver<W,T>::serialize(w, c, t); }


template<typename W, typename T> inline void sss(const W& w, const T& t, ::ChunkType type, ChunkVersion version)
{
  w.startChunk(type, version);
  sss(w, t);
  w.endChunk();
}

template<typename R, typename T> inline void uuu(const R& r, const Context& c, T& v) { Archiver<R,T>::unserialize(r, c, v); }
template<typename T, typename R> inline T uuu(const R& r, const Context& c) { return Archiver<R,T>::unserialize(r, c); }

template<typename W, typename T> inline void uuu(const W& w, const Context& c, T& t, ::ChunkType type) {
  if (w.seekToChunk(type))
    uuu(w, c, t);
}

#ifndef WIN32
template<typename> using void_t = void;
#endif
template<typename T, typename = void> struct has_mapped_type : std::false_type { };
template<typename T> struct has_mapped_type<T, void_t<typename T::mapped_type>> : std::true_type { };

template<typename T, typename = void> struct has_save_type : std::false_type { };
template<typename T> struct has_save_type<T, void_t<std::integral_constant<pix::archive::ChunkType, T::SAVE_TYPE>>> : std::true_type { };

template<typename W, typename T>
class Archiver
{
public:
  template<class U = T, typename enable_if<is_pod<U>::value, int>::type = 0> static void serialize(const W& w, const T& v) { w.write(v); }
  template<class U = T, typename enable_if<is_pod<U>::value, int>::type = 0> static T unserialize(const W& r, const Context& c) { T v; r.read(v); return v; }
  
  template<class U = T, typename enable_if<has_mapped_type<U>::value && has_save_type<typename U::mapped_type>::value, int>::type = 0>
  static void serialize(const W& w, const T& container)
  {
    w.startChunk(T::mapped_type::SAVE_TYPE, 0);
    u32 size = static_cast<u32>(container.size());
    w.write(size);
    for (const auto& pair : container)
    {
      Archiver<W,std::pair<typename T::key_type, typename T::mapped_type>>::serialize(w, pair);
    }
    w.endChunk();
  }
  
  template<class U = T, typename std::enable_if<has_mapped_type<U>::value && !has_save_type<typename U::mapped_type>::value, int>::type = 0>
  static void serialize(const W& w, const T& container)
  {
    u32 size = static_cast<u32>(container.size());
    w.write(size);
    for (const auto& pair : container)
    {
      Archiver<W,std::pair<typename T::key_type, typename T::mapped_type>>::serialize(w, pair);
    }
  }

  template<class U = T, typename std::enable_if<has_mapped_type<U>::value && has_save_type<typename U::mapped_type>::value && !is_base_of<require_manual_serialization, typename U::mapped_type>::value, int>::type = 0>
  static void unserialize(const W& r, const Context& c, T& container)
  {
    if (r.seekToChunk(T::mapped_type::SAVE_TYPE))
    {
      u32 size;
      r.read(size);
      for (u32 i = 0; i < size; ++i)
      {
        auto pair = Archiver<W,std::pair<typename T::key_type, typename T::mapped_type>>::unserialize(r,c);
        container[pair.first] = pair.second;
      }
    }
  }
  
  template<class U = T, typename std::enable_if<
    has_mapped_type<U>::value &&
    has_save_type<typename U::mapped_type>::value &&
    is_base_of<require_manual_serialization, typename U::mapped_type>::value, int>::type = 0>
  static void unserialize(const W& r, const Context& c, T& container)
  {
    if (r.seekToChunk(T::mapped_type::SAVE_TYPE))
    {
      u32 size;
      r.read(size);
      for (u32 i = 0; i < size; ++i)
      {
        Archiver<W,std::pair<typename T::key_type, typename T::mapped_type>>::unserialize(r,c,container);
      }
    }
  }
  
  template<class U = T, typename std::enable_if<has_mapped_type<U>::value && !has_save_type<typename U::mapped_type>::value, int>::type = 0>
  static void unserialize(const W& r, const Context& c, T& container)
  {
    u32 size;
    r.read(size);
    for (u32 i = 0; i < size; ++i)
    {
      auto pair = Archiver<W,std::pair<typename T::key_type, typename T::mapped_type>>::unserialize(r,c);
      container[pair.first] = pair.second;
    }
  }
  
  template<typename U = T, typename std::enable_if<!has_mapped_type<U>::value && has_save_type<typename U::value_type>::value, int>::type = 0>
  static void serialize(const W& w, const T& container)
  {
    w.startChunk(T::value_type::SAVE_TYPE, 0);
    u32 size = static_cast<u32>(container.size());
    w.write(size);
    for (const auto& object : container)
      Archiver<W, typename T::value_type>::serialize(w, object);
    w.endChunk();
  }
  
  template<typename U = T, typename std::enable_if<!has_mapped_type<U>::value && !has_save_type<typename U::value_type>::value && is_same<typename U::value_type,typename U::value_type>::value, int>::type = 0>
  static void serialize(const W& w, const T& container)
  {
    u32 size = static_cast<u32>(container.size());
    w.write(size);
    for (const auto& object : container)
      Archiver<W, typename T::value_type>::serialize(w, object);
  }
  
  template<typename TT> static void inline emplace(TT&& value, std::list<TT>& collection) { collection.push_back(value); }
  template<typename TT> static void inline emplace(TT&& value, std::set<TT>& collection) { collection.insert(value); }
  template<typename TT> static void inline emplace(TT&& value, std::unordered_set<TT>& collection) { collection.insert(value); }
  template<typename TT> static void inline emplace(TT&& value, std::vector<TT>& collection) { collection.push_back(value); }
  
  template<typename U = T, typename enable_if<
    !has_mapped_type<U>::value &&
    !is_base_of<require_placement_move, typename U::value_type>::value &&
    has_save_type<typename U::value_type>::value
  , int>::type = 0>
  static void unserialize(const W& r, const Context& c, T& container)
  {
    if (r.seekToChunk(T::value_type::SAVE_TYPE))
    {
      u32 size;
      r.read(size);
      for (u32 i = 0; i < size; ++i)
        emplace(Archiver<W, typename T::value_type>::unserialize(r, c), container);
    }
  }
  
  template<typename U = T, typename enable_if<
    !has_mapped_type<U>::value &&
    !is_base_of<require_placement_move, typename U::value_type>::value &&
    !has_save_type<typename U::value_type>::value
  , int>::type = 0>
  static void unserialize(const W& r, const Context& c, T& container)
  {
    u32 size;
    r.read(size);
    for (u32 i = 0; i < size; ++i)
      emplace(Archiver<W, typename T::value_type>::unserialize(r, c), container);
  }
  
  template<typename U = T, typename enable_if<
    !has_mapped_type<U>::value &&
    is_base_of<require_placement_move, typename U::value_type>::value &&
    has_save_type<typename U::value_type>::value
  , int>::type = 0>
  static void unserialize(const W& r, const Context& c, T& container)
  {
    if (r.seekToChunk(T::value_type::SAVE_TYPE))
    {
      u32 size;
      r.read(size);
      for (u32 i = 0; i < size; ++i)
        Archiver<W, typename T::value_type>::unserialize(r, c, container);
    }
  }
};

#pragma mark Common Serializers

template <typename W>
class Archiver<W, TMapTile>
{
public:
  static void serialize(const W& w, CMapTile tile)
  {
    Coordinate coord = Coordinate(tile);
    w.write(coord);
  }
  
  static TMapTile unserialize(const W& r, const Context& c)
  {
    Coordinate coord;
    r.read(coord);
    return c.tileAt(coord.x, coord.y);
  }
};

template<typename W>
class Archiver<W, char*>
{
public:
  static void serialize(const W& w, const char* string)
  {
    u16 length = static_cast<u16>(strlen(string)+1);
    w.write(length);
    w.write(string, 1, length);
  }
  
  static char* unserialize(const W& r, const Context& c)
  {
    u16 length;
    r.read(length);
    char* buffer = new char[length];
    r.read(buffer, 1, length);
    return buffer;
  }
};

template<typename W, typename T>
class Archiver<W, std::unique_ptr<T>>
{
public:
  static void serialize(const W& w, const std::unique_ptr<T>& ptr) { Archiver<W, T*>::serialize(w, ptr.get()); }
  static std::unique_ptr<T> unserialize(const W& r, const Context& c) { std::make_unique<T>(Archiver<W, T*>::unserialize(r, c)); }
};

#pragma mark Player Serializer

#pragma mark World Data Serializers

template<typename W>
class Archiver<W, World>
{
public:

  
public:
  static void serialize(const W& w, const Context& c)
  {
    w.startChunk(::ChunkType::WORLD_HEADER, 0, ChunkCompression::NONE);
    
    MapHeader header;
    memset(&header, 0, sizeof(MapHeader));
    header.version = WorldManager::GAME_VERSION;
    header.uuid = c.entry()->uuid;
    header.timestamp = static_cast<u32>(c.entry()->timestamp);
    
    header.width = c.world()->width();
    header.height = c.world()->height();
    header.options = c.entry()->options;
    
    strncpy(header.author, "author", 16);
    strncpy(header.name, c.entry()->title.c_str(), 32);

    if (w.type() == ::ArchiveType::WORLD_FILE)
    {
      header.viewport = Coordinate(c.player()->viewportX, c.player()->viewportY);
      header.spawn = Coordinate(c.player()->spawnX, c.player()->spawnY);
    }

    w.write(header);
    
    w.endChunk();
  }
  
  static bool unserialize(const W& r, const Context& c, MapHeader& header)
  {
    if (r.seekToChunk(::ChunkType::WORLD_HEADER))
    {
      r.read(header);
      return true;
    }

    return false;
  }
};

template<typename W>
class Archiver<W, Weather>
{
private:
  struct MapWeather
  {
    float poissonSum;
    WeatherType type;
    u8 poissonSkipped;
  };
public:
  static void serialize(const W& w, const Weather& weather)
  {
    MapWeather sweather = { weather.poissonSum, weather.weather, weather.poissonSkipped };
    sss(w, sweather);
  }
  
  static void unserialize(const W& r, const Context& c, Weather& weather)
  {
    MapWeather sweather = uuu<MapWeather>(r, c);
    weather.initWeather(c.world()->theme(), sweather.type, sweather.poissonSum, sweather.poissonSkipped);
    c.world()->weatherType = sweather.type;
  }
};

template<typename W>
class Archiver<W, WorldTime>
{
private:
  struct MapTime
  {
    u32 ticks;
    DayPhase phase;
  };

public:
  static void serialize(const W& w, const WorldTime& time)
  {
    MapTime stime = { time.getTicks(), time.getPhase() };
    sss(w, stime);
  }
  
  static void unserialize(const W& r, const Context& c, WorldTime& weather)
  {
    MapTime time = uuu<MapTime>(r, c);
    c.world()->time.setTime(static_cast<DayPhase>(time.phase), time.ticks);
  }
};

void SaveMapTile::storeTiles(SaveMapTile* smap, const MapTile* map, u32 t)
{
  static_assert(sizeof(SaveMapTile::Flags) == sizeof(u8), "Must be 1 byte");
  
  memset(smap, 0, sizeof(SaveMapTile)*t);
  
  for (u32 i = 0; i < t; ++i)
  {
    smap[i].top = map[i].top;
    smap[i].middle = map[i].middle;
    std::copy(begin(map[i].decos), end(map[i].decos), begin(smap[i].decos));
    smap[i].electro = map[i].electro;
    smap[i].fluid = map[i].fluid;
    smap[i].flags.spring = map[i].spring;
    smap[i].flags.rotation = map[i].rotation;
  }
}

void SaveMapTile::loadTileMap(const SaveMapTile* smap, MapTile* map, u32 w, u32 h)
{
  for (u32 x = 0; x < w; ++x)
    loadTileColumn(smap + x*h, map + x*h, x, h);
}

inline void SaveMapTile::loadTileColumn(const SaveMapTile* smap, MapTile* column, u32 x, u32 h)
{
  for (u32 y = 0; y < h; ++y)
  {
    const auto& stile = smap[y];
    auto& tile = column[y];
    
    tile.x = x;
    tile.y = y;
    
    tile.top = stile.top;
    tile.middle = stile.middle;
    
    std::copy(begin(stile.decos), end(stile.decos), begin(tile.decos));
    
    tile.electro = stile.electro;
    tile.fluid = stile.fluid;
    tile.spring = stile.flags.spring;
    tile.rotation = stile.flags.rotation;
  }
}

template<typename W>
class Archiver<W, BlockMap>
{
private:
  static constexpr ChunkVersion TILE_MAP_VERSION = 1;
  
  static void updateTileMap(ChunkVersion version, SaveMapTile* smap, u32 length)
  {
    if (version == 0)
      updateTileMapTo1(smap, length);
  }
  
  static void updateTileMapTo1(SaveMapTile* smap, u32 length)
  {
    for (u32 i = 0; i < length; ++i)
    {
      if (smap[i].top.type == static_cast<BlockType>(OldBlockType::BLOCK_WOOD_FENCE_RIGHT_DOOR_0))
      {
        smap[i].top.type = BLOCK_WOOD_FENCE_DOOR;
        smap[i].flags.rotation = ROTATION_FLIPX;
      }
    }
  }
  
public:
  static void serialize(const W& w, const BlockMap& map)
  {
    w.startChunk(::ChunkType::SKY_LINE, 0);
    auto* skyLine = map.skyLine.get();
    w.write(skyLine, sizeof(MapY), map.w);
    w.endChunk();
    
    auto* smap = new SaveMapTile[map.w*map.h];
    SaveMapTile::storeTiles(smap, map.map, map.w*map.h);
    w.startChunk(::ChunkType::TILE_MAP, TILE_MAP_VERSION, ChunkCompression::COMPRESSED, sizeof(SaveMapTile)*map.w*map.h);
    w.write(smap, sizeof(SaveMapTile), map.w*map.h);
    delete [] smap;
    w.endChunk();
    
    auto* fog = map.fogMap.get();
    
    if (fog)
    {
      FogMap::data_type* data = new FogMap::data_type[fog->size*2];
      std::copy(fog->getVisibilityData(), fog->getVisibilityData() + fog->size, data);
      std::copy(fog->getVisitedData(), fog->getVisitedData() + fog->size, data + fog->size);
      w.startChunk(::ChunkType::FOG_MAP, 0, ChunkCompression::COMPRESSED, sizeof(FogMap::data_type) * fog->size*2);
      w.write(data, sizeof(FogMap::data_type), fog->size*2);
      delete [] data;
      w.endChunk();
    }
  }
  
  enum class OldBlockType : u16
  {
    BLOCK_WOOD_FENCE_RIGHT_DOOR_0 = 2403
  };
  
  static void unserialize(const W& r, const Context& c, BlockMap& map)
  {
    if (r.seekToChunk(::ChunkType::SKY_LINE))
      r.read(map.skyLine.get(), sizeof(MapY), map.w);
    
    if (r.seekToChunk(::ChunkType::TILE_MAP))
    {
      SaveMapTile* smap = new SaveMapTile[map.w*map.h];
      r.read(smap, sizeof(SaveMapTile), map.w*map.h);
      
      ChunkVersion version = r.getVersion();
      while (version < TILE_MAP_VERSION)
      {
        updateTileMap(version, smap, map.w*map.h);
        ++version;
      }

      SaveMapTile::loadTileMap(smap, map.map, map.w, map.h);

      delete [] smap;
    }
    
    if (r.seekToChunk(::ChunkType::FOG_MAP) && map.fogMap)
    {
      auto* fog = map.fogMap.get();
      
      FogMap::data_type* data = new FogMap::data_type[fog->size*2];
      r.read(data, sizeof(FogMap::data_type), fog->size*2);
      std::copy(data, data + fog->size, fog->getVisibilityData());
      std::copy(data + fog->size, data + fog->size*2, fog->getVisitedData());
      delete [] data;
    }
  }
};

#pragma mark Chest Serializer

template<typename W>
class Archiver<W, Chest*>
{
private:
  struct SaveMapChestInfo
  {
    u32 x, y;
    u32 slots;
  };
  
public:
  static void serialize(const W& w, const Chest* chest)
  {
    SaveMapChestInfo saveChestInfo = {chest->tile->x, chest->tile->y, chest->slots};
    
    w.write(saveChestInfo);
    
    Item* items = new Item[chest->slots];

    for (u32 j = 0; j < chest->slots; ++j)
      items[j] = Item(chest->getAt(j));
  
    w.write(items, sizeof(Item), chest->slots);
    
    delete [] items;
  }
  
  static Chest* unserialize(const W& r, const Context& c)
  {
    SaveMapChestInfo saveChest;
    
    r.read(saveChest);
    
    Chest* chest = new Chest(c.tileAt(saveChest.x, saveChest.y), saveChest.slots);
    
    Item* items = new Item[chest->slots];
    r.read(items, sizeof(Item), chest->slots);
  
    for (u32 w = 0; w < chest->slots; ++w)
      chest->setAt(w, c.dupe(items[w]));
    
    delete [] items;
    
    return chest;
  }
};

#pragma mark Forge Serializer

template<typename W>
class Archiver<W, Forge*>
{
private:
  struct SaveMapForge
  {
    u16 x, y;
    Forge::Status status;
    Item items[3];
  };
  
public:
  static void serialize(const W& w, const Forge* forge)
  {
    SaveMapForge saveForge = { forge->tile->x, forge->tile->y, forge->getStatus() };
    forge->data.serialize(saveForge.items);
    w.write(saveForge);
  }
  
  static Forge* unserialize(const W& r, const Context& c)
  {
    SaveMapForge saveForge;
    r.read(saveForge);
    Forge* f = new Forge(c.tileAt(saveForge.x, saveForge.y));
    f->setStatus(saveForge.status);
    f->data.unserialize(saveForge.items, c.dupeFunc());
    return f;
  }
};

#pragma mark Sign Serializer

template<typename W>
class Archiver<W, Sign*>
{
public:
  static void serialize(const W& w, const Sign* sign)
  {
    sss(w, sign->tile);
    sss(w, sign->getText());
  }
  
  static Sign* unserialize(const W& r, const Context& c)
  {
    TMapTile tile = uuu<TMapTile>(r, c);
    char* string = uuu<char*>(r, c);
    return new Sign(tile, string);
  }
  
  void itemChanged(int index) { }
};

#pragma mark Stable Serializer

template<typename W>
class Archiver<W, Stable*>
{
private:
  struct SaveMapStableMob
  {
    StabledMob type;
    u16 data;
    u16 feed;
    
    SaveMapStableMob() : type(STABLE_MOB_NONE) { }
  };
  
  struct SaveMapStable
  {
    u16 x, y;
    SaveMapStableMob mobs[STABLE_SIZE];
    Item item;
    SaveMapStable(u16 x, u16 y, const Item& item) : x(x), y(y), item(item) { }
    SaveMapStable() = default;
  };

public:
  static void serialize(const W& w, const Stable* stable)
  {
    SaveMapStable saveStable = SaveMapStable(stable->tile->x, stable->tile->y, stable->foodSlot());

    for (int i = 0; i < STABLE_SIZE; ++i)
    {
      MobEntity *mob = stable->mobs[i];
      if (mob)
      {
        auto& slot = saveStable.mobs[i];
        slot.type = Stables::saveMobForMob(mob->mobSpec()->index);
        slot.data = mob->data;
        slot.feed = mob->feed;
      }
    }
    
    w.write(saveStable);
  }
  
  static Stable* unserialize(const W& r, const Context& c)
  {
    SaveMapStable saveStable;
    r.read(saveStable);
    
    Stable* stable = new Stable(c.tileAt(saveStable.x, saveStable.y));
    stable->setFoodSlot(c.dupe(saveStable.item));
    
    for (const auto& smob : saveStable.mobs)
    {
      if (smob.type != STABLE_MOB_NONE)
      {
        MobType mobIndex = Stables::mobForSaveMob(smob.type);
        
        if (mobIndex != MOB_NONE)
        {
          MobEntity* mob = Logic::i->spawnMob(Data::mob(mobIndex), GroupSpecType::BREED, stable->tile->position(), stable->tile, 0.15f, PI/2);
          mob->data = smob.data;
          mob->feed = smob.feed;
          stable->placeMob(mob);
        }
      }
    }
    
    return stable;
  }
};

#pragma mark AlchemyLab Serializer

template<typename W>
class Archiver<W, AlchemyLab*>
{
public:
  static void serialize(const W& w, const AlchemyLab* lab)
  {
    sss(w, lab->tile);
    Item items[AlchemyLab::SLOTS];
    lab->data.serialize(items);
    w.write(items);
  }
  
  static AlchemyLab* unserialize(const W& r, const Context& c)
  {
    TMapTile tile = uuu<TMapTile>(r, c);
    AlchemyLab* lab = new AlchemyLab(tile);
    Item items[AlchemyLab::SLOTS];
    r.read(items);
    lab->data.unserialize(items, c.dupeFunc());
    return lab;
  }
  
};

#pragma mark Shelf Serializer

template<typename W>
class Archiver<W, Shelf*>
{
private:
  static const QuarterPosition positions[4];

public:
  static void serialize(const W& w, const Shelf* shelf)
  {
    sss(w, shelf->tile);
    Item items[SHELF_MAX_ITEMS];
    
    for (size_t i = 0; i < SHELF_MAX_ITEMS; ++i)
      items[i] = Item(shelf->items[i]);
      
    w.write(items);
  }
  
  static Shelf* unserialize(const W& r, const Context& c)
  {
    TMapTile tile = uuu<TMapTile>(r,c);
    Shelf* shelf = c.world()->shelves.instantiate(tile);
    
    Item items[SHELF_MAX_ITEMS];
    r.read(items);
    for (int i = 0; i < SHELF_MAX_ITEMS; ++i)
    {
      Item* item = c.dupe(items[i]);
      if (item)
        shelf->spawnTreasure(item, positions[i]);
    }
    
    return shelf;
  }
};

template<typename W> const QuarterPosition Archiver<W, Shelf*>::positions[4] = { UL, UR, DL, DR };

#pragma mark Plant Serializer

template<typename W>
class Archiver<W, Plant>
{
private:
  struct SaveMapPlant
  {
    PlantType type;
    u16 x;
    u16 y;
    Plant::Type ptype;
    
    union
    {
      s8 growData;
      FlowerData flower;
    };
  };
  
public:
  static void serialize(const W& w, const Plant& plant)
  {
    SaveMapPlant splant = { plant.type, plant.x, plant.y, plant.ptype, { 0 } };

    if (plant.ptype == Plant::Type::NORMAL)
      splant.growData = plant.growData;
    else if (plant.ptype == Plant::Type::FLOWER)
      splant.flower = plant.flower;
    
    w.write(splant);
    
    if (plant.ptype == Plant::Type::TREE)
      sss(w, plant.tree);
  }
  
  static void unserialize(const W& r, const Context& c, std::list<Plant>& collection)
  {
    SaveMapPlant sp = { PLANT_TREE, 0, 0, Plant::Type::NORMAL, { 0 } };
    r.read(sp);
    
    if (sp.ptype == Plant::Type::TREE)
      collection.emplace_back(sp.type, sp.x, sp.y, uuu<Tree*>(r,c));
    else if (sp.ptype == Plant::Type::NORMAL)
      collection.emplace_back(sp.type, sp.x, sp.y, sp.growData);
    else if (sp.ptype == Plant::Type::FLOWER)
      collection.emplace_back(sp.x, sp.y, sp.flower);
  }
};

template<typename W>
class Archiver<W, Tree*>
{
private:
  struct SaveMapTree
  {
    u8 spec;
    u8 currentLevel;
    u8 levelsCount;
    u8 levelHeight;
    s8 currentWidthL, currentWidthR;
    u16 x, y;
    SaveMapTree(u16 x, u16 y, u8 s, u8 cl, u8 lh, u8 cwl, u8 cwr, u8 lc) :
    x(x), y(y), spec(s), currentLevel(cl), levelHeight(lh), currentWidthL(cwl), currentWidthR(cwr), levelsCount(lc) { }
    SaveMapTree() = default;
  };
  
  struct SaveMapTreeLevel
  {
    s8 widthToGoL, widthToGoR;
    s8 maxWidthL, maxWidthR;
    u16 x, y;
    SaveMapTreeLevel(s8 wl, s8 wr, s8 mwl, s8 mwr, u16 x, u16 y) : widthToGoL(wl), widthToGoR(wr), maxWidthL(mwl), maxWidthR(mwr), x(x), y(y) { }
    SaveMapTreeLevel() = default;
  };
  
public:
  static void serialize(const W& w, const Tree* tree)
  {
    SaveMapTree stree = SaveMapTree(tree->x, tree->y, tree->spec->index, tree->currentLevel, tree->levelHeight, tree->currentWidthL, tree->currentWidthR, static_cast<u8>(tree->levels.size()));
    w.write(stree);
    
    for (const auto& level : tree->levels)
    {
      SaveMapTreeLevel slevel = SaveMapTreeLevel(level.widthToGoL, level.widthToGoR, level.maxWidthL, level.maxWidthR, level.x, level.y );
      w.write(slevel);
    }
  }
  
  static Tree* unserialize(const W& r, const Context& c)
  {
    SaveMapTree stree;
    r.read(stree);
    
    const TreeSpec *spec = CustomInfo::treeSpec(static_cast<TreeType>(stree.spec));
    Tree *tree = new Tree(stree.x, stree.y, spec, false);
    tree->currentLevel = stree.currentLevel;
    tree->currentWidthL = stree.currentWidthL;
    tree->currentWidthR = stree.currentWidthR;
    tree->levelHeight = stree.levelHeight;
    
    if (tree->levelHeight > 0)
      tree->levelSpec = spec->get(tree->currentLevel-1);
    
    for (int i = 0; i < stree.levelsCount; ++i)
    {
      SaveMapTreeLevel slevel;
      r.read(slevel);
      
      TreeLevel level = TreeLevel(spec, slevel.x, slevel.y, slevel.maxWidthL, slevel.maxWidthR, false);
      level.widthToGoL = slevel.widthToGoL;
      level.widthToGoR = slevel.widthToGoR;
      
      tree->levels.push_front(level);
    }
    
    return tree;
  }
};

template<typename W>
class Archiver<W, std::pair<TMapTile,FruitBlock>>
{
public:
  static void serialize(const W& w, const pair<TMapTile,FruitBlock>& fruit)
  {
    sss(w, fruit.first);
  }
  
  static void unserialize( const W& r, const Context& c, map<TMapTile, FruitBlock>& fruitBlocks)
  {
    TMapTile tile = uuu<TMapTile>(r,c);
    const CustomFruitInfo *fruitInfo = CustomInfo::fruitInfoForBlock(tile->type());
    
    if (fruitInfo)
      fruitBlocks[tile] = FruitBlock(fruitInfo);

    //assert(fruitInfo);
  }
};

template<typename W>
class Archiver<W, Plants>
{
public:
  static void serialize(const W& w, const Plants& plants)
  {
    sss(w, plants.plants);
    sss(w, plants.fruitBlocks);
    sss(w, plants.trees.decay, ::ChunkType::LEAVES_DECAY, 0);
    //sss(w, plants.trees.decay);
  }
  
  static void unserialize(const W& r, const Context& c, Plants& plants)
  {
    uuu(r, c, plants.plants);
    uuu(r, c, plants.fruitBlocks);
    uuu(r, c, plants.trees.decay, ::ChunkType::LEAVES_DECAY);
  }
  
};

#pragma mark WorldLock Serializer
template<typename W>
class Archiver<W, WorldLock>
{
public:
  static void serialize(const W& w, const WorldLock& lock)
  {
    sss(w, lock.tile);
    w.write(lock.radius);
  }
  
  static WorldLock unserialize(const W& r, const Context& c)
  {
    TMapTile tile = uuu<TMapTile>(r, c);
    u8 radius;
    r.read(radius);
    return WorldLock(tile, radius);
  }
};

template<typename W>
class Archiver<W, WorldLocks>
{
public:
  static void serialize(const W& w, const WorldLocks& locks)
  {
    sss(w, locks.locks);
  }
  
  static void unserialize(const W& r, const Context& c, WorldLocks& locks)
  {
    uuu(r, c, locks.locks);
  }
};

#pragma mark Fluids Serializer
template<typename W>
class Archiver<W, Fluids>
{
public:
  static void serialize(const W& w, const Fluids& fluids)
  {
    sss(w, *fluids.innBuffer);
    sss(w, *fluids.extBuffer);
  }
  
  static void unserialize(const W& r, const Context& c, Fluids& fluids)
  {
    std::set<TMapTile> buffer;
    uuu(r, c, buffer);
    for (TMapTile tile : buffer)
      fluids.extBuffer->insert(tile);
    buffer.clear();
    uuu(r, c, buffer);
    for (TMapTile tile : buffer)
      fluids.extBuffer->insert(tile);
  }
};

#pragma mark Circuit Serializer
template<typename W>
class Archiver<W, ElectroOp>
{
public:
  static void serialize(const W& w, const ElectroOp& op)
  {
    sss(w, op.tile);
    w.write(op.type);
  }
  
  static ElectroOp unserialize(const W& r, const Context& context)
  {
    TMapTile tile = uuu<TMapTile>(r, context);
    CircuitOperationType type;
    r.read(type);
    return ElectroOp(tile, type);
  }
};

template<typename W>
class Archiver<W, ClockedEntry>
{
public:
  static void serialize(const W& w, const ClockedEntry& op)
  {
    sss(w, op.tile);
    w.write(op.startTicks);
    w.write(op.ticks);
  }
  
  static ClockedEntry unserialize(const W& r, const Context& context)
  {
    TMapTile tile = uuu<TMapTile>(r, context);
    u16 startTicks, ticks;
    r.read(startTicks);
    r.read(ticks);
    return ClockedEntry(tile, startTicks, ticks);
  }
};

template<typename W>
class Archiver<W, std::pair<CMapTile,Devices::DeviceData>>
{
public:
  static void serialize(const W& w, const std::pair<CMapTile,Devices::DeviceData>& op)
  {
    sss(w, op.first);
    w.write(op.second);
  }
  
  static std::pair<CMapTile,Devices::DeviceData> unserialize(const W& r, const Context& context)
  {
    CMapTile tile = uuu<TMapTile>(r, context);
    Devices::DeviceData data;
    r.read(data);
    return make_pair(tile, data);
  }
};


template<typename W>
class Archiver<W, Circuitry>
{
public:
  static void serialize(const W& w, const Circuitry& c)
  {
    sss(w, c.logic.deviceSet);
    sss(w, *c.logic.extBuffer);
    sss(w, c.logic.clockedPieces);
    sss(w, c.devices.deviceData);
  }
  
  static void unserialize(const W& r, const Context& context, Circuitry& c)
  {
    uuu(r, context, c.logic.deviceSet);
    uuu(r, context, *c.logic.extBuffer);
    uuu(r, context, c.logic.clockedPieces);
    uuu(r, context, c.devices.deviceData);
  }
};

#pragma mark Achievements Serializer
template<typename W>
class Archiver<W, Achievements>
{
private:
  using data_type = u32;
  static constexpr size_t max_count = 256;
  static constexpr size_t bits_per_data = sizeof(data_type)*8;
  static constexpr ChunkVersion VERSION = 1;
  
public:
  static void serialize(const W& w)
  {
    w.startChunk(::ChunkType::PLAYER_ACHIEVEMENTS, 1);
    data_type achievements[max_count/bits_per_data] = { 0 };
    for (const pix::Achieve* achieve = Achievements::cbegin(); achieve != Achievements::cend(); ++achieve)
    {
      if (achieve->discovered)
      {
        size_t i = achieve->index;
        achievements[i/bits_per_data] |= 1 << (i%bits_per_data);

      }
    }

    w.write(achievements);
    w.endChunk();
  }
  
  static bool unserialize(const W& r)
  {
    if (r.seekToChunk(::ChunkType::PLAYER_ACHIEVEMENTS))
    {
      if (r.getVersion() == VERSION)
      {
        data_type achievements[max_count/bits_per_data];
        r.read(achievements);
        Achievements::reset();
        
        for (s16 i = 0; i < max_count; ++i)
        {
          bool value = (achievements[i/bits_per_data] & (1 << (i%bits_per_data))) != 0;

          if (value)
          {
            pix::Achieve* achieve = Achievements::achieveForIndex(i);
            if (achieve) achieve->discovered = true;
          }
        }
      }
      else
        r.closeChunk();
      
      return true;
    }
    
    return false;
  }
};

#pragma mark Mobs Serializer
template<typename W>
class Archiver<W, Mobs>
{
private:
  struct SaveCreativeMob
  {
    u16 x, y;
    MobType index;
  };

  static inline  bool shouldBeSerialized(GroupSpecType type)
  {
    switch (type)
    {
      case GroupSpecType::CREATURE:
      case GroupSpecType::MONSTER:
      case GroupSpecType::POCKET_PET:
      case GroupSpecType::SPECIAL_MONSTER:
        return true;
      default:
        return false;
    }
  };
  
public:
  static void serialize(const W& w, const Mobs& mobs)
  {
    u32 count = 0;
    
    for (MobEntity* entity : Entities::mobs)
      if (shouldBeSerialized(entity->gtype))
        ++count;
    
    sss(w, count);
    
    for (MobEntity* entity : Entities::mobs)
      if (shouldBeSerialized(entity->gtype))
      {
        sss(w, entity->tile());
        sss(w, entity->gtype);
        sss(w, entity->spec->index);
      }
  }
  
  static void unserialize(const W& r, const Context& c, Mobs& mobs)
  {
    u32 count = uuu<u32>(r, c);
    for (u32 i = 0; i < count; ++i)
    {
      TMapTile tile = uuu<TMapTile>(r, c);
      GroupSpecType gtype = uuu<GroupSpecType>(r, c);
      MobType type = uuu<MobType>(r, c);
      Logic::i->spawnMob(Data::mob(type), gtype, tile->position(), tile, 0.0, 0, true);
    }
    mobs.refreshCounters();

  }
};

#pragma mark Collection Serializer

template<typename W, typename T>
class Archiver<W, LogicBlockContainer<T>>
{
public:
  static void serialize(const W& w, const LogicBlockContainer<T>& container)
  {
    w.startChunk(T::SAVE_TYPE, 0);
    u32 size = container.count();
    w.write(size);
    for (const auto& object : container)
      Archiver<W,T*>::serialize(w, object.get());
    w.endChunk();
  }
  
  static void unserialize(const W& r, const Context& c, LogicBlockContainer<T>& container)
  {
    if (r.seekToChunk(T::SAVE_TYPE))
    {
      u32 size;
      r.read(size);
      for (u32 i = 0; i < size; ++i)
        container.push_front(Archiver<W,T*>::unserialize(r, c));
    }
  }
};

bool WorldManager::isSavingMap = false;
bool WorldManager::isSavingChar = false;

void WorldManager::crypt(byte* data, u32 length)
{
  // omitted
}

Item* WorldManager::dupeItem(const Item& item)
{
  const ItemSpec *spec = Item::specPt(item.type);

  if (!spec || item.type == NOTHING || item.type == INVALID)
    return nullptr;

  Item* r = new Item(item);
  if (r->count == 0) r->count = 1;
  else if (r->count > MAX_STACK_COUNT) r->count = MAX_STACK_COUNT;

  if (!spec->isStackable() && !spec->hasVariants() && r->count > 1)
    r->count = 1;

  if (r->variant >= spec->data.variants)
    r->variant = spec->randomVariant();

  return r;
}

DupeItemFunction WorldManager::dupeItemFunctionForVersion(GameVersion version)
{
  /* conversion from 2.9.1.6 to 2.9.1.7 - manufact reduced */
  if (version == GameVersion::VERSION_2_9_1_0)
  {
    return [] (const Item& item)
    {
      Item* pitem = dupeItem(item);
      
      if (pitem)
      {
        constexpr size_t TIERS = 3, OLD_AMOUNT = 28, NEW_AMOUNT = 5;
        
        static const u16 manufactOldIds[][OLD_AMOUNT] = {
          {61, 62, 317, 318, 573, 574, 829, 830, 1085, 1086, 1341, 1342, 1597, 1598, 1853, 1854, 2109, 2110, 2365, 2366, 2621, 2622, 2877, 2878, 3133, 3134, 3389, 3390 },
          {58, 59, 314, 315, 570, 571, 826, 827, 1082, 1083, 1338, 1339, 1594, 1595, 1850, 1851, 2106, 2107, 2362, 2363, 2618, 2619, 2874, 2875, 3130, 3131, 3386, 3387 },
          {55, 56, 311, 312, 567, 568, 823, 824, 1079, 1080, 1335, 1336, 1591, 1592, 1847, 1848, 2103, 2104, 2359, 2360, 2615, 2616, 2871, 2872, 3127, 3128, 3383, 3384 }
        };
        
        static const ItemID manufactnewIds[][NEW_AMOUNT] = {
          { STRANGE_COPPER_MANUFACT, UNIQUE_COPPER_MANUFACT, WONDERFUL_COPPER_MANUFACT, SHINY_COPPER_MANUFACT, ANTIQUE_COPPER_MANUFACT },
          { STUNNING_SILVER_MANUFACT, PHENOMENAL_SILVER_MANUFACT, MYSTERIOUS_SILVER_MANUFACT, AMAZING_SILVER_MANUFACT, ASTONISHING_SILVER_MANUFACT },
          { UNTHINKABLE_GOLD_MANUFACT, FABULOUS_GOLD_MANUFACT, MIRACULOUS_GOLD_MANUFACT, PORTENTOUS_GOLD_MANUFACT, PRODIGIOUS_GOLD_MANUFACT }
        };
        
        for (size_t i = 0; i < TIERS; ++i)
        {
          if (std::find(begin(manufactOldIds[i]), end(manufactOldIds[i]), pitem->type) != end(manufactOldIds[i]))
          {
            pitem->type = manufactnewIds[i][C::randi(NEW_AMOUNT)];
            pitem->variant = Item::specPt(pitem->type)->randomVariant();
            break;
          }
        }
      }
      
      return pitem;
    };
  }
  
  return WorldManager::dupeItem;
}

void WorldManager::sendCompressedWorldDataDataToBeSaved(World *world)
{
  PlayerSprite::manageHoldingLightOnAllSprites(true);
  
  for (list<Explosive>::const_iterator it = world->explosives.begin(); it != world->explosives.end(); ++it)
  {
    TMapDeco deco = it->tile->findDeco(BLOCK_TNT_ARMED);
    if (deco) deco->replace(BLOCK_TNT);
  }
}

void WorldManager::restoreWorldDataToBeSaved(World *world)
{
  for (list<Explosive>::iterator it = world->explosives.begin(); it != world->explosives.end(); ++it)
  {
    TMapDeco deco = it->tile->findDeco(BLOCK_TNT);
    if (deco) deco->replace(BLOCK_TNT_ARMED);
  }
  
  PlayerSprite::manageHoldingLightOnAllSprites(false);
}

bool WorldManager::saveWorld(const FileHandle& file, WorldEntry* entry, World* world, Player* player)
{
#ifdef _DEBUG_GAME
  clock_t vclock = clock();
#endif
  
  Context context = Context(world, entry, player, nullptr, WorldManager::dupeItem);
  using Writer = pix::archive::ArchiveWriter;
  Writer writer = Writer(::ArchiveType::WORLD_FILE, "JJXM", 19, FileHandle::write, file);
  
  writer.begin();
  Archiver<Writer, World>::serialize(writer, context);
  sendCompressedWorldDataDataToBeSaved(world);
  sss(writer, *world->map);
  restoreWorldDataToBeSaved(world);
  sss(writer, world->time, ::ChunkType::TIME, 0);
  sss(writer, world->weather, ::ChunkType::WEATHER, 0);
  sss(writer, world->chests);
  sss(writer, world->forges);
  sss(writer, world->signs);
  sss(writer, static_cast<LogicBlockContainer<Stable>&>(world->stables));
  sss(writer, world->labs);
  sss(writer, static_cast<LogicBlockContainer<Shelf>&>(world->shelves));
  sss(writer, world->plants);
  sss(writer, world->locks);
  sss(writer, world->map->fluids, ::ChunkType::FLUIDS, 0);
  sss(writer, world->circuitry, ::ChunkType::CIRCUITRY, 0);
  sss(writer, world->mobs, ::ChunkType::MOBS, 0);
  writer.end();
  
  /*info.treasuresCount = 0;
  FOR_EACH_TREASURE
    if (entity->isDeathDrop())
      ++info.treasuresCount;
  FOR_EACH_TREASURE_END*/
  
  /*FOR_EACH_TREASURE
  if (entity->isDeathDrop())
  {
    SaveTreasureEntity treasure = entity->serializeForFile();
    FileUtils::wwrite(&treasure, sizeof(SaveTreasureEntity), 1, file);
  }
  FOR_EACH_TREASURE_END*/
  
  JJ_DEBUG("[SAVE] Saved world in %2.3fs",(clock()-vclock)/(float)CLOCKS_PER_SEC)
  
  return !writer.hasEncounteredError();
}

WorldEntry* WorldManager::scanWorld(const pix::Path& path, const FileHandle& in)
{
  if (!in)
    return nullptr;
  
  Context context = Context(nullptr, nullptr, nullptr, nullptr, nullptr);
  using Reader = pix::archive::ArchiveReader;
  Reader reader = Reader(in, FileHandle::read);
  bool valid = reader.load("JJXM");

  WorldEntry* entry = nullptr;
  
  if (valid && (reader.type() == ::ArchiveType::ADVENTURE_FILE || reader.type() == ::ArchiveType::WORLD_FILE))
  {
    MapHeader header;
    bool success = Archiver<Reader, World>::unserialize(reader, context, header);
    
    if (success)
    {
      entry = new WorldEntry(path, header.timestamp, header.name, header.options);
      entry->width = header.width;
      entry->height = header.height;
      entry->version = header.version;
      entry->uuid = header.uuid;
    }
  }

  reader.end();
  return entry;
}

bool WorldManager::saveAdventure(const FileHandle& file, World* world)
{
  Context context = Context(world, FileManager::entry(), nullptr, nullptr, nullptr);
  using Writer = pix::archive::ArchiveWriter;
  Writer writer = Writer(::ArchiveType::ADVENTURE_FILE, "JJXM", 2, FileHandle::write, file);

  writer.begin();
  
  Archiver<Writer, World>::serialize(writer, context);
  
  writer.startChunk(::ChunkType::PORTALS, 0);
  const vector<Portal>& portals = Portals::portals;
  u32 count = static_cast<u32>(portals.size());
  writer.write(count);
  for (const Portal& p : portals)
    writer.write(p);
  writer.endChunk();
  
  writer.end();
  
  return !writer.hasEncounteredError();
}

bool WorldManager::loadWorld(const FileHandle& file, World* world, Player *player)
{
  #ifdef _DEBUG_GAME
    clock_t vclock = clock();
  #endif
  
  file.seek(0, SEEK_SET);

  Context context = Context(world, nullptr, player, nullptr, WorldManager::dupeItem);
  using Reader = ArchiveReader;
  Reader reader = Reader(file, FileHandle::read);
  
  bool valid = reader.load(::ArchiveType::WORLD_FILE, "JJXM");
  
  if (valid)
  {
    MapHeader header;
    Archiver<Reader, World>::unserialize(reader, context, header);
    
    /* if game version is lesser than current then we update dupe item function so that it can manage modifications to the item database */
    if (header.version < GAME_VERSION)
      context.updateDupeFunction(dupeItemFunctionForVersion(header.version));
    
    
    player->viewportX = header.viewport.x;
    player->viewportY = header.viewport.y;
    player->spawnX = header.spawn.x;
    player->spawnY = header.spawn.y;

    uuu(reader, context, *world->map);
    uuu(reader, context, world->time, ::ChunkType::TIME);
    uuu(reader, context, world->weather, ::ChunkType::WEATHER);
    uuu(reader, context, world->chests);
    uuu(reader, context, world->forges);
    uuu(reader, context, world->signs);
    uuu(reader, context, static_cast<LogicBlockContainer<Stable>&>(world->stables));
    uuu(reader, context, world->labs);
    uuu(reader, context, static_cast<LogicBlockContainer<Shelf>&>(world->shelves));
    uuu(reader, context, world->plants);
    uuu(reader, context, world->locks);
    uuu(reader, context, world->map->fluids, ::ChunkType::FLUIDS);
    uuu(reader, context, world->circuitry, ::ChunkType::CIRCUITRY);
#ifndef _CHEAT_NO_MOBS
    uuu(reader, context, world->mobs, ::ChunkType::MOBS); // if creative
#endif
  }
  
  reader.end();

  bool error = reader.hasEncounteredError();
  
  if (!error)
  {
    BlockMap* map = world->map;
    map->lighting.precomputeLighting();
    //map->fluids.precompute();
    map->precomputeShadows();
  }

  JJ_DEBUG("[SAVE] Loaded world in %2.3fs",(clock()-vclock)/(float)CLOCKS_PER_SEC)

  return !error;
}

bool WorldManager::loadAdventure(const FileHandle& file)
{
  Context context = Context(nullptr, nullptr, nullptr, nullptr, WorldManager::dupeItem);
  using Reader = ArchiveReader;
  Reader reader = Reader(file, FileHandle::read);
  
  bool valid = reader.load(::ArchiveType::ADVENTURE_FILE, "JJXM");
  
  if (valid)
  {
    reader.seekToChunk(::ChunkType::PORTALS);
    u32 count;
    reader.read(count);
    Portals::portals.reserve(count);
    for (u32 i = 0; i < count; ++i)
    {
      Portal p;
      reader.read(p);
      Portals::portals.push_back(p);
    }
  }
  
  reader.end();
  
  return !reader.hasEncounteredError();
}

bool WorldManager::savePlayer(const FileHandle& file, PlayerEntry* entry, Player* player)
{
  using Writer = pix::archive::ArchiveWriter;
  Writer writer = Writer(::ArchiveType::PLAYER_FILE, "JJXC", 5, FileHandle::write, file);
  
  writer.begin();
  
  writer.startChunk(::ChunkType::PLAYER_HEADER, 0);
  SavePlayerHeader header;
  header.version = GAME_VERSION;
  header.bodyStyle = player->bodyStyle();
  strncpy(header.name, player->name, 16);
  header.themes = player->getRealThemes();
  header.flags = entry->flags;
  header.difficulty = entry->difficulty;
  header.uuid = entry->uuid;
  writer.write(header);
  writer.endChunk();
  writer.startChunk(::ChunkType::PLAYER_INVENTORY, 0);
  SavePlayerInventory inventory;
  const auto& inv = player->inventory;
  inv->quickslots.serialize(inventory.quickslots);
  inv->cquickslots.serialize(inventory.cquickslots);
  inv->miniCraft.serialize(inventory.minicraft);
  inv->bag.serialize(inventory.bag);
  inv->equip.serialize(inventory.equip);
  inv->fancyEquip.serialize(inventory.fancyEquip);
  inv->minicraftOutput.serialize(inventory.minicraftOutput);
  inv->ammoSlot.serialize(inventory.ammoSlot);
  writer.write(inventory);
  writer.endChunk();
  writer.startChunk(::ChunkType::PLAYER_CRAFTBOOK, 0);
  writer.write(player->craftBook->getBookData().getData());
  writer.write(player->craftBook->getPotionData().getData());
  writer.endChunk();
  Archiver<Writer, Achievements>::serialize(writer);
  writer.startChunk(::ChunkType::PLAYER_STATUS, 0);
  SavePlayerStatus status;
  Stats* stats = player->getStats();
  auto it = stats->begin();
  for_each(begin(status.effects), end(status.effects), [] (SavePlayerPotionEffect& buff) { buff.ticks = 0; });
  for (size_t i = 0; i < 4 && it != stats->end(); ++i, ++it)
    status.effects[i] = { it->effect, it->remainingTicks };
  status.hitPoints = player->getHealth();
  writer.write(status);
  writer.endChunk();
  writer.end();

  return !writer.hasEncounteredError();
}


bool WorldManager::loadPlayer(const FileHandle& file, Player* player)
{
  JJ_DEBUG("[LOAD] Loading player")

  using Reader = ArchiveReader;
  Reader reader = Reader(file, FileHandle::read);
  
  bool valid = reader.load(::ArchiveType::PLAYER_FILE, "JJXC");
  
  GameVersion version = GAME_VERSION;
  
  if (valid)
  {
    bool chunkFound = reader.seekToChunk(::ChunkType::PLAYER_HEADER);
    if (chunkFound)
    {
      SavePlayerHeader header;
      
      reader.read(header);
      
      player->setBodyStyle(header.bodyStyle);
      strncpy(player->name, header.name, 16);
      player->discoverTheme(header.themes);
      
      version = header.version;
    }
    
    chunkFound = chunkFound && reader.seekToChunk(::ChunkType::PLAYER_INVENTORY);
    if (chunkFound)
    {
      SavePlayerInventory info;
      const auto& inv = player->inventory;

      reader.read(info);
      
      const DupeItemFunction& dupe = dupeItemFunctionForVersion(version);
      const auto dupeShop = [&dupe] (const Item& item) { return Shop::blockPremiumItem(item.type) ? nullptr : dupe(item); };
      
      if (!reader.hasEncounteredError())
      {
        inv->quickslots.unserialize(info.quickslots, dupeShop);
        inv->cquickslots.unserialize(info.cquickslots, dupe);
        inv->miniCraft.unserialize(info.minicraft, dupeShop);
        inv->bag.unserialize(info.bag, dupeShop);
        inv->equip.unserialize(info.equip, dupeShop);
        inv->fancyEquip.unserialize(info.fancyEquip, dupeShop);
        inv->minicraftOutput.unserialize(info.minicraftOutput, dupeShop);
        inv->ammoSlot.unserialize(info.ammoSlot, dupeShop);
      }
    }
    
    chunkFound = chunkFound && reader.seekToChunk(::ChunkType::PLAYER_CRAFTBOOK);
    if (chunkFound)
    {
      reader.read(player->craftBook->getBookData().getData());
      reader.read(player->craftBook->getPotionData().getData());
      player->craftBook->computeStatus();
    }
    
    chunkFound = chunkFound && Archiver<Reader, Achievements>::unserialize(reader);
    
    chunkFound = chunkFound && reader.seekToChunk(::ChunkType::PLAYER_STATUS);
    if (chunkFound)
    {
      SavePlayerStatus status;
      reader.read(status);

      player->setHealth(status.hitPoints);

      for_each(begin(status.effects), end(status.effects), [player] (SavePlayerPotionEffect& buff) {
        if (buff.ticks != 0)
          player->getStats()->addEffectRaw(buff.effect, buff.ticks);
      });
    }
    
    reader.end();
    
    bool valid = !reader.hasEncounteredError();
    
    if (valid)
    {
      player->PlayerBase::updateStats();
      if (player->getHealth() == 0.0f)
        player->setHealth(player->getMaxHealth());
    }
    
    return valid;
  }
  
  return false;
}

PlayerEntry* WorldManager::scanPlayer(const pix::Path &path, const FileHandle &file)
{
  using Reader = ArchiveReader;
  Reader reader = Reader(file, FileHandle::read);
  
  bool valid = reader.load(::ArchiveType::PLAYER_FILE, "JJXC");
  
  PlayerEntry* entry = nullptr;
  
  if (valid)
  {
    if (reader.seekToChunk(::ChunkType::PLAYER_HEADER))
    {
      SavePlayerHeader header;
      
      reader.read(header);
      
      if (!reader.hasEncounteredError())
      {
        #ifdef _CHEAT_ALL_THEMES
          header.themes = THEME_ALL;
        #endif
        
        entry = new PlayerEntry(path, header.name, header.themes);
        entry->setBodyStyle(header.bodyStyle);
        entry->flags = header.flags;
        entry->difficulty = header.difficulty;
        entry->uuid = header.uuid;
      }
    }
    
    if (entry)
    {
      if (reader.seekToChunk(::ChunkType::PLAYER_INVENTORY))
      {
        SavePlayerInventory inventory;
        reader.read(inventory);
        
        bool canSwitchToSimple = true;
        for (int i = 0; i < 3; ++i)
          for (int j = 0; j < 3; ++j)
            if (inventory.minicraft[i][j].type != NOTHING && inventory.minicraft[i][j].type != 0)
              canSwitchToSimple = false;
        
        if (inventory.minicraftOutput.type != NOTHING && inventory.minicraftOutput.type != 0)
          canSwitchToSimple = false;
        
        entry->canSwitchToSimple = canSwitchToSimple;
        
        for (int i = 0; i < EQUIP_PIECES; ++i)
          entry->setEquip(i, inventory.fancyEquip[i].valid() ? inventory.fancyEquip[i] : inventory.equip[i]);
        
        entry->setItem(inventory.quickslots[0]);
      }
      
      if (reader.seekToChunk(::ChunkType::PLAYER_CRAFTBOOK))
      {
        CraftBook* craftbook = new CraftBook();
        reader.read(craftbook->getBookData().getData());
        reader.read(craftbook->getPotionData().getData());
        entry->setCraftbook(craftbook);
      }
    }
  }
  
  reader.end();

  if (reader.hasEncounteredError())
  {
    delete entry;
    entry = nullptr;
  }
  
  
  
  return entry;
}

/*

skyline
tilemap
chests
forges
shelves
plants
tree leaves
signs
fruits
stables
fluids inn buffer
fluids ext buffer
world locks
creative mobs
labs
circuitry



TILE_MAP = 0,
SKY_LINE, *
CHESTS, *
FORGES, *
SHELVES,
PLANTS,
TREE_LEAVES,
SIGNS,
FRUITS,
STABLES,
FLUIDS,
LOCKS,
MOBS,
LABS,
CIRCUITS

*/
