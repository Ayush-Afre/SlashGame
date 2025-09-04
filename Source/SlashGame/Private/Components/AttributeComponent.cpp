// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/AttributeComponent.h"


UAttributeComponent::UAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	CurrentHealth = 100.0f;
	MaxHealth = 100.0f;

	GoldCount = 0;
	SoulCount = 0;

	CurrentStamina = 100.0f;
	MaxStamina = 100.0f;
	DodgeCost = 15.0f;
	StaminaRegenRate = 50.0f;

}

void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAttributeComponent::ReceiveDamage(float Damage)
{
	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);
}

float UAttributeComponent::GetHealthPercent()
{
	return CurrentHealth / MaxHealth;
}

float UAttributeComponent::GetStaminaPercent()
{
	return CurrentStamina / MaxStamina;
}

void UAttributeComponent::RegenerateStamina()
{
	TObjectPtr<UWorld> World = GetWorld();
	if (World)
	{
		CurrentStamina = FMath::Clamp(CurrentStamina + StaminaRegenRate * World->GetDeltaSeconds(), 0.0f, MaxStamina);
	}
	UE_LOG(LogTemp, Warning, TEXT("%f"), CurrentStamina);
}

void UAttributeComponent::UseStamina()
{
	CurrentStamina = FMath::Clamp(CurrentStamina - DodgeCost, 0.0f, MaxStamina);
	UE_LOG(LogTemp, Warning, TEXT("%f"), CurrentStamina);
}

bool UAttributeComponent::IsAlive()
{
	return CurrentHealth > 0.0f;
}

int32 UAttributeComponent::UpdateSoulCount(int32 Souls)
{
	SoulCount += Souls;
	return SoulCount;
}

int32 UAttributeComponent::UpdateGoldCount(int32 Gold)
{
	GoldCount += Gold;
	return GoldCount;
}

void UAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}
