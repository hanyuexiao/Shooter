#include "Items/Manifest/Inv_ItemManifest.h"

#include "Items/Inv_InventoryItem.h"
#include "Items/Components/Inv_ItemComponent.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Widgets/Composite/Inv_CompositeBase.h"

//复印的过程
UInv_InventoryItem* FInv_ItemManifest::Manifest(UObject* NewOuter)
{
	//   调用UE的全局函数NewObject<T>()来在内存中实例化一个新的UInv_InventoryItem。
	// - NewOuter: 指定了这个新UObject的“所有者”，这对于UE的垃圾回收(GC)至关重要。
	// - UInv_InventoryItem::StaticClass(): 告诉NewObject我们要创建的具体类型。
	UInv_InventoryItem* Item = NewObject<UInv_InventoryItem>(NewOuter,UInv_InventoryItem::StaticClass());
	Item->SetItemManifest(*this);

	for (auto& Fragment : Item->GetItemManifestMutable().GetFragmentsMutable())
	{
		Fragment.GetMutable().Manifest();
	}
	ClearFragments();
	
	return Item;
}

void FInv_ItemManifest::AssimilateInventoryFragments(UInv_CompositeBase* Composite) const
{
	const auto& InventoryItemFragments = GetAllFragmentsOfType<FInv_InventoryItemFragment>();
	for (const auto* Fragment : InventoryItemFragments)
	{
		Composite->ApplyFunction([Fragment](UInv_CompositeBase* Widget)
		{
			Fragment->Assimilate(Widget);
		});
	}
}


void FInv_ItemManifest::SpawnPickupActor(const UObject* WorldContextObject, const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	if (!IsValid(PickupActorClass) || !IsValid(WorldContextObject)) return;

	AActor* SpawnedActor = WorldContextObject->GetWorld()->SpawnActor<AActor>(PickupActorClass, SpawnLocation, SpawnRotation);
	if (!IsValid(SpawnedActor)) return;

	// Set the item manifest, item category, item type, etc.
	UInv_ItemComponent* ItemComp = SpawnedActor->FindComponentByClass<UInv_ItemComponent>();
	check(ItemComp);

	ItemComp->InitItemManifest(*this);
}


void FInv_ItemManifest::ClearFragments()
{
	for (auto& Fragment : Fragments)
	{
		Fragment.Reset();
	}
	Fragments.Empty();
}