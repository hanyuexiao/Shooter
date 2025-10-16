// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Util/GridIndexing3.h"
#include "Inv_SlottedItem.generated.h"

class UInv_InventoryItem;
class UImage;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSlottedItemClicked,int32,GridIndex,const FPointerEvent&,MouseEvent);
/**
 * 
 */
UCLASS()
class INVENTORY_API UInv_SlottedItem : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void NativeOnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& MouseEvent) override;
	// 是否可重叠
	bool IsStackable() const {return bIsStackable;}
	// 设置是否可重叠
	void SetIsStackable(bool bStackable) {bIsStackable = bStackable;}
	// 获取物品图像图标
	UImage* GetImageIcon() const {return Image_Icon;}
	// 初始化网格序号
	void SetGridIndex(int32 Index) {GridIndex = Index;}
	// 获取网格序号
	int32 GetGridIndex() const {return GridIndex;}
	// 设置空间尺寸
	void SetGridDimensions(const FIntPoint& Dimensions){GridDimensions = Dimensions;}
	// 获取空间尺寸
	FIntPoint GetGridDimensions() const {return GridDimensions;}
	// 设置物品信息
	void SetInventoryItem(UInv_InventoryItem* Item);
	// 获取物品信息
	UInv_InventoryItem* GetInventoryItem() const {return InventoryItem.Get();}
	// 设置物品图标
	void SetImageBrush (const FSlateBrush& Brush) const;
	// 更新存储数量
	void UpdateStackCount(int32 StackCount);

	FSlottedItemClicked OnSlottedItemClicked;
	
private:
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_Icon;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_StackCount;
	
	int32 GridIndex;
	FIntPoint GridDimensions;
	TWeakObjectPtr<UInv_InventoryItem> InventoryItem;
	bool bIsStackable{false};
};
