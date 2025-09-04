// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Soul.h"
#include "Interfaces/PickupInterface.h"
#include "Kismet/KismetSystemLibrary.h"
//#include "HitRe"

ASoul::ASoul()
{
	SoulsMin = 1;
	SoulsMax = 10;
	OffsetRate = -15.0f;
}

void ASoul::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const float LocationZ = GetActorLocation().Z;
	if (LocationZ > DesiredZ)
	{
		const FVector DeltaLocation = FVector(0.0f, 0.0f, DeltaTime * OffsetRate);
		AddActorWorldOffset(DeltaLocation);
	}
}



void ASoul::BeginPlay()
{
	Super::BeginPlay();

	FVector StartLocation = GetActorLocation();
	FVector EndLocation = StartLocation - FVector(0.0f, 0.0f, 2000.f);
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery1);
	TArray<TObjectPtr<AActor>> ActorsToIgnore;
	ActorsToIgnore.Add(GetOwner());
	EDrawDebugTrace::Type DrawDebugType = EDrawDebugTrace::Persistent;
	FHitResult OutHit;

	bool bDidTrace = UKismetSystemLibrary::LineTraceSingleForObjects(
		this,
		StartLocation,
		EndLocation,
		ObjectTypes,
		false,
		ActorsToIgnore,
		DrawDebugType,
		OutHit,
		true
	);
	if (bDidTrace)
	{
		DesiredZ = OutHit.ImpactPoint.Z + 75.0f;
	}

}

void ASoul::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	IPickupInterface* PickupInterface = Cast<IPickupInterface>(OtherActor);

	if (PickupInterface)
	{
		PickupInterface->PickupSoul(FMath::RandRange(SoulsMin, SoulsMax));
		SpawnPickupSystem();
		SpawnPickupSound();
		Destroy();
	}

}
