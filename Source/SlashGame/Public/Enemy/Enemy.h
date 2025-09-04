// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "Characters/CharacterTypes.h"
#include "Enemy.generated.h"

// Forward Declrations
class USoundBase;
class UHealthBarComponent;
class AAIController;
class UPawnSensingComponent;
class AWeapon;
class UMotionWarpingComponent;
class ASoul;

UCLASS()
class SLASHGAME_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

	// BASE FUNCTIONS
public:
	AEnemy();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	//COMPONENTS
private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UHealthBarComponent> HealthBarWidget;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPawnSensingComponent> PawnSensing;

	void SetHealthBarVisibility(bool Visibility);

	// HIT REACTION
protected:
	virtual void Die_Implementation() override;

	float DeathLifeSpan;

public:
	bool isDead();

	void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// PATROLLING
protected:
	void CheckPatrolTarget();

	bool InTargetRange(AActor* Target, double Radius);

	void MoveToTarget(AActor* Target);

	AActor* ChoosePatrolTarget();

private:
	UPROPERTY()
	TObjectPtr<AAIController> EnemyController;

	UPROPERTY(EditAnywhere)
	double PatrolRadius;

	UPROPERTY(EditAnywhere, category = "AI Navigation")
	float WaitMin;

	UPROPERTY(EditAnywhere, category = "AI Navigation")
	float WaitMax;

	void LoseInterest();

	void StartPatrolling();

	UPROPERTY(EditAnywhere, Category = "Combat")
	float PatrollingSpeed;

	//Enums
protected:
	UPROPERTY(BlueprintReadOnly)
	EEnemyState EnemyState;

private:
	// Current Patrol Target
	UPROPERTY(EditInstanceOnly, category = "AI Navigation")
	TObjectPtr<AActor> PatrolTarget;

	// List of patrol targets
	UPROPERTY(EditInstanceOnly, category = "AI Navigation")
	TArray<TObjectPtr<AActor>> PatrolTargets;

	FTimerHandle PatrolTimer;

	void PatrolTimerFinished();

	void ClearPatrolTimer();

	//Chasing  
protected:
	void CheckCombatTarget();

	UFUNCTION()
	void PawnSeen(APawn* SeenPawn);

private:
	UPROPERTY()
	TObjectPtr<AActor> CombatTarget;

	UPROPERTY(EditAnywhere, Category = "Combat")
	double CombatRadius;

	UPROPERTY(EditAnywhere, Category = "Combat")
	double AttackRadius;

	UPROPERTY(EditAnywhere, Category = "Combat")
	double AcceptanceRadius;

	void StartChasing();

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ChasingSpeed;

	bool IsOutsideCombatRange();

	bool IsChasing();

	// Weapon

	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> WeaponClass;

	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackMin;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackMax;

	bool IsOutsideAttackRange();

	bool IsInsideAttackRadius();

	bool IsAttacking();

	bool IsEngaged();

	void StartAttackTimer();

	void ClearAttackTimer();

	UPROPERTY(EditAnywhere, Category = "Combat")
	double WarpTargetDistance;

public:
	virtual void Destroyed() override;

protected:
	virtual bool canAttack() override;

	virtual void HandleDamage(float DamageAmount) override;

	virtual void Attack() override;

	UFUNCTION(BlueprintCallable)
	void UpdateTranslationWarpTarget();

	UFUNCTION(BlueprintCallable)
	void UpdateRotationWarpTarget();

	virtual void AttackEnd() override;

	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<ASoul> SoulClass;

	void SpawnSoul();
};
