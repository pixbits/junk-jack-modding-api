//
//  NetProtocol.h
//  Junk Jack
//
//  Created by Jack on 3/14/12.
//  Copyright (c) 2012 Pixbits. All rights reserved.
//
#ifndef _NET_PROTOCOL_H_
#define _NET_PROTOCOL_H_

#define PROTOCOL_VERSION (0x01)

#include "pix/utils/BitSet.h"

#include "game/entities/Entity.h"
#include "data/TreasureTypes.h"
#include "game/world/Forges.h"
#include "game/gameplay/Alchemy.h"
#include "game/world/circuits/Circuitry.h"

enum Sound : s16;
enum MobType : s16;
enum class FacingDirection : s8;
enum DifficultyId : u8;
enum class GameVersion : u32;

namespace stats
{
  enum Statistic : u16;
}

enum eClientState
{
  CLIENT_STATE_NONE = 0,
  CLIENT_STATE_LOBBY = 1,
  CLIENT_STATE_CONNECTED = 2,
  CLIENT_STATE_WORLD_RECEIVED = 3,
  CLIENT_STATE_LIST_RECEIVED = 4,
  CLIENT_STATE_READY = 5
};

enum eWaitState
{
  WAIT_STATE_NONE,
  WAIT_STATE_RESTORE,
  
  WAIT_STATE_CLIENT_LOGIN,
  WAIT_STATE_SERVER_RESIGN
};

enum PacketType : u8
{
  NET_MANAGE = 0,
  NET_LOBBY,
  NET_PLAYER,
  NET_WORLD_DATA,
  NET_WORLD,
  NET_ENTITY,
  NET_EFFECT,
  
  NET_CHEST,
  NET_STABLE,
  NET_SIGN,
  NET_FORGE,
  NET_TRADE,
  NET_ALCHEMY,
  NET_FISHING,
  NET_DEVICE,
  
  NET_LOGIC_BLOCK,
  
  NET_MULTI_PART = 0xFF
};

enum MPRole : u8
{
  ROLE_CLIENT,
  ROLE_SERVER,
  ROLE_MISSING_PLAYER
};

enum class ManageProtocol : u8
{
  SERVER_IS_FULL,
  DIFFERENT_GAME_VERSION
};

enum PacketSubType : u8
{
  NET_MANAGE_MASTER,
  NET_MANAGE_ROLE_CHECK,
  NET_MANAGE_LOGIN,
  NET_MANAGE_LOGIN_RESPONSE,
  NET_MANAGE_LOGOUT,
  NET_MANAGE_CLIENT_READY,
  NET_MANAGE_PLAYER_LIST_REQUEST,
  NET_MANAGE_PLAYER_LIST_ENTRY,
  NET_PLACEHOLDER1,
  NET_MANAGE_WORLD_REQUEST,
  NET_MANAGE_WORLD_DATA_RECEIVED,
  NET_MANAGE_WAIT_STATE,
  NET_MANAGE_PROTOCOL,
  
  NET_LOBBY_ENTRY,
  NET_LOBBY_CHOICE,
  
  NET_PLAYER_SPAWN,
  NET_PLAYER_SPAWN_SET,
  NET_PLAYER_INITIAL_POSITION,
  NET_PLAYER_WARP,
  NET_PLAYER_EXITED,
  NET_PLAYER_MOVEMENT,
  NET_PLAYER_UPDATE_ITEM,
  NET_PLAYER_UPDATE_EQUIP,
  NET_PLAYER_UPDATE_HEAD,
  NET_PLAYER_THROW_ITEM,
  NET_PLAYER_SHOOT_ITEM,
  NET_PLAYER_TOGGLED_BLOCK,
  NET_PLAYER_SHELF_PLACE,
  NET_PLAYER_CONSUME_ITEM,
  NET_PLAYER_CONSUME_TOOL,
  NET_PLAYER_CONSUME_AMMO,
  NET_PLAYER_USE_ITEM,
  NET_PLAYER_BUFF_ADD,
  NET_PLAYER_BUFF_REM,
  NET_PLAYER_BUFF_ADD_RAW,
  NET_PLAYER_BUFF_REM_RAW,
  NET_PLAYER_CAPTURE_MOB,
  NET_PLAYER_ATTACK,
  NET_PLAYER_BREATHE_UPDATE,
  NET_PLAYER_HURT,
  NET_PLAYER_HEAL,
  NET_PLAYER_DEATH,
  NET_PLAYER_DEATH_EFFECT,
  NET_PLAYER_CHAT,
  NET_PLAYER_CONSOLE,
  NET_PLAYER_MESSAGE,
  NET_PLAYER_CREATIVE_MOVEMENT,
  NET_PLAYER_STAT_INCREASE,
  NET_PLAYER_SET_STATUS,
  NET_PLAYER_TRADE,
  NET_PLAYER_SLEEP_START,
  NET_PLAYER_SLEEP_CANCEL,
  NET_PLAYER_SLEEP_OPEN_DIALOG,
  NET_PLAYER_PORTAL_TRAVEL,
  NET_PLAYER_PORTAL_TRAVEL_INIT,
  NET_PLAYER_PUSH_BARE,
  NET_PLAYER_PET_SUMMON,
  NET_PLAYER_PUSH,
  NET_PLAYER_CRAFTBOOK_DATA,
  NET_PLAYER_CRAFTBOOK_DISCOVER,
  
  NET_WORLD_EDIT,
  NET_WORLD_FLUID_INIT,
  NET_WORLD_FIRE_IGNITED,
  NET_WORLD_FIRE_EXTINGUISHED,
  NET_WORLD_FIRE_INIT,
  NET_WORLD_INPUT_DEVICE_STATE,
  NET_WORLD_CIRCUIT_INIT,
  
  NET_WORLD_INFO,
  NET_WORLD_TIME,
  NET_WORLD_TIME_SET,
  NET_WORLD_COLUMN,
  NET_WORLD_COMPRESSED_DATA,
  NET_WORLD_TIME_TICK,
  NET_WORLD_WEATHER_CHANGED,
  NET_WORLD_TIME_STOP,
  NET_WORLD_FOG_MAP,
  NET_WORLD_SKY_LINE,
  
  NET_ENTITY_SPAWN,
  NET_ENTITY_DELETE,
  NET_ENTITY_PUSH,
  NET_ENTITY_PUSH_PET,
  NET_ENTITY_FALL,
  NET_ENTITY_JUMP,
  NET_ENTITY_WALK,
  NET_ENTITY_SHOOT,
  NET_ENTITY_STOP,
  NET_ENTITY_WARP_PET,
  NET_ENTITY_BOBBER_STATUS,
  NET_ENTITY_FISH_RESET,
  NET_ENTITY_MESSAGE,
  NET_MOB_STATUS,
  
  NET_ENTITY_GATHER,
  NET_ENTITY_GATHER_SUCCESS,
  NET_ENTITY_GATHER_FAILURE,
  
  NET_EFFECT_PLAYER,
  NET_EFFECT_TILE,
  NET_EFFECT_ENTITY,
  NET_EFFECT_SOUND,
  
  NET_CHEST_OPEN,
  NET_CHEST_SET,
  NET_CHEST_SWITCH_SHEET,
  
  NET_STABLE_OPEN,
  NET_STABLE_SET,
  
  NET_SIGN_OPEN,
  NET_SIGN_SET,
  NET_SIGN_ENGRAVE,
  
  NET_FORGE_OPEN,
  NET_FORGE_SET,
  NET_FORGE_STATUS,
  NET_FORGE_UPDATE,
  
  NET_LOGIC_BLOCK_CLOSE,
  
  NET_TRADE_START_REQUEST,
  NET_TRADE_REQUEST,
  NET_TRADE_RESPONSE,
  NET_TRADE_REFUSE,
  NET_TRADE_BUSY,
  NET_TRADE_OPEN,
  NET_TRADE_CLOSE,
  NET_TRADE_ADD_ITEM,
  NET_TRADE_REMOVE_ITEM,
  NET_TRADE_CONFIRM,
  NET_TRADE_UNCONFIRM,
  NET_TRADE_COMPLETING,
  NET_TRADE_CAN_COMPLETE,
  NET_TRADE_CANT_COMPLETE,
  NET_TRADE_COMPLETE,
  NET_TRADE_FAILED,
  
  NET_ALCHEMY_OPEN,
  NET_ALCHEMY_SET,
  NET_ALCHEMY_CRAFT_REQUEST,
  NET_ALCHEMY_CRAFT_DONE,
  
  NET_FISHING_CANCEL,
  NET_FISHING_START,
  NET_FISHING_ACTION,
  NET_FISHING_HOOKED,
  
  NET_DEVICE_CONFIG_REQUEST,
  NET_DEVICE_CONFIG_UPDATE
};

struct WorldEdit;

#pragma mark Management

#if defined(ENABLE_NET_CODE_PACKING)
#pragma pack(push, 1)

struct NetItem
{
  ItemDataNative data;
  ItemID type;
  u16 count;
  s16 uses;
  u8 variant;
  
  NetItem() = default;
  NetItem(const Item* item)
  {
    if (item)
    {
      this->type = item->type;
      this->variant = item->variant;
      this->count = item->count;
      this->uses = item->uses;
      this->data = item->data.data;
    }
    else
      this->type = NOTHING;
  }
  
  NetItem(const Item& item) : type(item.type), variant(item.variant), count(item.count), uses(item.uses), data(item.data.data) { }
};
#else
using NetItem = Item;
#endif

struct NetHeader
{
  PacketType type;
  PacketSubType subType;
  
  NetHeader() { }
  
  NetHeader(PacketType type, PacketSubType subtype) : type(type), subType(subtype) { }
};

struct NetEntitySpawn
{
  const NetHeader header{NET_ENTITY, NET_ENTITY_SPAWN};
  ItemDataNative vdata;
  EntityId ident;
  EntityData data;
  u16 type;
  u8 variant; 
  bool paused;
  
  NetEntitySpawn() { paused = false; }
};

struct NetManage
{
  NetHeader header;
  u16 data;
  
  NetManage(PacketSubType subType, u16 d) : header(NET_MANAGE, subType), data(d) { }
};

struct NetManageProtocol
{
  const NetHeader header{ NET_MANAGE, NET_MANAGE_PROTOCOL };
  const ManageProtocol data;
  
  NetManageProtocol(ManageProtocol data) : data(data) { }
};

struct NetManageWaitState
{
  const NetHeader header{NET_MANAGE, NET_MANAGE_WAIT_STATE};
  const u8 state;
  
  NetManageWaitState(eWaitState state) : state(state) { }
};

struct NetMasterPacket
{
  const NetHeader header{NET_MANAGE, NET_MANAGE_MASTER};
  const GameVersion version;
  
  NetMasterPacket(GameVersion v) : version(v) { }
};

struct NetMasterRoleCheck
{
  const NetHeader header{NET_MANAGE, NET_MANAGE_ROLE_CHECK};
  const MPRole role;
  const u8 player;
  
  NetMasterRoleCheck(MPRole role, u8 p) : role(role), player(p) { }
};

struct NetLoginRequest
{
  const NetHeader header{NET_MANAGE, NET_MANAGE_LOGIN};
  PlayerID player;
  char name[32];
  const GameVersion version;
  
  NetLoginRequest(const char *n, GameVersion version) : version(version) { strncpy(name,n,32); }
};

struct NetClientReady
{
  const NetHeader header{NET_MANAGE, NET_MANAGE_CLIENT_READY};
  const u8 maxHealth;
  const u8 health;
  
  NetClientReady(u8 h, u8 mh) : maxHealth(mh), health(h) { }
};

struct NetManagePlayerEntry
{
  const NetHeader header{NET_MANAGE, NET_MANAGE_PLAYER_LIST_ENTRY};
  const PlayerID ident;
  bool itself;
  char name[32];
  
  NetManagePlayerEntry(PlayerID ident, const char *n) : ident(ident), itself(false) { strncpy(name,n,32); }
};

#pragma mark World

struct NetWorldInfo
{
  const NetHeader header{NET_WORLD_DATA, NET_WORLD_INFO};
  u16 width, height;
  u16 spawnX, spawnY;
  u16 viewportX, viewportY;
  u32 ticks;
  DayPhase dayPhase;
  bool timeRunning;
  WeatherType weather;
  WorldThemeId theme;
  DifficultyId difficulty;
  WorldOptions options;
  u32 dataLength;
  
  NetWorldInfo() { }
};

struct NetWorldTime
{
  const NetHeader header{NET_WORLD_DATA, NET_WORLD_TIME};
  const DayPhase phase;
  const u16 ticks;
  
  NetWorldTime(DayPhase phase, u16 ticks) : phase(phase), ticks(ticks) { }
};

struct NetWorldTimeSet
{
  const NetHeader header{NET_WORLD_DATA, NET_WORLD_TIME_SET};
  const DayPhase phase;
  const u32 ticks;
  
  NetWorldTimeSet(DayPhase phase, u32 ticks) : phase(phase), ticks(ticks) { }
};

struct NetWorldTimeStop
{
  const NetHeader header{NET_WORLD,NET_WORLD_TIME_STOP};
  const bool stop;
  
  NetWorldTimeStop(const bool stop) : stop(stop) { }
};

struct NetWorldColumn
{
  const NetHeader header{NET_WORLD_DATA, NET_WORLD_COLUMN};
  const u16 column;

  NetWorldColumn(u16 column) : column(column) { }
};

struct NetWorldCompressedData
{
  const NetHeader header{NET_WORLD_DATA, NET_WORLD_COMPRESSED_DATA};
  const u32 length;
  
  NetWorldCompressedData(u32 length) : length(length) { }
};

struct NetWorldEdit
{
  NetHeader header{NET_WORLD, NET_WORLD_EDIT};
  u8 player;
  WorldEdit info;
  
  NetWorldEdit() { }
};

struct NetWorldEditBulk
{
  NetHeader header;
  u8 player;
  
  NetWorldEditBulk(PacketSubType type) { header.type = NET_WORLD; header.subType = type; }
};

struct NetWorldFluidInit
{
  const NetHeader header{NET_WORLD, NET_WORLD_FLUID_INIT};
  const u16 count;
  
  NetWorldFluidInit(u16 count) : count(count) { }
};

struct NetElectroOp
{
  Coordinate tile;
  CircuitOperationType type;
};

struct NetClockedPiece
{
  Coordinate tile;
  u16 ticks;
  u16 startTicks;
};

struct NetDeviceData
{
  Coordinate tile;
  Devices::DeviceData data;
};

struct NetWorldCircuitInit
{
  const NetHeader header{NET_WORLD, NET_WORLD_CIRCUIT_INIT};
  const u32 deviceSet;
  const u32 extBuffer;
  const u32 clockedPieces;
  const u32 deviceData;
  
  NetWorldCircuitInit(u32 deviceSet, u32 extBuffer, u32 clockedPieces, u32 deviceData) : deviceSet(deviceSet), extBuffer(extBuffer), clockedPieces(clockedPieces), deviceData(deviceData) { }
};

struct NetWorldDeviceInputState
{
  const NetHeader header{NET_WORLD, NET_WORLD_INPUT_DEVICE_STATE};
  const u16 x;
  const u16 y;
  const ProvidedStatus status;
  
  NetWorldDeviceInputState(TMapTile tile, ProvidedStatus status) : x(tile->x), y(tile->y), status(status) { }
};

struct NetWorldFireInit
{
  const NetHeader header{NET_WORLD, NET_WORLD_FIRE_INIT};
  const u16 count;
  
  NetWorldFireInit(u16 count) : count(count) { }
};

struct NetWorldFireIgnited
{
  const NetHeader header{NET_WORLD, NET_WORLD_FIRE_IGNITED};
  const u16 x;
  const u16 y;
  
  NetWorldFireIgnited(TMapTile tile) : x(tile->x), y(tile->y) { }
};

struct NetWorldFireExtinguished
{
  const NetHeader header{NET_WORLD, NET_WORLD_FIRE_EXTINGUISHED};
  const u16 x;
  const u16 y;
  
  NetWorldFireExtinguished(TMapTile tile) : x(tile->x), y(tile->y) { }
};

struct NetWorldWeatherChanged
{
  const NetHeader header{NET_WORLD, NET_WORLD_WEATHER_CHANGED};
  const WeatherType type;
  
  NetWorldWeatherChanged(WeatherType type) : type(type) { }
};

#pragma mark Entities

template<typename Data, PacketSubType TYPE>
struct NetEntityMessage
{
  const NetHeader header{NET_ENTITY, TYPE};
  const EntityId ident;
  const SimplePoint position;
  const Data data;
  
  NetEntityMessage(EntityId idd, SimplePoint pos, const Data& data) : ident(idd), data(data), position(pos) { }
};
  
using NetEntityPush = NetEntityMessage<EntityPushData, NET_ENTITY_PUSH>;
using NetEntityFall = NetEntityMessage<EntityFallData, NET_ENTITY_FALL>;

struct NetEntityJump
{
  const NetHeader header{NET_ENTITY, NET_ENTITY_JUMP};
  const EntityId ident;
  const SimplePoint accel;
  const SimplePoint position;
  
  NetEntityJump(EntityId ident, SimplePoint acc, SimplePoint pos) : ident(ident), accel(acc), position(pos) { }
};

struct NetEntityPushPet
{
  const NetHeader header{NET_ENTITY, NET_ENTITY_PUSH_PET};
  const bool ignoreBounce;
  const bool ignoreTile;
  const EntityId ident;
  const SimplePoint position;
  const float magnitude;
  const float angle;
  const u16 x, y;
  
  NetEntityPushPet(EntityId idd, SimplePoint pos, float magnitude, float angle, bool ignoreBounce, TMapTile ignoreTile) : ident(idd), ignoreBounce(ignoreBounce), ignoreTile(ignoreTile != nullptr), magnitude(magnitude), angle(angle), position(pos),
  x(ignoreTile != nullptr ? ignoreTile->x : 0), y(ignoreTile != nullptr ? ignoreTile->y : 0) { }
};

struct NetEntityWarpPet
{
  const NetHeader header{NET_ENTITY, NET_ENTITY_WARP_PET};
  const EntityId ident;
  const u16 x, y;
  
  NetEntityWarpPet(EntityId ident, TMapTile tile) : ident(ident), x(tile->x), y(tile->y) { }
};

struct NetEntityDelete
{
  const NetHeader header{NET_ENTITY, NET_ENTITY_DELETE};
  const u16 count;
  
  NetEntityDelete(u16 count) : count(count)
  {
    static_assert(sizeof(NetEntityDelete) == 4, "Must be 4 bytes");
  }
};

struct NetEntityShoot
{
  const NetHeader header{NET_ENTITY,NET_ENTITY_SHOOT};
  const EntityId ident;
  const SimplePoint position;
  const float waitTime;
  const FacingDirection direction;
  
  NetEntityShoot(EntityId idd, SimplePoint pos, float time, FacingDirection dir) : ident(idd), position(pos), waitTime(time), direction(dir) { }
};

struct NetEntityStop
{
  const NetHeader header{NET_ENTITY, NET_ENTITY_STOP};
  const EntityId ident;
  
  NetEntityStop(EntityId idd) : ident(idd) { }
};

struct NetMobStatus
{
  const NetHeader header{NET_ENTITY, NET_MOB_STATUS};
  const EntityId ident;
  const u8 status;
  
  NetMobStatus(EntityId idd, u8 s) : ident(idd), status(s) { }
};
  
struct NetBobberStatus
{
  NetHeader header{NET_ENTITY, NET_ENTITY_BOBBER_STATUS};
  BobberStatus status;
  EntityId ident;
  SimplePoint position;
  SimplePoint bound;
  
  NetBobberStatus() { }
  NetBobberStatus(EntityId ident, BobberStatus status, SimplePoint pos, SimplePoint bound) : ident(ident), status(status), position(pos), bound(bound) { }
};

struct NetFishReset
{
  const NetHeader header{NET_ENTITY, NET_ENTITY_FISH_RESET};
  const u16 x, y;
  
  NetFishReset(TMapTile tile) : x(tile->x), y(tile->y) { }
};
  

#pragma mark Gather

struct NetEntityGather
{
  const NetHeader header{NET_ENTITY, NET_ENTITY_GATHER};
  const EntityId ident;
  const NetItem item;
  const bool realEntity;
  
  NetEntityGather(EntityId ident, const Item& item, bool real) : ident(ident), item(item), realEntity(real) { }
};

struct NetEntityGatherSuccess
{
  const NetHeader header{NET_ENTITY, NET_ENTITY_GATHER_SUCCESS};
  const EntityId ident;

  NetEntityGatherSuccess(EntityId idd) : ident(idd) { }
};

struct NetEntityGatherFailure
{
  const NetHeader header{NET_ENTITY, NET_ENTITY_GATHER_FAILURE};
  const EntityId ident;
  
  NetEntityGatherFailure(EntityId idd) : ident(idd) { }
};


#pragma mark Player

struct NetPlayerSpawn
{
  const NetHeader header{NET_PLAYER, NET_PLAYER_SPAWN};
  const PlayerID player;
  const u16 x;
  const u16 y;
  
  NetPlayerSpawn(PlayerID p, u16 x, u16 y) : player(p), x(x), y(y) { }
};

struct NetPlayerInitialPosition
{
  const NetHeader header{NET_PLAYER, NET_PLAYER_INITIAL_POSITION};
  const PlayerID player;
  const PlayerPosition position;
  
  NetPlayerInitialPosition(PlayerID p, PlayerPosition position) : player(p), position(position) { }
};

struct NetPlayerPush
{
  const NetHeader header{NET_PLAYER, NET_PLAYER_PUSH};
  const PlayerID player;
  const float magnitude;
  const float angle;
  
  NetPlayerPush(PlayerID p, float magnitude, float angle) : player(p), magnitude(magnitude), angle(angle) { }
};

struct NetPlayerSpawnSet
{
  const NetHeader header{NET_PLAYER, NET_PLAYER_SPAWN_SET};
  const u16 x;
  const u16 y;
  
  NetPlayerSpawnSet(u16 x, u16 y) : x(x), y(y) { }
};

struct NetPlayerWarp
{
  const NetHeader header{NET_PLAYER, NET_PLAYER_WARP};
  const PlayerID player;
  const u16 x;
  const u16 y;
  
  NetPlayerWarp(PlayerID p, u16 x, u16 y) : player(p), x(x), y(y) { }
};

struct NetPlayerStatus
{
  const NetHeader header{NET_PLAYER, NET_PLAYER_SET_STATUS};
  const PlayerID player;
  const PlayerStatus status;
  const bool value;
  
  NetPlayerStatus(PlayerID p, PlayerStatus status, bool value) : player(p), status(status), value(value) { }
};

struct NetPlayerMovement
{
  const NetHeader header{NET_PLAYER, NET_PLAYER_MOVEMENT};
  const PlayerID player;
  const MovementInfo info;
  
  NetPlayerMovement(PlayerID p, MovementInfo&& i) : player(p), info(i) { }
};

struct NetPlayerUpdateItem
{
  const NetHeader header{NET_PLAYER, NET_PLAYER_UPDATE_ITEM};
  const NetItem item;
  const s8 quickslot;
  const PlayerID player;
  
  NetPlayerUpdateItem(PlayerID player, const Item *item, s8 quickslot) : player(player), item(item), quickslot(quickslot) { }
};

struct NetPlayerUpdateEquip
{
  NetHeader header{NET_PLAYER, NET_PLAYER_UPDATE_EQUIP};
  const NetItem item;
  const EquipClass type;
  const s8 index;
  const PlayerID player;
  
  NetPlayerUpdateEquip(PlayerID player, const Item *item, s8 index, EquipClass type) : player(player), type(type), item(item), index(index) { }
};

struct NetPlayerUpdateHead
{
  const NetHeader header{NET_PLAYER, NET_PLAYER_UPDATE_HEAD};
  const BodyStyle index;
  const PlayerID player;
  
  NetPlayerUpdateHead(PlayerID p, BodyStyle i) : player(p), index(i) { }
};

struct NetPlayerThrow
{
  const NetHeader header{NET_PLAYER, NET_PLAYER_THROW_ITEM};
  const NetItem item;
  const SimplePoint position;
  const float strength;
  const float angle;
  const bool thrown;
  
  NetPlayerThrow(Item *item, SimplePoint pos, float str, float ang, bool thr) :
    item(item), position(pos), strength(str), angle(ang), thrown(thr) { }
};

struct NetPlayerShoot
{
  const NetHeader header{NET_PLAYER, NET_PLAYER_SHOOT_ITEM};
  const ThrowData info;
  const SimplePoint position;
  const float strength;
  const float angle;
  
  NetPlayerShoot(const ThrowData& info, SimplePoint pos, float strength, float angle) :
    info(info), position(pos), strength(strength), angle(angle) { }
};

struct NetPlayerShelfPlace
{
  const NetHeader header{NET_PLAYER, NET_PLAYER_SHELF_PLACE};
  const u16 x;
  const u16 y;
  const QuarterPosition which;
  
  NetPlayerShelfPlace(TMapTile tile, QuarterPosition wh) : x(tile->x), y(tile->y), which(wh) { }
};

struct NetPlayerToggledBlock
{
  const NetHeader header{NET_PLAYER, NET_PLAYER_TOGGLED_BLOCK};
  const u16 x, y;
  const BlockLayer layer;
  
  NetPlayerToggledBlock(u16 x, u16 y, BlockLayer layer) : x(x), y(y), layer(layer) { }
};

struct NetPlayerConsumeItem
{
  const NetHeader header{NET_PLAYER, NET_PLAYER_CONSUME_ITEM};
  const u8 index;
  
  NetPlayerConsumeItem(u8 index) : index(index) { }
};

struct NetPlayerConsumeTool
{
  const NetHeader header{NET_PLAYER, NET_PLAYER_CONSUME_TOOL};
  const u8 index;
  
  NetPlayerConsumeTool(u8 index) : index(index) { }
};

struct NetPlayerUseItem
{
  const NetHeader header{NET_PLAYER, NET_PLAYER_USE_ITEM};
  const NetItem item;
  
  NetPlayerUseItem(Item* item) : item(item) { }
};

struct NetPlayerBuff
{
  NetHeader header{NET_PLAYER,NET_PLAYER_BUFF_ADD};
  PotionEffect effect;
  PlayerID playerId;
  
  NetPlayerBuff(PacketSubType type, PotionEffect effect, PlayerID playerId) : header(NET_PLAYER,type), effect(effect), playerId(playerId) { }
};

struct NetPlayerBuffRaw
{
  const NetHeader header;
  const PlayerID player;
  const PotionEffect effect;
  const u16 ticks;
  
  NetPlayerBuffRaw(bool add, PlayerID player, PotionEffect effect, u16 ticks) :
  header(NET_PLAYER, add ? NET_PLAYER_BUFF_ADD_RAW : NET_PLAYER_BUFF_REM_RAW),
  player(player), effect(effect), ticks(ticks) { }
  
  bool isAdding() const { return header.subType == NET_PLAYER_BUFF_ADD_RAW; }
};


struct NetPlayerAttack
{
  const NetHeader header{NET_PLAYER, NET_PLAYER_ATTACK};
  const EntityId ident;
  
  NetPlayerAttack(EntityId idd) : ident(idd) { }
};

struct NetPlayerCapture
{
  const NetHeader header{NET_PLAYER, NET_PLAYER_CAPTURE_MOB};
  const EntityId ident;
  
  NetPlayerCapture(EntityId idd) : ident(idd) { }
};

struct NetPlayerHealth
{
  const NetHeader header;
  const float value;
  const DamageType data;
  
  NetPlayerHealth(PacketSubType type, float value, DamageType data = DAMAGE_SLASH) : header(NET_PLAYER, type), value(value), data(data) { }
};

struct NetPlayerBreathe
{
  const NetHeader header{NET_PLAYER, NET_PLAYER_BREATHE_UPDATE};
  const int value;
  
  NetPlayerBreathe(int value) : value(value) { }
};

struct NetPlayerDeath
{
  const NetHeader header{NET_PLAYER, NET_PLAYER_DEATH};
  const PlayerID playerId;
  
  NetPlayerDeath(PlayerID pid) : playerId(pid) { }
};

struct NetPlayerDeathEffect
{
  const NetHeader header{NET_PLAYER, NET_PLAYER_DEATH_EFFECT};
  NetItem item;
  
  NetPlayerDeathEffect(const Item& item) : item(NetItem(item)) { }
};

struct NetPlayerToyPush
{
  const NetHeader header{NET_PLAYER,NET_PLAYER_PUSH_BARE};
  const SimplePoint location;
  const bool inverted;
  
  NetPlayerToyPush(SimplePoint location, bool inverted) : location(location), inverted(inverted) { }
};

struct NetPlayerChat
{
  const NetHeader header{NET_PLAYER, NET_PLAYER_CHAT};
  const PlayerID player;
  char message[CHAT_MESSAGE_LENGTH];
  
  NetPlayerChat(PlayerID player, const char *m) : player(player)
  {
    strncpy(message, m, CHAT_MESSAGE_LENGTH);
  }
};

struct NetPlayerConsole
{
  const NetHeader header{NET_PLAYER, NET_PLAYER_CONSOLE};
  const PlayerID player;
  char message[CHAT_MESSAGE_LENGTH];
  
  NetPlayerConsole(PlayerID player, const char *m) : player(player)
  {
    strncpy(message, m, CHAT_MESSAGE_LENGTH);
  }
};

struct NetPlayerMessage
{
  const NetHeader header{NET_PLAYER, NET_PLAYER_MESSAGE};
  char message[CHAT_MESSAGE_LENGTH];
  s8 player;
  MessageType type;
  
  NetPlayerMessage(MessageType t, const char *m, s8 p = -1)
  {
    player = p;
        
    strncpy(message, m, CHAT_MESSAGE_LENGTH);
    type = t;
  }
};

struct NetPlayerCreativeMovement
{
  const NetHeader header{NET_PLAYER, NET_PLAYER_CREATIVE_MOVEMENT};
  const PlayerID player;
  const bool flying;
  
  NetPlayerCreativeMovement(PlayerID player, bool flying) : player(player), flying(flying) { }
};

struct NetPlayerStatIncrease
{
  const NetHeader header{NET_PLAYER, NET_PLAYER_STAT_INCREASE};
  const stats::Statistic stat;
  const u16 data;
  
  NetPlayerStatIncrease(stats::Statistic stat, u16 data) : stat(stat), data(data) { }
};

struct NetPlayerPetSummon
{
  const NetHeader header{NET_PLAYER, NET_PLAYER_PET_SUMMON};
  const NetItem item;
  
  NetPlayerPetSummon(Item* item) : item(item) { }
};

struct NetPlayerCraftbookData
{
  using bit_set_type = pix::SimpleBitSet<MAX_BOOK_SIZE, u32>;
  
  const NetHeader header{NET_PLAYER, NET_PLAYER_CRAFTBOOK_DATA};
  bool allDiscovered;
  bit_set_type::value_type data[bit_set_type::SIZE];
  
  NetPlayerCraftbookData(bool allDiscovered, const pix::SimpleBitSet<MAX_BOOK_SIZE, u32>& data) : allDiscovered(allDiscovered)
  {
    data.serialize(this->data);
  }
};

struct NetPlayerCraftbookDiscover
{
  const NetHeader header{NET_PLAYER, NET_PLAYER_CRAFTBOOK_DISCOVER};
  CraftPageIndex index;
  
  NetPlayerCraftbookDiscover(CraftPageIndex index) : index(index) { }
};

struct NetPlayerPortalTravel
{
  const NetHeader header{NET_PLAYER, NET_PLAYER_PORTAL_TRAVEL};
  const WorldThemeId theme;
  NetPlayerPortalTravel(WorldThemeId theme) : theme(theme) { }
};

#pragma mark Effects

struct NetEffectPlayer
{
  const NetHeader header{NET_EFFECT, NET_EFFECT_PLAYER};
  const PlayerID player;
  const GfxEffect effect;
  const u8 data;
  
  NetEffectPlayer(PlayerID player, GfxEffect effect, u8 data) : player(player), effect(effect), data(data) { }
};

struct NetEffectTile
{
  const NetHeader header{NET_EFFECT, NET_EFFECT_TILE};
  const u16 x;
  const u16 y;
  const GfxEffect effect;
  
  NetEffectTile(u16 x, u16 y, GfxEffect effect) : x(x), y(y), effect(effect) { }
};

struct NetEffectEntity
{
  const NetHeader header{NET_EFFECT, NET_EFFECT_ENTITY};
  const EntityId ident;
  const GfxEffect effect;
  const u8 data;
  
  NetEffectEntity(EntityId ident, GfxEffect effect, u8 data) : ident(ident), effect(effect), data(data) { }
};

struct NetEffectSound
{
  const NetHeader header{NET_EFFECT, NET_EFFECT_SOUND};
  const u16 x, y;
  const Sound sound;
  const u16 param;
  
  NetEffectSound(CMapTile tile, Sound sound, u16 param = 0) : x(tile->x), y(tile->y), sound(sound), param(param) { }
};


#pragma mark Chests

struct NetChestSet
{
  const NetHeader header{NET_CHEST, NET_CHEST_SET};
  const u16 x, y;
  const u8 cx;
  const NetItem item;
  
  NetChestSet(u16 x, u16 y, u8 cx, Item *item) :
    x(x), y(y), cx(cx), item(item) { }
};

struct NetChestOpen
{
  const NetHeader header{NET_CHEST, NET_CHEST_OPEN};
  const u8 slots, offset;
  const u16 x, y;
  
  NetChestOpen(u16 x, u16 y, u8 slots, u8 offset) : x(x), y(y), slots(slots), offset(offset) { }
};

struct NetChestSwitchSheet
{
  const NetHeader header{NET_CHEST, NET_CHEST_SWITCH_SHEET};
  const u16 x, y;
  const s8 offset;
  
  NetChestSwitchSheet(u16 x, u16 y, s8 offset) : x(x), y(y), offset(offset) { }
};

#pragma mark Alchemy

struct NetAlchemyLabOpen
{
  const NetHeader header{NET_ALCHEMY, NET_ALCHEMY_OPEN};
  const u16 x, y;
  std::array<NetItem, 5> items;
  
  NetAlchemyLabOpen(u16 x, u16 y, const ItemGroup<INVENTORY_ALCHEMY, 5>& items) : x(x), y(y), items({NetItem(items[0]),NetItem(items[1]),NetItem(items[2]),NetItem(items[3]),NetItem(items[4])}) { }
};

struct NetAlchemySet
{
  const NetHeader header{NET_ALCHEMY, NET_ALCHEMY_SET};
  const u16 x, y;
  const u8 index;
  const NetItem item;
  
  NetAlchemySet(u16 x, u16 y, u8 index, Item *item) : x(x), y(y), index(index), item(item) { }
};

struct NetAlchemyCraftDone
{
  NetHeader header{NET_ALCHEMY, NET_ALCHEMY_CRAFT_DONE};
  const PotionInfo info;
  
  NetAlchemyCraftDone(PotionInfo info) : info(info) { }
};


#pragma mark Forges

struct NetForgeOpen
{
  const NetHeader header{NET_FORGE, NET_FORGE_OPEN};
  const u16 x, y;
  const Forge::Status status;
  const std::array<NetItem, 3> items;
  
  NetForgeOpen(u16 x, u16 y, const Forge::Status& status, const ItemGroup<INVENTORY_FORGE,3>& items) : x(x), y(y), status(status), items({NetItem(items[0]), NetItem(items[1]), NetItem(items[2])}) { }
};

struct NetForgeStatus
{
  const NetHeader header{NET_FORGE, NET_FORGE_STATUS};
  const Forge::Status status;
  
  NetForgeStatus(const Forge::Status& status) : status(status) { }
};

struct NetForgeSet
{
  const NetHeader header{NET_FORGE, NET_FORGE_SET};
  const u16 x, y;
  const u8 index;
  const NetItem item;
  
  NetForgeSet(u16 x, u16 y, u8 index, Item *item) : x(x), y(y), index(index), item(item) { }
};

struct NetForgeUpdate
{
  const NetHeader header{NET_FORGE, NET_FORGE_UPDATE};
  const u16 x, y;
  const Forge::Status status;
  const std::array<NetItem, 3> items;
  
  NetForgeUpdate(u16 x, u16 y, const Forge::Status& status, const ItemGroup<INVENTORY_FORGE,3>& items) : x(x), y(y), status(status), items({NetItem(items[0]),NetItem(items[1]),NetItem(items[2])}) { }
};

#pragma mark Stables

struct NetStableOpen
{
  const NetHeader header{NET_STABLE, NET_STABLE_OPEN};
  const u16 x, y;
  const std::array<MobType, STABLE_SIZE> mobs;
  const std::array<u8, STABLE_SIZE> feed;
  const NetItem food;
  
  NetStableOpen(u16 x, u16 y, MobType m1, MobType m2, MobType m3, u8 f1, u8 f2, u8 f3, const Item& food) :
    x(x), y(y), mobs({m1,m2,m3}), feed({f1,f2,f3}), food(food) { }
};

struct NetStableSet
{
  const NetHeader header{NET_STABLE, NET_STABLE_SET};
  const u16 x, y;
  const NetItem item;
  
  NetStableSet(u16 x, u16 y, Item *item) : x(x), y(y), item(item) { }
};

struct NetSignData
{
  const NetHeader header;
  const u16 x, y;
  char text[128];
  const bool edit;
  //s8 engrave;
  
  NetSignData(PacketSubType subType, u16 x, u16 y, const char *text, bool edit, s8 engrave = -1) : header({NET_SIGN, subType}), x(x), y(y), edit(edit)
  {
    if (text)
      strncpy(this->text, text, 128);
    else
      memset(this->text, 0, sizeof(char)*128);
  }
};

struct NetSignEngrave
{
  const NetHeader header{NET_SIGN, NET_SIGN_ENGRAVE};
  const u16 x, y;
  const u8 engrave;
  
  NetSignEngrave(u16 x, u16 y, u8 engrave) : x(x), y(y), engrave(engrave) { }
};

#pragma mark Trades

struct NetTradeStartRequest
{
  const NetHeader header{NET_TRADE, NET_TRADE_START_REQUEST};
  const u16 x;
  const u16 y;
  
  NetTradeStartRequest(TMapTile tile) : x(tile->x), y(tile->y) { }
};

struct NetTradeRequest
{
  const NetHeader header{NET_TRADE, NET_TRADE_REQUEST};
  const PlayerID reqId;
  const PlayerID respId;
  char playerName[32];
  
  NetTradeRequest(PlayerID reqId, PlayerID respId, const char *name) : reqId(reqId), respId(respId) {
    strncpy(playerName, name, 32);
  }
  
  NetTradeRequest() : reqId(PlayerID::HOST), respId(PlayerID::HOST) { }
};

struct NetTradeResponse
{
  const NetHeader header{NET_TRADE, NET_TRADE_RESPONSE};
  const PlayerID reqId;
  const PlayerID respId;
  
  NetTradeResponse(PacketSubType type, PlayerID reqId, PlayerID respId) : header{NET_TRADE,type}, reqId(reqId), respId(respId)
  {
  }
};

struct NetTradeOpen
{
  const NetHeader header{NET_TRADE, NET_TRADE_OPEN};
  char playerName[32];
  
  NetTradeOpen(const char *name) {
    strncpy(playerName, name, 32);
  }
};

struct NetTradeItem
{
  const NetHeader header;
  const NetItem item;
  
  NetTradeItem(bool add, const Item& item) : header({NET_TRADE, add ? NET_TRADE_ADD_ITEM : NET_TRADE_REMOVE_ITEM}), item(item) { }
};

#pragma mark Fishing

struct NetFishingAction
{
  const NetHeader header{NET_FISHING, NET_FISHING_ACTION};
  const float dt;
  
  NetFishingAction(float dt) : dt(dt) { }
};

#pragma mark Devices

struct NetDeviceConfig
{
  const NetHeader header;
  const u16 x, y;
  const DeviceConfiguration config;
  
  NetDeviceConfig(PacketSubType subType, TMapTile tile, DeviceConfiguration config = DeviceConfiguration()) : header{NET_DEVICE, subType}, x(tile->x), y(tile->y), config(config) { }
};

#if defined(ENABLE_NET_CODE_PACKING)
#pragma pack(pop)
#endif

#endif
