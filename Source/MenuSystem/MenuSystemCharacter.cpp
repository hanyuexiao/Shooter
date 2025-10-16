// Copyright Epic Games, Inc. All Rights Reserved.

#include "MenuSystemCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AMenuSystemCharacter
AMenuSystemCharacter::AMenuSystemCharacter():
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this,&ThisClass::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this,&ThisClass::OnFindSessionsComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this,&ThisClass::OnJoinSessionComplete))
		
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		 OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMenuSystemCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AMenuSystemCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMenuSystemCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMenuSystemCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AMenuSystemCharacter::CreateGameSession()
{
	//Called when pressing the 1 key
	if (!OnlineSessionInterface.IsValid()) return;

	auto ExistingSession = OnlineSessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
	{
		OnlineSessionInterface->DestroySession(NAME_GameSession);
	}
	OnlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
	SessionSettings->bIsLANMatch = false;
	SessionSettings->NumPublicConnections = 4; 
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bAllowJoinViaPresence = true;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->bUsesPresence = true;
	SessionSettings->bUseLobbiesIfAvailable = true;
	SessionSettings->Set(FName("MatchType"),FString("FreeForAll"),EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	// Name_GameSession来标识该Session的作用是什么，是干什么用的
	OnlineSessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(),NAME_GameSession,*SessionSettings);
}

void AMenuSystemCharacter::JoinGameSession()
{
	// Find game sessions
	if (!OnlineSessionInterface.IsValid()) return;

	OnlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
	
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch -> MaxSearchResults = 10000;
	SessionSearch -> bIsLanQuery = false;
	SessionSearch -> QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	OnlineSessionInterface -> FindSessions(*LocalPlayer -> GetPreferredUniqueNetId(),SessionSearch.ToSharedRef());
	
}

void AMenuSystemCharacter::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(        
				-1,
				15.f,
				FColor::Blue,
				FString::Printf(TEXT("Created session: %s"),*SessionName.ToString())
				);
		}

		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(FString("Lobby?listen"));
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString(TEXT("Failed to create session!"))
				);
		}
	}
}

void AMenuSystemCharacter::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (!OnlineSessionInterface.IsValid()) return;
	
	// 先检查搜索本身是否成功
	if (!bWasSuccessful || !SessionSearch.IsValid())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, TEXT("Find Sessions failed!"));
		}
		return;
	}
	
	OnlineSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
	
	// 如果成功，再处理结果
	for (auto Result : SessionSearch->SearchResults)
	{
		auto Id = Result.GetSessionIdStr();
		FString User = Result.Session.OwningUserName;
		FString MatchType;
		Result.Session.SessionSettings.Get(FName("MatchType"), MatchType);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Cyan,
				FString::Printf(TEXT("ID: %s, User: %s"),*Id,*User)
				);
		}
		if (MatchType == FString("FreeForAll"))
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					15.f,
					FColor::Cyan,
					FString::Printf(TEXT("Joining Match Type: %s"), *MatchType)
				);
			}

			// Steam complains about bUseLobbiesIfAvailable and bUsesPresence must match
			// so changing these in the results, which should NOT be needed!
			// Joining fails otherwise, though, so doing it for now.
			Result.Session.SessionSettings.bUseLobbiesIfAvailable = true;
			Result.Session.SessionSettings.bUsesPresence = true;

			const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
			OnlineSessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, Result);

			break;
		}
	}
	
}

void AMenuSystemCharacter::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{

	// 调试第1步: 检查这个函数是否被调用，以及加入的结果是什么
	if (GEngine)
	{
		// 将枚举类型转换为整数，方便打印
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Green,
			FString::Printf(TEXT("OnJoinSessionComplete called. SessionName: %s, Result: %d"), *SessionName.ToString(), static_cast<int32>(Result))
		);
	}

	if (!OnlineSessionInterface.IsValid())	return;

	// 调试第2步: 只有当Result为Success时，才进行跳转
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		FString Address;
		// 调试第3步: 检查是否能成功获取到服务器的连接地址
		// 注意：这里使用传入的 SessionName 参数，而不是硬编码的 NAME_GameSession
		if (OnlineSessionInterface->GetResolvedConnectString(SessionName, Address))
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					15.f,
					FColor::Yellow,
					FString::Printf(TEXT("Connect string FOUND: %s"), *Address)
				);
			}

			// 调试第4步: 检查PlayerController是否有效
			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController)
			{
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(
						-1,
						15.f,
						FColor::Cyan,
						TEXT("PlayerController is valid. Attempting ClientTravel...")
					);
				}
				// 执行客户端跳转
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
			else
			{
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(
						-1,
						15.f,
						FColor::Red,
						TEXT("ERROR: PlayerController is NOT valid!")
					);
				}
			}
		}
		else
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					15.f,
					FColor::Red,
					TEXT("ERROR: GetResolvedConnectString FAILED!")
				);
			}
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString::Printf(TEXT("Join session was NOT successful. Result code: %d"), static_cast<int32>(Result))
			);
		}
	}
}


void AMenuSystemCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AMenuSystemCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}
