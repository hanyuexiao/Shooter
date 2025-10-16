// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inv_HighLightable.h"
#include "Components/StaticMeshComponent.h"
#include "Inv_HighlightableStaticMesh.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORY_API UInv_HighlightableStaticMesh : public UStaticMeshComponent,public IInv_HighLightable
{
	GENERATED_BODY()

public:

	virtual void HighLight_Implementation() override;
	
	virtual void UnHighLight_Implementation() override;

private:
	UPROPERTY(EditAnywhere,Category = "Inventory")
	TObjectPtr<UMaterialInterface> HighLightMarerial;
};
