// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "InputActionValue.h"
#include "CharacterTypes.h"
#include "Interfaces/PickupInterface.h"
#include "SlashCharacter.generated.h"


// Forward Declarations
class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class UGroomComponent;
class AItem;
class ASoul;
class ATreasure;
class UAnimMontage;
class USlashOverlay;


UCLASS()
class SLASHGAME_API ASlashCharacter : public ABaseCharacter, public IPickupInterface
{
	GENERATED_BODY()

	// BASE FUNCTIONS
public:
	ASlashCharacter();

protected:
	virtual void BeginPlay() override;

	void InitializeSlashOverlay();

public:
	virtual void Tick(float DeltaTime) override;

	// PLAYER 
private:
	UPROPERTY(VisibleDefaultsOnly)
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> ViewCamera;

	UPROPERTY(VisibleDefaultsOnly, Category = Hair)
	TObjectPtr<UGroomComponent> Hair;

	UPROPERTY(VisibleDefaultsOnly, Category = Hair)
	TObjectPtr<UGroomComponent> EyeBrows;

	// PLAYER INPUT
protected:
	// Input Actions
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> SlashMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> Movement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> AttackAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> DodgeAction;

private:
	// Input action callbacks

	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void Interact();

	virtual void  Attack() override;

	void Dodge();

public:
	void Jump() override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ANIMATION
protected:
	// Play Montage function

	void PlayEquipMontage(const FName& SectionName);

	void PlayDodgeMontage();

	UPROPERTY(EditAnywhere, Category = "Animation Montages")
	TArray<FName> DodgeSectionNames;

private:
	// Animation montages
	UPROPERTY(EditDefaultsOnly, Category = "Animation Montages")
	TObjectPtr<UAnimMontage> EquipMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation Montages")
	TObjectPtr<UAnimMontage> DodgeMontage;

protected:
	//Blueprint Animation end callbacks
	virtual void AttackEnd() override;

	UFUNCTION(BlueprintCallable)
	virtual void DodgeEnd();

	UFUNCTION(BlueprintCallable)
	void Disarm();

	UFUNCTION(BlueprintCallable)
	void EquipWeapon();

	UFUNCTION(BlueprintCallable)
	void FinishedEquipping();

	UFUNCTION(BlueprintCallable)
	void HitReactEnd();

	// ATTACKING
public:
	FORCEINLINE TObjectPtr<AItem> GetOverlappingItem() const { return OverlappingItem; }

	FORCEINLINE ECharacterState GetCharacterState() const { return CurrentCharacterState; }

	FORCEINLINE void SetCharacterState(ECharacterState State) { CurrentCharacterState = State; }

	void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;

	void SetOverlappingItem(AItem* Item) override;

	void PickupSoul(int32 Soul) override;

	void PickupGold(int32 Gold) override;
private:
	UPROPERTY(VisibleInstanceOnly)
	TObjectPtr<AItem> OverlappingItem;

	virtual bool canAttack() override;

	bool canUnequip();

	bool canEquip();

	bool IsOccupied();

	bool HasEnoughStamina();

	FTimerHandle RegenStaminaTimer;

	FTimerHandle WaitForStaminaRegenStart;

	void StartStaminaRegen();

	void RegenStamina();

	void ClearStaminaRegen();
	
	void StaminaRegenCooldownStart();

	void ClearStaminaRegenCooldown();

	float AnimBlendOutTime;

	// Enums
	ECharacterState CurrentCharacterState;

	EActionState CurrentActionState;

	//Damage related stuff

	TObjectPtr<USlashOverlay> SlashOverlay;

	void SetHUDHealth();

	void SetHUDStamina();
protected:
	virtual void Die_Implementation() override;

public:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;


};
