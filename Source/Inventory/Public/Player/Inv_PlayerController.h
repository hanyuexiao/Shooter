// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Inv_PlayerController.generated.h"

class UInv_InventoryComponent;
class UInv_HUDWidget;
class UInputMappingContext;
class UInputAction;
/**
 * 
 */
UCLASS()
class INVENTORY_API AInv_PlayerController : public APlayerController
{
	GENERATED_BODY()
            
public:
	AInv_PlayerController();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable,Category = "Iventory")
	void TogglleInventory();
	
protected:
	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;
private:

	void PrimaryInteract();
	void CreateHUDWidge();
	void TraceForItem();

	/**
	 * 
	 */
	TWeakObjectPtr<UInv_InventoryComponent> InventoryComponent;
	
	UPROPERTY(EditDefaultsOnly,Category = "Inventory")
	TObjectPtr<UInputMappingContext> DefaultIMC;

	UPROPERTY(EditDefaultsOnly,Category = "Inventory")
	TObjectPtr<UInputAction> PrimaryInteractAction;
	
	UPROPERTY(EditDefaultsOnly,Category = "Inventory")
	TObjectPtr<UInputAction> ToggleInventoryAction;

	UPROPERTY(EditDefaultsOnly,Category = "Inventory")
	TSubclassOf<UInv_HUDWidget> HUDWidgetClass;
	
	UPROPERTY(EditDefaultsOnly,Category = "Inventory")
	double TraceLength;

	UPROPERTY(EditDefaultsOnly,Category = "Inventory")
	TEnumAsByte<ECollisionChannel> ItemTraceChannel;
	
	TWeakObjectPtr<AActor> ThisActor;
	TWeakObjectPtr<AActor> LastActor;
	
	UPROPERTY()
	TObjectPtr<UInv_HUDWidget> HUDWidget;
	
};
