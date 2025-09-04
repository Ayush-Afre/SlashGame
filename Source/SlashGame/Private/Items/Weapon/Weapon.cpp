// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapon/Weapon.h"

#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Interfaces/HitInterface.h"


AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	WeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponCollision"));
	WeaponBox->SetupAttachment(GetRootComponent());
	WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponBox->SetCollisionResponseToAllChannels(ECR_Overlap);
	WeaponBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	BoxTraceStartLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace Start Location"));
	BoxTraceStartLocation->SetupAttachment(GetRootComponent());

	BoxTraceEndLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace End Location"));
	BoxTraceEndLocation->SetupAttachment(GetRootComponent());

	Damage = 20.f;
	BoxTraceExtent = FVector(5.0f);
	bShowBoxDebug = false;
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeapon::AttachMeshToSocket(USceneComponent* InParent, FName InSocketName)
{
	FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
	ItemMesh->AttachToComponent(InParent, TransformRules, InSocketName);
}

void AWeapon::Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator)
{
	CurrentItemState = EItemState::EIS_Equipped;
	AttachMeshToSocket(InParent, InSocketName);
	SetOwner(NewOwner);
	SetInstigator(NewInstigator);

	if (EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), EquipSound, GetActorLocation());
	}
	if (CollisionSphere)
	{
		CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CollisionSphere->DestroyComponent();
	}
	if (ItemEffect)
	{
		ItemEffect->Deactivate();
	}
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	WeaponBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnBoxOverlap);
}


void AWeapon::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ActorIsSameType(OtherActor)) return;

	FHitResult BoxHitResult;
	BoxTrace(BoxHitResult);
	
	if (BoxHitResult.GetActor())
	{
		if (ActorIsSameType(BoxHitResult.GetActor())) return;

		UGameplayStatics::ApplyDamage(BoxHitResult.GetActor(), Damage, GetInstigator()->GetController(), this, UDamageType::StaticClass());
		ExecuteGetHit(BoxHitResult);
		CreateFields(BoxHitResult.ImpactPoint);
		IgnoreActors.AddUnique(BoxHitResult.GetActor());
	}
}

bool AWeapon::ActorIsSameType(AActor* OtherActor)
{

	return GetOwner()->ActorHasTag(TEXT("Enemy")) && OtherActor->ActorHasTag(TEXT("Enemy"));

}

void AWeapon::ExecuteGetHit(FHitResult& BoxHitResult)
{
	IHitInterface* HitInterface = Cast<IHitInterface>(BoxHitResult.GetActor());
	if (HitInterface)
	{
		HitInterface->Execute_GetHit(BoxHitResult.GetActor(), BoxHitResult.ImpactPoint,GetOwner());
	}
}

void AWeapon::BoxTrace(FHitResult& BoxHit)
{
	const FVector StartLocation = BoxTraceStartLocation->GetComponentLocation();
	const FVector EndLocation = BoxTraceEndLocation->GetComponentLocation();

	const FVector HalfSize = BoxTraceExtent;
	const FRotator Orientation = BoxTraceStartLocation->GetComponentRotation();

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	ActorsToIgnore.Add(this->GetOwner());


	bool bDidTrace = UKismetSystemLibrary::BoxTraceSingle(
		this,
		StartLocation,
		EndLocation,
		HalfSize,
		Orientation,
		TraceTypeQuery1,
		false,
		ActorsToIgnore,
		bShowBoxDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		BoxHit,
		true
	);
}
