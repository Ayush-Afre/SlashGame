// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "BaseCharacter.generated.h"

//Forward Declrations
class AWeapon;
class UAttributeComponent;
class UNiagaraSystem;
class UAnimMontage;
class USlashOverlay;

UCLASS()
class SLASHGAME_API ABaseCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

	// BASE FUNCTIONS
public:
	ABaseCharacter();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;


	// ATTACKING
public:

	UFUNCTION(BlueprintCallable)
	void UpdateWeaponCollision(ECollisionEnabled::Type CollisionEnabled);

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAttributeComponent> Attributes;

	UPROPERTY(VisibleAnywhere, Category = Weapon)
	TObjectPtr<AWeapon> EquippedWeapon;

	virtual void Attack();

	UFUNCTION(BlueprintNativeEvent)
	void Die();

	virtual bool canAttack();

	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Death")
	TObjectPtr<UAnimMontage> DeathMontage;

	void PlayHitReactMontage(const FName& SectionName);

	void DirectionalHitReact(const FVector& ImpactPoint);

	void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter);

	bool IsAlive();

	void PlayHitSound(const FVector& ImpactPoint);

	void SpawnHitParticles(const FVector& ImpactPoint);

	virtual void HandleDamage(float DamageAmount);

	virtual void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName);

	UPROPERTY(EditAnywhere)
	TArray<FName> AttackMontageSections;

	virtual int32 PlayAttackMontage();

	int32 PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames);

	UPROPERTY(EditAnywhere)
	TArray<FName> DeathMontageSections;

	virtual int32 PlayDeathMontage();

	void StopAttackMontage();

private:

	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<USoundBase> HitSound;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TObjectPtr<UNiagaraSystem> BloodEffect;
	
};
