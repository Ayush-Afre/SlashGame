// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/HitInterface.h"
#include "BreakableActor.generated.h"

class ATreasure;
class UGeometryCollectionComponent;
class UCapsuleComponent;
class USoundBase;
class UMaterialInstanceDynamic;

UCLASS()
class SLASHGAME_API ABreakableActor : public AActor, public IHitInterface
{
	GENERATED_BODY()
	
public:	
	ABreakableActor();
	
	virtual void Tick(float DeltaTime) override;

	void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter);


protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UCapsuleComponent> Capsule;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USoundBase> HitSound;

	UPROPERTY(BlueprintReadWrite,EditDefaultsOnly)
	TObjectPtr<UMaterialInstanceDynamic> BreakablePotMaterial;


private:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	TObjectPtr<UGeometryCollectionComponent> GeometryCollection;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true))
	TArray<TSubclassOf<ATreasure>> TreasureClasses;

	UFUNCTION()
	void OnActorBreak(const FChaosBreakEvent& BreakEvent);
	
	bool bBroken;
};
