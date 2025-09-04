// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Soul.generated.h"

// Forward Declarations 
class UNiagaraComponent;

UCLASS()
class SLASHGAME_API ASoul : public AItem
{
	GENERATED_BODY()
public:
	ASoul();

	virtual void Tick(float DeltaTime) override;

protected: 
	virtual void BeginPlay() override;

	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
private:

	UPROPERTY(EditAnywhere, Category = "Souls")
	int32 SoulsMin;

	UPROPERTY(EditAnywhere, Category = "Souls")
	int32 SoulsMax;

	float DesiredZ;

	UPROPERTY(EditAnywhere, Category = "Souls")
	float OffsetRate;
};
