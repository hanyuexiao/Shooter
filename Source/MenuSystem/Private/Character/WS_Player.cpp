#include "Character/WS_Player.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Weapons/WS_Weapon.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "MenuSystem/MenuSystem.h"
#include "Character/Controller/WS_PlayerControll.h"
#include "GameModes/BlasterGamemode.h"
#include "TimerManager.h"
#include "Character/BlasterPlayerState.h"
#include "Component/ActorComponent/BuffComponent.h"
#include "Component/ActorComponent/LagCompensationComponent.h"
#include "Components/BoxComponent.h"
#include "GameState/BlasterGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"


// Sets default values
AWS_Player::AWS_Player()
{
	PrimaryActorTick.bCanEverTick = true;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom -> TargetArmLength = 600.f;
	CameraBoom -> bUsePawnControlRotation = true;
	
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom,USpringArmComponent::SocketName);
	FollowCamera -> bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement() -> bOrientRotationToMovement = true;
	
	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent -> SetIsReplicated(true);

	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);

	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));
	
	GetCharacterMovement() -> NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent() -> SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh() -> SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);
	
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached Grenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/** 
	* Hit boxes for server-side rewind
	*/

	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxes.Add(FName("head"), head);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitCollisionBoxes.Add(FName("pelvis"), pelvis);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"), spine_02);

	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_03"), spine_03);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitCollisionBoxes.Add(FName("upperarm_l"), upperarm_l);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitCollisionBoxes.Add(FName("upperarm_r"), upperarm_r);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	HitCollisionBoxes.Add(FName("hand_l"), hand_l);

	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	HitCollisionBoxes.Add(FName("hand_r"), hand_r);
	
	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitCollisionBoxes.Add(FName("thigh_l"), thigh_l);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitCollisionBoxes.Add(FName("thigh_r"), thigh_r);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	HitCollisionBoxes.Add(FName("calf_l"), calf_l);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxes.Add(FName("calf_r"), calf_r);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitCollisionBoxes.Add(FName("foot_l"), foot_l);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	HitCollisionBoxes.Add(FName("foot_r"), foot_r);

	for (auto Box : HitCollisionBoxes)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void AWS_Player::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AWS_Player, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(AWS_Player, Health);
	DOREPLIFETIME(AWS_Player, Shield);
	DOREPLIFETIME(AWS_Player, bDisableGameplay);
}

void AWS_Player::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void AWS_Player::Elim(bool bPlayerLeftGame)
{
	
	DropOrDestroyWeapons();
	MulticastElim(bPlayerLeftGame);
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&AWS_Player::ElimTimerFinished,
		ElimDelay
	);
}

void AWS_Player::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;
	if (PlayerController)
	{
		PlayerController->SetHUDWeaponAmmo(0);
	}
	
	bElimmed = true;
	PlayElimMontage();

	// Start dissolve effect
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	StartDissolve();

	// Disable character movement
	bDisableGameplay = true;
	GetCharacterMovement() -> DisableMovement();
	if (CombatComponent)
	{
		CombatComponent -> FireButtonPressed(false);
	}
	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Spawn elim bot
	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ElimBotEffect,
			ElimBotSpawnPoint,
			GetActorRotation()
		);
	}
	if (ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			ElimBotSound,
			GetActorLocation()
		);
	}
	
	bool bHideSniperScope = IsLocallyControlled() && 
		CombatComponent && 
		CombatComponent->bAiming && 
		CombatComponent->EquippedWeapon && 
		CombatComponent->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	if (bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
	GetWorldTimerManager().SetTimer(
     		ElimTimer,
     		this,
     		&AWS_Player::ElimTimerFinished,
     		ElimDelay
     	);
}

void AWS_Player::ElimTimerFinished()
{
	ABlasterGamemode* BlasterGamemode = GetWorld()->GetAuthGameMode<ABlasterGamemode>();
	if (BlasterGamemode && !bLeftGame)
	{
		BlasterGamemode->RequestRespawn(this,Controller);
	}
	if (bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
	}
}

void AWS_Player::Destroyed()
{
	Super::Destroyed();

	if (ElimBotComponent)
	{
		ElimBotComponent -> DestroyComponent();
	}
	
	ABlasterGamemode* BlasterGameMode = Cast<ABlasterGamemode>(UGameplayStatics::GetGameMode(this));
	bool bMatchNotInProgress = BlasterGameMode && BlasterGameMode->GetMatchState() != MatchState::InProgress;
	
	if (CombatComponent && CombatComponent->EquippedWeapon && bMatchNotInProgress)
	{
		CombatComponent->EquippedWeapon->Destroy();
	}
}

void AWS_Player::SpawnDefaultWeapon()
{
	ABlasterGamemode* BlasterGameMode = Cast<ABlasterGamemode>(UGameplayStatics::GetGameMode(this));
	UWorld* World = GetWorld();
	if (BlasterGameMode && World && !bElimmed && DefaultWeaponClass)
	{
		AWS_Weapon* StartingWeapon = World->SpawnActor<AWS_Weapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true;
		if (CombatComponent)
		{
			CombatComponent->EquipWeapon(StartingWeapon);
		}
	}
}

void AWS_Player::ServerLeaveGame_Implementation()
{
	ABlasterGamemode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGamemode>();
	BlasterPlayerState = BlasterPlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;
	if (BlasterGameMode && BlasterPlayerState)
	{
		BlasterGameMode->PlayerLeftGame(BlasterPlayerState);
	}
}

void AWS_Player::MulticastGainedTheLead_Implementation()
{
	if (CrownSystem == nullptr) return;
	if (CrownComponent == nullptr)
	{
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CrownSystem,
			GetCapsuleComponent(),
			FName(),
			GetActorLocation() + FVector(0.f, 0.f, 110.f),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}
	if (CrownComponent)
	{
		CrownComponent->Activate();
	}
}

void AWS_Player::MulticastLostTheLead_Implementation()
{
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
}

void AWS_Player::BeginPlay()
{
	Super::BeginPlay();

	SpawnDefaultWeapon();
	UpdateHUDAmmo();
	UpdateHUDHealth();
	UpdateHUDShield();
	
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AWS_Player::ReceiveDamage);
	}
	if (AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}
}

void AWS_Player::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	RotateInPlace(DeltaTime);
	HideCameraIfCharacterClose();
	PollInit();
}

void AWS_Player::RotateInPlace(float DeltaTime)
{
	if (bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
}

void AWS_Player::DropOrDestroyWeapon(AWS_Weapon* Weapon)
{
	if (Weapon == nullptr) return;
	if (Weapon->bDestroyWeapon)
		{
			Weapon->Destroy();
		}
	else
		{
			Weapon->Dropped();
		}
}

void AWS_Player::DropOrDestroyWeapons()
{
	if (CombatComponent)
	{
		if (CombatComponent->EquippedWeapon)
		{
			DropOrDestroyWeapon(CombatComponent->EquippedWeapon);
		}
		if (CombatComponent->SecondaryWeapon)
		{
			DropOrDestroyWeapon(CombatComponent->SecondaryWeapon);
		}
	}
}

void AWS_Player::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent -> BindAction(MoveForwardAction,ETriggerEvent::Triggered,this,&AWS_Player::MoveForward);
		EnhancedInputComponent -> BindAction(MoveRightAction,ETriggerEvent::Triggered,this,&AWS_Player::MoveRight);
		EnhancedInputComponent -> BindAction(TurnAction,ETriggerEvent::Triggered,this,&AWS_Player::Turn);
		EnhancedInputComponent -> BindAction(LookUpAction,ETriggerEvent::Triggered,this,&AWS_Player::LookUp);
		EnhancedInputComponent -> BindAction(JumpAction,ETriggerEvent::Started,this,&ACharacter::Jump);
		EnhancedInputComponent -> BindAction(EquipWeaponAction,ETriggerEvent::Started,this,&AWS_Player::EquipButtonPressed);
		EnhancedInputComponent -> BindAction(CrouchAction,ETriggerEvent::Started,this,&AWS_Player::CrouchButtonPressed);
		EnhancedInputComponent -> BindAction(AimAction,ETriggerEvent::Triggered,this,&AWS_Player::AimButtonPressed);
		EnhancedInputComponent -> BindAction(AimAction,ETriggerEvent::Completed,this,&AWS_Player::AimButtonRelease);
		EnhancedInputComponent -> BindAction(FireAction,ETriggerEvent::Started,this,&AWS_Player::FireButtonPressed);
		EnhancedInputComponent -> BindAction(FireAction,ETriggerEvent::Completed,this,&AWS_Player::FireButtonReleased);
		EnhancedInputComponent -> BindAction(ReloadAction,ETriggerEvent::Started,this,&AWS_Player::ReloadButtonPressed);
		EnhancedInputComponent -> BindAction(GrenadeAction,ETriggerEvent::Started,this,&AWS_Player::GrenadeButtonPressed);
	}
}

void AWS_Player::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (CombatComponent)
	{
		CombatComponent->Player = this;
	}
	if (Buff)
	{
		Buff->Character = this;
		Buff->SetInitialSpeeds(
			GetCharacterMovement()->MaxWalkSpeed, 
			GetCharacterMovement()->MaxWalkSpeedCrouched
		);
		Buff->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}
	if (LagCompensation)
	{
		LagCompensation->Character = this;
		if (Controller)
		{
			LagCompensation->Controller = Cast<AWS_PlayerControll> (Controller);
		}
	}
}

void AWS_Player::PlayFireMontage(bool bAiming)
{
	if (CombatComponent == nullptr || CombatComponent -> EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh() -> GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance -> Montage_Play(FireWeaponMontage);
		FName SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		UE_LOG(LogTemp,Warning,TEXT("SectionName: %s"), *(SectionName.ToString()));
		AnimInstance -> Montage_JumpToSection(SectionName);
	} 
}

void AWS_Player::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);

		int32 RandomIndex = FMath::RandRange(1,3);

		FString SectionNameString = FString::Printf(TEXT("Death_%d"),RandomIndex);

		FName SectionName = FName(*SectionNameString);
		
		AnimInstance->Montage_JumpToSection(SectionName);
	}

}

void AWS_Player::PlayReloadMontage()
{
	if (CombatComponent == nullptr || CombatComponent -> EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		
		FName SectionName;
		
		switch (CombatComponent -> EquippedWeapon -> GetWeaponType())
		{
			case EWeaponType::EWT_AssaultRifle:
				SectionName = FName("Rifle");
				break;
				
			case EWeaponType::EWT_RocketLauncher:
				SectionName = FName("RocketLauncher");
				break;
			
			case EWeaponType::EWT_Pistol:
				SectionName = FName("Pistol");
				break;
			case EWeaponType::EWT_SubmachineGun:
				SectionName = FName("SubmachineGun");
				break;
			case EWeaponType::EWT_Shotgun:
				SectionName = FName("Shotgun");
				break;
			case EWeaponType::EWT_SniperRifle:
				SectionName = FName("SniperRifle");
				break;
			case EWeaponType::EWT_GrenadeLauncher:
				SectionName = FName("GrenadeLauncher");
				break;
			default:
				SectionName = FName("Rifle");
				break;;
		}

		AnimInstance->Montage_JumpToSection(SectionName);
		
	}
}

void AWS_Player::PlayHitReactMontage()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AWS_Player::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

void AWS_Player::PlaySwapMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && SwapMontage)
	{
		AnimInstance->Montage_Play(SwapMontage);
	}
}

void AWS_Player::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,class AController* InstigatorController, AActor* DamageCauser)
{
	if (bElimmed) return;
	float DamageToHealth = Damage;
	
	if (Shield > 0.f)
	{
		if (Shield >= Damage)
		{
			Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield);
			DamageToHealth = 0.f;
		}
		else
		{
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage);
			Shield = 0.f;
		}
	}
	Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);
	UpdateHUDHealth();
	UpdateHUDShield();
	PlayHitReactMontage();

	if (Health == 0.f)
	{
		ABlasterGamemode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGamemode>();
		if (BlasterGameMode)
		{
			PlayerController = PlayerController == nullptr ? Cast<AWS_PlayerControll>(Controller) : PlayerController;
			AWS_PlayerControll* AttackerController = Cast<AWS_PlayerControll>(InstigatorController);
			BlasterGameMode->PlayerEliminated(this, PlayerController, AttackerController);
		}
	}
}

void AWS_Player::MoveForward(const FInputActionValue& AxisValue)
{
	if (bDisableGameplay) return;
	
	float Value = AxisValue.Get<float>();
	
	if (Controller != nullptr && Value != 0.0f)
	{
		const FRotator YawRotator(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

		const FVector Direction(FRotationMatrix(YawRotator).GetUnitAxis(EAxis::X));

		AddMovementInput(Direction,Value);
	}
}

void AWS_Player::MoveRight(const FInputActionValue& AxisValue)
{
	if (bDisableGameplay) return;
	
	float Value = AxisValue.Get<float>();
	
	if (Controller != nullptr && Value != 0.0f)
	{
		const FRotator YawRotator(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

		const FVector Direction(FRotationMatrix(YawRotator).GetUnitAxis(EAxis::Y));

		AddMovementInput(Direction,Value);
	}
}

void AWS_Player::Turn(const FInputActionValue& AxisValue)
{
	float Value = AxisValue.Get<float>();

	AddControllerYawInput(Value);
}

void AWS_Player::LookUp(const FInputActionValue& AxisValue)
{
	float Value = AxisValue.Get<float>();

	AddControllerPitchInput(-Value);
}

void AWS_Player::Jump(const FInputActionValue& AxisValue)
{
	if (bDisableGameplay) return;
	
	float Value = AxisValue.Get<float>();
	AddMovementInput(GetActorUpVector() * Value);
}

void AWS_Player::FireButtonPressed()
{
	if (CombatComponent)
	{
		CombatComponent -> FireButtonPressed(true);
	}
}

void AWS_Player::FireButtonReleased()
{
	if (CombatComponent)
	{
		CombatComponent -> FireButtonPressed(false);
	}
}

void AWS_Player::EquipButtonPressed()
{
	if (bDisableGameplay) return;
	
	if (CombatComponent)
	{
		if (CombatComponent->CombatState == ECombatState::ECS_Unoccupied) ServerEquipButtonPressed();
		bool bSwap = CombatComponent->ShouldSwapWeapons() && 
			!HasAuthority() && 
			CombatComponent->CombatState == ECombatState::ECS_Unoccupied && 
			OverlappingWeapon == nullptr;

		if (bSwap)
		{
			PlaySwapMontage();
			CombatComponent->CombatState = ECombatState::ECS_SwappingWeapons;
			bFinishedSwapping = false;
		}
	}
}

void AWS_Player::Server_EquipButtonPressed_Implementation()
{
	if (CombatComponent)
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
	}
}

void AWS_Player::CrouchButtonPressed()
{
	if (bDisableGameplay) return;
	
	bIsCrouched = !bIsCrouched;
	if (bIsCrouched)
	{
		Crouch();
	}
	else
	{
		UnCrouch();
	}
}

void AWS_Player::ReloadButtonPressed()
{
	if (bDisableGameplay) return;
	
	if (CombatComponent)
	{
		CombatComponent -> Reload();
	}
	
}

void AWS_Player::AimButtonPressed()
{
	if (bDisableGameplay) return;
	
	CombatComponent->SetAiming(true);
}

void AWS_Player::AimButtonRelease()
{
	if (bDisableGameplay) return;
	
	CombatComponent->SetAiming(false);
}

void AWS_Player::GrenadeButtonPressed()
{
	if (CombatComponent)
	{
		CombatComponent->ThrowGrenade();
	}
}

float AWS_Player::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void AWS_Player::AimOffset(float DeltaTime)
{
	if (CombatComponent && CombatComponent -> EquippedWeapon == nullptr) return;
	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement() -> IsFalling();
	
	if (Speed == 0.f && !bIsInAir) //Standing still,not jump
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f,GetBaseAimRotation().Yaw,0.f);
		FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation,StartingAimRotation);
		AO_Yaw = -DeltaRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	else if (Speed > 0.f || bIsInAir) // running,or jumping
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f,GetBaseAimRotation().Yaw,0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		//TurnInPlace(DeltaTime);
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;

	CalculateAO_Pitch();

}

void AWS_Player::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	// 网络传输的优化压缩算法会更改值，需要修正回去
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void AWS_Player::SimProxiesTurn()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;
	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	UE_LOG(LogTemp, Warning, TEXT("ProxyYaw: %f"), ProxyYaw);

	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void AWS_Player::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void AWS_Player::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled()) return;
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void AWS_Player::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if (Health < LastHealth)
	{
		PlayHitReactMontage();
	}
}

void AWS_Player::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if (Shield < LastShield)
	{
		PlayHitReactMontage();
	}
}

void AWS_Player::UpdateHUDHealth()
{
	PlayerController = PlayerController == nullptr ? Cast<AWS_PlayerControll>(Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController -> SetHUDHealth(Health,MaxHealth);
	}
}

void AWS_Player::UpdateHUDShield()
{
	PlayerController = PlayerController == nullptr ? Cast<AWS_PlayerControll>(Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDShield(Shield, MaxShield);
	}
}

void AWS_Player::UpdateHUDAmmo()
{
	PlayerController = PlayerController == nullptr ? Cast<AWS_PlayerControll>(Controller) : PlayerController;
	if (PlayerController && CombatComponent && CombatComponent->EquippedWeapon)
	{
		PlayerController->SetHUDCarriedAmmo(CombatComponent->CarriedAmmo);
		PlayerController->SetHUDWeaponAmmo(CombatComponent->EquippedWeapon->GetAmmo());
	}
}

void AWS_Player::PollInit()
{
	if (BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState)
		{
			BlasterPlayerState->AddToScore(0.f);
			BlasterPlayerState->AddToDefeats(0);

			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));

			if (BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(BlasterPlayerState))
			{
				MulticastGainedTheLead();
			}
		}
	}
}

void AWS_Player::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void AWS_Player::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &AWS_Player::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void AWS_Player::SetOverlappingWeapon(AWS_Weapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void AWS_Player::ServerEquipButtonPressed_Implementation()
{
	if (CombatComponent)
	{
		if (OverlappingWeapon)
		{
			CombatComponent->EquipWeapon(OverlappingWeapon);
		}
		else if (CombatComponent->ShouldSwapWeapons())
		{
			CombatComponent->SwapWeapons();
		}
	}
}

void AWS_Player::OnRep_OverlappingWeapon(AWS_Weapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

bool AWS_Player::IsWeaponEquipped()
{
	return (CombatComponent && CombatComponent -> EquippedWeapon);
}

FVector AWS_Player::GetHitTarget() const
{
	if (CombatComponent == nullptr) return FVector();
	return CombatComponent -> HitTarget;

}

ECombatState AWS_Player::GetCombatState() const
{
	if (CombatComponent == nullptr) return ECombatState::ECS_MAX;
	return CombatComponent -> CombatState;
}

bool AWS_Player::IsLocallyReloading()
{
	if (CombatComponent == nullptr) return false;
	return CombatComponent -> bLocallyReloading;
}

const bool AWS_Player::IsAiming()
{
	return (CombatComponent && CombatComponent->bAiming);
}

AWS_Weapon* AWS_Player::GetEquippedWeapon()
{
	if (CombatComponent == nullptr) return nullptr;
	return CombatComponent -> EquippedWeapon;
}




