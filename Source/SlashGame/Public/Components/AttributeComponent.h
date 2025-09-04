// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SLASHGAME_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttributeComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

private:

	// Current Health
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float CurrentHealth;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float MaxHealth;

	UPROPERTY(VisibleAnywhere, Category = "Actor Attributes")
	int32 GoldCount;

	UPROPERTY(VisibleAnywhere, Category = "Actor Attributes")
	int32 SoulCount;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float CurrentStamina;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float MaxStamina;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float DodgeCost;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float StaminaRegenRate;

public:

	void ReceiveDamage(float Damage);

	float GetHealthPercent();

	float GetStaminaPercent();

	FORCEINLINE float GetStamina() { return CurrentStamina; }

	FORCEINLINE float GetDodgeCost() { return DodgeCost; }

	FORCEINLINE bool IsStaminaFull() { return CurrentStamina == MaxStamina; }

	void RegenerateStamina();

	void UseStamina();

	bool IsAlive();

	int32 UpdateSoulCount(int32 Souls);

	int32 UpdateGoldCount(int32 Gold);
};
