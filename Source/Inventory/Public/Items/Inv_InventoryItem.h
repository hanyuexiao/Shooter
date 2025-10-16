// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Items/Manifest/Inv_ItemManifest.h"
#include "Inv_InventoryItem.generated.h"

struct FInstancedStruct;
/**
 * 
 */
UCLASS()
class INVENTORY_API UInv_InventoryItem : public UObject
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override{return true;}
	
	void SetItemManifest(const FInv_ItemManifest& Manifest);
	
	//我们希望提供一个安全的、只读的GetItemManifest()接口，因此我们把它声明为const函数。
	//⇩
	//因为它是一个const函数，所以它在访问ItemManifest成员时，只能调用const版本的.Get<T>()。
	//⇩
	//因为.Get<T>()返回的是一个只读的const引用，所以我们函数的返回值也必须是只读的const引用。
	
	// 获取背包物品的清单  因为.Get返回值是const 所以函数前要加const
	const FInv_ItemManifest& GetItemManifest() const {return ItemManifest.Get<FInv_ItemManifest>();}
	
	FInv_ItemManifest& GetItemManifestMutable() {return ItemManifest.GetMutable<FInv_ItemManifest>();}
	bool IsStackable() const;
	bool IsConsumable() const;
	int32 GetTotalStackCount() const {return TotalStackCount;}
	void SetTotalStackCount(int32 Count) {TotalStackCount = Count;}
private:
	// FInstancedStruct是什么？ 什么时候会想到使用FInstancedStruct
	// 与TSubclassOf有什么区别
	UPROPERTY(VisibleAnywhere,meta = (BaseStruct = "/Script/Inventory.Inv_ItemManifest"),Replicated)
	FInstancedStruct ItemManifest;

	UPROPERTY(Replicated)
	int32 TotalStackCount{0};
};

template <typename FragmentType>
const FragmentType* GetFragment(const UInv_InventoryItem* Item, const FGameplayTag& Tag)
{
	if (!IsValid(Item)) return nullptr;
	//先从Item里获取清单Manifest
	const FInv_ItemManifest& Manifest = Item->GetItemManifest();
	return Manifest.GetFragmentOfTypeWithTag<FragmentType>(Tag);
}
