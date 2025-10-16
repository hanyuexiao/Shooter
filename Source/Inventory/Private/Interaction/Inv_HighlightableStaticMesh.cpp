// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Inv_HighlightableStaticMesh.h"

void UInv_HighlightableStaticMesh::HighLight_Implementation()
{
	SetOverlayMaterial(HighLightMarerial);
}

void UInv_HighlightableStaticMesh::UnHighLight_Implementation()
{
	SetOverlayMaterial(nullptr);
}
