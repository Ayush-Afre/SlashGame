// Fill out your copyright notice in the Description page of Project Settings.


#include "Breakable/BreakableActor.h"

#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Items/Treasure.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"

ABreakableActor::ABreakableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	GeometryCollection = CreateDefaultSubobject<UGeometryCollectionComponent>("GeometryCollection");
	RootComponent = GeometryCollection;

	GeometryCollection->SetGenerateOverlapEvents(true);
	GeometryCollection->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GeometryCollection->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->SetupAttachment(GetRootComponent());
	Capsule->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Capsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);

	bBroken = false;
}


void ABreakableActor::BeginPlay()
{
	Super::BeginPlay();

	BreakablePotMaterial = UKismetMaterialLibrary::CreateDynamicMaterialInstance(GetWorld(), GeometryCollection->GetMaterial(0));
	GeometryCollection->SetMaterial(0, BreakablePotMaterial);

	GeometryCollection->OnChaosBreakEvent.AddDynamic(this, &ABreakableActor::OnActorBreak);
}

void ABreakableActor::OnActorBreak(const FChaosBreakEvent& BreakEvent)
{
	if (bBroken) return;
	bBroken = true;
	Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TObjectPtr<UWorld> World = GetWorld();
	if (World)
	{
		UGameplayStatics::PlaySoundAtLocation(World, HitSound, GetActorLocation());
		GEngine->AddOnScreenDebugMessage(1, 10.0f, FColor::Red, FString("Actor has been broken!"));
		if (TreasureClasses.Num() > 0)
		{
			FVector Location = GetActorLocation();
			Location.Z += 75.0f;
			const int32 Selection = FMath::RandRange(0, TreasureClasses.Num() - 1);
			World->SpawnActor<ATreasure>(TreasureClasses[Selection], Location, GetActorRotation());
		}
	}
}

// Called every frame
void ABreakableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABreakableActor::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	// bBroken = true;
	GeometryCollection->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	GeometryCollection->OnActorEnableCollisionChanged();

	// TObjectPtr<UWorld> World = GetWorld();
	// if (World && TreasureClasses.Num() > 0)
	// {
	// 	FVector Location = GetActorLocation();
	// 	Location.Z += 75.0f;
	// 	const int32 Selection = FMath::RandRange(0, TreasureClasses.Num() - 1);
	// 	World->SpawnActor<ATreasure>(TreasureClasses[Selection], Location, GetActorRotation());
	// }
}
