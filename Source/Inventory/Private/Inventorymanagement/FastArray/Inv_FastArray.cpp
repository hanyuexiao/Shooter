#include "Inventorymanagement/FastArray/Inv_FastArray.h"

#include "InventoryManagement/Components/Inv_InventoryComponent.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Components/Inv_ItemComponent.h"

FInv_InventoryEntry::FInv_InventoryEntry()
{
	
}

FInv_InventoryFastArray::FInv_InventoryFastArray(): OwnerComponent(nullptr)
{
	
}

FInv_InventoryFastArray::FInv_InventoryFastArray(UActorComponent* InOwnerComponent): OwnerComponent(InOwnerComponent)
{
	
}

TArray<UInv_InventoryItem*> FInv_InventoryFastArray::GetAllItems() const
{
	TArray<UInv_InventoryItem*> Results;
	Results.Reserve(Entries.Num());
	for (const auto& Entry : Entries)
	{
		if (!IsValid(Entry.Item)) continue;
		Results.Add(Entry.Item);
	}
	return Results;
}

void FInv_InventoryFastArray::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	UInv_InventoryComponent* IC = Cast<UInv_InventoryComponent>(OwnerComponent);
	if (!IsValid(IC)) return;

	for (int32 Index : RemovedIndices)
	{
		IC -> OnItemRemoved.Broadcast(Entries[Index].Item);
	}
}

void FInv_InventoryFastArray::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	UInv_InventoryComponent* IC = Cast<UInv_InventoryComponent>(OwnerComponent);
	if (!IsValid(IC)) return;
	for (int32 Index : AddedIndices)
	{
		IC -> OnItemAdded.Broadcast(Entries[Index].Item);
	}
}

UInv_InventoryItem* FInv_InventoryFastArray::AddEntry(UInv_ItemComponent* ItemComponent)
{
	check(OwnerComponent);
	AActor* OwningActor = OwnerComponent->GetOwner();
	check(OwningActor -> HasAuthority());
	UInv_InventoryComponent* IC = Cast<UInv_InventoryComponent>(OwnerComponent);
	if (!IsValid(IC)) return nullptr;

	//传入FastArray所需要的数据格式 FInv_InventoryEntry
	FInv_InventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
	//   a. ItemComponent->GetItemManifest(): 从传入的ItemComponent身上，获取它的“物品清单(ItemManifest)”这个数据结构。这就是我们的“菜谱”。
	//   b. .Manifest(OwningActor): 调用这份“菜谱”自己的工厂方法Manifest()，并告诉它，新创建的物品的Outer将是OwningActor。
	//   c. NewEntry.Item = ...: 将Manifest()函数返回的、新创建的UInv_InventoryItem指针，赋值给我们刚刚准备好的空“盘子”NewEntry的Item成员。
	NewEntry.Item = ItemComponent -> GetItemManifest().Manifest(OwningActor);

	//在逻辑层面上对物体进行增加
	IC->AddRepSubobj(NewEntry.Item);
	
	// 让服务器知道需要同步多少个物体
	MarkItemDirty(NewEntry);
	
	return NewEntry.Item;
}

UInv_InventoryItem* FInv_InventoryFastArray::AddEntry(UInv_InventoryItem* Item)
{
	check(OwnerComponent);
	AActor* OwningActor = OwnerComponent->GetOwner();
	check(OwningActor -> HasAuthority());

	FInv_InventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.Item = Item;

	MarkItemDirty(NewEntry);
	return Item;
}

void FInv_InventoryFastArray::RemoveEntry(UInv_InventoryItem* Item)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FInv_InventoryEntry& Entry = *EntryIt;
		if (Entry.Item == Item)
		{
			EntryIt.RemoveCurrent();
			MarkArrayDirty();
		}
	}
		
}

UInv_InventoryItem* FInv_InventoryFastArray::FindFirstItemByType(const FGameplayTag& ItemType)
{
	auto* FoundItem = Entries.FindByPredicate([Type = ItemType](const FInv_InventoryEntry& Entry)
	{
		return IsValid(Entry.Item) && Entry.Item->GetItemManifest().GetItemType().MatchesTagExact(Type);
	});
	return FoundItem ? FoundItem->Item : nullptr;
}
