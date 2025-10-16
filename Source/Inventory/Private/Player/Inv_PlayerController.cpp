// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Inv_PlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Interaction/Inv_HighLightable.h"
#include "InventoryManagement/Components/Inv_InventoryComponent.h"
#include "Items/Components/Inv_ItemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/HUD/Inv_HUDWidget.h"


AInv_PlayerController::AInv_PlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	TraceLength = 500.0;
	ItemTraceChannel = ECollisionChannel::ECC_GameTraceChannel1;
}

void AInv_PlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TraceForItem();
}

void AInv_PlayerController::TogglleInventory()
{
	if(!InventoryComponent.IsValid()) return;
	InventoryComponent->ToggleInventoryMenu();
	
	if (InventoryComponent->IsMenuOpen())
	{
		HUDWidget->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		HUDWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void AInv_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	//Input Action
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if(IsValid(Subsystem))
	{
		Subsystem->AddMappingContext(DefaultIMC,0);  
	}

	InventoryComponent = FindComponentByClass<UInv_InventoryComponent>();
	
	CreateHUDWidge();
}

void AInv_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

	EnhancedInputComponent ->BindAction(PrimaryInteractAction,ETriggerEvent::Started,this,&AInv_PlayerController::PrimaryInteract);

	EnhancedInputComponent ->BindAction(ToggleInventoryAction,ETriggerEvent::Started,this,&AInv_PlayerController::TogglleInventory);

}

void AInv_PlayerController::PrimaryInteract()
{
	if (!ThisActor.IsValid()) return;

	UInv_ItemComponent* ItemComp = ThisActor -> FindComponentByClass<UInv_ItemComponent>();
	if (!IsValid(ItemComp) || !InventoryComponent.IsValid()) return;

	InventoryComponent->TryAddItem(ItemComp);
}

void AInv_PlayerController::CreateHUDWidge()
{
	//IsLocalController -> 判断是否是由本地玩家控制，可用于多人游戏判断是否是不是本地计算机控制
	if(!IsLocalController()) return;
	//Initial HUD 
	HUDWidget = CreateWidget<UInv_HUDWidget>(this,HUDWidgetClass);
	if(IsValid(HUDWidget))
	{
		HUDWidget->AddToViewport();
	}
}

void AInv_PlayerController::TraceForItem()
{
	
	// TODO: Line Trace for Item Interaction
	// 此功能允许玩家通过瞄准世界场景中的物品来与其互动。
	// 该逻辑将在玩家控制器的 Tick 函数中每帧执行。

	// 1. 声明两个 TWeakObjectPtr<AActor> 变量，用于追踪当前和最后追踪的 Actor。
	// - TWeakObjectPtr<AActor> ThisActor;
	// - TWeakObjectPtr<AActor> LastActor;

	// 2. Define a custom ECollisionChannel for item tracing in Project Settings -> Collision.
	// - 创建一个名为“ItemTrace”的新“追踪通道”，默认响应为“忽略”。

	// 3. 将 ECollisionChannel 公开为 UPROPERTY，以便在编辑器中设置。
	// - 使用 TEnumAsByte<ECollisionChannel> 将通道公开到“详细信息”面板。
	// - UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	// - TEnumAsByte<ECollisionChannel> ItemTraceChannel;

	// 4. 实现一个函数（例如 TraceForItem()）来执行线描。
	// - 使用 DeprojectScreenToWorld 获取从玩家屏幕中心开始的线描的起点和终点。
	// - 使用自定义 ItemTraceChannel 执行 LineTraceSingleByChannel。
	// - 线描将仅检测设置为阻挡此特定通道的 Actor。

	// 5. 更新 ThisActor 和 LastActor 指针。
	// - 赋值 LastActor = ThisActor;
	// - 赋值 ThisActor = HitResult.GetActor();

	// 6. 实现逻辑来处理状态转换（当瞄准新物品或停止追踪时）。
	// - 检查 ThisActor 是否有效且不等于 LastActor（正在瞄准新的 Actor）。
	// - 执行“OnStartTracing”逻辑，例如突出显示新的 Actor 或显示交互提示。
	// - 检查 LastActor 是否有效且不等于 ThisActor（瞄准已移开）。
	// - 执行“OnStopTracing”逻辑，例如取消突出显示旧的 Actor 或隐藏提示。
	
	if(!IsValid(GEngine) || !IsValid(GEngine->GameViewport)) return;

	//Get ViewSize 
	FVector2D ViewportSize;
	GEngine->GameViewport->GetViewportSize(ViewportSize);
	//Get View Centern Point
	const FVector2D ViewportCenter = ViewportSize / 2.f;

	
	FVector TraceStart;
	FVector Forword;
	
	//TODO 2D Location transform to 3D World Locaition
	
	if(!UGameplayStatics::DeprojectScreenToWorld(this,ViewportCenter,TraceStart,Forword)) return;
	
	const FVector TraceEnd = TraceStart + Forword * TraceLength;
	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult,TraceStart,TraceEnd,ItemTraceChannel);

	LastActor = ThisActor;
	ThisActor = HitResult.GetActor();

	if(!ThisActor.IsValid())
	{
		if(IsValid(HUDWidget)) HUDWidget -> HidePickupMessage();
	}
	
	if(ThisActor == LastActor) return;

	if(ThisActor.IsValid())
	{
		
		if(UActorComponent* HightLightable = ThisActor->FindComponentByInterface(UInv_HighLightable::StaticClass()); IsValid(HightLightable))
		{
			IInv_HighLightable::Execute_HighLight(HightLightable);
		}
		
		UInv_ItemComponent* ItemComponent = ThisActor->FindComponentByClass<UInv_ItemComponent>();
		if(!IsValid(ItemComponent)) return;

		if(IsValid(HUDWidget)) HUDWidget->ShowPickupMessage(ItemComponent->GetPickupMessage());
	}
	

	if(LastActor.IsValid())
	{
		if(UActorComponent* HightLightable = LastActor->FindComponentByInterface(UInv_HighLightable::StaticClass()); IsValid(HightLightable))
		{
			IInv_HighLightable::Execute_UnHighLight(HightLightable);
		}
	}
}


