#include "Enemy/Enemy.h"

#include "AIController.h"
#include "MotionWarpingComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/AttributeComponent.h"
#include "Items/Weapon/Weapon.h"
#include "Items/Soul.h"
#include "HUD/HealthBarComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Navigation/PathFollowingComponent.h"
#include "Perception/PawnSensingComponent.h"


AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetCollisionObjectType(ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);


	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBar"));
	HealthBarWidget->SetupAttachment(GetRootComponent());
	HealthBarWidget->SetWidgetSpace(EWidgetSpace::Screen);

	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
	PawnSensing->SightRadius = 4000.0f;
	PawnSensing->SetPeripheralVisionAngle(45.0f);

	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarping"));

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	EnemyState = EEnemyState::EES_Patroling;

	PatrolRadius = 200.0f;
	PatrollingSpeed = 125.0f;
	CombatRadius = 1000.0f;
	ChasingSpeed = 300.0f;
	AttackRadius = 150.0f;
	AcceptanceRadius = 50.0f;
	WaitMin = 5.0f;
	WaitMax = 10.0f;
	AttackMin = 0.2f;
	AttackMax = 0.8f;
	DeathLifeSpan = 5.0f;
	WarpTargetDistance = 75.0f;
}


void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	Tags.Add(FName("Enemy"));

	SetHealthBarVisibility(false);
	GetCharacterMovement()->MaxWalkSpeed = PatrollingSpeed;
	EnemyController = Cast<AAIController>(GetController());

	MoveToTarget(PatrolTarget);

	if (PawnSensing)
	{
		PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen);
	}

	TObjectPtr<UWorld> World = GetWorld();
	if (World && WeaponClass) {
		TObjectPtr<AWeapon> DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
		DefaultWeapon->Equip(GetMesh(), FName("WeaponSocket"), this, this);
		EquippedWeapon = DefaultWeapon;
	}
}


void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (isDead()) { return; }

	if (EnemyState > EEnemyState::EES_Patroling)
	{
		CheckCombatTarget();
	}
	else
	{
		CheckPatrolTarget();
	}

	if (CombatTarget)
	{
		UpdateTranslationWarpTarget();
		UpdateRotationWarpTarget();
	}


}


bool AEnemy::isDead()
{
	return EnemyState == EEnemyState::EES_Dead;
}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter);
	if (IsAlive())
	{
		SetHealthBarVisibility(true);
		ClearPatrolTimer();
		ClearAttackTimer();
		StopAttackMontage();
		UpdateWeaponCollision(ECollisionEnabled::NoCollision);
		if (IsInsideAttackRadius())
		{
			StartAttackTimer();
		}
	}
	else
	{
		Die_Implementation();
	}
}


float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	CombatTarget = EventInstigator->GetPawn();

	if (IsInsideAttackRadius())
	{
		EnemyState = EEnemyState::EES_Attacking;
	}
	else if (IsOutsideAttackRange())
	{
		StartChasing();
	}

	return DamageAmount;
}

void AEnemy::Die_Implementation()
{
	Super::Die_Implementation();

	EnemyState = EEnemyState::EES_Dead;
	UpdateWeaponCollision(ECollisionEnabled::NoCollision);
	ClearAttackTimer();
	SetHealthBarVisibility(false);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetLifeSpan(DeathLifeSpan);
	GetCharacterMovement()->bOrientRotationToMovement = false;
	SpawnSoul();
}

void AEnemy::SpawnSoul()
{
	TObjectPtr<UWorld> World = GetWorld();
	if (World && SoulClass)
	{
		const FVector Location = GetActorLocation() + FVector(0.0f, 0.0f, 125.0f);
		TObjectPtr<ASoul> SpawnedSoul = World->SpawnActor<ASoul>(SoulClass, Location, GetActorRotation());
		if (SpawnedSoul)
		{
			SpawnedSoul->SetOwner(this);
		}
	}
}

void AEnemy::MoveToTarget(AActor* Target)
{
	if (EnemyController == nullptr || Target == nullptr) return;

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
	EnemyController->MoveTo(MoveRequest);
}

void AEnemy::PatrolTimerFinished()
{
	MoveToTarget(PatrolTarget);
}

void AEnemy::ClearPatrolTimer()
{
	GetWorldTimerManager().ClearTimer(PatrolTimer);
}

bool AEnemy::InTargetRange(AActor* Target, double Radius)
{
	if (Target == nullptr) return false;
	const double DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();
	return DistanceToTarget <= Radius;
}

AActor* AEnemy::ChoosePatrolTarget()
{
	TArray<TObjectPtr<AActor>> VaildTargets;
	for (auto Target : PatrolTargets)
	{
		if (Target != PatrolTarget)
		{
			VaildTargets.AddUnique(Target);
		}
	}

	const int32 NumPatrolTargets = VaildTargets.Num();
	if (NumPatrolTargets > 0)
	{
		const int32 TargetSelection = FMath::RandRange(0, NumPatrolTargets - 1);
		return VaildTargets[TargetSelection];
	}


	return nullptr;
}

void AEnemy::LoseInterest()
{
	CombatTarget = nullptr;
	SetHealthBarVisibility(false);
}

void AEnemy::StartPatrolling()
{
	EnemyState = EEnemyState::EES_Patroling;
	GetCharacterMovement()->MaxWalkSpeed = PatrollingSpeed;
	MoveToTarget(PatrolTarget);
}

void AEnemy::PawnSeen(APawn* SeenPawn)
{
	bool bShouldChaseTarget = EnemyState != EEnemyState::EES_Dead &&
		EnemyState != EEnemyState::EES_Chasing &&
		EnemyState < EEnemyState::EES_Attacking &&
		SeenPawn->ActorHasTag(FName("EngageableTarget")) &&
		!SeenPawn->ActorHasTag(FName("Dead"));
	if (bShouldChaseTarget)
	{
		CombatTarget = SeenPawn;
		ClearPatrolTimer();
		StartChasing();
	}
}

void AEnemy::StartChasing()
{
	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
	MoveToTarget(CombatTarget);
}

bool AEnemy::IsOutsideCombatRange()
{
	return !InTargetRange(CombatTarget, CombatRadius);
}

bool AEnemy::IsChasing()
{
	return EnemyState == EEnemyState::EES_Chasing;
}


bool AEnemy::IsOutsideAttackRange()
{
	return !InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsInsideAttackRadius()
{
	return InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsAttacking()
{
	return EnemyState == EEnemyState::EES_Attacking;
}

bool AEnemy::IsEngaged()
{
	return EnemyState == EEnemyState::EES_Engaged;
}

void AEnemy::StartAttackTimer()
{
	EnemyState = EEnemyState::EES_Attacking;

	const float AttackTime = FMath::RandRange(AttackMin, AttackMax);
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
}

void AEnemy::ClearAttackTimer()
{
	GetWorldTimerManager().ClearTimer(AttackTimer);
}

void AEnemy::UpdateTranslationWarpTarget()
{
	if (CombatTarget)
	{


		const FVector CombatTargetLocation = CombatTarget->GetActorLocation();
		const FVector Location = GetActorLocation();
		FVector TargetToMe = (Location - CombatTargetLocation).GetSafeNormal();
		TargetToMe *= WarpTargetDistance;
		FVector TranslationTarget = CombatTargetLocation + TargetToMe;

		MotionWarpingComponent->AddOrUpdateWarpTargetFromLocation(FName("TranslationTarget"), TranslationTarget);
	}

}

void AEnemy::UpdateRotationWarpTarget()
{
	if (CombatTarget) {
		FVector RotationWarpTarget = CombatTarget->GetActorLocation();

		MotionWarpingComponent->AddOrUpdateWarpTargetFromLocation(FName("RotationTarget"), RotationWarpTarget);
	}

}



void AEnemy::Destroyed()
{
	if (EquippedWeapon) {
		EquippedWeapon->Destroy();
	}
}

bool AEnemy::canAttack()
{
	bool bcanAttack = IsInsideAttackRadius() && !IsAttacking() && !IsEngaged() && !isDead();
	return bcanAttack;
}

void AEnemy::HandleDamage(float DamageAmount)
{
	Super::HandleDamage(DamageAmount);

	if (HealthBarWidget && Attributes) {
		HealthBarWidget->SetHealthPercent(Attributes->GetHealthPercent()); // normalized value
	}

}

void AEnemy::Attack()
{
	if (CombatTarget && CombatTarget->ActorHasTag(FName("Dead")))
	{
		CombatTarget = nullptr;
		return;
	}
	EnemyState = EEnemyState::EES_Engaged;
	Super::Attack();
	PlayAttackMontage();
}

void AEnemy::AttackEnd()
{
	EnemyState = EEnemyState::EES_NoState;
	CheckCombatTarget();

}



void AEnemy::CheckCombatTarget()
{

	if (IsOutsideCombatRange())
	{
		ClearAttackTimer();
		LoseInterest();
		if (!IsEngaged()) {
			StartPatrolling();
		}
	}
	else if (IsOutsideAttackRange() && !IsChasing())
	{
		ClearAttackTimer();
		if (!IsEngaged()) {
			StartChasing();
		}
	}
	else if (canAttack())
	{
		StartAttackTimer();
	}
}


void AEnemy::CheckPatrolTarget()
{
	if (InTargetRange(PatrolTarget, PatrolRadius))
	{
		PatrolTarget = ChoosePatrolTarget();
		const float WaitTime = FMath::RandRange(5.0f, 10.0f);
		GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, WaitTime);
	}
}


void AEnemy::SetHealthBarVisibility(bool Visibility)
{
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(Visibility);
	}
}