// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryManagement/Components/Inv_InventoryComponent.h"

#include "Blueprint/UserWidget.h"
#include "Items/Components/Inv_ItemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/Inventory/InventoryBase/Inv_InventoryBase.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Fragments/Inv_ItemFragment.h"


UInv_InventoryComponent::UInv_InventoryComponent() : InventoryList(this)
{

	PrimaryComponentTick.bCanEverTick = false;
	// 第1行: 确保组件本身会被复制
	SetIsReplicatedByDefault(true);
	// 第2行: 【关键开关】将这个布尔值设为true，就等于告诉引擎：
	// “除了我自己的属性，请同时留意我手动注册的一个子对象列表，并帮我同步它们。”
	bReplicateUsingRegisteredSubObjectList = true;
	bInventoryMenuOpen = false;
}

void UInv_InventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass,InventoryList);
}

void UInv_InventoryComponent::TryAddItem(UInv_ItemComponent* ItemComponent)
{
	//本地检查
	FInv_SlotAvailabilityResult Result = InventoryMenu->HasRoomForItem(ItemComponent);

	UInv_InventoryItem* FoundItem = InventoryList.FindFirstItemByType(ItemComponent->GetItemManifest().GetItemType());
	Result.Item = FoundItem;
	if (Result.TotalRoomToFill == 0)
	{
		NoRoomInInventory.Broadcast();
		return;
	}

	if (Result.Item.IsValid() && Result.bStackable)
	{
		//Add stacks to an item that already exists in the inventory.We only want to update the stack count.
		//not creat a new item of this type
		OnStackChange.Broadcast(Result);
		Server_AddStacksToItem(ItemComponent,Result.TotalRoomToFill,Result.Remainder);
	}
	else if (Result.TotalRoomToFill > 0)
	{
		//创建新的背包网格给新物品
		//This item type doesn't exist in the inventory. Creat a new one and update all pertinent slots
		Server_AddNewItem(ItemComponent,Result.bStackable ? Result.TotalRoomToFill : 0);
	}
	
}

void UInv_InventoryComponent::Server_AddNewItem_Implementation(UInv_ItemComponent* ItemComponent, int32 StackCount)
{
	UInv_InventoryItem* NewItem  = InventoryList.AddEntry(ItemComponent);
	NewItem->SetTotalStackCount(StackCount);
	
	// NM_ListenServer 和 NM_Standalone都是什么？
	if (GetOwner() -> GetNetMode() == NM_ListenServer || GetOwner() -> GetNetMode() == NM_Standalone)
	{
		OnItemAdded.Broadcast(NewItem);
	}
	
	//TODO: Tell the Item Component to destroy its owning actor.
	ItemComponent->PickedUp();
}

void UInv_InventoryComponent::Server_AddStacksToItem_Implementation(UInv_ItemComponent* ItemComponent, int32 StackCount,int32 Remainder)
{
	const FGameplayTag& ItemType = IsValid(ItemComponent) ? ItemComponent->GetItemManifest().GetItemType() : FGameplayTag::EmptyTag;
	UInv_InventoryItem* Item = InventoryList.FindFirstItemByType(ItemType);
	if (!IsValid(Item)) return;

	Item->SetTotalStackCount(Item->GetTotalStackCount() + StackCount);

	//TODO : Destory the Item if the remainder is zero
	//Otherwise,update the stack count for the item pickup
	if (Remainder == 0)
	{
		ItemComponent->PickedUp();
	}
	else if (FInv_StackableFragment* StackableFragment = ItemComponent->GetItemManifest().GetFragmentOfTypeMutable<FInv_StackableFragment>())
	{
		StackableFragment->SetStackCount(Remainder);
	}
}

void UInv_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	ConstructInventory();
}

void UInv_InventoryComponent::Server_DropItem_Implementation(UInv_InventoryItem* Item, int32 StackCount)
{
	const int32 NewStackCount = Item->GetTotalStackCount() - StackCount;
	if (NewStackCount <= 0)
	{
		InventoryList.RemoveEntry(Item);
	}
	else
	{
		Item->SetTotalStackCount(NewStackCount);
	}

	SpawnDroppedItem(Item, StackCount);
}


void UInv_InventoryComponent::Server_ConsumeItem_Implementation(UInv_InventoryItem* Item)
{
	const int32 NewStackCount = Item->GetTotalStackCount() - 1;
	if (NewStackCount <= 0)
	{
		InventoryList.RemoveEntry(Item);
	}
	else
	{
		Item->SetTotalStackCount(NewStackCount);
	}

	// Get the consumable fragment and call Consume()
	// (Actually create the Consumable Fragement)
	
	if (FInv_ConsumableFragment* ConsumableFragment = Item->GetItemManifestMutable().GetFragmentOfTypeMutable<FInv_ConsumableFragment>())
	{
		ConsumableFragment->OnConsume(OwningController.Get());
	}
}


void UInv_InventoryComponent::SpawnDroppedItem(UInv_InventoryItem* Item, int32 StackCount)
{
	
	//Spawn the dropped item in the level
	const APawn* OwningPawn = OwningController->GetPawn();
	FVector RotatedForward = OwningPawn->GetActorForwardVector();
	RotatedForward = RotatedForward.RotateAngleAxis(FMath::FRandRange(DropSpawnAngleMin, DropSpawnAngleMax), FVector::UpVector);
	FVector SpawnLocation = OwningPawn->GetActorLocation() + RotatedForward * FMath::FRandRange(DropSpawnDistanceMin, DropSpawnDistanceMax);
	SpawnLocation.Z -= RelativeSpawnElevation;
	const FRotator SpawnRotation = FRotator::ZeroRotator;

	// TODO: Have the Item Manifest spawn the pickup actor.
	FInv_ItemManifest& ItemManifest = Item->GetItemManifestMutable();
	if (FInv_StackableFragment* StackableFragment = ItemManifest.GetFragmentOfTypeMutable<FInv_StackableFragment>())
	{
		StackableFragment->SetStackCount(StackCount);
	}
	ItemManifest.SpawnPickupActor(this, SpawnLocation, SpawnRotation);
}

void UInv_InventoryComponent::ToggleInventoryMenu()
{
	if(bInventoryMenuOpen)
	{
		CloseInventoryMenu();
	}
	else
	{
		OpenInventoryMenu();
	}
	OnInventoryMenuToggled.Broadcast(bInventoryMenuOpen);
}

void UInv_InventoryComponent::AddRepSubobj(UObject* SubObj)
{
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && IsValid(SubObj))
	{
		//这个函数又是什么作用？ why
		AddReplicatedSubObject(SubObj);
	}
}

void UInv_InventoryComponent::ConstructInventory()
{
	OwningController = Cast<APlayerController>(GetOwner());
	checkf(OwningController.IsValid(),TEXT("Inventory Component should have a Player Controller as Owner"));
	if(!OwningController->IsLocalController()) return;

	InventoryMenu = CreateWidget<UInv_InventoryBase>(OwningController.Get(),InventoryMenuClass);
	InventoryMenu->AddToViewport();
	CloseInventoryMenu();
}

void UInv_InventoryComponent::OpenInventoryMenu()
{
	if(!IsValid(InventoryMenu)) return;

	//TODO Open the Inventory
	InventoryMenu->SetVisibility(ESlateVisibility::Visible);
	bInventoryMenuOpen = true;

	//TODO want to show the mouse
	if(!OwningController.IsValid()) return;

	FInputModeGameAndUI InputMode;
	OwningController->SetInputMode(InputMode);
	OwningController->SetShowMouseCursor(true);
}

void UInv_InventoryComponent::CloseInventoryMenu()
{
	if(!IsValid(InventoryMenu)) return;

	InventoryMenu->SetVisibility(ESlateVisibility::Collapsed);
	bInventoryMenuOpen = false;

	if(!OwningController.IsValid()) return;
	
	FInputModeGameOnly InputMode;
	OwningController->SetInputMode(InputMode);
	OwningController->SetShowMouseCursor(false);
}

void UInv_InventoryComponent::Server_EquipSlotClicked_Implementation(UInv_InventoryItem* ItemToEquip, UInv_InventoryItem* ItemToUnequip)
{
	Multicast_EquipSlotClicked(ItemToEquip, ItemToUnequip);
}

void UInv_InventoryComponent::Multicast_EquipSlotClicked_Implementation(UInv_InventoryItem* ItemToEquip, UInv_InventoryItem* ItemToUnequip)
{
	// Equipment Component will listen to these delegates
	OnItemEquipped.Broadcast(ItemToEquip);
	OnItemUnequipped.Broadcast(ItemToUnequip);
}



