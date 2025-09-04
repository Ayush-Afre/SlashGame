// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SlashOverlay.generated.h"

//Forward Declarations
class UProgressBar;
class UTextBlock;


UCLASS()
class SLASHGAME_API USlashOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetHealthPercent(float Percent);

	void SetStaminaPercent(float Percent);

	void SetGoldNumber(int32 Gold);

	void SetSoulsNumber(int32 Souls);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthProgressBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> StaminaProgressBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CoinAmount;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SoulsAmount;


};
