#pragma once

#include "Inv_GridTypes.generated.h"

class UInv_InventoryItem;

UENUM(BlueprintType)
enum class EInv_ItemCategory : uint8
{
	Equippable,
	Consumable,
	Craftable,
	None
};

USTRUCT()
struct FInv_SlotAvaliability
{
	GENERATED_BODY()

	FInv_SlotAvaliability(){}
	FInv_SlotAvaliability(int32 ItemIndex,int32 Room,bool bHasItem) : Index(ItemIndex),AmountToFill(Room),bItemAtIndex(bHasItem){}

	//最小单元格的位置，标识哪一个格子 -> 在哪里操作
	int32 Index{INDEX_NONE};
	//计划放入这个格子的Item数量 -> 操作什么
	int32 AmountToFill{0};
	//这个格子是否已经有Item; -> 如何操作
	bool bItemAtIndex{false};
};

USTRUCT()
struct FInv_SlotAvailabilityResult
{
	GENERATED_BODY()

	FInv_SlotAvailabilityResult() {}

	//用弱指针指向已经在背包存在的物品
	//Why use Weak?
	TWeakObjectPtr<UInv_InventoryItem> Item;
	
	//总共能放下多少Item
	int32 TotalRoomToFill{0};
	//最总还剩下多少Item放不下
	int32 Remainder{0};
	//该Item是否可以堆叠？
	bool bStackable{false};
	 
	TArray<FInv_SlotAvaliability> SlotAvailabilities;
};

UENUM(BlueprintType)
enum class EInv_TileQuadrant : uint8
{
	TopLeft,
	TopRight,
	BottomLeft,
	BottomRight,
	None
};


USTRUCT(BlueprintType)
struct FInv_TileParameters
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite,EditAnywhere,Category="Inventory")
	FIntPoint TileCoordinats{};
	
	UPROPERTY(BlueprintReadWrite,EditAnywhere,Category="Inventory")
	int TileIndex{INDEX_NONE};
	
	UPROPERTY(BlueprintReadWrite,EditAnywhere,Category="Inventory")
	EInv_TileQuadrant TileQuadrant{EInv_TileQuadrant::None};
};

inline bool operator==(const FInv_TileParameters A,const FInv_TileParameters B)
{
	return A.TileCoordinats == B.TileCoordinats && A.TileIndex == B.TileIndex && A.TileQuadrant == B.TileQuadrant;
}


USTRUCT()
struct FInv_SpaceQueryResult
{
	GENERATED_BODY()

	// True if the space queried has no items in it
	bool bHasSpace{false};

	// Valid if there's a single item we can swap with
	TWeakObjectPtr<UInv_InventoryItem> ValidItem = nullptr;

	// Upper left index of the valid item, if there is one
	int32 UpperLeftIndex{INDEX_NONE};
};