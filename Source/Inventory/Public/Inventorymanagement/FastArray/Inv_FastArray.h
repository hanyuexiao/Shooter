#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "Inv_FastArray.generated.h"

struct FGameplayTag;
class UInv_ItemComponent;
class UInv_InventoryComponent;
class UInv_InventoryItem;


/** A single entry in an inventory **/
// 进入快速序列化数组要求要用FFastArraySerializerItem将要放入的东西包装起来
USTRUCT(BlueprintType)
struct FInv_InventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FInv_InventoryEntry();

private:
	friend struct FInv_InventoryFastArray;
	friend UInv_InventoryComponent;
	
	UPROPERTY()
	TObjectPtr<UInv_InventoryItem> Item = nullptr;
	
};

//库存容器 物品列表 背包
/** List of inventory items */
USTRUCT(BlueprintType)
struct FInv_InventoryFastArray:public FFastArraySerializer
{
	GENERATED_BODY()
	
	FInv_InventoryFastArray();
	FInv_InventoryFastArray(UActorComponent* InOwnerComponent);

	TArray<UInv_InventoryItem*> GetAllItems() const;


	// 回调函数。当客户端接收到服务器的数据更新后，引擎会自动调用它们。
	// FFastArraySerializxer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices,int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices,int32 FinalSize);
	// End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FInv_InventoryEntry,FInv_InventoryFastArray>(Entries,DeltaParams,*this);
	}

	UInv_InventoryItem* AddEntry(UInv_ItemComponent* ItemComponent);
	UInv_InventoryItem* AddEntry(UInv_InventoryItem* Item);
	void RemoveEntry(UInv_InventoryItem* Item);
	UInv_InventoryItem* FindFirstItemByType(const FGameplayTag& ItemType);
	
private:
	friend UInv_InventoryComponent;


	//实际存储所有物品的数组
	//Replicated list of items
	UPROPERTY()
	TArray<FInv_InventoryEntry> Entries;
	
	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;
};




template<>
struct TStructOpsTypeTraits<FInv_InventoryFastArray> : public TStructOpsTypeTraitsBase2<FInv_InventoryFastArray>
{
	enum { WithNetDeltaSerializer = true};
};