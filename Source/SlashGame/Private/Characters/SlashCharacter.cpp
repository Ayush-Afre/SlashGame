// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/SlashCharacter.h"

#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GroomComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/AttributeComponent.h"
#include "HUD/SlashHUD.h"
#include "HUD/SlashOverlay.h"

#include "Items/Item.h"
#include "Items/Weapon/Weapon.h"


// Sets default values
ASlashCharacter::ASlashCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 600.0f, 0.0f);

	GetMesh()->SetCollisionObjectType(ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 300.0f;
	SpringArm->bUsePawnControlRotation = true;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>("ViewCamera");
	ViewCamera->SetupAttachment(SpringArm);

	Hair = CreateDefaultSubobject<UGroomComponent>("Hair");
	Hair->SetupAttachment(GetMesh());
	Hair->AttachmentName = FString("head");

	EyeBrows = CreateDefaultSubobject<UGroomComponent>("EyeBrows");
	EyeBrows->SetupAttachment(GetMesh());
	EyeBrows->AttachmentName = FString("head");

	CurrentCharacterState = ECharacterState::ECS_Unequipped;
	CurrentActionState = EActionState::EAS_Unoccupied;

	AutoPossessPlayer = EAutoReceiveInput::Player0;

	AnimBlendOutTime = 0.25f;
}

// Called when the game starts or when spawned
void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();
	Tags.Add(FName("EngageableTarget"));

	TObjectPtr<APlayerController> PlayerController = Cast<APlayerController>(GetController());

	if (PlayerController)
	{
		TObjectPtr<UEnhancedInputLocalPlayerSubsystem> SubSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (SubSystem)
		{
			SubSystem->AddMappingContext(SlashMappingContext, 0);
		}
	}

	// Add A Delay to Initalize Overlay because it might not be constructed yet
	FTimerHandle InitalizeOverlay;
	GetWorldTimerManager().SetTimer(InitalizeOverlay, this, &ASlashCharacter::InitializeSlashOverlay, 0.1f);


}

void ASlashCharacter::InitializeSlashOverlay()
{
	TObjectPtr<APlayerController> PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		TObjectPtr<ASlashHUD> SlashHUD = Cast<ASlashHUD>(PlayerController->GetHUD());
		if (SlashHUD)
		{
			SlashOverlay = SlashHUD->GetSlashOverlay();
			if (Attributes && SlashOverlay)
			{
				SlashOverlay->SetHealthPercent(Attributes->GetHealthPercent());
				SlashOverlay->SetStaminaPercent(Attributes->GetStaminaPercent());
				SlashOverlay->SetGoldNumber(0);
				SlashOverlay->SetSoulsNumber(0);
			}
		}
	}
}

// Called every frame
void ASlashCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ASlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	TObjectPtr<UEnhancedInputComponent> EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(Movement, ETriggerEvent::Triggered, this, &ASlashCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Jump);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Interact);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Attack);
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Dodge);

	}
}

void ASlashCharacter::Move(const FInputActionValue& Value)
{
	if (CurrentActionState != EActionState::EAS_Unoccupied) return;
	const FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller)
	{
		const FRotator ControllerRotation = GetControlRotation();
		const FRotator YawRotation(0.0f, ControllerRotation.Yaw, 0.0f);

		const FVector DirectionForward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector DirectionRight = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(DirectionForward, MovementVector.Y);
		AddMovementInput(DirectionRight, MovementVector.X);
	}
}

void ASlashCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Direction = Value.Get<FVector2D>();
	if (Controller)
	{
		AddControllerYawInput(Direction.X);
		AddControllerPitchInput(Direction.Y);
	}
}

void ASlashCharacter::Interact()
{
	TObjectPtr<AWeapon> OverlappingWeapon = Cast<AWeapon>(OverlappingItem);

	if (OverlappingWeapon)
	{
		if (EquippedWeapon) {
			EquippedWeapon->Destroy();
		}
		OverlappingWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
		CurrentCharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
		EquippedWeapon = OverlappingWeapon;
		OverlappingItem = nullptr;
	}
	else
	{
		if (canUnequip())
		{
			PlayEquipMontage(FName("Unequip"));
			CurrentCharacterState = ECharacterState::ECS_Unequipped;
		}
		else if (canEquip() && EquippedWeapon)
		{
			PlayEquipMontage(FName("Equip"));
			CurrentCharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
		}
	}
}


void ASlashCharacter::Attack()
{
	Super::Attack();
	// Do not play attack if there is no weapon
	if (canAttack())
	{
		PlayAttackMontage();
		CurrentActionState = EActionState::EAS_Attacking;
	}
}

void ASlashCharacter::Dodge()
{
	if (!IsOccupied() || !HasEnoughStamina()) return;
	ClearStaminaRegenCooldown();
	ClearStaminaRegen();
	PlayDodgeMontage();
	CurrentActionState = EActionState::EAS_Dodge;
	if (Attributes) 
	{
		Attributes->UseStamina();
		SetHUDStamina();
	}
	StaminaRegenCooldownStart();
}

void ASlashCharacter::Jump()
{
	if (CurrentActionState == EActionState::EAS_Unoccupied)
	{
		Super::Jump();
	}
}

void ASlashCharacter::PlayEquipMontage(const FName& SectionName)
{
	TObjectPtr<UAnimInstance> AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquipMontage)
	{
		AnimInstance->Montage_Play(EquipMontage);
		AnimInstance->Montage_JumpToSection(SectionName, EquipMontage);
		CurrentActionState = EActionState::EAS_EquippingWeapon;
	}
}

void ASlashCharacter::PlayDodgeMontage()
{
	if (DodgeMontage && DodgeSectionNames.Num() > 0)
	{
		PlayRandomMontageSection(DodgeMontage, DodgeSectionNames);
	}
}

void ASlashCharacter::AttackEnd()
{
	CurrentActionState = EActionState::EAS_Unoccupied;

	TObjectPtr<UAnimInstance> AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->StopAllMontages(AnimBlendOutTime);
	}
}

void ASlashCharacter::DodgeEnd()
{
	CurrentActionState = EActionState::EAS_Unoccupied;
}


void ASlashCharacter::Disarm()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
	}
}

void ASlashCharacter::EquipWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
	}
}

void ASlashCharacter::FinishedEquipping()
{
	CurrentActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::HitReactEnd()
{
	CurrentActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter);

	UpdateWeaponCollision(ECollisionEnabled::NoCollision);
	CurrentActionState = EActionState::EAS_HitReaction;


}

void ASlashCharacter::SetOverlappingItem(AItem* Item)
{
	OverlappingItem = Item;
}

void ASlashCharacter::PickupSoul(int32 Soul)
{
	if (Attributes && SlashOverlay)
	{
		SlashOverlay->SetSoulsNumber(Attributes->UpdateSoulCount(Soul));

	}
}

void ASlashCharacter::PickupGold(int32 Gold)
{
	if (Attributes && SlashOverlay)
	{
		SlashOverlay->SetGoldNumber(Attributes->UpdateGoldCount(Gold));
	}
}

bool ASlashCharacter::canAttack()
{
	return CurrentCharacterState != ECharacterState::ECS_Unequipped && CurrentActionState == EActionState::EAS_Unoccupied;
}

bool ASlashCharacter::canUnequip()
{
	return CurrentActionState == EActionState::EAS_Unoccupied && CurrentCharacterState != ECharacterState::ECS_Unequipped;
}

bool ASlashCharacter::canEquip()
{
	return CurrentActionState == EActionState::EAS_Unoccupied && CurrentCharacterState == ECharacterState::ECS_Unequipped;
}

bool ASlashCharacter::IsOccupied()
{
	return CurrentActionState == EActionState::EAS_Unoccupied;
}

bool ASlashCharacter::HasEnoughStamina()
{
	return Attributes && Attributes->GetStamina() > Attributes->GetDodgeCost();
}

void ASlashCharacter::StartStaminaRegen()
{
	GetWorldTimerManager().SetTimer(RegenStaminaTimer, this, &ASlashCharacter::RegenStamina, 0.1f,true);

}

void ASlashCharacter::RegenStamina()
{

	if (!Attributes->IsStaminaFull())
	{
		Attributes->RegenerateStamina();
		SetHUDStamina();
	}
	else 
	{
		ClearStaminaRegen();
	}
}

void ASlashCharacter::ClearStaminaRegen()
{
	GetWorldTimerManager().ClearTimer(RegenStaminaTimer);

}

void ASlashCharacter::StaminaRegenCooldownStart()
{
	GetWorldTimerManager().SetTimer(WaitForStaminaRegenStart, this, &ASlashCharacter::StartStaminaRegen, 2.0f);
}

void ASlashCharacter::ClearStaminaRegenCooldown()
{
	GetWorldTimerManager().ClearTimer(WaitForStaminaRegenStart);
}

float ASlashCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	SetHUDHealth();
	return DamageAmount;
}

void ASlashCharacter::SetHUDHealth()
{
	if (SlashOverlay && Attributes)
	{
		SlashOverlay->SetHealthPercent(Attributes->GetHealthPercent());
	}
}

void ASlashCharacter::SetHUDStamina()
{
	if (SlashOverlay && Attributes)
	{
		SlashOverlay->SetStaminaPercent(Attributes->GetStaminaPercent());
	}
}

void ASlashCharacter::Die_Implementation()
{
	Super::Die_Implementation();
	CurrentActionState = EActionState::EAS_Dead;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Tags.Add(FName("Dead"));
}
